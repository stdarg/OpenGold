#include "opengold/srd5.h"
#include <algorithm>
#include <cstdlib>
#include <set>
#include <stdexcept>

namespace opengold::srd5 {
using namespace rules;
namespace {
const std::array<std::string,6> ability_names{"STR","DEX","CON","INT","WIS","CHA"};
struct Class { const char* id; const char* name; int die; const char* description; };
constexpr std::array<Class,12> classes{{
    {"barbarian","Barbarian",12,"A fierce warrior. Primary ability: Strength. Hit Die: d12."},
    {"bard","Bard",8,"An inspiring performer and spellcaster. Primary ability: Charisma. Hit Die: d8."},
    {"cleric","Cleric",8,"A wielder of divine magic. Primary ability: Wisdom. Hit Die: d8."},
    {"druid","Druid",8,"A guardian of nature. Primary ability: Wisdom. Hit Die: d8."},
    {"fighter","Fighter",10,"A master of weapons. Primary ability: Strength or Dexterity. Hit Die: d10."},
    {"monk","Monk",8,"A disciplined martial artist. Primary abilities: Dexterity and Wisdom. Hit Die: d8."},
    {"paladin","Paladin",10,"An oathbound warrior. Primary abilities: Strength and Charisma. Hit Die: d10."},
    {"ranger","Ranger",10,"A hunter and wilderness explorer. Primary abilities: Dexterity and Wisdom. Hit Die: d10."},
    {"rogue","Rogue",8,"An expert in stealth and precision. Primary ability: Dexterity. Hit Die: d8."},
    {"sorcerer","Sorcerer",6,"A spellcaster with innate magic. Primary ability: Charisma. Hit Die: d6."},
    {"warlock","Warlock",8,"A spellcaster empowered by a pact. Primary ability: Charisma. Hit Die: d8."},
    {"wizard","Wizard",6,"A scholar of arcane magic. Primary ability: Intelligence. Hit Die: d6."}
}};
class CreatorRules final : public CharacterRules {
public:
    Identity identity() const override {return {"srd5","5.2.1","character-creation.1"};}
    std::vector<CreationChoice> choices(CreationField field) const override;
    std::vector<ScoreAdjustment> adjustments(std::string_view background) const override;
    std::array<AbilityRoll,6> roll(std::uint64_t& state) const override;
    CharacterSheet evaluate(const CharacterDraft& draft,bool require_name) const override;
};
std::vector<CreationChoice> CreatorRules::choices(CreationField field) const
{
    switch(field) {
    case CreationField::race:return {
        {"dragonborn","Dragonborn","A draconic humanoid."}, {"dwarf","Dwarf","A hardy folk. Dwarven Toughness adds 1 to starting HP."},
        {"elf","Elf","A folk with a connection to the Fey."}, {"gnome","Gnome","A small, inventive folk."},
        {"goliath","Goliath","A folk with giant ancestry."}, {"halfling","Halfling","A small, resourceful folk."},
        {"human","Human","A versatile and adaptable folk."}, {"orc","Orc","A resilient, determined folk."},
        {"tiefling","Tiefling","A folk with a fiendish legacy."}};
    case CreationField::gender:return {{"female","Female","Gender does not change attributes or restrict class or appearance choices."},
        {"male","Male","Gender does not change attributes or restrict class or appearance choices."},
        {"nonbinary","Nonbinary","Gender does not change attributes or restrict class or appearance choices."}};
    case CreationField::character_class:{std::vector<CreationChoice> result;
        for(const auto& c:classes)result.push_back({c.id,c.name,c.description});return result;}
    case CreationField::alignment:return {
        {"lawful_good","Lawful Good","Guided by compassion, duty, and order."},
        {"neutral_good","Neutral Good","Seeks to help others, with flexibility about rules."},
        {"chaotic_good","Chaotic Good","Values kindness and individual freedom."},
        {"lawful_neutral","Lawful Neutral","Guided by rules, tradition, or a personal code."},
        {"neutral","Neutral","Balances competing principles or acts according to circumstances."},
        {"chaotic_neutral","Chaotic Neutral","Values personal freedom and independence."},
        {"lawful_evil","Lawful Evil","Pursues selfish ends through structure and rules."},
        {"neutral_evil","Neutral Evil","Pursues selfish ends without loyalty to order or freedom."},
        {"chaotic_evil","Chaotic Evil","Acts destructively with little regard for rules or others."}};
    case CreationField::background:return {
        {"acolyte","Acolyte","Attribute bonuses: Intelligence, Wisdom, Charisma."},
        {"criminal","Criminal","Attribute bonuses: Dexterity, Constitution, Intelligence."},
        {"sage","Sage","Attribute bonuses: Constitution, Intelligence, Wisdom."},
        {"soldier","Soldier","Attribute bonuses: Strength, Dexterity, Constitution."}};
    }
    throw std::runtime_error("Unknown creation field");
}
std::vector<ScoreAdjustment> CreatorRules::adjustments(std::string_view background) const
{
    std::array<unsigned,3> allowed;
    if(background=="acolyte")allowed={3,4,5};
    else if(background=="criminal")allowed={1,2,3};
    else if(background=="sage")allowed={2,3,4};
    else if(background=="soldier")allowed={0,1,2};
    else throw std::runtime_error("Unknown background");
    std::vector<ScoreAdjustment> result;
    for(auto first:allowed)for(auto second:allowed)if(first!=second) {
        ScoreAdjustment a;a.label=ability_names[first]+" +2, "+ability_names[second]+" +1";
        a.bonuses[first]=2;a.bonuses[second]=1;result.push_back(a);
    }
    ScoreAdjustment a;a.label="+1 to "+ability_names[allowed[0]]+", "+ability_names[allowed[1]]+", "+ability_names[allowed[2]];
    for(auto n:allowed)a.bonuses[n]=1;result.push_back(a);return result;
}
std::array<AbilityRoll,6> CreatorRules::roll(std::uint64_t& state) const
{
    const auto die=[&]() {
        // SplitMix64 and rejection sampling, stable across platforms.
        std::uint64_t z;
        do {z=(state+=0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;
            z=(z^(z>>27))*0x94d049bb133111ebULL;z^=z>>31;} while(z<4);
        return static_cast<int>(z%6)+1;
    };
    std::array<AbilityRoll,6> result;
    for(auto& r:result) {for(auto& n:r.dice)n=die();r.discarded=static_cast<unsigned>(std::min_element(r.dice.begin(),r.dice.end())-r.dice.begin());}
    return result;
}
CharacterSheet CreatorRules::evaluate(const CharacterDraft& d,bool require_name) const
{
    const auto label=[&](CreationField field,const std::string& id) {
        const auto options=choices(field);
        const auto found=std::find_if(options.begin(),options.end(),[&](const auto& c){return c.id==id;});
        if(found==options.end())throw std::runtime_error("Invalid character selection: "+id);
        return found->label;
    };
    CharacterSheet s;s.identity=identity();s.name=d.name;
    s.race=label(CreationField::race,d.race);s.gender=label(CreationField::gender,d.gender);
    s.character_class=label(CreationField::character_class,d.character_class);
    s.alignment=label(CreationField::alignment,d.alignment);s.background=label(CreationField::background,d.background);
    if(require_name && (d.name.empty()||d.name.size()>160 || d.name.find_first_not_of(" \t\r\n")==std::string::npos ||
        std::any_of(d.name.begin(),d.name.end(),[](unsigned char c){return c<32||c==127;})))
        throw std::runtime_error("Enter a name before finishing your character.");
    if(!d.rolled)throw std::runtime_error("Roll your six attribute scores first.");
    const auto options=adjustments(d.background);
    if(d.adjustment>=options.size())throw std::runtime_error("Invalid background bonuses");
    s.bonuses=options[d.adjustment].bonuses;
    std::set<unsigned> used;
    for(unsigned i=0;i<6;++i) {
        if(d.assignment[i]>=6||!used.insert(d.assignment[i]).second)throw std::runtime_error("Each roll must be assigned exactly once");
        s.base[i]=d.rolls[d.assignment[i]].total();s.scores[i]=s.base[i]+s.bonuses[i];
        if(s.scores[i]>20)throw std::runtime_error("Background bonuses cannot raise a score above 20");
        s.modifiers[i]=ability_modifier(s.scores[i]);
    }
    const auto c=std::find_if(classes.begin(),classes.end(),[&](const auto& c){return c.id==d.character_class;});
    s.hit_die=c->die;const int racial_hp=d.race=="dwarf"?1:0;
    s.hit_points=s.hit_die+s.modifiers[2]+racial_hp;
    s.hp_explanation=std::to_string(s.hit_die)+" (maximum d"+std::to_string(s.hit_die)+") "+
        (s.modifiers[2]<0?"- ":"+ ")+std::to_string(std::abs(s.modifiers[2]))+" (Constitution)"+
        (racial_hp?" + 1 (Dwarven Toughness)":"")+" = "+std::to_string(s.hit_points)+" HP";
    return s;
}
}
std::unique_ptr<rules::CharacterRules> character_rules(){return std::make_unique<CreatorRules>();}
}
