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
        else if(!info.recovery.can_rest)info.denial=RestDenial::vitality;
        else if(info.wait_milliseconds)info.denial=RestDenial::cooldown;
        result.push_back(std::move(info));
    }
    return result;
}
bool CampaignParty::rest(){return rest(RestKind::long_rest).has_value();}
std::optional<RestResult> CampaignParty::rest(RestKind kind)
{
    editable();const auto timing=policy(*rules_,kind);
    if(!timing.duration_minutes)throw std::runtime_error("Invalid rules rest duration");
    RestResult result{kind,timing.duration_minutes};
    for(const auto& info:rest_info(kind))if(info.denial==RestDenial::none)result.members.push_back(info.id);
    if(result.members.empty())return std::nullopt;
    auto next=state_;
    if(kind==RestKind::short_rest&&next.next_rest_session==std::numeric_limits<std::uint64_t>::max())
        throw std::runtime_error("Rest identity exhausted");
    // Eligibility is captured at the start. Time and effects advance once for
    // the whole roster; only the eligible active members receive rest benefits.
    elapse(next,std::uint64_t(timing.duration_minutes)*60000);
    for(auto id:result.members){
        auto& m=*std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& value){return value.id==id;});
        if(kind==RestKind::short_rest)rules_->recover_short_rest(m.vitals,m.character.sheet());
        else{
            rules_->recover(m.vitals,m.character.sheet());
            m.last_rest_minutes=next.time_minutes;
            m.last_rest_subminute_milliseconds=next.subminute_milliseconds;
        }
    }
    if(kind==RestKind::short_rest){
        result.spending=RestTicket{next.next_rest_session++,1};
        next.short_rest=ShortRestSession{*result.spending,next.time_minutes,next.subminute_milliseconds,result.members};
    }
    state_=std::move(next);return result;
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
    state_=std::move(next);return result;
}
void CampaignParty::finish_short_rest(RestTicket ticket)
{
    require_rest_ticket(ticket);state_.short_rest.reset();
}
}
