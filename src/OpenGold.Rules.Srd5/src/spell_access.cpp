#include "spell_access.h"
#include <algorithm>
#include <array>
#include <set>
#include <stdexcept>
namespace opengold::srd5::detail {
using namespace rules;
namespace {
constexpr std::string_view source="class:wizard:spellcasting";
struct Spell {std::string_view id,label;unsigned level,mask;};
// Existing spell implementations only. This is not the complete Wizard list.
constexpr std::array spells{
    Spell{"sacred_flame","Sacred Flame",0,128},Spell{"fire_bolt","Fire Bolt",0,1},Spell{"poison_spray","Poison Spray",0,64},Spell{"magic_missile","Magic Missile",1,4},
    Spell{"scorching_ray","Scorching Ray",2,16},Spell{"blindness","Blindness",2,32}};
void require(bool ok){if(!ok)throw std::runtime_error("Invalid spell grant, spellbook entry or preparation");}
const Spell& find(std::string_view id){
    for(const auto& spell:spells)if(spell.id==id)return spell;
    throw std::runtime_error("Unsupported spell knowledge");
}
FeatureGrant grant(std::string_view id,unsigned level,std::string_view origin=source){return {"spell:"+std::string(id),std::string(origin),level,{{"access",find(id).level?"spellbook":"cantrip"}}};}
}
bool is_spell_grant(const FeatureGrant& grant){return grant.id.starts_with("spell:");}
std::vector<FeatureGrant> without_spell_grants(std::span<const FeatureGrant> grants){
    std::vector<FeatureGrant> result;for(const auto& g:grants)if(!is_spell_grant(g))result.push_back(g);return result;
}
TrainingChoiceGroup starting_cantrip_options(std::string_view klass){
    if(klass=="cleric")return {"class:cleric:spellcasting","Cleric cantrips",3,
        {{"sacred_flame","Sacred Flame","Dexterity save: 1d8 Radiant damage, visible creature within 60 feet."}}};
    if(klass!="wizard")return {};
    return {std::string(source),"Wizard cantrips",3,
        {{"fire_bolt","Fire Bolt","Ranged spell attack: 1d10 Fire damage, 120 feet."},
         {"poison_spray","Poison Spray","Ranged spell attack: 1d12 Poison damage, 30 feet."}}};
}
std::vector<FeatureGrant> starting_spell_grants(std::string_view klass,const std::optional<std::vector<std::string>>& cantrips){
    if(klass=="cleric"){
        std::vector<FeatureGrant> result;std::set<std::string> unique;
        for(const auto& id:cantrips.value_or(std::vector<std::string>{})){
            require(id=="sacred_flame"&&unique.insert(id).second);result.push_back(grant(id,1,"class:cleric:spellcasting"));
        }return result;
    }
    if(klass!="wizard"){require(!cantrips||cantrips->empty());return {};}
    const auto chosen=cantrips.value_or(std::vector<std::string>{"fire_bolt"});
    require(chosen.size()<=3);std::set<std::string> unique;std::vector<FeatureGrant> result;
    for(const auto& id:chosen){require((id=="fire_bolt"||id=="poison_spray")&&unique.insert(id).second);result.push_back(grant(id,1));}
    result.push_back(grant("magic_missile",1));return result;
}
SpellAccess spell_access(std::span<const FeatureGrant> grants,std::string_view klass,unsigned level,std::span<const std::string> prepared){
    SpellAccess result;
    if(klass=="Cleric"){
        require(level>=1&&level<=4);
        require(std::find(grants.begin(),grants.end(),FeatureGrant{"feature:spellcasting","class:cleric",1,{}})!=grants.end());
        result.cantrip_choices=level==4?4:3;std::set<std::string> known;
        for(const auto& g:grants)if(is_spell_grant(g)){
            require(g.id=="spell:sacred_flame"&&g==grant("sacred_flame",1,"class:cleric:spellcasting")&&known.insert(g.id).second);
            result.cantrips.push_back({"sacred_flame","Sacred Flame",g.source_id,g.level});
        }return result; // Leveled preparation and Divine Order remain their existing increments.
    }
    if(klass!="Wizard"){
        require(std::none_of(grants.begin(),grants.end(),is_spell_grant));
        return result; // Each other class's preparation policy has its own issue.
    }
    require(level>=1&&level<=4);
    require(std::find(grants.begin(),grants.end(),FeatureGrant{"feature:spellcasting","class:wizard",1,{}})!=grants.end());
    result.cantrip_choices=level==4?4:3;result.spellbook_choices=6+2*(level-1);result.prepared_choices=level+3;
    std::set<std::string> known;std::array<unsigned,5> books{},cantrips{};
    for(const auto& g:grants)if(is_spell_grant(g)){
        require(g.source_id==source&&g.level>=1&&g.level<=level&&known.insert(g.id).second);
        const auto& spell=find(std::string_view(g.id).substr(6));require(spell.id!="sacred_flame");
        require(g==grant(spell.id,g.level));
        require(spell.level==0||spell.level<=(g.level>=3?2u:1u));
        auto& list=spell.level?result.spellbook:result.cantrips;
        list.push_back({std::string(spell.id),std::string(spell.label),g.source_id,g.level});
        if(spell.level)++books[g.level];else ++cantrips[g.level];
    }
    require(books[1]<=6&&cantrips[1]<=3&&cantrips[2]==0&&cantrips[3]==0&&cantrips[4]<=1);
    for(unsigned n=2;n<=level;++n)require(books[n]<=2);
    // Book selection controls remain separate; cantrips are now explicit.
    require(std::find(grants.begin(),grants.end(),grant("magic_missile",1))!=grants.end());
    std::set<std::string> selected;
    for(const auto& id:prepared){
        require(selected.insert(id).second&&std::any_of(result.spellbook.begin(),result.spellbook.end(),[&](const auto& s){return s.id==id;}));
        result.prepared.push_back(id);
    }
    require(result.prepared.size()<=result.prepared_choices);return result;
}
void learn_advancement_spells(CharacterSheet& sheet,std::span<const std::string> selected){
    if(sheet.character_class!="Wizard")return;
    auto next=sheet.grants;
    for(const auto& id:selected){require(find(id).level!=0);
        if(std::none_of(next.begin(),next.end(),[&](const auto& g){return g.id=="spell:"+id;}))next.push_back(grant(id,sheet.level));}
    (void)spell_access(next,sheet.character_class,sheet.level,selected);sheet.grants=std::move(next);
}
unsigned known_cantrip_mask(const SpellAccess& access){
    unsigned result=0;for(const auto& s:access.cantrips)result|=find(s.id).mask;return result;
}
unsigned wizard_casting_mask(const SpellAccess& access){
    unsigned result=known_cantrip_mask(access);
    for(const auto& id:access.prepared)result|=find(id).mask;return result;
}
}
