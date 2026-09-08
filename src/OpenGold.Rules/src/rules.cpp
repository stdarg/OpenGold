#include "opengold/rules.h"
#include <stdexcept>
namespace opengold::rules {
CharacterProfile RulesModule::character_profile(const CharacterSheet&, std::span<const std::string>) const
{ throw std::runtime_error("This rules module does not support campaign characters"); }
unsigned RulesModule::experience_for_level(unsigned) const
{ throw std::runtime_error("This rules module does not support advancement"); }
bool RulesModule::advance_character(CharacterSheet&, VitalState&) const
{ throw std::runtime_error("This rules module does not support advancement"); }
void RulesModule::recover(VitalState&, const CharacterSheet&) const
{ throw std::runtime_error("This rules module does not support recovery"); }
RestPolicy RulesModule::long_rest_policy() const
{ throw std::runtime_error("This rules module does not support recovery"); }
void RulesModule::temple_heal(VitalState&, const CharacterSheet&, std::uint64_t&) const
{ throw std::runtime_error("This rules module does not support temple healing"); }
bool Battlefield::contains(Cell p) const noexcept
{ return p.x>=0 && p.y>=0 && p.x<width && p.y<height; }
unsigned Battlefield::at(Cell p) const noexcept
{
    if(!contains(p))return 1;
    const auto index=static_cast<std::size_t>(p.y)*static_cast<std::size_t>(width)+static_cast<std::size_t>(p.x);
    return index<terrain.size()?terrain[index]:1;
}
}
