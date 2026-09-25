#include "opengold/campaign_party.h"
#include <algorithm>
#include <limits>

namespace opengold {
namespace {
rules::RestPolicy policy(const rules::RulesModule& rules,RestKind kind)
{
    switch(kind){
    case RestKind::short_rest:return rules.short_rest_policy();
    case RestKind::long_rest:return rules.long_rest_policy();
    }
    throw std::runtime_error("Invalid rest kind");
}
}
std::vector<MemberRestInfo> CampaignParty::rest_info(RestKind kind) const
{
    const auto timing=policy(*rules_,kind);
    std::vector<MemberRestInfo> result;
    for(auto id:state_.slots)if(id){
        const auto& m=member(id);
        MemberRestInfo info{id,rules_->recovery_info(m.character.sheet(),m.vitals)};
        if(kind==RestKind::long_rest&&m.last_rest_minutes){
            const auto elapsed=state_.time_minutes-*m.last_rest_minutes;
            if(elapsed<=timing.wait_after_rest_minutes){
                const auto needed=std::uint64_t(timing.wait_after_rest_minutes-elapsed)*60000+m.last_rest_subminute_milliseconds;
                if(needed>state_.subminute_milliseconds)info.wait_milliseconds=needed-state_.subminute_milliseconds;
            }
        }
        if(combat_)info.denial=RestDenial::combat;
        else if(state_.short_rest)info.denial=RestDenial::spending;
        else if(state_.rest_activity)info.denial=RestDenial::activity;
        else if(!info.recovery.can_rest)info.denial=RestDenial::vitality;
        else if(info.wait_milliseconds)info.denial=RestDenial::cooldown;
        result.push_back(std::move(info));
    }
    return result;
}
bool CampaignParty::rest(){return rest(RestKind::long_rest).has_value();}
namespace {
void advance_ticket(RestTicket& ticket){
    if(ticket.revision==std::numeric_limits<std::uint64_t>::max())throw std::runtime_error("Rest revision exhausted");
    ++ticket.revision;
}
RestTicket new_ticket(PartyState& state){
    if(state.next_rest_session==std::numeric_limits<std::uint64_t>::max())throw std::runtime_error("Rest identity exhausted");
    return {state.next_rest_session++,1};
}
}
std::optional<RestResult> CampaignParty::rest(RestKind kind)
{
    // Existing safe-camp/inn callers retain an atomic operation. The same
    // activity engine also supports hosts which stop at an interruption.
    auto before=state_;
    try{
        const auto ticket=begin_rest(kind);if(!ticket)return std::nullopt;
        return advance_rest(*ticket,remaining_rest_milliseconds(),state_.rest_activity->work);
    }catch(...){state_=std::move(before);throw;}
}
std::optional<RestTicket> CampaignParty::begin_rest(RestKind kind)
{
    editable();RestActivity activity;
    static_cast<rules::RestProgress&>(activity)=rules_->begin_rest(kind);
    activity.started_minutes=state_.time_minutes;
    activity.started_subminute_milliseconds=state_.subminute_milliseconds;
    for(const auto& info:rest_info(kind))if(info.denial==RestDenial::none)activity.members.push_back(info.id);
    if(activity.members.empty())return std::nullopt;
    auto next=state_;activity.ticket=new_ticket(next);next.rest_activity=activity;state_=std::move(next);return activity.ticket;
}
void CampaignParty::require_activity_ticket(RestTicket ticket) const
{
    outside_combat();
    if(!state_.rest_activity||state_.rest_activity->ticket!=ticket)throw std::runtime_error("Expired rest activity request");
}
std::uint64_t CampaignParty::remaining_rest_milliseconds() const
{
    if(!state_.rest_activity)return 0;
    return rules_->remaining_rest(*state_.rest_activity);
}
void CampaignParty::short_rest_benefits(PartyState& state,const std::vector<MemberId>& members) const
{
    if(state.short_rest)throw std::runtime_error("Finish Short Rest spending first");
    std::vector<MemberId> eligible;
    for(auto id:members){
        auto& member=*std::find_if(state.roster.begin(),state.roster.end(),[&](const auto& m){return m.id==id;});
        if(!rules_->recovery_info(member.character.sheet(),member.vitals).can_rest)continue;
        rules_->recover_short_rest(member.vitals,member.character.sheet());eligible.push_back(id);
    }
    if(!eligible.empty())state.short_rest=ShortRestSession{new_ticket(state),state.time_minutes,state.subminute_milliseconds,std::move(eligible)};
}
void CampaignParty::interrupt_rest_state(PartyState& state,RestInterruption cause) const
{
    const auto outcome=rules_->interrupt_rest(*state.rest_activity,cause);
    if(outcome.benefit==rules::RestBenefit::short_rest)short_rest_benefits(state,state.rest_activity->members);
    if(!outcome.progress){state.rest_activity.reset();return;}
    static_cast<rules::RestProgress&>(*state.rest_activity)=*outcome.progress;
    advance_ticket(state.rest_activity->ticket);
}
void CampaignParty::interrupt_rest(RestTicket ticket,RestInterruption cause)
{
    require_activity_ticket(ticket);auto next=state_;interrupt_rest_state(next,cause);state_=std::move(next);
}
std::optional<RestResult> CampaignParty::advance_rest(RestTicket ticket,std::uint64_t milliseconds,RestWork work)
{
    require_activity_ticket(ticket);
    if(state_.short_rest)throw std::runtime_error("Resolve Hit Dice choices before advancing rest");
    const auto outcome=rules_->advance_rest(*state_.rest_activity,milliseconds,work);
    auto next=state_;auto& activity=*next.rest_activity;
    elapse(next,milliseconds);advance_ticket(activity.ticket);
    RestResult result{activity.kind,outcome.completed_duration_milliseconds/60000,{}};
    if(outcome.benefit==rules::RestBenefit::short_rest){
        short_rest_benefits(next,activity.members);
        if(next.short_rest){result.spending=next.short_rest->ticket;result.members=next.short_rest->members;}
    }else if(outcome.benefit==rules::RestBenefit::long_rest)for(auto id:activity.members){
        auto& member=*std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==id;});
        if(!rules_->recovery_info(member.character.sheet(),member.vitals).can_rest)continue;
        rules_->recover(member.vitals,member.character.sheet());result.members.push_back(id);member.last_rest_minutes=next.time_minutes;member.last_rest_subminute_milliseconds=next.subminute_milliseconds;
    }
    if(outcome.progress)static_cast<rules::RestProgress&>(activity)=*outcome.progress;
    else next.rest_activity.reset();
    state_=std::move(next);
    if(outcome.completed_duration_milliseconds)return result;
    return std::nullopt;
}

