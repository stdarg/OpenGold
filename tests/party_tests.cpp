#include "opengold/campaign_party.h"
#include "opengold/character_creator.h"
#include "opengold/combat_demo.h"
#include "opengold/rolf_tour.h"
#include "opengold/srd5.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace opengold;
using namespace opengold::rules;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected,"Operation should reject");}
std::unique_ptr<RulesModule> module(){return srd5::load(std::filesystem::path(OPENGOLD_SOURCE_DIR)/"data/rules/srd-5.2.1/combat.rules");}
Character character(std::string klass="fighter",std::string name="Ada")
{
    CharacterCreator creator(srd5::character_rules(),42);
    creator.select(CreationField::race,"human");creator.select(CreationField::character_class,klass);
    creator.roll();creator.name(std::move(name));
    for(unsigned i=0;i<6;++i)creator.assign_roll(i,i);
    // Authored fixtures may have scores below the creator's optional starting-class minimums.
    return Character(creator.rules(),creator.draft(),creator.appearance());
}
por::Equipment item(unsigned type,unsigned price=10)
{por::Equipment e;e.stored.type=type;e.stored.value=price;e.stored.stack_size=1;return e;}
void roster_and_equipment()
{
    CampaignParty party(module());auto original=character();const auto pc=party.add_pc(original);
    original.inventory().add("other","External item");check(party.member(pc).character.inventory().empty(),"Party owns character independently");
    for(unsigned i=1;i<6;++i)party.add_pc(character());rejects([&]{party.add_pc(character());});
    const auto npc=party.recruit("MON:explicit-profile",character());party.recruit("MON:second-profile",character());
    rejects([&]{party.recruit("MON:third-profile",character());});
    party.set_wealth(npc,{0,0,0,123,0,0,0});party.remove(npc);party.recruit("MON:explicit-profile",character());
    check(party.member(npc).wealth[3]==123&&party.state().roster.size()==8,"Re-recruit preserves original NPC instance");
    party.remove(pc);party.rejoin(pc);check(party.state().slots[0]==pc,"Rejoin restores PC position");
    party.set_wealth(pc,{0,0,0,100,0,0,0});party.purchase(pc,item(36));party.purchase(pc,item(59));
    const auto items=party.member(pc).character.inventory().items();const auto sword=items[0].id,shield=items[1].id;
    const auto ac=party.profile(pc).armor_class;party.equip(pc,sword);party.equip(pc,shield);
    check(party.profile(pc).armor_class==ac+2&&party.member(pc).wealth[3]==80,"Purchased shield changes actual rules AC");
    check(party.has_item(59)&&!party.has_item(55),"Party item query uses original types");
    party.purchase(pc,item(55));party.equip(pc,party.member(pc).character.inventory().items()[2].id);
    check(party.profile(pc).armor_class==18,"Armor and shield combine in rules module");
    check(party.profile(pc).item_modifiers.find("Shield: +2 AC")!=std::string::npos&&party.profile(pc).item_modifiers.find("Chain mail")!=std::string::npos,"Modifier report includes every equipped effect");
    rejects([&]{party.purchase(pc,item(50,500));});check(party.member(pc).wealth[3]==70,"Unaffordable buy is atomic");
    party.purchase(pc,item(1));rejects([&]{party.equip(pc,party.member(pc).character.inventory().items().back().id);});
    check(party.member(pc).equipped.size()==3,"Unsupported item does not change equipment");
    party.unequip(pc,shield);check(party.profile(pc).armor_class==16,"Unequipping updates AC");
    party.begin_combat();rejects([&]{party.remove(pc);});rejects([&]{party.purchase(pc,item(8));});party.end_combat();
    CampaignParty wizard(module());auto mage=wizard.add_pc(character("wizard"));wizard.set_wealth(mage,{0,0,0,50,0,0,0});
    wizard.purchase(mage,item(59));const int naked_ac=wizard.profile(mage).armor_class;wizard.equip(mage,1);check(wizard.profile(mage).armor_class==naked_ac,"Untrained shield is allowed but adds no AC");
    auto bard=wizard.add_pc(character("bard"));check(wizard.profile(bard).armor_class>0,"All classes can display an equipment profile outside combat");
}
void untrained_equipment()
{
    auto rules=module();const auto mage=character("wizard");const auto& s=mage.sheet();
    const std::array<std::string,1> sword{"longsword"},mace{"mace"},armor{"leather"},shield{"shield"};
    check(rules->character_profile(s,sword).melee_attack_bonus==s.modifiers[0],"Untrained longsword omits proficiency");
    check(rules->character_profile(s,mace).melee_attack_bonus==s.modifiers[0]+2,"SRD wizard is proficient in simple weapons including mace");
    const auto p=rules->character_profile(s,armor);
    check(p.strength_dexterity_disadvantage&&p.armor_class==11+s.modifiers[1],"Untrained armor keeps AC with disadvantage");
    check(p.item_modifiers.find("cannot cast spells")!=std::string::npos,"Untrained penalty has an equipment source");
    check(!rules->character_profile(s,{}).strength_dexterity_disadvantage,"Removing armor clears disadvantage");
    for(const auto& c:srd5::character_rules()->choices(CreationField::character_class)){
        const auto pc=character(c.id);
        const bool no_light=c.id=="monk"||c.id=="sorcerer"||c.id=="wizard";
        check(rules->character_profile(pc.sheet(),armor).strength_dexterity_disadvantage==no_light,"Light armor training matches all SRD classes");
        const bool shield_training=c.id=="barbarian"||c.id=="cleric"||c.id=="druid"||c.id=="fighter"||c.id=="paladin"||c.id=="ranger";
        const auto guarded=rules->character_profile(pc.sheet(),shield);
        const int base=10+pc.sheet().modifiers[1]+(c.id=="barbarian"?std::max(0,pc.sheet().modifiers[2]):0);
        check(guarded.armor_class==base+(shield_training?2:0),"Shield training and Monk unarmored restriction match SRD");
    }
    Encounter e{{4,4,std::vector<std::uint8_t>(16)},{{1,"campaign-character","Mage",0,{1,1},p.data},{2,"bandit","Bandit",1,{2,1}}}};
    bool tested=false;
    for(unsigned seed=0;seed<100&&!tested;++seed){auto combat=rules->create(e,seed);if(combat->snapshot().actor!=1)continue;
        const auto commands=combat->legal_commands();
        check(std::none_of(commands.begin(),commands.end(),[](const auto& c){return c.verb=="fire_bolt"||c.verb=="magic_missile";}),"Untrained armor prevents spellcasting");
        const auto hit=std::find_if(commands.begin(),commands.end(),[](const auto& c){return c.verb=="melee";});
        check(hit!=commands.end()&&combat->submit(*hit),"Untrained armored attack is allowed");
        const auto log=combat->snapshot().log;check(std::any_of(log.begin(),log.end(),[](const auto& line){return line.find("disadvantage")!=std::string::npos;}),"Armor penalty applies to actual attack rolls");
        auto restored=rules->restore(combat->save());check(restored->save()==combat->save(),"Untrained equipment penalties survive combat restore");tested=true;
    }
    check(tested,"Exercised armored wizard combat");
}
void finish(CombatDemo& fight)
{
    for(unsigned n=0;n<2000&&fight.combat().snapshot().outcome==Outcome::ongoing;++n)
        check(fight.submit(choose_demo_command(fight.combat())),"Accepted combat command");
    check(fight.combat().snapshot().outcome!=Outcome::ongoing,"Fight terminates");
}
void combat_handoff()
{
    auto party=std::make_shared<CampaignParty>(module());auto pc=party->add_pc(character("wizard","Mage"));
    const auto guard=party->recruit("guard",character());party->set_wealth(pc,{0,0,0,50,0,0,0});party->purchase(pc,item(8));party->equip(pc,1);
    auto start=party->checkpoint();start.roster[0].vitals.hit_points-=2;party->restore(start);
    const auto hp=party->member(pc).vitals.hit_points;
    CombatDemo fight(module());fight.campaign_party(party);fight.training();
    const auto before=fight.combat().snapshot();
    const auto mage=std::find_if(before.combatants.begin(),before.combatants.end(),[&](const auto& a){return a.id==pc;});
    check(mage!=before.combatants.end()&&mage->hit_points==hp&&mage->name=="Mage","Combat starts with created identity and live HP");
    const auto initial=party->member(pc).vitals.resources;finish(fight);
    check(!party->in_combat(),"Combat releases party edits on finish");
    for(const auto& actor:fight.combat().snapshot().combatants)if(actor.side==0){
        const auto& member=party->member(actor.id);
        const auto growth=member.character.sheet().hit_points-actor.max_hit_points;
        check(member.vitals.hit_points==actor.hit_points+(actor.hit_points>0?growth:0)&&member.vitals.dead==actor.dead,
            "Victory applies combat HP followed by rules advancement without reviving anyone");
    }
    const auto finished=fight.combat().snapshot();
    check(std::find_if(finished.combatants.begin(),finished.combatants.end(),[&](const auto& a){return a.id==pc;})->persistent.resources!=initial,"Mage spent spell resources before advancement");
    check(party->member(pc).character.inventory().items().size()==1&&party->member(pc).equipped.size()==1,"Inventory survives combat");
    const auto guard_state=party->member(guard).vitals;party->remove(guard);party->rejoin(guard);
    check(party->member(guard).vitals==guard_state,"Recruitment preserves combat HP and resources");
    if(!party->member(pc).vitals.dead&&party->member(pc).vitals.hit_points>0){
        const auto retained=party->member(pc).vitals;fight.training(80);
        check(party->member(pc).vitals==retained,"Next encounter does not refill resources");finish(fight);
    }
}
void progression_and_services()
{
    CampaignParty party(module());const auto pc=party.add_pc(character("fighter","Progress"));
    auto damaged=party.checkpoint();damaged.roster[0].vitals.hit_points-=2;party.restore(damaged);
    const auto starting_hp=party.member(pc).character.sheet().hit_points;
    party.award_experience(300,"quest:slums");
    check(party.member(pc).experience==300&&party.member(pc).character.sheet().level==2,"Quest XP advances exactly at level threshold");
    check(party.member(pc).character.sheet().hit_points>starting_hp&&party.profile(pc).hit_points==party.member(pc).character.sheet().hit_points,"Level HP applies to combat profile");
    party.award_experience(300,"quest:slums");
    check(party.member(pc).experience==300,"Repeated reward id does not award XP twice");
    check(party.rest()&&party.time_hours()==8&&party.member(pc).vitals.hit_points==party.member(pc).character.sheet().hit_points,"Long rest restores HP and advances campaign time");
    check(!party.rest()&&party.time_hours()==8,"Repeated long rest is denied without advancing time");
    party.advance_time(16*60);check(party.rest()&&party.time_hours()==32,"Long rest is allowed after the required wait");
    rejects([&]{party.temple_heal(pc);});
    auto wounded=party.checkpoint();wounded.roster[0].vitals={0,false,"SRD1 0 0 2 2 1","Unconscious"};party.restore(wounded);
    check(!party.rest()&&party.time_hours()==32,"Unconscious members cannot start a long rest");
    party.set_wealth(pc,{0,0,0,50,0,0,0});auto before=party.checkpoint();auto temple_before=party.member(pc).vitals;
    rejects([&]{party.temple_heal(pc);});check(party.member(pc).wealth[3]==50&&party.member(pc).vitals==temple_before,"Rejected temple request is atomic");
    check(party.state().random_state==before.random_state,"Rejected payment preserves random state");
    party.set_wealth(pc,{0,0,0,100,0,0,0});party.temple_heal(pc);
    check(party.member(pc).vitals.hit_points>0&&party.member(pc).wealth[3]==0&&party.member(pc).vitals.resources=="SRD1 0 0 0 0 0","Healing charges once, clears death saves and preserves spent resources");
    const auto checkpoint=party.checkpoint();
    CampaignParty restored(module());restored.restore(checkpoint);
    check(restored.member(pc).vitals==party.member(pc).vitals&&restored.time_hours()==32&&restored.member(pc).wealth[3]==0,"Native checkpoint retains recovery, payments and clock");
    restored.award_experience(300,"quest:slums");check(restored.member(pc).experience==300,"Checkpoint retains claimed rewards");
    auto dead=checkpoint;dead.roster[0].vitals={0,true,"SRD1 0 0 0 3 0"};party.restore(dead);
    rejects([&]{party.temple_heal(pc);});check(party.member(pc).vitals.dead,"Cheap healing cannot resurrect");
    party.restore(checkpoint);party.remove(pc);rejects([&]{party.temple_heal(pc);});check(!party.rest(),"Empty party cannot rest");party.rejoin(pc);
    rejects([&]{party.award_experience(std::numeric_limits<unsigned>::max(),"overflow");});
    check(party.member(pc).experience==300&&party.state().claimed_rewards.size()==1,"Overflow does not partially award XP");
    party.award_experience(600,"next quest");check(party.member(pc).experience==900&&party.member(pc).character.sheet().level==2,"XP retained at the supported level ceiling");
}
void caster_advancement()
{
    for(const auto* klass:{"wizard","cleric"}){
        CampaignParty party(module());auto c=character(klass);auto draft=c.creation_data();draft.race="dwarf";
        c=Character(*srd5::character_rules(),draft,{});const auto pc=party.add_pc(c);
        auto spent=party.checkpoint();spent.roster[0].vitals={c.sheet().hit_points-2,false,"SRD1 0 0 0 0 0"};party.restore(spent);
        party.award_experience(299,"below");check(party.member(pc).character.sheet().level==1,"Below threshold does not advance");
        party.award_experience(1,"threshold");const auto& m=party.member(pc);
        const auto growth=std::max(1,c.sheet().hit_die/2+1+c.sheet().modifiers[2])+1;
        check(m.character.sheet().hit_points==c.sheet().hit_points+growth&&m.vitals.hit_points==m.character.sheet().hit_points-2,"Dwarven growth preserves HP deficit");
        check(m.vitals.resources=="SRD1 0 1 0 0 0","Advancement grants new slot without refilling spent slots");
        check(party.rest()&&party.member(pc).vitals.resources=="SRD1 0 3 0 0 0","Level-two long rest restores three slots");
        auto participants=party.participants();participants.push_back({1000,"bandit","Bandit",1,{9,4}});
        auto rules=module();auto combat=rules->create({{12,9,std::vector<std::uint8_t>(108)},participants},42);
        const auto saved=combat->save();check(rules->restore(saved)->save()==saved,"Advanced profile and resources round-trip through combat checkpoint");
    }
}
void temple_pooling()
{
    CampaignParty party(module());const auto payer=party.add_pc(character()),target=party.add_pc(character("wizard"));
    party.set_wealth(payer,{0,0,0,40,0,0,0});party.set_wealth(target,{0,0,0,50,0,0,0});
    auto state=party.checkpoint();state.roster[1].vitals={0,false,"SRD1 0 0 2 2 1"};party.restore(state);
    rejects([&]{party.temple_heal(target);});
    check(party.member(payer).wealth[3]==40&&party.member(target).wealth[3]==50&&party.member(target).vitals==state.roster[1].vitals,"Insufficient pooled funds debit neither purse");
    party.set_wealth(target,{0,0,0,60,0,0,0});party.temple_heal(target);
    check(party.member(payer).wealth[3]==0&&party.member(target).wealth[3]==0&&party.member(target).vitals.hit_points>0&&party.member(payer).vitals==state.roster[0].vitals,"Pooled service heals only the requested target");
}
void dynamic_checkpoint()
{
    auto rules=module();const auto c=character("wizard");const auto profile=rules->character_profile(c.sheet(),{});
    Encounter e{{4,4,std::vector<std::uint8_t>(16)},{{1,"campaign-character","Mage",0,{0,0},profile.data},{2,"bandit","Bandit",1,{3,3}}}};
    auto session=rules->create(e,42);const auto bytes=session->save();auto restored=rules->restore(bytes);
    check(restored->save()==bytes,"Dynamic character profile round-trips exactly");
    const auto command=choose_demo_command(*session);check(session->submit(command)&&restored->submit(command),"Restored command accepted");
    check(session->save()==restored->save(),"Dynamic checkpoint deterministic continuation");
    auto unsupported=e;unsupported.participants[0].character_profile=rules->character_profile(character("bard").sheet(),{}).data;
    rejects([&]{(void)rules->create(unsupported,42);}); // Profiles are broader than the combat implementation.
    for(const auto& c:srd5::character_rules()->choices(CreationField::character_class)){
        const auto pc=character(c.id);const auto p=rules->character_profile(pc.sheet(),{});
        const int expected=10+pc.sheet().modifiers[1]+(c.id=="monk"?pc.sheet().modifiers[4]:c.id=="barbarian"?pc.sheet().modifiers[2]:0);
        check(p.armor_class==std::max(expected,10+pc.sheet().modifiers[1])&&p.hit_points==pc.sheet().hit_points,"All twelve classes have correct unarmored AC and HP profiles");
    }
    e.participants[0].state=VitalState{99999,false,{}};rejects([&]{(void)rules->create(e,42);});
    e.participants[0].state=VitalState{1,false,"SRD1 0 99 0 0 0"};rejects([&]{(void)rules->create(e,42);});
}
using Bytes=std::vector<std::uint8_t>;
std::shared_ptr<const por::EclProgram> program(Bytes body)
{
    Bytes bytes{0,0};for(int n=0;n<5;++n)bytes.insert(bytes.end(),{1,1,0x15,0x99});
    bytes.push_back(0);bytes.insert(bytes.end(),body.begin(),body.end());return std::make_shared<const por::EclProgram>(por::EclProgram::decode(bytes,"party integration"));
}
void settle(por::RolfTourSession& town)
{for(unsigned n=0;n<100&&town.snapshot().phase==por::TourPhase::running;++n)town.advance(.5);check(town.snapshot().phase!=por::TourPhase::faulted,"Town script fault");}
void recovery_hosts()
{
    auto party=std::make_shared<CampaignParty>(module());const auto pc=party->add_pc(character());
    auto state=party->checkpoint();state.roster[0].vitals={1,false,"SRD1 0 0 0 0 0"};party->restore(state);
    auto resources=std::make_shared<por::PhlanResources>();
    auto p=program({0});
    por::RolfTourSession allowed({},p,{},0x9914,{},resources);allowed.campaign_party(party);settle(allowed);
    check(allowed.explore(por::ExplorationCommand::camp),"Camp starts pre-camp entry");settle(allowed);
    check(allowed.can_leave()&&party->time_hours()==8&&party->member(pc).vitals.hit_points==party->member(pc).character.sheet().hit_points,"Allowed ECL camp recovers party");
    check(allowed.script_variable(0x6c19)==party->member(pc).vitals.hit_points,"Camp synchronizes script HP");
    check(allowed.script_variable(0x49c9)==20,"Camp updates original hour register");
    allowed.explore(por::ExplorationCommand::look);settle(allowed);
    check(party->member(pc).vitals.hit_points==party->member(pc).character.sheet().hit_points,"Next event does not overwrite recovered HP");
    party->restore(state);
    auto denied_program=program({9,0,255,1,0xd3,0x6d,0});
    por::RolfTourSession denied({},denied_program,{},0x9914,{},resources);denied.campaign_party(party);settle(denied);
    denied.explore(por::ExplorationCommand::camp);settle(denied);
    check(denied.can_leave()&&party->state().time_minutes==0&&party->member(pc).vitals.hit_points==1,"Denied pre-camp gives no time or recovery");
    // Slot 2 arms a guaranteed five-minute interruption. Slot 3 records execution.
    Bytes bytes{0,0};for(int n=0;n<5;++n)bytes.insert(bytes.end(),{1,1,0x15,0x99});bytes.push_back(0);
    Bytes pre{9,0,1,1,0xd2,0x6d,9,0,101,1,0xd3,0x6d,0};
    const unsigned interrupt=0x9915+pre.size();bytes[16]=interrupt&255;bytes[17]=interrupt>>8;
    bytes.insert(bytes.end(),pre.begin(),pre.end());bytes.insert(bytes.end(),{9,0,1,1,0x10,0x98,0});
    auto interrupted_program=std::make_shared<const por::EclProgram>(por::EclProgram::decode(bytes,"camp interruption"));
    por::RolfTourSession interrupted({},interrupted_program,{},0x9914,{},resources);interrupted.campaign_party(party);settle(interrupted);
    interrupted.explore(por::ExplorationCommand::camp);settle(interrupted);
    check(interrupted.can_leave()&&interrupted.script_variable(0x9810)==1&&party->state().time_minutes==5&&party->member(pc).vitals.hit_points==1,"Interruption entry runs without granting rest benefits");
    check(interrupted.script_variable(0x49c7)==5,"Interruption advances original minute register");
    party->restore(state);
    auto inn_program=program({56,0,9,0});
    por::RolfTourSession inn({},inn_program,{},0x9914,{},resources);inn.campaign_party(party);settle(inn);
    inn.explore(por::ExplorationCommand::look);settle(inn);
    check(inn.can_leave()&&party->time_hours()==8&&inn.script_variable(0x49c9)==20,"PROGRAM 9 replies with restored HP and advanced clock");
    party->restore(state);
    auto temple_program=program({9,0,1,1,0xe2,0x6d,36,9,0,1,1,0x11,0x98,0});
    por::RolfTourSession temple({},temple_program,{},0x9914,{},resources);temple.campaign_party(party);settle(temple);
    temple.explore(por::ExplorationCommand::look);settle(temple);
    const auto ticket=temple.snapshot().continue_ticket;
    check(temple.snapshot().choices.size()==2&&!temple.choose(ticket+1,0),"Temple offers wounded target and cancel with stale-ticket rejection");
    check(!temple.choose(ticket,0)&&party->member(pc).vitals.hit_points==1&&party->state().random_state==state.random_state,"Unaffordable temple choice leaves request and party unchanged");
    party->set_wealth(pc,{0,0,0,100,0,0,0});
    check(temple.choose(ticket,0)&&!temple.choose(ticket,0),"Temple payment accepted exactly once");settle(temple);
    check(temple.can_leave()&&temple.script_variable(0x6de2)==0&&temple.script_variable(0x9811)==1&&party->member(pc).wealth[3]==0&&party->member(pc).vitals.hit_points>1,"Temple resumes ECL with healed HP and charged purse");
    // An unsupported continuation refunds the complete service, including dice.
    party->restore(state);party->set_wealth(pc,{0,0,0,100,0,0,0});
    auto failed=program({9,0,1,1,0xe2,0x6d,36,56,0,0,0});
    por::RolfTourSession rollback({},failed,{},0x9914,{},resources);rollback.campaign_party(party);settle(rollback);
    rollback.explore(por::ExplorationCommand::look);settle(rollback);check(rollback.choose(rollback.snapshot().continue_ticket,0),"Service before unsupported continuation");settle(rollback);
    if(party->member(pc).wealth[3]!=100||party->member(pc).vitals.hit_points!=1||party->state().random_state!=state.random_state)
        throw std::runtime_error("Failed service rollback: gold="+std::to_string(party->member(pc).wealth[3])+", HP="+std::to_string(party->member(pc).vitals.hit_points)+", "+rollback.snapshot().dialogue);
    check(rollback.choose(rollback.snapshot().continue_ticket,0)&&rollback.can_leave(),"Rollback clears pending temple ticket");
}
void reward_reentry()
{
    auto party=std::make_shared<CampaignParty>(module());const auto pc=party->add_pc(character("wizard"));party->recruit("guard",character());
    for(unsigned visit=0;visit<2;++visit){
        if(visit){party->advance_time(24*60);check(party->rest(),"Recover before second preview");}
        CombatDemo fight(module());fight.campaign_party(party);fight.training();finish(fight);
        check(fight.combat().snapshot().outcome==Outcome::victory,"Representative victory");
        check(party->member(pc).experience==300&&party->state().claimed_rewards.size()==1,"Recreated combat scene cannot duplicate its reward");
        check(!fight.submit({}),"Finished combat rejects more commands");
    }
}
void script_handoff()
{
    auto party=std::make_shared<CampaignParty>(module());auto first=party->add_pc(character("fighter","First")),second=party->add_pc(character("cleric","Second"));
    party->set_wealth(first,{0,0,0,100,0,0,0});party->set_wealth(second,{0,0,0,200,0,0,0});
    // WHO; write selected HP; store; FIND ITEM; shop; exit.
    auto p=program({57,0,0,9,0,3,1,0x19,0x6c,10,0,129,10,0,0,
        39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,9,0,1,1,0x6c,0x6e,36,
        29,1,0x10,0x98,30,1,0x1b,0x6c,0,0,1,0x11,0x98,1,0x12,0x98,1,0x13,0x98,1,0x14,0x98,
        54,0,7,0,70,50,0,59,22,9,0,1,1,0x15,0x98,0});
    auto resources=std::make_shared<por::PhlanResources>();resources->programs.emplace(0,p);resources->treasure[54]={item(59)};
    resources->npc_profiles.emplace(7,character("fighter","Script guard"));
    por::RolfTourSession town({},p,{},0x9914,{},resources);town.campaign_party(party);settle(town);town.explore(por::ExplorationCommand::look);settle(town);
    check(town.snapshot().choices.size()==2,"WHO displays real party");const auto ticket=town.snapshot().continue_ticket;
    check(!town.choose(ticket+1,1)&&town.choose(ticket,1)&&!town.choose(ticket,1),"WHO handles ticket once");settle(town);
    check(party->selected()==second&&party->member(second).vitals.hit_points==3,"ECL HP writes target selected real character");
    check(town.snapshot().phase==por::TourPhase::shopping,"Original shop opens for selected member");
    check(party->selected()==second,"Script slot scans do not change the chosen buyer");
    check(town.buy(town.snapshot().continue_ticket,0),"Buy for selected member");
    check(party->member(second).wealth[3]==190&&party->member(first).wealth[3]==100,"Purchase debits only selected purse");
    check(party->has_item(59),"Script item query sees purchase");town.leave_shop(town.snapshot().continue_ticket);settle(town);
    check(town.script_variable(0x6BC1)==190&&town.can_leave(),"Shop return synchronizes selected character");
    check(town.script_variable(0x9810)>0&&town.script_variable(0x9811)==6&&town.script_variable(0x9812)==6&&town.script_variable(0x9813)==6&&town.script_variable(0x9814)==0,"ECL party queries write all results");
    check(town.script_variable(0x9815)==1,"FIND ITEM drives actual bytecode branch after purchase");
    check(party->state().slots[6]&&party->member(party->state().slots[6]).morale==70,"ADD NPC uses explicit conversion and requested morale");
    party->equip(second,1);check(party->profile(second).armor_class==12+party->member(second).character.sheet().modifiers[1],"Cleric equips purchased shield");
    // A supported store followed by an unsupported query must roll the party back.
    const auto hp=party->member(second).vitals.hit_points;
    auto failed=program({10,0,1,9,0,0,1,0x19,0x6c,10,0,129,30,1,0xa7,0x6b,0,0,1,0x11,0x98,1,0x12,0x98,1,0x13,0x98,1,0x14,0x98,0});
    por::RolfTourSession rollback({},failed,{},0x9914,{},resources);rollback.campaign_party(party);settle(rollback);
    rollback.explore(por::ExplorationCommand::look);settle(rollback);
    check(rollback.snapshot().phase==por::TourPhase::awaiting_continue&&party->member(second).vitals.hit_points==hp,"Unsupported event restores authoritative party checkpoint");
}
}
int main()
{
    try{roster_and_equipment();untrained_equipment();combat_handoff();progression_and_services();caster_advancement();temple_pooling();dynamic_checkpoint();script_handoff();recovery_hosts();reward_reentry();std::cout<<"Party integration tests passed\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
