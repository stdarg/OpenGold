#ifndef OPENGOLD_SRD5_WEAPONS_H
#define OPENGOLD_SRD5_WEAPONS_H
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
};
inline constexpr std::array weapons{
    Weapon{.key="club",.dice=1,.sides=4,.light=true},
    Weapon{.key="dagger",.dice=1,.sides=4,.finesse=true,.range=20,.long_range=60,.light=true},
    Weapon{.key="handaxe",.dice=1,.sides=6,.range=20,.long_range=60,.light=true},
    Weapon{"javelin",1,6,1,false,false,false,30,120},
    Weapon{.key="light_hammer",.dice=1,.sides=4,.range=20,.long_range=60,.light=true},
    Weapon{"mace",1,6},
    // OpenGoldBox quarterstaffs require both hands.
    Weapon{"quarterstaff",1,6,2},
    // OpenGoldBox spears require both hands.
    Weapon{"spear",1,6,2,false,false,false,20,60},
    Weapon{"dart",1,4,1,false,true,true,20,60},
    Weapon{"light_crossbow",1,8,2,false,false,true,80,320},
    Weapon{"shortbow",1,6,2,false,false,true,80,320},
    Weapon{"sling",1,4,1,false,false,true,30,120},
    // OpenGoldBox battle axes require both hands.
    Weapon{"battleaxe",1,8,2,true}, Weapon{"flail",1,8,1,true},
    Weapon{"glaive",1,10,2,true,false,false,0,0,10},
    Weapon{"greatsword",2,6,2,true},
    Weapon{"halberd",1,10,2,true,false,false,0,0,10},
    Weapon{"longsword",1,8,1,true}, Weapon{"morningstar",1,8,1,true},
    Weapon{"pike",1,10,2,true,false,false,0,0,10},
    Weapon{.key="scimitar",.dice=1,.sides=6,.martial=true,.finesse=true,.light=true},
    Weapon{.key="shortsword",.dice=1,.sides=6,.martial=true,.finesse=true,.light=true},
    // OpenGoldBox tridents require both hands.
    Weapon{"trident",1,8,2,true,false,false,20,60},
    Weapon{"warhammer",1,8,1,true}, Weapon{"war_pick",1,8,1,true},
    Weapon{"longbow",1,8,2,true,false,true,150,600},
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
