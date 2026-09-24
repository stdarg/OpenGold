#ifndef OPENGOLD_SRD5_SPELL_COMPONENTS_H
#define OPENGOLD_SRD5_SPELL_COMPONENTS_H
#include <array>
#include <string_view>
namespace opengold::srd5::detail {
struct SpellComponents {std::string_view id;bool verbal{},somatic{};};
// SRD 5.2.1 spell descriptions. These six spells have no Material component.
// Component substitution and speech-blocking sources are separate increments.
inline constexpr std::array spell_component_definitions{
    SpellComponents{"fire_bolt",true,true},SpellComponents{"cure_wounds",true,true},
    SpellComponents{"magic_missile",true,true},SpellComponents{"healing_word",true,false},
    SpellComponents{"scorching_ray",true,true},SpellComponents{"blindness",true,false}};
inline const SpellComponents* spell_components(std::string_view command){
    if(command.ends_with("_2"))command.remove_suffix(2);
    for(const auto& spell:spell_component_definitions)if(spell.id==command)return &spell;
    return nullptr;
}
}
#endif
