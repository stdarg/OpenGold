#include "opengold/character_rules.h"
#include <algorithm>
#include <numeric>
#include <stdexcept>
namespace opengold::rules {
int AbilityRoll::total() const
{
    if(discarded>=dice.size() || std::any_of(dice.begin(),dice.end(),[](int n){return n<1||n>6;}) ||
        dice[discarded]!=*std::min_element(dice.begin(),dice.end()))
        throw std::runtime_error("Invalid 4d6 roll");
    return std::accumulate(dice.begin(),dice.end(),0)-dice[discarded];
}
}
