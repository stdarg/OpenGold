#ifndef OPENGOLDBOX_CHARACTER_COLORS_H
#define OPENGOLDBOX_CHARACTER_COLORS_H
#include "localization.h"
#include "opengold/character_art.h"
#include <godot_cpp/variant/color.hpp>

namespace presentation {
inline constexpr std::array<const char*,16> character_colors{
    N_("Black"),N_("Blue"),N_("Green"),N_("Cyan"),N_("Red"),N_("Magenta"),N_("Brown"),N_("Light gray"),
    N_("Dark gray"),N_("Light blue"),N_("Light green"),N_("Light cyan"),N_("Light red"),N_("Pink"),N_("Yellow"),N_("White")};
inline constexpr std::array<const char*,6> character_regions{
    N_("Weapon"),N_("Body"),N_("Hair / Face"),N_("Shield"),N_("Arms"),N_("Legs")};
inline godot::Color character_color(unsigned index)
{
    const auto c=opengold::por::character_color(index);
    return {c[0]/255.f,c[1]/255.f,c[2]/255.f};
}
}
#endif
