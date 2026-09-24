#include "opengold/campaign_party.h"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

namespace opengold {
namespace {
constexpr std::array<std::uint16_t,7> money{0x6BBB,0x6BBD,0x6BBF,0x6BC1,0x6BC3,0x6BC5,0x6BC7};
}
std::string equipment_conversion(const por::Equipment& item)
{
    const auto& raw=item.stored;
    if(raw.magic_bonus||raw.cursed_raw||std::any_of(raw.effect_codes.begin(),raw.effect_codes.end(),[](auto n){return n!=0;}))
        return "por:unsupported:"+std::to_string(raw.type);
    switch(raw.type){
    case 1:return "battleaxe";case 2:return "handaxe";
    case 3:case 5:case 10:case 11:case 14:case 15:case 16:case 17:case 40:return "glaive";
    case 4:case 18:case 19:return "halberd";
    case 6:case 22:case 33:return "quarterstaff";
    case 7:return "club";case 8:return "dagger";case 9:return "dart";case 12:return "flail";
    case 13:case 25:case 27:case 29:case 32:return "pike";
    case 20:return "warhammer";case 21:return "javelin";case 23:return "mace";
    case 24:return "morningstar";case 26:return "war_pick";case 30:return "scimitar";
    case 31:return "spear";case 34:case 35:case 36:return "longsword";
    case 37:return "shortsword";case 38:return "greatsword";case 39:return "trident";
    case 41:case 43:case 45:return "longbow";case 42:case 44:return "shortbow";
    case 46:return "light_crossbow";case 47:return "sling";case 79:return "wand";
    case 50:return "leather";case 55:return "chain_mail";case 59:return "shield";
    default:return "por:unsupported:"+std::to_string(raw.type);}
}
CampaignParty::CampaignParty(std::unique_ptr<rules::RulesModule> rules):rules_(std::move(rules))
{if(!rules_)throw std::runtime_error("Party requires a rules module");}
void CampaignParty::outside_combat() const {if(combat_)throw std::runtime_error("Finish combat before changing the party");}
void CampaignParty::editable() const {
    outside_combat();if(state_.short_rest)throw std::runtime_error("Finish Short Rest spending before changing the party");
}
const PartyMember& CampaignParty::member(MemberId id) const
{
    const auto it=std::find_if(state_.roster.begin(),state_.roster.end(),[&](const auto& m){return m.id==id;});
    if(it==state_.roster.end())throw std::runtime_error("Unknown party member");return *it;
}
PartyMember& CampaignParty::edit(MemberId id)
{return const_cast<PartyMember&>(std::as_const(*this).member(id));}
void CampaignParty::select(unsigned slot)
{outside_combat();if(slot>=8||!state_.slots[slot])throw std::runtime_error("Empty party position");state_.selected=slot;}
void CampaignParty::join(MemberId id,bool npc)
{
    if(std::find(state_.slots.begin(),state_.slots.end(),id)!=state_.slots.end())throw std::runtime_error("Already in party");
    const auto start=npc?6:0,end=npc?8:6;
    for(int i=start;i<end;++i)if(!state_.slots[i]){state_.slots[i]=id;if(!selected())state_.selected=i;return;}
    throw std::runtime_error(npc?"Both NPC positions are occupied":"All six PC positions are occupied");
}
MemberId CampaignParty::add_pc(Character character)
{
    editable();if(state_.roster.size()>=128)throw std::runtime_error("Roster is full");
    if(std::none_of(state_.slots.begin(),state_.slots.begin()+6,[](auto id){return !id;}))throw std::runtime_error("All six PC positions are occupied");
    const auto id=state_.next_id;const auto hp=character.sheet().hit_points;
    state_.roster.push_back({id,std::move(character),{},{hp,false,{}}});++state_.next_id;join(id,false);return id;
}
MemberId CampaignParty::recruit(std::string source,Character converted,unsigned morale)
{
    editable();if(source.empty()||morale>255)throw std::runtime_error("NPC conversion requires a source identity and byte morale");
    const auto found=std::find_if(state_.roster.begin(),state_.roster.end(),[&](const auto& m){return m.npc_source==source;});
    if(found!=state_.roster.end()){const auto id=found->id;rejoin(id);return id;}
    if(state_.roster.size()>=128)throw std::runtime_error("Roster is full");
    if(state_.slots[6]&&state_.slots[7])throw std::runtime_error("Both NPC positions are occupied");
    (void)rules_->character_profile(converted.sheet(),{}); // Reject unknown conversion before recruitment.
    const auto id=state_.next_id;const auto hp=converted.sheet().hit_points;
    state_.roster.push_back({id,std::move(converted),std::move(source),{hp,false,{}},{},{},morale});++state_.next_id;join(id,true);return id;
}
void CampaignParty::rejoin(MemberId id){editable();join(id,!member(id).npc_source.empty());}
void CampaignParty::remove(MemberId id)
{
    editable();(void)member(id);auto it=std::find(state_.slots.begin(),state_.slots.end(),id);
    if(it==state_.slots.end())throw std::runtime_error("Member is not in party");*it=0;
    if(!selected())for(unsigned i=0;i<8;++i)if(state_.slots[i]){state_.selected=i;break;}
}
rules::CharacterProfile CampaignParty::profile(MemberId id) const
{
    const auto& m=member(id);std::vector<std::string> keys;
    for(auto equipped:m.equipped){auto item=m.character.inventory().find(equipped);
        if(!item)throw std::runtime_error("Equipped item is missing");keys.push_back(item->get().definition_id);}
    return rules_->character_profile(m.character.sheet(),keys,m.equipment);
}
void CampaignParty::equip(MemberId id,std::uint64_t item)
{
    editable();auto next=member(id).equipped;
    if(std::find(next.begin(),next.end(),item)!=next.end())return;
    auto& m=edit(id);const auto found=m.character.inventory().find(item);if(!found)throw std::runtime_error("Unknown item");
    const auto info=rules_->equipment_info(found->get().definition_id);
    if(info.slot==rules::EquipmentSlot::weapon)
        std::erase_if(next,[&](auto key){return rules_->equipment_info(m.character.inventory().find(key)->get().definition_id).slot==rules::EquipmentSlot::weapon;});
    std::vector<std::string> keys;for(auto key:next)keys.push_back(m.character.inventory().find(key)->get().definition_id);
    keys.push_back(found->get().definition_id);
    const auto equipment=info.slot==rules::EquipmentSlot::weapon?rules::EquipmentState{}:m.equipment;
    (void)rules_->character_profile(m.character.sheet(),keys,equipment);
    next.push_back(item);m.equipped=std::move(next);m.equipment=equipment;
}
void CampaignParty::unequip(MemberId id,std::uint64_t item)
{
    editable();auto& m=edit(id);auto& items=m.equipped;
    if(std::find(items.begin(),items.end(),item)==items.end())return;
    if(equipment_info(id,item).slot==rules::EquipmentSlot::weapon)m.equipment={};
    std::erase(items,item);
}
void CampaignParty::set_grip(MemberId id,unsigned hands)
{
    editable();auto& m=edit(id);const auto current=profile(id);
    if(std::none_of(current.grips.begin(),current.grips.end(),[&](const auto& choice){return choice.hands==hands&&choice.available;}))
        throw std::runtime_error("This grip is incompatible with the equipped weapon or shield.");
    m.equipment={hands};
}
rules::EquipmentInfo CampaignParty::equipment_info(MemberId id,std::uint64_t item) const
{
    const auto found=member(id).character.inventory().find(item);
    if(!found)throw std::runtime_error("Unknown item");
    return rules_->equipment_info(found->get().definition_id);
}
void CampaignParty::purchase(MemberId id,const por::Equipment& item)
{
    editable();auto& m=edit(id);
    if(m.character.inventory().items().size()>=16)throw std::runtime_error("Inventory is full (16 items)");
    if(m.wealth[3]<item.stored.value)throw std::runtime_error("Not enough gold");
    auto inventory=m.character.inventory();auto sources=m.item_sources;
    const auto key=inventory.add(equipment_conversion(item),item.label(),std::max(1u,unsigned(item.stored.stack_size)),item.stored.type);
    sources.emplace(key,item);
    m.character.inventory()=std::move(inventory);m.item_sources=std::move(sources);
    m.wealth[3]-=item.stored.value;
}
void CampaignParty::set_wealth(MemberId id,std::array<std::uint16_t,7> wealth){editable();edit(id).wealth=wealth;}
bool CampaignParty::award_loot(const std::array<unsigned,7>& wealth,const std::vector<por::Equipment>& items,std::string reward_id)
{
    editable();if(reward_id.empty()||reward_id.size()>160)throw std::runtime_error("Loot requires a bounded stable identity");
    if(std::find(state_.claimed_rewards.begin(),state_.claimed_rewards.end(),reward_id)!=state_.claimed_rewards.end())return true;
    if(state_.claimed_rewards.size()>=1024||items.size()>256)throw std::runtime_error("Loot collection exceeds supported limits");
    auto next=state_;std::vector<std::size_t> recipients;
    for(auto id:next.slots)if(id){const auto found=std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==id;});if(!found->vitals.dead)recipients.push_back(found-next.roster.begin());}
    if(recipients.empty())return false;
    for(unsigned coin=0;coin<7;++coin){
        auto remaining=wealth[coin];
        for(auto index:recipients){auto& purse=next.roster[index].wealth[coin];const auto amount=std::min(remaining,unsigned(65535-purse));purse+=amount;remaining-=amount;}
        if(remaining)return false;
    }
    for(const auto& item:items){
        const auto recipient=*std::min_element(recipients.begin(),recipients.end(),[&](auto a,auto b){return next.roster[a].character.inventory().items().size()<next.roster[b].character.inventory().items().size();});
        auto& m=next.roster[recipient];
        // Encounter rewards are retained even beyond the shop's purchase cap.
        const auto id=m.character.inventory().add(equipment_conversion(item),item.label(),std::max(1u,unsigned(item.stored.stack_size)),item.stored.type);m.item_sources.emplace(id,item);
    }
    next.claimed_rewards.push_back(std::move(reward_id));state_=std::move(next);return true;
}
void CampaignParty::award_experience(unsigned amount,std::string reward_id)
{
    editable();
    if(reward_id.empty()||reward_id.size()>160)throw std::runtime_error("Reward requires a bounded stable identity");
    if(std::find(state_.claimed_rewards.begin(),state_.claimed_rewards.end(),reward_id)!=state_.claimed_rewards.end())return;
    if(state_.claimed_rewards.size()>=1024)throw std::runtime_error("Reward history is full");
    auto next=state_;
    bool awarded=false;
    for(auto id:next.slots)if(id){
        auto& member=*std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==id;});
        if(member.vitals.dead)continue;
        if(amount>std::numeric_limits<unsigned>::max()-member.experience)throw std::runtime_error("Experience overflow");
        awarded=true;
        member.experience+=amount;
    }
    if(!awarded)throw std::runtime_error("Reward requires a living active member");
    next.claimed_rewards.push_back(std::move(reward_id));state_=std::move(next);
}
bool CampaignParty::can_advance(MemberId id) const
{
    if(combat_||state_.short_rest)return false;const auto& m=member(id);const auto options=rules_->advancement_options(m.character.sheet());
    return !m.vitals.dead&&options.level&&m.experience>=rules_->experience_for_level(options.level);
}
rules::AdvancementOptions CampaignParty::advancement_options(MemberId id) const
{return rules_->advancement_options(member(id).character.sheet());}
rules::AdvancementChoice CampaignParty::default_advancement(MemberId id) const
{return rules_->default_advancement(member(id).character.sheet());}
PartyMember CampaignParty::preview_advancement(MemberId id,const rules::AdvancementChoice& choice) const
{
    if(!can_advance(id))throw std::runtime_error("This character is not ready to level up");
    auto next=member(id);const auto level=next.character.sheet().level;
    if(!next.character.advance(*rules_,next.vitals,choice)||next.character.sheet().level!=level+1)throw std::runtime_error("Unsupported advancement");
    return next;
}
void CampaignParty::advance(MemberId id,const rules::AdvancementChoice& choice)
{
    editable();auto member=preview_advancement(id,choice);auto next=state_;
    *std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==id;})=std::move(member);state_=std::move(next);
}
PartyMember CampaignParty::preview_training(MemberId id,const rules::CharacterRules& creation_rules,
    const rules::TrainingChoices& choices) const
{
    editable();auto next=member(id);
    next.character=next.character.preview_training(creation_rules,*rules_,choices);
    rules_->validate_character_state(next.character.sheet(),next.vitals);
    return next;
}
void CampaignParty::complete_training(MemberId id,const rules::CharacterRules& creation_rules,const rules::TrainingChoices& choices)
{
    auto member=preview_training(id,creation_rules,choices);auto next=state_;
    *std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==id;})=std::move(member);state_=std::move(next);
}
void CampaignParty::advance_time(unsigned minutes)
{
    advance_time_milliseconds(std::uint64_t(minutes)*60000);
}
void CampaignParty::advance_time_milliseconds(std::uint64_t milliseconds)
{
    outside_combat();auto next=state_;elapse(next,milliseconds);if(milliseconds)next.short_rest.reset();state_=std::move(next);
}
void CampaignParty::elapse(PartyState& state,std::uint64_t milliseconds,std::span<const MemberId> in_combat) const
{
    const auto remainder=state.subminute_milliseconds+milliseconds%60000;
    const auto minutes=milliseconds/60000+remainder/60000;
    if(minutes>std::numeric_limits<std::uint64_t>::max()-state.time_minutes)throw std::runtime_error("Campaign clock overflow");
    std::vector<rules::Participant> participants;
    for(const auto& member:state.roster){
        if(std::find(in_combat.begin(),in_combat.end(),member.id)!=in_combat.end())continue;
        std::vector<std::string> gear;
        for(auto id:member.equipped)gear.push_back(member.character.inventory().find(id)->get().definition_id);
        const auto profile=rules_->character_profile(member.character.sheet(),gear,member.equipment);
        participants.push_back({member.id,"campaign-character",member.character.sheet().name,0,{},profile.data,member.vitals});
    }
    rules_->elapse(participants,milliseconds,state.random_state);
    for(const auto& participant:participants)
        std::find_if(state.roster.begin(),state.roster.end(),[&](const auto& m){return m.id==participant.id;})->vitals=*participant.state;
    state.time_minutes+=minutes;state.subminute_milliseconds=static_cast<unsigned>(remainder%60000);
}
void CampaignParty::temple_heal(MemberId target)
{
    editable();const auto& current=member(target);
    if(std::find(state_.slots.begin(),state_.slots.end(),target)==state_.slots.end())throw std::runtime_error("Temple target must be active");
    if(current.vitals.dead||current.vitals.hit_points>=current.character.sheet().hit_points)throw std::runtime_error("Cure Wounds requires a wounded living member");
    auto next=state_;auto& healed=*std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==target;});
    unsigned remaining=100;for(auto id:next.slots)if(id){auto& m=*std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& x){return x.id==id;});const auto paid=std::min<unsigned>(m.wealth[3],remaining);m.wealth[3]-=static_cast<std::uint16_t>(paid);remaining-=paid;}
    if(remaining)throw std::runtime_error("Party cannot afford temple service");
    rules_->temple_heal(healed.vitals,healed.character.sheet(),next.random_state);state_=std::move(next);
}
bool CampaignParty::has_item(unsigned type) const
{
    for(auto id:state_.slots)if(id)for(const auto& item:member(id).character.inventory().items())if(item.original_type==type)return true;
    return false;
}
unsigned CampaignParty::strength() const
{
    unsigned result=0;
    for(auto id:state_.slots)if(id){const auto& m=member(id);if(m.vitals.dead)continue;const auto p=profile(id);
        const auto& klass=m.character.sheet().character_class;
        // Explicit SRD-to-PoR adapter: descending AC=20-AC, THAC0=20-attack bonus.
        result+=(m.vitals.hit_points+5*std::max(0,p.armor_class-20)+5*std::max(0,p.melee_attack_bonus+1)+
            (klass=="Cleric"?4:klass=="Wizard"?8:0))/10;
    }
    return result&255;
}
std::array<unsigned,4> CampaignParty::query(unsigned address,unsigned effect) const
{
    if(address!=0x6C1B||effect)throw std::runtime_error("Unsupported CHECK PARTY attribute/effect conversion");
    unsigned count=0,low=255,high=0,total=0;
    for(auto id:state_.slots)if(id){const auto& m=member(id);
        const unsigned move=m.vitals.dead||m.vitals.hit_points==0?0:profile(id).movement_feet*2/5;
        low=std::min(low,move);high=std::max(high,move);total+=move;++count;}
    return count?std::array<unsigned,4>{low,high,total/count,0}:std::array<unsigned,4>{};
}
por::EclHostReply CampaignParty::character_reply(unsigned slot) const
{
    if(slot>=8)throw std::runtime_error("Invalid ECL party position");
    std::array<std::uint16_t,285> fields{};por::EclHostReply reply;
    if(const auto id=state_.slots[slot]){const auto& m=member(id);const auto& sheet=m.character.sheet();
        const auto& name=sheet.name;for(std::size_t n=0;n<name.size()&&n<15;++n)fields[n]=static_cast<unsigned char>(name[n]);
        fields[0x18]=sheet.scores[2];fields[0x100]=m.vitals.dead?0:1;fields[0x119]=m.vitals.hit_points;
        for(unsigned n=0;n<7;++n)fields[money[n]-0x6B00]=m.wealth[n];
    }
    for(unsigned n=0;n<fields.size();++n)reply.writes.push_back({static_cast<std::uint16_t>(0x6B00+n),fields[n]});
    reply.writes.push_back({0x6DB1,static_cast<std::uint16_t>(slot)});reply.writes.push_back({0x6DB4,static_cast<std::uint16_t>(slot)});return reply;
}
void CampaignParty::read_character(unsigned slot,const por::EclMachine& vm)
{
    editable();if(slot>=8)throw std::runtime_error("Invalid ECL party position");if(!state_.slots[slot])return;
    auto& m=edit(state_.slots[slot]);const auto hp=vm.variable(0x6C19);
    if(hp>m.character.sheet().hit_points||(m.vitals.dead&&hp))throw std::runtime_error("Unsupported script HP change");
    std::array<std::uint16_t,7> wealth;for(unsigned n=0;n<7;++n)wealth[n]=vm.variable(money[n]);
    auto vitals=m.vitals;rules_->set_hit_points(vitals,m.character.sheet(),hp);
    m.wealth=wealth;m.vitals=std::move(vitals);
}
void CampaignParty::validate(const PartyState& state)
{
    if(state.roster.size()>128||state.selected>=8||!state.next_id||state.claimed_rewards.size()>1024||state.subminute_milliseconds>=60000||!state.next_combat_scope||!state.next_rest_session)throw std::runtime_error("Invalid party checkpoint");
    std::set<MemberId> ids,active;std::set<std::string> sources,creation_sources;
    for(const auto& m:state.roster){
        if(!m.creation_source.empty()&&(m.creation_source.size()>160||!creation_sources.insert(m.creation_source).second))throw std::runtime_error("Invalid creation source checkpoint");
        if(!m.id||m.id>=state.next_id||!ids.insert(m.id).second||m.vitals.hit_points<0||
            (m.last_rest_minutes&&(*m.last_rest_minutes>state.time_minutes||
                (*m.last_rest_minutes==state.time_minutes&&m.last_rest_subminute_milliseconds>state.subminute_milliseconds)))||
            m.last_rest_subminute_milliseconds>=60000||(!m.last_rest_minutes&&m.last_rest_subminute_milliseconds)||
            m.vitals.hit_points>m.character.sheet().hit_points||(m.vitals.dead&&m.vitals.hit_points)||m.morale>255||
            (!m.npc_source.empty()&&!sources.insert(m.npc_source).second))throw std::runtime_error("Invalid roster checkpoint");
        std::set<std::uint64_t> equipment;
        for(auto item:m.equipped)if(!m.character.inventory().find(item)||!equipment.insert(item).second)throw std::runtime_error("Invalid equipment checkpoint");
    }
    if(std::any_of(state.claimed_rewards.begin(),state.claimed_rewards.end(),[](const auto& id){return id.empty()||id.size()>160;})||
       std::set<std::string>(state.claimed_rewards.begin(),state.claimed_rewards.end()).size()!=state.claimed_rewards.size())throw std::runtime_error("Invalid claimed rewards");
    for(unsigned slot=0;slot<8;++slot)if(auto id=state.slots[slot]){
        if(!ids.contains(id)||!active.insert(id).second)throw std::runtime_error("Invalid active party checkpoint");
        const auto it=std::find_if(state.roster.begin(),state.roster.end(),[&](const auto& m){return m.id==id;});
        if((slot<6)!=it->npc_source.empty())throw std::runtime_error("Invalid PC/NPC checkpoint position");
    }
    if(!active.empty()&&!state.slots[state.selected])throw std::runtime_error("Invalid selected member checkpoint");
    if(state.short_rest){
        const auto& rest=*state.short_rest;std::set<MemberId> members;
        if(!rest.ticket.session||rest.ticket.session>=state.next_rest_session||!rest.ticket.revision||
            rest.completed_minutes!=state.time_minutes||rest.completed_subminute_milliseconds!=state.subminute_milliseconds||
            rest.members.empty()||rest.members.size()>8)throw std::runtime_error("Invalid Short Rest checkpoint");
        for(auto id:rest.members){
            if(!active.contains(id)||!members.insert(id).second)throw std::runtime_error("Invalid Short Rest member");
            const auto& m=*std::find_if(state.roster.begin(),state.roster.end(),[&](const auto& value){return value.id==id;});
            if(m.vitals.dead||m.vitals.hit_points<1)throw std::runtime_error("Invalid Short Rest vitality");
        }
    }
}
void CampaignParty::restore(PartyState state){
    outside_combat();validate(state);
    for(const auto& m:state.roster){
        std::vector<std::string> gear;for(auto id:m.equipped)gear.push_back(m.character.inventory().find(id)->get().definition_id);
        (void)rules_->character_profile(m.character.sheet(),gear,m.equipment);
    }
    state_=std::move(state);
}
std::vector<rules::Participant> CampaignParty::participants() const
{
    std::vector<rules::Participant> result;
    for(unsigned slot=0;slot<8;++slot)if(auto id=state_.slots[slot]){
        const auto& m=member(id);if(m.vitals.dead)continue;
        const auto p=profile(id);
        result.push_back({id,"campaign-character",m.character.sheet().name,0,{1+int(slot/4),1+int(slot%4)*2},p.data,m.vitals});
    }
    if(result.empty())throw std::runtime_error("Add a living combat-ready character first");return result;
}
void CampaignParty::begin_combat(){outside_combat();if(state_.next_combat_scope==std::numeric_limits<std::uint64_t>::max())throw std::runtime_error("Combat identity exhausted");combat_=true;combat_registered_=false;combat_elapsed_=0;}
void CampaignParty::apply_combat(const rules::Snapshot& snapshot)
{
    if(!combat_||snapshot.identity!=rules_->identity())throw std::runtime_error("Combat rules identity mismatch");
    if(snapshot.elapsed_milliseconds<combat_elapsed_)throw std::runtime_error("Combat clock moved backward");
    auto next=state_;
    std::vector<MemberId> active;for(const auto& actor:snapshot.combatants)if(actor.side==0)active.push_back(actor.id);
    // Combat has already advanced active actors' effects and mortality. Only reserves
    // need campaign-side updates, preventing duplicate recovery rolls.
    elapse(next,snapshot.elapsed_milliseconds-combat_elapsed_,active);
    for(const auto& actor:snapshot.combatants)if(actor.side==0){
        const auto it=std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==actor.id;});
        if(it==next.roster.end()||actor.max_hit_points!=it->character.sheet().hit_points)throw std::runtime_error("Combat party identity mismatch");
        std::vector<std::string> gear;for(auto id:it->equipped)gear.push_back(it->character.inventory().find(id)->get().definition_id);
        (void)rules_->character_profile(it->character.sheet(),gear,actor.equipment);
        it->vitals=actor.persistent;it->equipment=actor.equipment;
    }
    next.short_rest.reset();
    if(!combat_registered_)++next.next_combat_scope;
    state_=std::move(next);combat_elapsed_=snapshot.elapsed_milliseconds;combat_registered_=true;
}
}
