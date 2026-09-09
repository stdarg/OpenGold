#include "opengold/srd5.h"
#include "opengold/combat_demo.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace opengold;using namespace opengold::rules;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F&& f,const char* message){bool failed=false;try{f();}catch(const std::exception&){failed=true;}check(failed,message);}
std::filesystem::path pack(){return std::filesystem::path(OPENGOLD_SOURCE_DIR)/"data/rules/srd-5.2.1/combat.rules";}
Encounter duel(std::string profile="vanguard") {
    Battlefield b{12,9,std::vector<std::uint8_t>(108,0)};b.terrain[4*12+4]=1;b.terrain[2*12+1]=2;
    return {b,{{1,profile,"Hero",0,{2,2}},{2,"bandit","Bandit",1,{3,2}}}};
}
Command command(const CombatSession& session,std::string_view verb,Cell cell={}) {
    const auto offered=session.legal_commands();
    auto it=std::find_if(offered.begin(),offered.end(),[&](const auto& c){return c.verb==verb&&(verb!="move"||c.destination==cell);});
    if(it==offered.end())throw std::runtime_error("Missing command: "+std::string(verb));return *it;
}
std::unique_ptr<CombatSession> hero_first(const RulesModule& module,Encounter encounter) {
    for(unsigned seed=0;seed<100;++seed){auto session=module.create(encounter,seed);if(session->snapshot().actor==1)return session;}
    throw std::runtime_error("No hero-first seed");
}
std::unique_ptr<CombatSession> hero_first(const RulesModule& module,std::string profile="vanguard") {return hero_first(module,duel(profile));}
bool offers(const CombatSession& session,std::string_view verb) {
    const auto commands=session.legal_commands();return std::any_of(commands.begin(),commands.end(),[&](const auto& c){return c.verb==verb;});
}
CombatantView unit(const CombatSession& session,EntityId id) {
    const auto state=session.snapshot();for(const auto& a:state.combatants)if(a.id==id)return a;throw std::runtime_error("Missing combatant");
}
void next_round(CombatSession& session) {
    const auto round=session.snapshot().round;
    do{check(session.submit(command(session,"end")),"End turn accepted");}while(session.snapshot().round==round||session.snapshot().actor!=1);
}
void boundary_tests() {
    auto module=srd5::load(pack());
    bool reduced=false;
    for(unsigned seed=0;seed<40;++seed){auto plain=duel();auto ambushed=plain;ambushed.participants[0].surprised=true;
        const auto normal=module->create(plain,seed),surprise=module->create(ambushed,seed);
        check(unit(*surprise,1).initiative<=unit(*normal,1).initiative,"Surprise imposes initiative disadvantage");
        reduced|=unit(*surprise,1).initiative<unit(*normal,1).initiative;
        check(module->restore(surprise->save())->save()==surprise->save(),"Surprised initiative and RNG survive combat checkpoint");
    }
    check(reduced,"Surprise affects initiative across deterministic seeds");
    auto wide=duel();wide.battlefield={50,25,std::vector<std::uint8_t>(1250)};
    auto original=module->create(wide,42);check(module->restore(original->save())->snapshot().battlefield.width==50,"Original arena dimensions round trip");
    check(srd5::ability_modifier(9)==-1&&srd5::ability_modifier(13)==1,"Signed ability rounding");
    check(!srd5::attack_hits(1,100,1)&&srd5::attack_hits(20,-100,40),"Natural attack extremes");
    check(srd5::attack_hits(12,3,15)&&!srd5::attack_hits(11,3,15),"Attack meets ascending AC");
    auto session=hero_first(*module);const auto before=session->save();
    auto invalid=command(*session,"melee");invalid.actor=999;
    check(!session->submit(invalid)&&session->save()==before,"Invalid actor cannot change state or spend randomness");
    invalid=command(*session,"move",{1,2});invalid.destination={4,4};
    check(!session->submit(invalid)&&session->save()==before,"Invalid path cannot change state");
    auto attack=command(*session,"melee");check(session->submit(attack),"Melee command");
    const auto after=session->save();check(!session->submit(attack)&&session->save()==after,"Duplicate command rejected atomically");
    for(const auto& c:session->legal_commands())check(c.verb!="melee"&&c.verb!="ranged","Attack consumes the action");
    auto restored=module->restore(after);check(restored->save()==after,"Checkpoint preserves exact module state");
    auto mismatch=after;const auto version=session->snapshot().identity.version;const auto where=mismatch.find(version);mismatch.replace(where,version.size(),"9.9.9");
    rejects([&]{(void)module->restore(mismatch);},"Wrong rules version rejected");
    rejects([&]{(void)module->restore(after+"junk");},"Trailing checkpoint data rejected");
    rejects([&]{(void)module->restore(after.substr(0,after.size()/2));},"Truncated checkpoint rejected");
    auto wrong=duel();wrong.participants[0].definition="unsupported dragon";
    rejects([&]{(void)module->create(wrong,1);},"Unsupported content fails explicitly");
    wrong=duel();wrong.participants[1].cell=wrong.participants[0].cell;
    rejects([&]{(void)module->create(wrong,1);},"Overlapping starting participants rejected");

    session=hero_first(*module);check(session->submit(command(*session,"move",{1,2})),"Move away from enemy");
    check(session->snapshot().reaction_pending&&session->snapshot().actor==2,"Leaving reach offers a reaction before movement");
    const auto reaction=session->save();restored=module->restore(reaction);
    check(restored->save()==reaction,"Pending reaction survives save/load");
    const auto react=command(*session,"opportunity");check(session->submit(react)&&restored->submit(react),"Reaction resolves");
    check(session->save()==restored->save(),"Restored reaction rolls replay deterministically");
    const auto snap=session->snapshot();
    const auto hero=std::find_if(snap.combatants.begin(),snap.combatants.end(),[](const auto& a){return a.id==1;});
    check(hero->cell==Cell{1,2}&&hero->movement_feet==20,"Difficult terrain costs ten feet");
    session=hero_first(*module);session->submit(command(*session,"disengage"));session->submit(command(*session,"move",{1,2}));
    check(!session->snapshot().reaction_pending,"Disengage prevents opportunity attacks");

    auto a=module->create(duel(),77),b=module->create(duel(),77);
    for(unsigned turns=0;a->snapshot().outcome==Outcome::ongoing;++turns) {
        check(turns<200,"Duel finishes within bounded commands");
        const auto next=choose_demo_command(*a);check(a->submit(next)&&b->submit(next),"Same commands accepted");
        check(a->save()==b->save(),"Identical seeds and commands yield identical combat");
    }
    check(a->legal_commands().empty(),"Completed combat cannot accept extra actions");
    // Sessions retain immutable content independently of the module lifetime.
    module.reset();check(!session->snapshot().combatants.empty(),"Session owns shared immutable content lifetime");
}
void mechanics_tests() {
    CombatDemo training(srd5::load(pack()));training.training();
    for(unsigned i=0;training.combat().snapshot().outcome==Outcome::ongoing;++i) {
        check(i<200,"Training encounter finishes");check(training.submit(choose_demo_command(training.combat())),"Training AI command accepted");
    }
    auto module=srd5::load(pack());auto encounter=duel("adept");
    encounter.participants[1]={2,"vanguard","Enemy",1,{8,2}};
    encounter.participants.push_back({3,"vanguard","Reserve",1,{8,6}});
    auto session=hero_first(*module,encounter);
    for(int cast=0;cast<2;++cast) {
        check(session->submit(command(*session,"magic_missile")),"Spell command accepted");
        check(!unit(*session,1).action&&unit(*session,1).movement_feet==30,"Spell consumes action, not movement");
        next_round(*session);
    }
    check(!offers(*session,"magic_missile")&&offers(*session,"fire_bolt"),"Exhausted slots prevent leveled spells but allow cantrips");
    auto restored=module->restore(session->save());check(restored->save()==session->save(),"Spent spell slots persist");

    encounter=duel("adept");encounter.participants[1].cell={6,2};encounter.battlefield.terrain[2*12+4]=1;
    session=hero_first(*module,encounter);
    check(!offers(*session,"ranged")&&!offers(*session,"fire_bolt")&&!offers(*session,"magic_missile"),"Opaque obstacles block weapon and spell targeting");
    encounter.battlefield.terrain[2*12+4]=0;session=hero_first(*module,encounter);
    check(offers(*session,"ranged")&&offers(*session,"magic_missile"),"Clear sight enables targeting");
    encounter=duel();session=hero_first(*module,encounter);session->submit(command(*session,"ranged"));
    const auto ranged_log=session->snapshot().log;
    check(std::any_of(ranged_log.begin(),ranged_log.end(),[](const auto& line){return line.find("disadvantage")!=std::string::npos;}),"Adjacent hostile imposes ranged disadvantage");

    // A critical can kill the adjacent bandit; its vacated cell is traversable
    // and a checkpoint may legitimately contain a living actor over a corpse.
    encounter.participants.push_back({3,"bandit","Reserve",1,{10,7}});bool tested=false;
    for(unsigned seed=0;seed<500&&!tested;++seed) {
        session=module->create(encounter,seed);if(session->snapshot().actor!=1)continue;
        session->submit(command(*session,"melee"));if(!unit(*session,2).dead)continue;
        const auto log=session->snapshot().log;
        if(std::none_of(log.begin(),log.end(),[](const auto& line){return line.find("CRITICAL")!=std::string::npos;}))continue;
        check(session->submit(command(*session,"move",{3,2})),"Can move onto defeated enemy cell");
        restored=module->restore(session->save());check(restored->save()==session->save(),"Corpse overlap survives checkpoint restore");tested=true;
    }
    check(tested,"Critical damage can exceed normal attack maximum");

    for(const auto profile:{"healer","vanguard"}) {
        encounter=duel(profile);encounter.participants[1]={2,"adept","Enemy caster",1,{8,2}};
        session=hero_first(*module,encounter);session->submit(command(*session,"end"));
        check(session->snapshot().actor==2,"Enemy caster turn");
        session->submit(command(*session,"magic_missile"));session->submit(command(*session,"end"));
        const auto before=unit(*session,1);check(before.hit_points<before.max_hit_points,"Damage before healing");
        const bool cleric=std::string_view(profile)=="healer";
        session->submit(command(*session,cleric?"cure_wounds":"second_wind"));
        const auto healed=unit(*session,1);
        check(healed.hit_points>before.hit_points&&healed.hit_points<=healed.max_hit_points,"Healing restores HP up to maximum");
        check(cleric?!healed.action:(!healed.bonus_action&&healed.action),"Healing spends the correct action budget");
    }

    encounter=duel("bandit");encounter.participants[1]={2,"adept","Enemy caster",1,{8,2}};
    encounter.participants.push_back({3,"vanguard","Ally",0,{1,5}});tested=false;
    for(unsigned seed=0;seed<100&&!tested;++seed) {
        session=module->create(encounter,seed);if(session->snapshot().actor!=2)continue;
        auto missile=command(*session,"magic_missile");missile.target=1;
        check(session->submit(missile),"Target player with spell");if(unit(*session,1).hit_points>0)continue;
        check(!unit(*session,1).dead&&session->snapshot().outcome==Outcome::ongoing,"Zero HP incapacitates player while ally can fight");
        for(int i=0;i<3;++i)session->submit(command(*session,"end"));
        const auto log=session->snapshot().log;
        check(std::any_of(log.begin(),log.end(),[](const auto& line){return line.find("death save")!=std::string::npos;}),"Unconscious player makes death saves on its turn");tested=true;
    }
    check(tested,"Exercised player unconscious/death-save flow");
}
void installed() {
    const auto directory=std::getenv("OPENGOLD_GAME_DIR");if(!directory)return;
    CombatDemo demo(srd5::load(pack()));demo.slums(directory,42);
    for(unsigned i=0;!demo.has_combat();++i){check(i<10&&demo.waiting(),"Original script reaches combat");demo.continue_script();}
    check(demo.combat().snapshot().combatants.size()==8,"Fixed four-person party and four original orcs");
    for(unsigned i=0;demo.combat().snapshot().outcome==Outcome::ongoing;++i) {
        check(i<1000,"Original Slums combat completes");check(demo.submit(choose_demo_command(demo.combat())),"Real combat command accepted");
    }
    const auto outcome=demo.combat().snapshot().outcome;
    while(demo.waiting())demo.continue_script();
    check(demo.script_complete(),"Original ECL resumes after real combat");
    check(demo.script_variable(0x6DC7)==(outcome==Outcome::victory?0:128),"Actual outcome mapped to original ECL");
    if(outcome==Outcome::victory) {
        check(demo.script_variable(0x6DC8)==4,"Actual defeated count returned");
        check(demo.script_variable(0x4ACA)==255&&demo.script_variable(0x4ABB)==1,"Victory updates original event state");
        demo.revisit();check(demo.script_complete()&&!demo.waiting(),"Completed event does not replay the fight");
        check(demo.script_variable(0x4ABB)==1,"Revisit preserves fight count");
    }
    std::cout<<"Original Slums event completed with real rules combat: "<<(outcome==Outcome::victory?"victory":"defeat")<<".\n";
}
}
int main(){try{boundary_tests();mechanics_tests();installed();std::cout<<"Rules tests passed.\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
