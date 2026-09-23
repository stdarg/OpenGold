#include "opengold/srd5.h"
#include "opengold/combat_demo.h"
#include "opengold/character_rules.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
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
std::unique_ptr<CombatSession> actor_first(const RulesModule& module,Encounter encounter,EntityId id) {
    for(unsigned seed=0;seed<100;++seed){auto session=module.create(encounter,seed);if(session->snapshot().actor==id)return session;}
    throw std::runtime_error("No matching first actor seed");
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
void turn_budget_tests()
{
    auto module=srd5::load(pack());
    CharacterDraft draft;draft.race="human";draft.gender="female";draft.character_class="wizard";
    draft.background="sage";draft.alignment="neutral_good";draft.name="Mage";draft.rolled=true;
    for(auto& roll:draft.rolls)roll={{6,5,4,1},3};
    auto sheet=srd5::character_rules()->evaluate(draft,true);VitalState unused;
    for(unsigned level=2;level<=3;++level){auto choice=module->default_advancement(sheet);
        if(level==3)choice.spells={"magic_missile","scorching_ray"};
        check(module->advance_character(sheet,unused,choice),"Create a caster with level-two spells");}
    for(const auto verb:{"melee","ranged","fire_bolt","magic_missile","magic_missile_2","scorching_ray"})
    for(const bool move_first:{false,true}){
        auto e=duel();e.participants[1].definition="vanguard";
        e.participants.push_back({3,"vanguard","Reserve enemy",1,{10,7}});
        const bool weapon=std::string_view(verb)=="melee"||std::string_view(verb)=="ranged";
        if(weapon)e.participants[0].state=VitalState{10,false,"SRD1 2 0 0 0 0"};
        else e.participants[0].character_profile=module->character_profile(sheet,{}).data;
        auto session=hero_first(*module,e);
        if(move_first)check(session->submit(command(*session,"move",{2,3})),"Move before attacking");
        const auto before=unit(*session,1);const auto attack=command(*session,verb);
        check(session->submit(attack),"Offensive action accepted");
        check(session->snapshot().actor==1&&!session->snapshot().reaction_pending&&session->snapshot().elapsed_milliseconds==0,
            "Attacks and damaging spells retain the active turn without advancing time");
        const auto after=unit(*session,1);
        check(!after.action&&after.bonus_action==before.bonus_action&&after.reaction==before.reaction&&after.movement_feet==before.movement_feet,
            "Only the action and applicable spell slot are spent by the attack");
        const std::string resources=weapon?"SRD1 2 0 0 0 0":std::string_view(verb)=="fire_bolt"?"SRD2 0 4 2 0 0 0":
            std::string_view(verb)=="magic_missile"?"SRD2 0 3 2 0 0 0":"SRD2 0 4 1 0 0 0";
        check(after.persistent.resources==resources,"Cantrips preserve slots; leveled spells spend exactly the selected slot");
        const auto saved=session->save();auto restored=module->restore(saved);
        check(restored->save()==saved,"A post-attack checkpoint retains unused turn resources");
        auto repeated=attack;repeated.revision=session->snapshot().revision;
        check(!session->submit(repeated)&&!session->submit(attack)&&session->save()==saved,"Spent actions and stale tickets reject without consuming state or randomness");
        const auto move=command(*session,"move",{3,3});
        check(session->submit(move)&&restored->submit(move)&&session->save()==restored->save(),"Movement after an attack continues identically after reload");
        check(unit(*session,1).movement_feet==before.movement_feet-5&&!unit(*session,1).action,"Movement neither refreshes the action nor the movement budget");
        if(weapon){const auto wind=command(*session,"second_wind");
            check(session->submit(wind)&&restored->submit(wind)&&session->save()==restored->save(),"Second Wind remains usable after attacking and moving");
            check(!unit(*session,1).bonus_action&&!unit(*session,1).action&&unit(*session,1).persistent.resources=="SRD1 1 0 0 0 0", "Bonus healing does not refund the action or Second Wind");}
        const auto end=command(*session,"end");
        check(session->submit(end)&&restored->submit(end)&&session->save()==restored->save(),"Explicit End Turn continues deterministically");
        check(session->snapshot().actor!=1&&session->snapshot().elapsed_milliseconds>0,"End Turn advances initiative and time");
    }
    // A used Bonus Action must stay used when the action is taken afterward.
    auto e=duel();e.participants[1].definition="vanguard";e.participants[0].state=VitalState{10,false,"SRD1 2 0 0 0 0"};
    auto session=hero_first(*module,e);session->submit(command(*session,"second_wind"));session->submit(command(*session,"melee"));
    check(session->snapshot().actor==1&&!unit(*session,1).bonus_action&&!offers(*session,"second_wind"),"Attacking does not refresh a previously used Bonus Action");
    // Movement reactions still interrupt before the step and resume this turn.
    e.participants[0].state.reset();
    for(const auto resolution:{"decline","opportunity"}){
        session=hero_first(*module,e);session->submit(command(*session,"melee"));
        check(session->submit(command(*session,"move",{1,2}))&&session->snapshot().reaction_pending,"Moving after an attack can provoke an opportunity reaction");
        auto restored=module->restore(session->save());const auto response=command(*session,resolution);
        check(session->submit(response)&&restored->submit(response)&&session->save()==restored->save(),"Post-attack movement reaction restores and resolves in order");
        check(session->snapshot().actor==1&&unit(*session,1).cell==Cell{1,2}&&unit(*session,1).movement_feet==20&&!unit(*session,1).action,
            "Resolved reaction resumes remaining movement without another action");
    }
    // The existing facing reaction is removed by I06; until then, resolving it
    // must resume the attacker's remaining turn, including multiple reactors.
    e=duel();e.participants[1].definition="vanguard";e.participants[1].cell={1,2};
    e.participants.push_back({3,"bandit","Right guard",1,{3,2}});
    e.participants.push_back({4,"bandit","Other guard",1,{3,1}});
    session=hero_first(*module,e);auto left=command(*session,"melee");left.target=2;session->submit(left);
    unsigned reactions=0;
    while(session->snapshot().reaction_pending){
        check(++reactions<=2&&!offers(*session,"end")&&!offers(*session,"move"),"Pending reactions prevent advancing or moving early");
        auto restored=module->restore(session->save());const auto decline=command(*session,"decline");
        check(session->submit(decline)&&restored->submit(decline)&&session->save()==restored->save(),"Every queued reaction preserves checkpoint continuation");
    }
    check(reactions==2&&session->snapshot().actor==1&&!unit(*session,1).action&&unit(*session,1).movement_feet==30,"All reactions resolve before the attacker resumes");
    // The enemy controller explicitly ends a spent turn instead of moving
    // toward another attack it cannot make. Available bonus recovery runs first.
    for(const bool injured:{false,true}){
        e=duel();e.participants[1].definition="vanguard";e.participants[1].cell={8,2};e.participants[1].facing_left=true;
        if(injured)e.participants[1].state=VitalState{5,false,"SRD1 2 0 0 0 0"};
        session=actor_first(*module,e,2);session->submit(command(*session,"ranged"));
        check(session->snapshot().actor==2,"Enemy attacks also retain their turn");
        if(injured){auto choice=choose_demo_command(*session);check(choice.verb=="second_wind"&&session->submit(choice),"Enemy uses remaining bonus recovery after attacking");}
        const auto choice=choose_demo_command(*session);check(choice.verb=="end"&&session->submit(choice),"Enemy explicitly completes its spent turn");
        check(session->snapshot().actor!=2,"Enemy turn cannot stall after its attack");
    }
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
    const auto preview=session->movement_reach(1);
    const auto legal=session->legal_commands();
    check(std::count_if(legal.begin(),legal.end(),[](const auto& c){return c.verb=="move";})==preview.size(),
        "Active movement preview matches legal move count");
    for(const auto& cell:preview)check(std::any_of(legal.begin(),legal.end(),
        [&](const auto& c){return c.verb=="move"&&c.destination==cell;}),"Active movement preview uses legal destinations");
    check(!session->movement_reach(2).empty()&&session->movement_reach(999).empty(),
        "Off-turn combatants have rules-owned movement previews");
    auto invalid=command(*session,"melee");invalid.actor=999;
    check(!session->submit(invalid)&&session->save()==before,"Invalid actor cannot change state or spend randomness");
    invalid=command(*session,"move",{1,2});invalid.destination={4,4};
    check(!session->submit(invalid)&&session->save()==before,"Invalid path cannot change state");
    auto attack=command(*session,"melee");check(session->submit(attack),"Melee command");
    check(session->snapshot().outcome!=Outcome::ongoing||session->snapshot().actor==1,
        "Melee attack preserves the actor's remaining turn");
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

    auto flank=duel();
    flank.participants[1].cell={1,2};
    flank.participants.push_back({3,"bandit","Right Guard",1,{3,2}});
    session=hero_first(*module,flank);
    const auto left_attack=[&](const CombatSession& combat,EntityId target){
        const auto commands=combat.legal_commands();
        const auto found=std::find_if(commands.begin(),commands.end(),[&](const auto& c){return c.verb=="melee"&&c.target==target;});
        check(found!=commands.end(),"Expected left-side melee target");
        return *found;
    };
    check(!unit(*session,1).facing_left,"Combatants initially face right");
    check(session->submit(left_attack(*session,2)),"Hero attacks to the left");
    check(unit(*session,1).facing_left&&session->snapshot().reaction_pending&&session->snapshot().actor==3,
        "Hero turns left and exposes the adjacent enemy on the right");
    auto turning_checkpoint=session->save();
    restored=module->restore(turning_checkpoint);
    check(restored->save()==turning_checkpoint,"Facing and pending turn reaction survive save/load");
    auto striking=module->restore(turning_checkpoint);
    check(striking->submit(command(*striking,"opportunity")),"Turning opportunity command accepted");
    const auto strike_log=striking->snapshot().log;
    check(std::any_of(strike_log.begin(),strike_log.end(),
            [](const auto& line){return line.find("Right Guard -> Hero")!=std::string::npos;}),
        "Right-side enemy makes an opportunity attack on the turning attacker");
    const auto decline_turn=command(*session,"decline");
    check(session->submit(decline_turn)&&restored->submit(decline_turn)&&session->save()==restored->save(),
        "Declining the turning reaction resumes the turn deterministically");
    check(!session->snapshot().reaction_pending&&session->snapshot().actor==1&&unit(*session,1).facing_left,
        "Resolved turn reaction keeps the hero's remaining turn and facing");

    auto reverse=duel();
    reverse.participants[0].facing_left=true;
    reverse.participants.push_back({3,"bandit","Left Guard",1,{1,2}});
    session=hero_first(*module,reverse);
    check(session->submit(left_attack(*session,2)),"Hero attacks to the right from a left-facing pose");
    check(!unit(*session,1).facing_left&&session->snapshot().reaction_pending&&session->snapshot().actor==3,
        "Turning right exposes the adjacent enemy on the left");

    auto monster_flank=duel();
    monster_flank.participants[0].cell={1,2};
    monster_flank.participants[1].cell={2,2};
    monster_flank.participants.push_back({3,"vanguard","Right Hero",0,{3,2}});
    session=actor_first(*module,monster_flank,2);
    check(session->submit(left_attack(*session,1)),"Monster attacks to the left");
    check(unit(*session,2).facing_left&&session->snapshot().reaction_pending&&session->snapshot().actor==3,
        "Monster turns left and exposes the adjacent hero on the right");
    check(session->submit(command(*session,"opportunity")),"Hero opportunity attack on turning monster accepted");
    const auto hero_reaction_log=session->snapshot().log;
    check(std::any_of(hero_reaction_log.begin(),hero_reaction_log.end(),
        [](const auto& line){return line.find("Right Hero -> Bandit")!=std::string::npos;}),
        "A party character can strike the monster that turns away");

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
    auto legacy_rules=srd5::load(pack());
    check(legacy_rules->accepts_campaign_identity({"opengold.srd5","0.5.0","srd-5.2.1-demo.1/15286736505479635800"}),
        "Saved campaigns from the previous Kobold rules remain loadable");
    auto kobolds=duel();kobolds.participants[1].definition="slums-kobold";
    auto leader=kobolds;leader.participants[1].definition="slums-kobold-leader";
    auto kobold_rules=srd5::load(pack());bool checked_kobold=false,checked_leader=false;
    for(unsigned seed=0;seed<100&&(!checked_kobold||!checked_leader);++seed){
        if(!checked_kobold){auto fight=kobold_rules->create(kobolds,seed);if(fight->snapshot().actor==2){
            check(offers(*fight,"melee")&&!offers(*fight,"ranged"),"Dagger Kobold has no ranged attack");
            check(unit(*fight,2).type_name=="Kobold"&&unit(*fight,2).melee_weapon=="Dagger"&&
                !unit(*fight,2).ranged_attack_available,"Kobold hover data follows its melee-only rules");
            check(command(*fight,"melee").label=="Dagger attack","Kobold attack names its visible weapon");checked_kobold=true;}}
        if(!checked_leader){auto fight=kobold_rules->create(leader,seed);if(fight->snapshot().actor==2){
            check(offers(*fight,"melee")&&offers(*fight,"ranged"),"Kobold leader retains bow attack");
            check(unit(*fight,2).type_name=="Kobold Leader"&&unit(*fight,2).ranged_weapon=="Short bow"&&
                unit(*fight,2).ranged_attack_available,"Leader hover data follows its short bow rules");
            check(command(*fight,"ranged").label=="Short bow attack","Leader ranged attack names its bow");checked_leader=true;}}
    }
    check(checked_kobold&&checked_leader,"Exercised both Kobold attack profiles");
    auto sword_leader=leader;sword_leader.participants[1].definition="slums-kobold-leader-sword";
    bool checked_sword_leader=false;
    for(unsigned seed=0;seed<100&&!checked_sword_leader;++seed){
        auto fight=kobold_rules->create(sword_leader,seed);if(fight->snapshot().actor!=2)continue;
        check(offers(*fight,"melee")&&!offers(*fight,"ranged")&&
            command(*fight,"melee").label=="Short sword attack",
            "Original record 11 leader cannot fire a bow it does not carry");checked_sword_leader=true;
    }
    check(checked_sword_leader,"Exercised sword-only Kobold leader");
    kobolds.participants[1].cell={8,2};leader.participants[1].cell={8,2};
    checked_kobold=checked_leader=false;
    for(unsigned seed=0;seed<100&&(!checked_kobold||!checked_leader);++seed){
        if(!checked_kobold){auto fight=kobold_rules->create(kobolds,seed);if(fight->snapshot().actor==2){
            check(!offers(*fight,"ranged"),"Distant ordinary Kobold cannot attack without a bow");checked_kobold=true;}}
        if(!checked_leader){auto fight=kobold_rules->create(leader,seed);if(fight->snapshot().actor==2){
            check(!offers(*fight,"melee")&&fight->submit(command(*fight,"ranged")),
                "Distant leader can fire its short bow");checked_leader=true;}}
    }
    check(checked_kobold&&checked_leader,"Exercised distant Kobold attacks");
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
        check(session->snapshot().outcome!=Outcome::ongoing||session->snapshot().actor==1,
            "Damaging spell preserves remaining turn resources");
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
        check(session->snapshot().actor==1,"Lethal attack preserves the actor's turn while other enemies remain");
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
        session->submit(command(*session,"magic_missile"));
        const auto before=unit(*session,1);check(before.hit_points<before.max_hit_points,"Damage before healing");
        check(session->submit(command(*session,"end")),"Enemy caster explicitly finishes its turn");
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

    encounter=duel("bandit");encounter.participants[1]={2,"adept","Enemy caster",1,{8,2}};
    tested=false;
    for(unsigned seed=0;seed<500&&!tested;++seed){
        session=module->create(encounter,seed);if(session->snapshot().actor!=2)continue;
        auto missile=command(*session,"magic_missile");missile.target=1;
        session->submit(missile);
        if(unit(*session,1).hit_points>0)continue;
        check(!unit(*session,1).dead&&session->snapshot().outcome==Outcome::defeat,
            "Combat ends when the last party member is unconscious, without declaring them dead");
        tested=true;
    }
    check(tested,"Exercised all-unconscious party defeat flow");
}
void death_save_turn_entry_tests()
{
    auto module=srd5::load(pack());auto encounter=duel();
    encounter.participants.push_back({3,"vanguard","Ally",0,{1,5}});
    const auto death_rolls=[](const CombatSession& session){
        std::vector<int> rolls;
        for(const auto& line:session.snapshot().log)
            if(line.starts_with("Hero death save: "))rolls.push_back(std::stoi(line.substr(17)));
        return rolls;
    };
    // Exercise all four outcomes both in the first initiative slot and later.
    for(const bool first:{true,false}){
        bool natural_one=false,natural_twenty=false,success=false,failure=false;
        for(unsigned seed=0;seed<1000&&!(natural_one&&natural_twenty&&success&&failure);++seed){
            const auto healthy=module->create(encounter,seed);
            if((healthy->snapshot().actor==1)!=first)continue;
            auto wounded=encounter;wounded.participants[0].state=VitalState{0,false,"SRD1 1 0 2 1 0"};
            auto combat=module->create(wounded,seed);
            if(!first){
                check(death_rolls(*combat).empty(),"Death saves wait for the wounded actor's initiative slot");
                for(unsigned turns=0;turns<3&&death_rolls(*combat).empty();++turns)
                    check(combat->submit(command(*combat,"end")),"Advance to the wounded actor's turn");
            }
            const auto rolls=death_rolls(*combat);
            check(rolls.size()==1,"Every unstable turn entry makes exactly one death save, including the first slot");
            const auto hero=unit(*combat,1);const int roll=rolls.front();
            if(roll==20){
                natural_twenty=true;
                check(hero.hit_points==1&&!hero.dead&&hero.persistent.resources=="SRD1 1 0 0 0 0","Natural 20 restores 1 HP and clears both counters without restoring spent resources");
                check(combat->snapshot().actor==1&&hero.action&&hero.bonus_action&&hero.reaction,"Natural-20 recovery permits the current turn");
                if(first)check(combat->snapshot().elapsed_milliseconds==0,"Natural-20 recovery does not skip the first initiative slot");
            }else if(roll>=10){
                success=true;
                check(hero.hit_points==0&&!hero.dead&&hero.persistent.resources=="SRD1 1 0 0 0 1","Third success stabilizes and resets successes and failures");
                check(combat->snapshot().actor!=1,"Stable unconscious actors cannot act");
            }else if(roll==1){
                natural_one=true;
                check(hero.dead&&hero.persistent.resources=="SRD1 1 0 2 3 0","Natural 1 adds two failures and reaches death at three");
            }else{
                failure=true;
                check(!hero.dead&&hero.persistent.resources=="SRD1 1 0 2 2 0","Ordinary failure adds one and retains prior successes");
            }
            const auto saved=combat->save();auto restored=module->restore(saved);
            check(restored->save()==saved&&death_rolls(*restored)==rolls,"Checkpoint restore does not replay a turn-entry save");
            for(unsigned turns=0;turns<3;++turns){
                const auto end=command(*combat,"end");
                check(combat->submit(end)&&restored->submit(end)&&combat->save()==restored->save(),"Death-save continuation retains exact RNG and state after restore");
            }
        }
        check(natural_one&&natural_twenty&&success&&failure,"Exercise all death-save outcomes at each turn-entry position");
    }
    // Find the same first-slot seed without relying on the private dice stream.
    unsigned first_seed=0;
    while(module->create(encounter,first_seed)->snapshot().actor!=1){check(++first_seed<100,"Find first-slot seed");}
    for(const bool dead:{false,true}){
        auto skipped=encounter;
        skipped.participants[0].state=VitalState{0,dead,dead?"SRD1 1 0 0 3 0":"SRD1 1 0 0 0 1"};
        auto combat=module->create(skipped,first_seed);
        check(combat->snapshot().actor!=1&&death_rolls(*combat).empty(),"Stable and dead initial actors do not make death saves");
        auto restored=module->restore(combat->save());
        check(restored->save()==combat->save(),"Skipped initial actor preserves exact checkpoint state");
    }
    auto legacy=encounter;legacy.participants[0].state=VitalState{0,false,"SRD1 1 0 3 2 1"};
    auto migrated=module->create(legacy,first_seed);
    check(death_rolls(*migrated).empty()&&unit(*migrated,1).persistent.resources=="SRD1 1 0 0 0 1",
        "Older stable resource state is normalized without rolling or refilling resources");
}
void checkpoint_validation_tests()
{
    auto module = srd5::load(pack());
    auto session = hero_first(*module);
    session->submit(command(*session,"move",{1,2}));
    const auto checkpoint = session->save();
    std::vector<std::string> lines;
    std::istringstream input(checkpoint);
    for (std::string line; std::getline(input,line);) lines.push_back(line);
    const auto path_header = 4 + session->snapshot().combatants.size();
    const auto encode = [](const std::vector<std::string>& fields) {
        std::string result;
        for (const auto& line : fields) result += line + '\n';
        return result;
    };
    const auto reject_changes = [&](std::initializer_list<std::pair<std::size_t,std::string>> changes) {
        auto corrupt = lines;
        for (const auto& [line,text] : changes) corrupt.at(line) = text;
        rejects([&] { (void)module->restore(encode(corrupt)); }, "Malformed checkpoint accepted");
        check(session->save() == checkpoint, "Rejected restore altered the live session");
    };
    reject_changes({{path_header,"1025 0"}});
    reject_changes({{path_header,"1 2"}});
    reject_changes({{path_header,"0 0"},{path_header+1,""}}); // Reaction with no next step.
    reject_changes({{path_header+1,"99 99"}});
    reject_changes({{path_header+1,"0 8"}}); // Nonadjacent step on an otherwise open cell.
    reject_changes({{path_header+1,"3 2"}}); // Path enters the enemy's occupied cell.
    reject_changes({{path_header+2,"0 0"},{path_header+3,""}}); // Path without a pause.
    reject_changes({{path_header+2,"1 2"}});
    reject_changes({{path_header+2,"2 0"},{path_header+3,"2 2"}}); // Duplicate reactor.
    reject_changes({{path_header+3,"999"}});
    reject_changes({{path_header+3,"1"}}); // Mover cannot react against itself.
    reject_changes({{path_header+4,"81"}});
    // Fuzz regression: extra movement requires an already spent action. An
    // accepted 31-foot budget plus an unused Dash used to produce 61 feet,
    // which the next restore rejected. Preserve the genuine 60-foot boundary.
    auto actor_line = lines[4];
    std::istringstream actor_input(actor_line);
    EntityId id; std::string definition, name;
    int side, x, y, hp, initiative, movement;
    actor_input >> id >> std::quoted(definition) >> std::quoted(name)
                >> side >> x >> y >> hp >> initiative >> std::ws;
    const auto movement_start = static_cast<std::size_t>(actor_input.tellg());
    actor_input >> movement;
    const auto movement_end = static_cast<std::size_t>(actor_input.tellg());
    check(movement == 30, "Regression fixture has its original movement allowance");
    for (const auto extra : {31,60}) {
        auto corrupt_actor = actor_line;
        corrupt_actor.replace(movement_start,movement_end-movement_start,std::to_string(extra));
        reject_changes({{4,corrupt_actor}});
    }
    auto dashed = hero_first(*module);
    check(dashed->submit(command(*dashed,"dash")), "Dash is accepted from the normal movement boundary");
    check(unit(*dashed,1).movement_feet == 60 && !offers(*dashed,"dash"), "Dash spends the action for extra movement");
    check(module->restore(dashed->save())->save() == dashed->save(), "Legitimate doubled movement round trips");
    // Tickets wrap past reserved zero; round numbers saturate. Both boundary
    // states must remain playable and saveable after offered commands execute.
    for (const auto round : {100000u,std::numeric_limits<unsigned>::max()}) {
        auto boundary = lines;
        std::istringstream counters(boundary[3]);
        std::uint64_t rng; counters >> rng;
        boundary[3] = std::to_string(rng)+" "+std::to_string(std::numeric_limits<std::uint64_t>::max())+
                      " 0 "+std::to_string(round)+" 0 2";
        auto continued = module->restore(encode(boundary));
        const auto decline = command(*continued,"decline");
        check(continued->submit(decline) && continued->snapshot().revision == 1,"Ticket rollover skips reserved zero");
        check(!continued->submit(decline),"Pre-rollover ticket is stale");
        continued->submit(command(*continued,"end"));
        continued->submit(command(*continued,"end"));
        check(continued->snapshot().round == (round == 100000 ? 100001 : round),"Round increments or saturates without wrapping");
        check(module->restore(continued->save())->save() == continued->save(),"Counter boundary continuation round trips");
    }
    for (std::size_t count = 0; count < lines.size(); ++count) {
        auto truncated = lines;
        truncated.resize(count);
        rejects([&] { (void)module->restore(encode(truncated)); }, "Truncated checkpoint section accepted");
    }

    auto previous = lines;
    previous[0].replace(9,1,"4");
    previous.pop_back(); // Version 4 has no pending turn-reaction state.
    for(std::size_t actor=4;actor<path_header;++actor)
        previous[actor].resize(previous[actor].find_last_of(' ')); // Version 4 has no facing field.
    check(module->restore(encode(previous))->save()==checkpoint,"Version 4 checkpoint migrates to facing right");

    // Earlier formats omitted the empty character profile (v1), and the
    // second-level slot/feat flags (v1/v2). Their defaults must still round trip.
    for (const unsigned version : {1u,2u}) {
        auto legacy = lines;
        legacy[0].replace(9,1,std::to_string(version));
        legacy.resize(legacy.size()-4); // v4 scope/clock, two effect collections, and v5 turn state.
        for (std::size_t actor = 4; actor < path_header; ++actor) {
            const auto profile = legacy[actor].rfind("\"\"");
            check(profile != std::string::npos, "Expected fixture with no character profile");
            legacy[actor].resize(profile + (version == 2 ? 2 : 0));
        }
        check(module->restore(encode(legacy))->save() == checkpoint, "Legacy checkpoint defaults changed");
    }

    // A route pauses after spending 10 feet while leaving an enemy's reach.
    // Only its remaining movement may be charged when validating a restore.
    Battlefield corridor{5,3,std::vector<std::uint8_t>(15,1)};
    for (int x = 0; x < 5; ++x) corridor.terrain[5+x] = 0;
    corridor.terrain[1] = corridor.terrain[2] = 0; // Clear sight from the reactor.
    Encounter encounter{corridor,{{1,"vanguard","Mover",0,{0,1}},
                                 {2,"bandit","Reactor",1,{1,0}}}};
    session = hero_first(*module,encounter);
    check(session->submit(command(*session,"move",{4,1})), "Clear corridor move accepted");
    check(session->snapshot().reaction_pending && unit(*session,1).cell == Cell{2,1},
          "Route pauses when leaving the reactor's reach");
    check(unit(*session,1).movement_feet == 20, "Prefix spends its movement once");
    auto restored = module->restore(session->save());
    check(restored->save() == session->save(), "Paused movement checkpoint restores exactly");
    const auto decline = command(*session,"decline");
    check(session->submit(decline) && restored->submit(decline), "Both routes resume after the reaction");
    check(restored->save() == session->save(), "Restored suffix preserves deterministic continuation");
    check(unit(*session,1).cell == Cell{4,1} && unit(*session,1).movement_feet == 10,
          "Only the remaining route suffix spends movement after restore");
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
int main(){try{turn_budget_tests();boundary_tests();mechanics_tests();death_save_turn_entry_tests();checkpoint_validation_tests();installed();std::cout<<"Rules tests passed.\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
