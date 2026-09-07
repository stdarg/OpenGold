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
    switch(raw.type){case 8:return "dagger";case 23:return "mace";case 33:return "quarterstaff";
    case 36:return "longsword";case 50:return "leather";case 55:return "chain_mail";case 59:return "shield";
    default:return "por:unsupported:"+std::to_string(raw.type);}
}
CampaignParty::CampaignParty(std::unique_ptr<rules::RulesModule> rules):rules_(std::move(rules))
{if(!rules_)throw std::runtime_error("Party requires a rules module");}
void CampaignParty::editable() const {if(combat_)throw std::runtime_error("Finish combat before changing the party");}
const PartyMember& CampaignParty::member(MemberId id) const
{
    const auto it=std::find_if(state_.roster.begin(),state_.roster.end(),[&](const auto& m){return m.id==id;});
    if(it==state_.roster.end())throw std::runtime_error("Unknown party member");return *it;
}
PartyMember& CampaignParty::edit(MemberId id)
{return const_cast<PartyMember&>(std::as_const(*this).member(id));}
void CampaignParty::select(unsigned slot)
{editable();if(slot>=8||!state_.slots[slot])throw std::runtime_error("Empty party position");state_.selected=slot;}
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
    return rules_->character_profile(m.character.sheet(),keys);
}
void CampaignParty::equip(MemberId id,std::uint64_t item)
{
    editable();auto next=member(id).equipped;
    if(std::find(next.begin(),next.end(),item)!=next.end())return;
    auto& m=edit(id);const auto found=m.character.inventory().find(item);if(!found)throw std::runtime_error("Unknown item");
    std::vector<std::string> keys;for(auto key:next)keys.push_back(m.character.inventory().find(key)->get().definition_id);
    keys.push_back(found->get().definition_id);(void)rules_->character_profile(m.character.sheet(),keys);
    next.push_back(item);m.equipped=std::move(next);
}
void CampaignParty::unequip(MemberId id,std::uint64_t item)
{editable();auto& items=edit(id).equipped;std::erase(items,item);}
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
        const unsigned move=m.vitals.dead||m.vitals.hit_points==0?0:profile(id).movement_feet/5;
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
    m.wealth=wealth;m.vitals.hit_points=hp;
}
void CampaignParty::restore(PartyState state)
{
    editable();if(state.roster.size()>128||state.selected>=8||!state.next_id)throw std::runtime_error("Invalid party checkpoint");
    std::set<MemberId> ids,active;std::set<std::string> sources;
    for(const auto& m:state.roster){
        if(!m.id||m.id>=state.next_id||!ids.insert(m.id).second||m.vitals.hit_points<0||
            m.vitals.hit_points>m.character.sheet().hit_points||(m.vitals.dead&&m.vitals.hit_points)||m.morale>255||
            (!m.npc_source.empty()&&!sources.insert(m.npc_source).second))throw std::runtime_error("Invalid roster checkpoint");
        std::set<std::uint64_t> equipment;
        for(auto item:m.equipped)if(!m.character.inventory().find(item)||!equipment.insert(item).second)throw std::runtime_error("Invalid equipment checkpoint");
    }
    for(unsigned slot=0;slot<8;++slot)if(auto id=state.slots[slot]){
        if(!ids.contains(id)||!active.insert(id).second)throw std::runtime_error("Invalid active party checkpoint");
        const auto it=std::find_if(state.roster.begin(),state.roster.end(),[&](const auto& m){return m.id==id;});
        if((slot<6)!=it->npc_source.empty())throw std::runtime_error("Invalid PC/NPC checkpoint position");
    }
    if(!active.empty()&&!state.slots[state.selected])throw std::runtime_error("Invalid selected member checkpoint");
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
void CampaignParty::begin_combat(){editable();combat_=true;}
void CampaignParty::apply_combat(const rules::Snapshot& snapshot)
{
    if(!combat_||snapshot.identity!=rules_->identity())throw std::runtime_error("Combat rules identity mismatch");
    auto next=state_;
    for(const auto& actor:snapshot.combatants)if(actor.side==0){
        const auto it=std::find_if(next.roster.begin(),next.roster.end(),[&](const auto& m){return m.id==actor.id;});
        if(it==next.roster.end()||actor.max_hit_points!=it->character.sheet().hit_points)throw std::runtime_error("Combat party identity mismatch");
        it->vitals=actor.persistent;
    }
    state_=std::move(next);
}
}
