#include "opengold/campaign_save.h"
#include "opengold/character_creator.h"
#include "opengold/character_pool.h"
#include "opengold/srd5.h"
#include "combat_fixture.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace opengold;
using namespace opengold::rules;
namespace {
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"Invalid training must reject");}
auto module(){return srd5::load(std::filesystem::path(OPENGOLD_SOURCE_DIR)/"data/rules/srd-5.2.1/combat.rules");}
CharacterDraft draft(std::string klass="rogue",std::string background="criminal"){
    CharacterDraft d;d.race="human";d.gender="female";d.character_class=klass;d.background=background;
    d.alignment="neutral_good";d.name="Training tester";d.rolled=true;for(auto& r:d.rolls)r={{6,5,4,1},3};return d;
}
TrainingChoices choices(){return {{"origin:languages",{"elvish","dwarvish"}},
    {"class:rogue",{"acrobatics","investigation","perception","persuasion"}},
    {"class:rogue:expertise",{"stealth","perception"}},{"class:rogue:thieves_cant",{"undercommon"}}};}
Character hero(const CharacterDraft& d){return Character(*srd5::character_rules(),d,{});}
const SkillTraining& skill(const CharacterSheet& sheet,std::string_view id){
    const auto found=std::find_if(sheet.training.skills.begin(),sheet.training.skills.end(),[&](const auto& s){return s.id==id;});check(found!=sheet.training.skills.end(),"Skill exists");return *found;
}
std::string fixture(const char* name){std::ifstream in(std::filesystem::path(OPENGOLD_SOURCE_DIR)/"tests/fixtures"/name);check(bool(in),"Frozen fixture exists");return {std::istreambuf_iterator<char>(in),{}};}
void replace(std::string& text,std::string_view from,std::string_view to){const auto pos=text.find(from);check(pos!=text.npos,"Fixture field exists");text.replace(pos,from.size(),to);}
std::string corrupt(std::string bytes,std::string_view from,std::string_view to){
    auto body=bytes.substr(bytes.find('\n',bytes.find('\n')+1)+1);replace(body,from,to);
    std::uint64_t hash=14695981039346656037ULL;for(unsigned char c:body){hash^=c;hash*=1099511628211ULL;}
    return bytes.substr(0,bytes.find('\n')+1)+std::to_string(hash)+'\n'+body;
}
void creation_controls(){
    CharacterCreator creator(srd5::character_rules(),42);
    creator.select(CreationField::character_class,"rogue");creator.select(CreationField::background,"criminal");
    creator.next();creator.next();
    for(unsigned tries=0;;++tries){
        check(tries<100,"Roll a qualified Rogue fixture");creator.roll();for(unsigned i=0;i<6;++i)creator.assign_roll(i,i);
        if(creator.rules().class_eligible(creator.draft(),"rogue"))break;
    }
    creator.next();creator.next();
    check(creator.step()==CreationStep::training&&!creator.training_complete(),"Training follows Class and starts incomplete");
    rejects([&]{creator.next();});rejects([&]{creator.training_choice("unknown","elvish",true);});
    rejects([&]{creator.training_choice("origin:languages","abyssal",true);});
    rejects([&]{creator.training_choice("class:rogue:expertise","arcana",true);});
    // Independent, authored choices exercise dependent groups in UI order.
    const auto selected=choices();
    for(const auto* group:{"origin:languages","class:rogue","class:rogue:expertise","class:rogue:thieves_cant"})
        for(const auto& value:selected.at(group))creator.training_choice(group,value,true);
    check(creator.training_complete()&&creator.sheet().training.complete,"Every required choice permits completion");
    const auto before=creator.draft().training;
    creator.training_choice("origin:languages","elvish",true);
    rejects([&]{creator.training_choice("origin:languages","orc",true);});
    check(creator.draft().training==before,"Duplicate selection is idempotent; excessive selection rejects atomically");
    creator.next();creator.back();check(creator.draft().training==before,"Back preserves selected training");
    creator.training_choice("class:rogue","perception",false);
    check(creator.draft().training.at("class:rogue:expertise")==std::vector<std::string>{"stealth"},"Removing a skill removes only its dependent Expertise");
    creator.training_choice("class:rogue","perception",true);creator.training_choice("class:rogue:expertise","perception",true);
    creator.back();creator.back();creator.select(CreationField::background,"sage");
    check(creator.draft().training.at("class:rogue:expertise")==std::vector<std::string>{"perception"},"Changing background removes only lost proficiency's Expertise");
    check(std::is_permutation(creator.draft().training.at("class:rogue").begin(),creator.draft().training.at("class:rogue").end(),selected.at("class:rogue").begin(),selected.at("class:rogue").end()),"Changing background preserves valid skill choices");
    creator.select(CreationField::background,"criminal");creator.training_choice("class:rogue:expertise","stealth",true);
    creator.training_choice("class:rogue:thieves_cant","undercommon",false);creator.training_choice("class:rogue:thieves_cant","orc",true);
    creator.training_choice("origin:languages","dwarvish",false);creator.training_choice("origin:languages","orc",true);
    check(!creator.draft().training.contains("class:rogue:thieves_cant"),"Moving a language into starting choices invalidates only the duplicate Rogue choice");
    creator.training_choice("class:rogue:thieves_cant","undercommon",true);
    creator.select(CreationField::character_class,"fighter");
    check(creator.draft().training.size()==1&&creator.draft().training.at("origin:languages")==std::vector<std::string>({"elvish","orc"}),"Class change preserves starting languages and clears Rogue groups");
    creator.select(CreationField::character_class,"rogue");
    check(!creator.training_complete(),"Returning to Rogue does not invent cleared choices");
    for(const auto* group:{"class:rogue","class:rogue:expertise","class:rogue:thieves_cant"})
        for(const auto& value:selected.at(group))creator.training_choice(group,value,true);
    creator.next();creator.next();creator.next();creator.name("Created Rogue");creator.next();creator.next();
    const auto finished=creator.create_character();check(finished.sheet().training.complete,"Finished character retains the training selected through the creator");
    CampaignParty party(module());const auto id=party.add_pc(finished);const auto encoded=encode_campaign(party,nullptr,"creator");
    auto loaded=decode_campaign(encoded,creator.rules(),*module(),"creator",nullptr);CampaignParty restored(module());restored.restore(std::move(loaded.party));
    check(restored.member(id).character.creation_data().training==finished.creation_data().training,"Manually selected training survives party save/load");
    rejects([&]{creator.training_choice("origin:languages","elvish",false);});
    creator.restart();check(creator.draft().training.empty()&&!creator.training_complete(),"Restart clears training selections");
}
void preset_training(){
    por::CharacterArt art;Image head;head.width=88;head.height=40;head.rgba.assign(88*40*4,128);
    Image body;body.width=88;body.height=48;body.rgba.assign(88*48*4,128);
    art.heads.emplace(1,por::PortraitPart{"fixture",head});art.bodies.emplace(1,por::PortraitPart{"fixture",body});
    auto creation=srd5::character_rules();const auto pool=character_pool(*creation,art),again=character_pool(*creation,art);
    check(pool.size()==48,"Pool contains four presets for all twelve classes");std::map<std::string,unsigned> classes;
    for(unsigned i=0;i<pool.size();++i){
        const auto& c=pool[i];++classes[c.creation_data().character_class];
        check(c.sheet().training.complete&&c.creation_data().training==again[i].creation_data().training,"Preset training is complete and deterministic");
        check(c.sheet().training.languages.size()==(c.creation_data().character_class=="rogue"?5u:3u),"Preset languages are distinct and include all fixed and selected grants");
        CampaignParty party(module());const auto id=party.add_pc(c);const auto bytes=encode_campaign(party,nullptr,"preset");
        auto decoded=decode_campaign(bytes,*creation,*module(),"preset",nullptr);party.restore(std::move(decoded.party));
        check(party.member(id).character.creation_data().training==c.creation_data().training&&party.member(id).character.sheet().training.complete,"Preset training remains complete after adding and saving");
    }
    check(classes.size()==12&&std::all_of(classes.begin(),classes.end(),[](const auto& c){return c.second==4;}),"Every class has four completed presets");
}
void grants_and_checks(){
    auto creation=srd5::character_rules();auto d=draft();auto sheet=hero(d).sheet();
    check(!sheet.training.complete&&sheet.training.skills.size()==18,"Missing choices stay pending while all ordinary skill modifiers are available");
    check(sheet.training.languages.size()==2&&sheet.training.tools.size()==1,"Common, Thieves' Cant and Thieves' Tools are fixed grants");
    check(sheet.training.tools[0].sources.size()==2,"Rogue and Criminal tool grants retain both sources");
    check(skill(sheet,"stealth").bonus==5&&!skill(sheet,"stealth").expertise,"Criminal grants Dexterity +3 plus proficiency +2 without inventing Expertise");
    d.training=choices();sheet=hero(d).sheet();
    check(sheet.training.complete&&sheet.training.languages.size()==5,"Two standard languages and a distinct Rogue language complete the fixed language grants");
    check(skill(sheet,"stealth").bonus==7&&skill(sheet,"perception").bonus==6,"Expertise adds doubled +2 proficiency to the governing ability");
    check(skill(sheet,"investigation").bonus==4&&skill(sheet,"arcana").bonus==2,"Proficient and untrained skills use different bonuses");
    check(skill(sheet,"stealth").sources.size()==2,"Expertise and background proficiency have separate provenance");
    auto result=creation->ability_check(sheet,1,{},"thieves_tools");
    check(result.ability_modifier==3&&result.proficiency==2&&result.total==5&&!result.tool_advantage&&result.sources.size()==2,"Duplicate tool grants add proficiency only once");
    result=creation->ability_check(sheet,1,"sleight_of_hand","thieves_tools");
    check(result.total==5&&result.tool_advantage&&!result.expertise,"Using a proficient skill and tool grants advantage without stacking proficiency");
    result=creation->ability_check(sheet,1,"stealth","thieves_tools");
    check(result.total==7&&result.tool_advantage&&result.expertise,"Tool proficiency does not add again on top of Expertise");
    result=creation->ability_check(sheet,0,"stealth",{});check(result.total==6,"A rule can choose another governing ability without changing training");
    rejects([&]{creation->ability_check(sheet,6,"stealth",{});});rejects([&]{creation->ability_check(sheet,1,"unknown",{});});
    rejects([&]{creation->ability_check(sheet,1,{},"unknown");});
    // Independent proficiency table boundaries. This query is shared math,
    // not a claim that Rogue advancement beyond level one is integrated.
    for(const auto [level,bonus]:{std::pair{1,2},std::pair{4,2},std::pair{5,3},std::pair{9,4},std::pair{13,5},std::pair{17,6},std::pair{20,6}}){
        auto later=sheet;later.level=level;check(creation->ability_check(later,1,"stealth",{}).total==3+2*bonus,"Expertise uses the character-level proficiency table");
    }
    d.training["class:rogue"]={"stealth","investigation","perception","persuasion"};sheet=hero(d).sheet();
    check(skill(sheet,"stealth").sources.size()==3&&skill(sheet,"stealth").bonus==7,"Overlapping class/background skill and Expertise grants do not stack bonuses");
    for(const auto& klass:creation->choices(CreationField::character_class)){
        auto other=draft(klass.id,"sage");other.training={{"origin:languages",{"common_sign_language","orc"}}};
        const auto s=hero(other).sheet();check(s.training.languages.size()==(klass.id=="rogue"?4u:3u),"Starting languages are available for every class");
    }
}
void invalid_choices(){
    const auto valid=[](){auto d=draft();d.training=choices();return d;};
    const std::vector<std::pair<std::string,std::vector<std::string>>> bad{
        {"origin:languages",{"elvish","elvish"}},{"origin:languages",{"common","elvish"}},
        {"origin:languages",{"abyssal","elvish"}},{"origin:languages",{"elvish","dwarvish","orc"}},
        {"class:rogue",{"acrobatics","arcana","perception","persuasion"}},
        {"class:rogue",{"acrobatics","acrobatics","perception","persuasion"}},
        {"class:rogue:expertise",{"arcana","stealth"}},{"class:rogue:expertise",{"thieves_tools","stealth"}},
        {"class:rogue:expertise",{"stealth","stealth"}},{"class:rogue:thieves_cant",{"elvish"}},
        {"class:rogue:thieves_cant",{"thieves_cant"}},{"class:rogue:thieves_cant",{"unknown"}},
        {"class:rogue:expertise",{"stealth","perception","persuasion"}},{"unknown",{"elvish"}}};
    for(const auto& [group,values]:bad){auto d=valid();d.training[group]=values;rejects([&]{(void)hero(d);});}
    auto d=valid();d.character_class="fighter";rejects([&]{(void)hero(d);});
    d=valid();d.training["class:rogue"].erase(d.training["class:rogue"].begin()+2);
    rejects([&]{(void)hero(d);}); // Cannot retain Perception Expertise after losing its proficiency.
    d=valid();d.training.erase("class:rogue:expertise");check(!hero(d).sheet().training.complete,"Incomplete selections remain explicitly pending");
    auto sheet=hero(valid()).sheet();auto rules=module();
    for(const auto& grant:std::vector<FeatureGrant>{{"skill:arcana","class:rogue",1,{}},{"expertise:stealth","class:rogue:expertise",1,{}},
            {"tool:thieves_tools","background:criminal",1,{}},{"language:abyssal","origin:languages",1,{}}}){
        auto invalid=sheet;invalid.grants.push_back(grant);rejects([&]{(void)rules->character_profile(invalid,{});});
    }
    auto invalid=sheet;std::erase_if(invalid.grants,[](const auto& g){return g.id=="language:common";});
    rejects([&]{(void)rules->character_profile(invalid,{});});
}
void persistence(){
    auto d=draft();d.training=choices();CampaignParty party(module());const auto id=party.add_pc(hero(d));
    auto state=party.checkpoint();state.roster[0].vitals.hit_points-=2;party.restore(state);
    const auto bytes=encode_campaign(party,nullptr,"training-fixture");
    auto loaded=decode_campaign(bytes,*srd5::character_rules(),*module(),"training-fixture",nullptr);
    CampaignParty restored(module());restored.restore(std::move(loaded.party));
    check(encode_campaign(restored,nullptr,"training-fixture")==bytes&&restored.member(id).character.creation_data().training==d.training,"Choice order, source grants and wounded state round trip exactly");
    for(const auto& bad:{corrupt(bytes,"\"elvish\"","\"abyssal\""),corrupt(bytes,"\"tool:thieves_tools\"","\"tool:unknown\"")})
        rejects([&]{(void)decode_campaign(bad,*srd5::character_rules(),*module(),"training-fixture",nullptr);});
    check(encode_campaign(party,nullptr,"training-fixture")==bytes,"Failed load does not replace live state");
    auto rules=module();auto members=party.participants();members[0].cell={1,1};members.push_back({99,"vanguard","Enemy",1,{5,1}});
    auto combat=rules->create({{8,8,std::vector<std::uint8_t>(64)},members},42);const auto checkpoint=combat->save();
    check(checkpoint.find("expertise:stealth")!=checkpoint.npos&&rules->restore(checkpoint)->save()==checkpoint,"Combat recipes retain training and Expertise provenance");
    auto invalid=checkpoint;replace(invalid,"class:rogue:expertise","class:rogue:invalid");rejects([&]{(void)rules->restore(invalid);});
    auto old=decode_campaign(fixture("campaign-v8-training.ogs"),*srd5::character_rules(),*rules,"training-fixture",nullptr);
    CampaignParty migrated(module());migrated.restore(std::move(old.party));
    for(unsigned n=1;n<=4;++n){const auto& m=migrated.member(n);
        check(m.character.creation_data().training.empty()&&!m.character.sheet().training.complete,"Old saves do not receive invented training choices");
        check(m.vitals.hit_points==m.character.sheet().hit_points-2,"Training migration preserves wounds");
    }
    check(migrated.member(3).vitals.resources=="SRD1 1 0 0 0 0"&&migrated.member(4).vitals.resources=="SRD2 0 1 1 0 0 0","Migration preserves spent feat/class resources");
    check(migrated.member(1).character.sheet().training.tools.front().sources.size()==2,"Older Rogue/Criminal gains the two known fixed tool sources");
    const auto pending=encode_campaign(migrated,nullptr,"training-fixture");
    auto again=decode_campaign(pending,*srd5::character_rules(),*rules,"training-fixture",nullptr);CampaignParty twice(module());twice.restore(std::move(again.party));
    check(encode_campaign(twice,nullptr,"training-fixture")==pending,"Unresolved choices remain pending across repeated saves");
    auto old_combat=fixture("combat-v8-training.save");auto migrated_combat=rules->restore(old_combat);
    old_combat=test::with_hit_dice(old_combat,rules->identity(),{{1,1},{2,1},{3,4},{4,4},{99,0}});
    check(migrated_combat->save()==old_combat,"Legacy combat retains every old recipe, RNG, wound and resource without injecting new choices");
    auto fighter=draft("fighter");fighter.training={{"origin:languages",{"elvish","orc"}}};CampaignParty growing(module());const auto f=growing.add_pc(hero(fighter));
    growing.award_experience(2700,"training-xp");for(unsigned level=2;level<=3;++level)growing.advance(f,growing.default_advancement(f));
    auto choice=growing.default_advancement(f);choice.abilities={};choice.abilities[1]=2;growing.advance(f,choice);
    check(skill(growing.member(f).character.sheet(),"stealth").bonus==6,"Level-up rebuilds skill totals after an ability modifier changes");
}
void complete_saved_training(){
    auto creation=srd5::character_rules();auto rules=module();
    auto loaded=decode_campaign(fixture("campaign-v8-training.ogs"),*creation,*rules,"training-fixture",nullptr);
    CampaignParty party(module());party.restore(std::move(loaded.party));
    party.set_wealth(3,{0,0,0,37,0,0,2});
    por::Equipment sword;sword.stored.type=36;sword.stored.stack_size=1;sword.stored.value=10;
    party.purchase(3,sword);party.equip(3,1);party.set_grip(3,2);
    auto state=party.checkpoint();state.time_minutes=100;state.subminute_milliseconds=1234;state.random_state=918273;
    state.roster[2].last_rest_minutes=50;state.roster[2].last_rest_subminute_milliseconds=789;
    state.roster[2].vitals.resources="SRD3 1 0 0 0 0 0 FX1 2 1 1 1 77 99 \"Source caster\" 13 43000 2000";
    state.roster[2].vitals.description="Blinded";
    party.restore(state);party.remove(2); // Pending reserve members can also complete their choices.
    const auto original=encode_campaign(party,nullptr,"training-fixture");
    auto invalid=choices();invalid["class:rogue:expertise"]={"arcana","stealth"};
    rejects([&]{party.complete_training(1,*creation,invalid);});
    rejects([&]{party.complete_training(1,*creation,{});});
    rejects([&]{party.complete_training(99,*creation,choices());});
    check(encode_campaign(party,nullptr,"training-fixture")==original,"Invalid and incomplete confirmation leaves every campaign field unchanged");
    party.begin_combat();
    rejects([&]{(void)party.preview_training(1,*creation,choices());});
    rejects([&]{party.complete_training(1,*creation,choices());});party.end_combat();
    check(encode_campaign(party,nullptr,"training-fixture")==original,"Combat rejects training preview and confirmation without changing state");
    for(MemberId id=1;id<=4;++id){
        auto selected=id<=2?choices():TrainingChoices{{"origin:languages",{"elvish","orc"}}};
        if(id==2)selected["class:rogue:expertise"]={"investigation","perception"};
        const auto before=encode_campaign(party,nullptr,"training-fixture");const auto previous=party.member(id);
        const auto preview=party.preview_training(id,*creation,selected);
        check(encode_campaign(party,nullptr,"training-fixture")==before,"Opening, editing and discarding a preview has no campaign effects");
        const auto& character=preview.character;const auto& old=previous.character;
        check(character.sheet().training.complete&&character.creation_data().training==selected,"Preview includes the requested complete training");
        check(character.sheet().level==old.sheet().level&&character.sheet().scores==old.sheet().scores&&
            character.sheet().hit_point_modifiers==old.sheet().hit_point_modifiers&&character.sheet().hit_points==old.sheet().hit_points&&
            character.sheet().prepared_spells==old.sheet().prepared_spells&&character.advancements()==old.advancements(),
            "Reconstruction preserves attained levels, ASI, Constitution history, feat and spell choices");
        check(character.appearance()==old.appearance()&&std::equal(character.inventory().items().begin(),character.inventory().items().end(),old.inventory().items().begin(),old.inventory().items().end()),
            "Preview preserves appearance and full item records");
        for(const auto& grant:old.sheet().grants)check(std::find(character.sheet().grants.begin(),character.sheet().grants.end(),grant)!=character.sheet().grants.end(),"Existing grants retain exact provenance");
        check(preview.vitals==previous.vitals,"Preview preserves wounds, effects, timers, death state and spent resources exactly");
        auto expected=party.checkpoint();expected.roster[id-1]=preview;CampaignParty comparison(module());comparison.restore(expected);
        party.complete_training(id,*creation,selected);
        check(encode_campaign(party,nullptr,"training-fixture")==encode_campaign(comparison,nullptr,"training-fixture"),
            "Confirmation changes only the selected character, exactly as previewed; gear, wealth, clock, RNG and roster are unchanged");
    }
    check(skill(party.member(1).character.sheet(),"stealth").bonus==7,"Confirmed Criminal Expertise changes the check bonus");
    const auto completed=encode_campaign(party,nullptr,"training-fixture");
    rejects([&]{party.complete_training(1,*creation,choices());});
    rejects([&]{party.complete_training(3,*creation,{{"origin:languages",{"dwarvish","orc"}}});});
    check(encode_campaign(party,nullptr,"training-fixture")==completed,"Completed training cannot be reopened as a respec");
    auto reloaded=decode_campaign(completed,*creation,*rules,"training-fixture",nullptr);CampaignParty restored(module());restored.restore(std::move(reloaded.party));
    check(encode_campaign(restored,nullptr,"training-fixture")==completed,"Completed training and unchanged campaign resources survive exact save/reload");
    auto members=restored.participants();members.push_back({99,"vanguard","Enemy",1,{6,6}});
    auto combat=rules->create({{8,8,std::vector<std::uint8_t>(64)},members},42);
    check(rules->restore(combat->save())->save()==combat->save(),"Completed training is accepted in the next combat and its checkpoint");

    auto partial=draft();partial.training={{"origin:languages",{"elvish"}},{"class:rogue",{"perception"}}};
    CampaignParty incomplete(module());const auto id=incomplete.add_pc(hero(partial));
    const auto pending=encode_campaign(incomplete,nullptr,"partial");auto replacement=choices();replacement["origin:languages"]={"dwarvish","orc"};
    rejects([&]{incomplete.complete_training(id,*creation,replacement);});
    check(encode_campaign(incomplete,nullptr,"partial")==pending,"Already chosen entries cannot be replaced while other groups are pending");
    auto unconscious=incomplete.checkpoint();unconscious.roster[0].vitals={0,false,"SRD1 0 0 1 2 0"};incomplete.restore(unconscious);
    incomplete.complete_training(id,*creation,choices());
    check(incomplete.member(id).vitals==unconscious.roster[0].vitals,"Training completion does not stabilize or heal an unconscious character");
}
}
int main(){try{creation_controls();preset_training();grants_and_checks();invalid_choices();persistence();complete_saved_training();std::cout<<"Training grant and creation tests passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
