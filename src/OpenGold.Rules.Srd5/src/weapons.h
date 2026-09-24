#ifndef OPENGOLD_SRD5_WEAPONS_H
#define OPENGOLD_SRD5_WEAPONS_H
#include "damage.h"
#include <array>
#include <string_view>
namespace opengold::srd5::detail {
// SRD 5.2.1 p.91. Original names are converted by the game adapter.
struct Weapon {
    std::string_view key;
    int dice,sides;
    unsigned hands{1};
    bool martial{},finesse{},ranged{};
    int range{},long_range{},reach{5};
    bool light{};
    int versatile_sides{};
    DamageType type{DamageType::bludgeoning};
};
inline constexpr std::array weapons{
    Weapon{.key="club",.dice=1,.sides=4,.light=true},
    Weapon{.key="dagger",.dice=1,.sides=4,.finesse=true,.range=20,.long_range=60,.light=true,.type=DamageType::piercing},
    Weapon{.key="handaxe",.dice=1,.sides=6,.range=20,.long_range=60,.light=true,.type=DamageType::slashing},
    Weapon{.key="javelin",.dice=1,.sides=6,.range=30,.long_range=120,.type=DamageType::piercing},
    Weapon{.key="light_hammer",.dice=1,.sides=4,.range=20,.long_range=60,.light=true},
    Weapon{"mace",1,6},
    Weapon{.key="quarterstaff",.dice=1,.sides=6,.versatile_sides=8},
    Weapon{.key="spear",.dice=1,.sides=6,.range=20,.long_range=60,.versatile_sides=8,.type=DamageType::piercing},
    Weapon{.key="dart",.dice=1,.sides=4,.finesse=true,.ranged=true,.range=20,.long_range=60,.type=DamageType::piercing},
    Weapon{.key="light_crossbow",.dice=1,.sides=8,.hands=2,.ranged=true,.range=80,.long_range=320,.type=DamageType::piercing},
    Weapon{.key="shortbow",.dice=1,.sides=6,.hands=2,.ranged=true,.range=80,.long_range=320,.type=DamageType::piercing},
    Weapon{"sling",1,4,1,false,false,true,30,120},
    Weapon{.key="battleaxe",.dice=1,.sides=8,.martial=true,.versatile_sides=10,.type=DamageType::slashing}, Weapon{"flail",1,8,1,true},
    Weapon{.key="glaive",.dice=1,.sides=10,.hands=2,.martial=true,.reach=10,.type=DamageType::slashing},
    Weapon{.key="greatsword",.dice=2,.sides=6,.hands=2,.martial=true,.type=DamageType::slashing},
    Weapon{.key="halberd",.dice=1,.sides=10,.hands=2,.martial=true,.reach=10,.type=DamageType::slashing},
    Weapon{.key="longsword",.dice=1,.sides=8,.martial=true,.versatile_sides=10,.type=DamageType::slashing}, Weapon{.key="morningstar",.dice=1,.sides=8,.martial=true,.type=DamageType::piercing},
    Weapon{.key="pike",.dice=1,.sides=10,.hands=2,.martial=true,.reach=10,.type=DamageType::piercing},
    Weapon{.key="scimitar",.dice=1,.sides=6,.martial=true,.finesse=true,.light=true,.type=DamageType::slashing},
    Weapon{.key="shortsword",.dice=1,.sides=6,.martial=true,.finesse=true,.light=true,.type=DamageType::piercing},
    Weapon{.key="trident",.dice=1,.sides=8,.martial=true,.range=20,.long_range=60,.versatile_sides=10,.type=DamageType::piercing},
    Weapon{.key="warhammer",.dice=1,.sides=8,.martial=true,.versatile_sides=10},
    Weapon{.key="war_pick",.dice=1,.sides=8,.martial=true,.versatile_sides=10,.type=DamageType::piercing},
    Weapon{.key="longbow",.dice=1,.sides=8,.hands=2,.martial=true,.ranged=true,.range=150,.long_range=600,.type=DamageType::piercing},
    // A plain wand is a held focus, not a free spell or invented damage profile.
    Weapon{"wand",0,0}
};
inline const Weapon* weapon(std::string_view key)
{
    for(const auto& value:weapons)if(value.key==key)return &value;
    return nullptr;
}
}
#endif
