#include "opengold/campaign_save.h"
#include "opengold/srd5.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace opengold;
using namespace opengold::rules;
namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"Invalid choice must reject");}
auto module(){return srd5::load(std::filesystem::path(OPENGOLD_SOURCE_DIR)/"data/rules/srd-5.2.1/combat.rules");}
Character character(std::string klass){CharacterDraft d;d.race="human";d.gender="female";d.character_class=klass;d.alignment="neutral_good";d.background="sage";d.name="Advancement "+klass;d.rolled=true;for(auto& r:d.rolls)r={{6,5,4,1},3};return Character(*srd5::character_rules(),d,{});}
std::string saved(const CampaignParty& p){return encode_campaign(p,nullptr,"advancement-fixture");}
bool offers(const CombatSession& s,std::string_view verb){const auto commands=s.legal_commands();return std::any_of(commands.begin(),commands.end(),[&](const auto& c){return c.verb==verb;});}
Command command(const CombatSession& s,std::string_view verb){for(const auto& c:s.legal_commands())if(c.verb==verb)return c;throw std::runtime_error("Missing command: "+std::string(verb));}
CombatantView unit(const CombatSession& s,EntityId id){for(const auto& c:s.snapshot().combatants)if(c.id==id)return c;throw std::runtime_error("Missing actor");}
auto duel(const RulesModule& rules,const CampaignParty& party){
    auto participants=party.participants();participants[0].cell={1,1};participants.push_back({99,"vanguard","Target",1,{8,1}});
    Encounter encounter{{30,9,std::vector<std::uint8_t>(270)},participants};
    for(unsigned seed=0;seed<100;++seed){auto combat=rules.create(encounter,seed);if(combat->snapshot().actor==participants[0].id)return combat;}
    throw std::runtime_error("No first initiative seed");
}
void progression(){
    for(const char* klass:{"fighter","cleric","wizard"}){
        CampaignParty party(module());const auto id=party.add_pc(character(klass));
        const auto level1=saved(party);rejects([&]{party.advance(id,{});});check(saved(party)==level1,"Insufficient XP is atomic");
        party.award_experience(2700,"fourth-level");check(party.member(id).character.sheet().level==1,"XP does not auto-advance");
        auto wounded=party.checkpoint();wounded.roster[0].vitals.hit_points-=3;
        wounded.roster[0].vitals.resources=std::string_view(klass)=="fighter"?"SRD1 1 0 0 0 0":"SRD1 0 1 0 0 0";party.restore(std::move(wounded));
        party.begin_combat();check(!party.can_advance(id),"Advancement unavailable during combat");rejects([&]{party.advance(id,{});});party.end_combat();
        for(unsigned level=2;level<=4;++level){
            const auto before=saved(party);const auto old=party.member(id);auto choice=party.default_advancement(id);
            if(level==4){choice.abilities={};choice.abilities[2]=2;}
            auto bad=choice;bad.spells.push_back("unimplemented");rejects([&]{party.advance(id,bad);});check(saved(party)==before,"Rejected spell cannot change state or choices");
            if(level==4){bad=choice;bad.abilities[2]=1;rejects([&]{party.advance(id,bad);});check(saved(party)==before,"Incomplete ability allocation is atomic");}
            const auto preview=party.preview_advancement(id,choice);check(saved(party)==before,"Preview is entirely read only");
            party.advance(id,choice);const auto& now=party.member(id);
            check(now.character.sheet().level==level&&now.vitals==preview.vitals,"Confirmation matches HP and resource preview");
            check(now.character.sheet().hit_points-now.vitals.hit_points==3,"Level-up preserves pre-existing HP deficit");
            check(now.character.advancements().size()==level-1,"Each confirmed choice is retained");
            if(level==4){check(now.character.sheet().modifiers[2]==old.character.sheet().modifiers[2]+1,"Constitution ability points update modifier");
                const int expected=old.character.sheet().hit_die/2+1+old.character.sheet().modifiers[2]+4;
                check(now.character.sheet().hit_points-old.character.sheet().hit_points==expected,"Constitution growth applies retroactively to all four levels");}
            const auto bytes=saved(party);auto loaded=decode_campaign(bytes,*srd5::character_rules(),*module(),"advancement-fixture",nullptr);CampaignParty restored(module());restored.restore(std::move(loaded.party));
            check(saved(restored)==bytes,"Level and choice history survive full save reconstruction");
        }
        check(!party.can_advance(id),"Supported progression stops at level four");
        check(party.member(id).vitals.resources==(std::string_view(klass)=="fighter"?"SRD1 2 0 0 0 0":"SRD2 0 3 3 0 0 0"),"Only new resource capacity is added across three advancements");
    }
    CampaignParty capped(module());auto high=character("fighter").creation_data();for(auto& roll:high.rolls)roll={{6,6,6,1},3};const auto cap=capped.add_pc(Character(*srd5::character_rules(),high,{}));capped.award_experience(2700,"cap");
    for(unsigned level=2;level<=3;++level)capped.advance(cap,capped.default_advancement(cap));
    auto invalid=capped.default_advancement(cap);invalid.abilities={};invalid.abilities[2]=2;const auto unchanged=saved(capped);
    rejects([&]{capped.advance(cap,invalid);});check(saved(capped)==unchanged,"Ability cap rejects without partially applying level or HP");
    CampaignParty unsupported(module());const auto id=unsupported.add_pc(character("bard"));unsupported.award_experience(2700,"xp");check(!unsupported.can_advance(id),"Unsupported classes cannot select partial advancement");
}
void feats(){
    for(const char* feat:{"defense","savage_attacker"}){
        CampaignParty party(module());const auto id=party.add_pc(character("fighter"));party.award_experience(2700,"xp");
        for(unsigned level=2;level<=3;++level)party.advance(id,party.default_advancement(id));
        party.set_wealth(id,{0,0,0,100,0,0,0});for(unsigned type:{36,55}){por::Equipment e;e.stored.type=type;e.stored.stack_size=1;e.stored.value=1;party.purchase(id,e);party.equip(id,party.member(id).character.inventory().items().back().id);}
        const auto ac=party.profile(id).armor_class;auto choice=party.default_advancement(id);choice.feat=feat;choice.abilities={};party.advance(id,choice);
        if(std::string_view(feat)=="defense"){check(party.profile(id).armor_class==ac+1,"Defense adds AC in armor");party.unequip(id,2);check(party.profile(id).armor_class==10+party.member(id).character.sheet().modifiers[1],"Defense does not grant unarmored AC");}
        else {auto rules=module();auto participants=party.participants();participants[0].cell={1,1};participants.push_back({99,"vanguard","Target",1,{2,1}});Encounter e{{12,9,std::vector<std::uint8_t>(108)},participants};bool hit=false;
            for(unsigned seed=0;seed<100&&!hit;++seed){auto combat=rules->create(e,seed);if(combat->snapshot().actor!=id)continue;combat->submit(command(*combat,"melee"));for(const auto& log:combat->snapshot().log)hit|=log.find("Savage Attacker")!=std::string::npos;
                if(hit)check(rules->restore(combat->save())->save()==combat->save(),"Spent Savage Attacker state survives checkpoint");}
            check(hit,"Selected Savage Attacker modifies actual weapon damage");}
    }
}
void spells(){
    for(const char* klass:{"wizard","cleric"}){
        CampaignParty party(module());const auto id=party.add_pc(character(klass));party.award_experience(2700,"xp");
        for(unsigned level=2;level<=4;++level){auto choice=party.default_advancement(id);
            if(std::string_view(klass)=="cleric")choice.spells={"cure_wounds","healing_word"};else if(level>=3)choice.spells={"magic_missile","scorching_ray"};
            party.advance(id,choice);}
        auto state=party.checkpoint();state.roster[0].vitals.hit_points-=10;party.restore(std::move(state));
        auto rules=module();auto combat=duel(*rules,party);
        if(std::string_view(klass)=="wizard"){
            check(offers(*combat,"scorching_ray")&&offers(*combat,"magic_missile_2"),"Prepared wizard has second-level actions");
            combat->submit(command(*combat,"scorching_ray"));check(unit(*combat,id).persistent.resources.starts_with("SRD2 0 4 2 "),"Scorching Ray spends exactly one level-two slot");
            const auto bytes=combat->save();auto restored=rules->restore(bytes);check(restored->save()==bytes,"New spell resources persist exactly");
        }else{
            check(offers(*combat,"healing_word")&&offers(*combat,"healing_word_2"),"Cleric can select either healing slot level");
            const auto hp=unit(*combat,id).hit_points;combat->submit(command(*combat,"healing_word_2"));
            check(unit(*combat,id).hit_points>hp&&unit(*combat,id).action,"Healing Word heals and preserves action");
            check(!offers(*combat,"cure_wounds")&&!offers(*combat,"healing_word"),"A second spell slot cannot be spent in the same turn");
            const auto bytes=combat->save();auto restored=rules->restore(bytes);check(restored->save()==bytes&&!offers(*restored,"cure_wounds"),"Slot-per-turn restriction survives checkpoint");
        }
        party.begin_combat();party.apply_combat(combat->snapshot());party.end_combat();const auto bytes=saved(party);auto loaded=decode_campaign(bytes,*srd5::character_rules(),*rules,"advancement-fixture",nullptr);CampaignParty restored(module());restored.restore(std::move(loaded.party));check(saved(restored)==bytes,"Spent second-level slots survive campaign reload");
    }
}
}
int main(){try{progression();feats();spells();std::cout<<"Manual advancement tests passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
