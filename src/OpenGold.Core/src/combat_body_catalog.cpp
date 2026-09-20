#include "opengold/combat_body_catalog.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>

namespace opengold::por {
namespace {
constexpr std::array<std::string_view,13> ids{"unreviewed","unarmed","unarmed_shield","dagger","dagger_shield",
    "mace","mace_shield","sword","sword_shield","staff","staff_shield","bow","bow_shield"};
}
std::string_view combat_look_id(CombatLook look) {return ids.at(static_cast<unsigned>(look));}
CombatLook parse_combat_look(std::string_view id)
{
    const auto found=std::find(ids.begin(),ids.end(),id);
    if(found==ids.end())throw std::runtime_error("Unknown combat look: "+std::string(id));
    return static_cast<CombatLook>(found-ids.begin());
}
CombatBodyCatalog CombatBodyCatalog::load(const std::filesystem::path& file)
{
    std::ifstream in(file);
    if(!in)throw std::runtime_error("Cannot open combat body catalog: "+file.string());
    CombatBodyCatalog result;
    std::array<bool,32> seen{};
    std::string line;
    unsigned count=0;
    while(std::getline(in,line)) {
        if(line.empty()||line[0]=='#')continue;
        const auto tab=line.find('\t');
        if(tab==std::string::npos||line.find('\t',tab+1)!=std::string::npos)
            throw std::runtime_error("Invalid combat body catalog row");
        unsigned index{};
        try {std::size_t used{};index=std::stoul(line.substr(0,tab),&used);if(used!=tab)throw std::runtime_error("Invalid ID");}
        catch(...) {throw std::runtime_error("Invalid combat body ID");}
        if(index>=32||seen[index])throw std::runtime_error("Duplicate or out-of-range combat body ID");
        auto value=line.substr(tab+1);
        if(!value.empty()&&value.back()=='\r')value.pop_back();
        result.bodies[index]=parse_combat_look(value);
        seen[index]=true;++count;
    }
    if(!in.eof()||count!=32)throw std::runtime_error("Combat body catalog must contain all 32 bodies");
    return result;
}
unsigned CombatBodyCatalog::choose(std::span<const std::string> equipped,unsigned fallback) const
{
    bool shield=false;
    std::string_view weapon;
    for(const auto& item:equipped) {
        if(item=="shield")shield=true;
        else if(item=="dagger"||item=="mace"||item=="longsword"||item=="quarterstaff"||item=="shortbow"||item=="longbow")weapon=item;
    }
    const auto type=weapon=="dagger"?CombatLook::dagger:weapon=="mace"?CombatLook::mace:
        weapon=="longsword"?CombatLook::sword:weapon=="quarterstaff"?CombatLook::staff:
        (weapon=="shortbow"||weapon=="longbow")?CombatLook::bow:CombatLook::unarmed;
    const auto wanted=static_cast<CombatLook>(static_cast<unsigned>(type)+(shield?1:0));
    for(unsigned id=0;id<bodies.size();++id)if(bodies[id]==wanted)return id;
    // A weapon match takes priority over shield presence when exact art is absent.
    for(unsigned id=0;id<bodies.size();++id)if(type!=CombatLook::unarmed &&
        (bodies[id]==type||bodies[id]==static_cast<CombatLook>(static_cast<unsigned>(type)+1)))return id;
    if(type==CombatLook::unarmed)for(unsigned id=0;id<bodies.size();++id)if(bodies[id]==CombatLook::unarmed)return id;
    return fallback;
}
}
