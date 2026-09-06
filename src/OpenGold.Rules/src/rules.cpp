#include "opengold/rules.h"
namespace opengold::rules {
bool Battlefield::contains(Cell p) const noexcept
{ return p.x>=0 && p.y>=0 && p.x<width && p.y<height; }
unsigned Battlefield::at(Cell p) const noexcept
{
    if(!contains(p))return 1;
    const auto index=static_cast<std::size_t>(p.y)*static_cast<std::size_t>(width)+static_cast<std::size_t>(p.x);
    return index<terrain.size()?terrain[index]:1;
}
}