void CampaignParty::resume_rest(RestTicket ticket)
{
    require_activity_ticket(ticket);
    if(!state_.rest_activity->interrupted||state_.short_rest)throw std::runtime_error("Resolve interruption and Hit Dice choices before resuming");
    auto next=state_;auto& activity=*next.rest_activity;
    std::erase_if(activity.members,[&](auto id){const auto& m=member(id);return !rules_->recovery_info(m.character.sheet(),m.vitals).can_rest;});
    if(activity.members.empty())throw std::runtime_error("No eligible member can resume resting");
    static_cast<rules::RestProgress&>(activity)=rules_->resume_rest(activity);
    advance_ticket(activity.ticket);state_=std::move(next);
}
void CampaignParty::abandon_rest(RestTicket ticket)
{
    require_activity_ticket(ticket);state_.rest_activity.reset();state_.short_rest.reset();
}
bool CampaignParty::prepare_combat()
{
    outside_combat();
    if(state_.rest_activity&&!state_.rest_activity->interrupted){
        auto next=state_;interrupt_rest_state(next,RestInterruption::initiative);state_=std::move(next);
    }
    return !state_.short_rest;
}
void CampaignParty::require_rest_ticket(RestTicket ticket) const
{
    outside_combat();
    if(!state_.short_rest||state_.short_rest->ticket!=ticket||
        state_.short_rest->completed_minutes!=state_.time_minutes||
        state_.short_rest->completed_subminute_milliseconds!=state_.subminute_milliseconds)
        throw std::runtime_error("Expired Short Rest spending request");
}
rules::HitDieResult CampaignParty::spend_hit_die(RestTicket ticket,MemberId id)
{
    require_rest_ticket(ticket);
    const auto& session=*state_.short_rest;
    if(std::find(session.members.begin(),session.members.end(),id)==session.members.end())
        throw std::runtime_error("Member did not complete this Short Rest");
    if(ticket.revision==std::numeric_limits<std::uint64_t>::max())throw std::runtime_error("Rest revision exhausted");
    auto next=state_;
    auto& m=*std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& value){return value.id==id;});
    const auto result=rules_->spend_hit_die(m.vitals,m.character.sheet(),next.random_state);
    ++next.short_rest->ticket.revision;
    if(next.rest_activity)advance_ticket(next.rest_activity->ticket);
    state_=std::move(next);return result;
}
void CampaignParty::finish_short_rest(RestTicket ticket)
{
    require_rest_ticket(ticket);auto next=state_;next.short_rest.reset();
    if(next.rest_activity)advance_ticket(next.rest_activity->ticket);state_=std::move(next);
}
}
