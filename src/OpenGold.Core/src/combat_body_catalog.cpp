#include "opengold/combat_body_catalog.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string_view>

namespace opengold::por {
namespace {
unsigned index_field(std::string_view field,unsigned limit)
{
    if(field.empty()||field.find_first_not_of("0123456789")!=std::string_view::npos)
        throw std::runtime_error("Invalid combat catalog number");
    const auto value=std::stoul(std::string(field));
    if(value>=limit)throw std::runtime_error("Combat catalog number out of range");
    return static_cast<unsigned>(value);
}
std::string strip_cr(std::string text)
{if(!text.empty()&&text.back()=='\r')text.pop_back();return text;}
bool silver_name(std::string_view name)
{
    std::string lower(name);
    std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    return lower.find("silver")!=std::string::npos;
}
}
CombatBodyCatalog CombatBodyCatalog::load(const std::filesystem::path& assignments,
    const std::filesystem::path& options_file)
{
    CombatBodyCatalog result;
    std::ifstream options(options_file);
    if(!options)throw std::runtime_error("Cannot open combat look options: "+options_file.string());
    std::set<std::string> keys{"unreviewed"};
    std::string line;
    std::set<std::pair<int,bool>> item_types;
    while(std::getline(options,line)) {
        if(line.empty()||line[0]=='#')continue;
        const auto first=line.find('\t'),second=line.find('\t',first+1),third=line.find('\t',second+1);
        if(first==std::string::npos||second==std::string::npos||third==std::string::npos||line.find('\t',third+1)!=std::string::npos)
            throw std::runtime_error("Invalid combat look option row");
        const auto variant=strip_cr(line.substr(third+1));
        CombatLookOption option{line.substr(0,first),line.substr(first+1,second-first-1),
            static_cast<int>(index_field(std::string_view(line).substr(second+1,third-second-1),256)),
            variant=="silver"};
        if((variant!="ordinary"&&variant!="silver")||option.id.empty()||option.label.empty()||
            !keys.insert(option.id).second||!keys.insert(option.id+"_shield").second||
            !item_types.emplace(option.original_type,option.silver).second)
            throw std::runtime_error("Duplicate or invalid combat look option");
        result.options.push_back(std::move(option));
    }
    if(!options.eof()||result.options.empty())throw std::runtime_error("Invalid combat look options file");
    std::ifstream in(assignments);
    if(!in)throw std::runtime_error("Cannot open combat body catalog: "+assignments.string());
    std::array<bool,32> seen{};
    unsigned count=0;
    while(std::getline(in,line)) {
        if(line.empty()||line[0]=='#')continue;
        const auto tab=line.find('\t');
        if(tab==std::string::npos||line.find('\t',tab+1)!=std::string::npos)
            throw std::runtime_error("Invalid combat body catalog row");
        const auto index=index_field(std::string_view(line).substr(0,tab),32);
        const auto key=strip_cr(line.substr(tab+1));
        if(seen[index]||!keys.contains(key))throw std::runtime_error("Unknown or duplicate combat body assignment");
        result.bodies[index]=key;seen[index]=true;++count;
    }
    if(!in.eof()||count!=32)throw std::runtime_error("Combat body catalog must contain all 32 bodies");
    return result;
}
unsigned CombatBodyCatalog::choose(std::span<const CombatEquipment> equipped,unsigned fallback) const
{
    bool shield=false;
    const CombatEquipment* weapon=nullptr;
    int weapon_type=-1;
    for(const auto& item:equipped) {
        if(item.original_type==59||item.definition_id=="shield")shield=true;
        else {
            int type=item.original_type;
            if(type<0) {
                if(item.definition_id=="dagger")type=8;
                else if(item.definition_id=="mace")type=23;
                else if(item.definition_id=="quarterstaff")type=33;
                else if(item.definition_id=="longsword")type=36;
                else if(item.definition_id=="longbow")type=43;
                else if(item.definition_id=="shortbow")type=44;
            }
            if(std::any_of(options.begin(),options.end(),[&](const auto& option){return option.original_type==type;}))
                {weapon=&item;weapon_type=type;}
        }
    }
    const auto match=[&](std::string_view key)->unsigned{
        for(unsigned id=0;id<bodies.size();++id)if(bodies[id]==key)return id;
        return bodies.size();
    };
    if(!weapon) {
        const auto unarmed=match(shield?"type_0_shield":"type_0");
        return unarmed<bodies.size()?unarmed:fallback;
    }
    const auto desired_silver=silver_name(weapon->name);
    const auto find_option=[&](bool silver)->const CombatLookOption*{
        const auto it=std::find_if(options.begin(),options.end(),[&](const auto& option){
            return option.original_type==weapon_type&&option.silver==silver;});
        return it==options.end()?nullptr:&*it;
    };
    const auto option=find_option(desired_silver)?find_option(desired_silver):find_option(false);
    if(!option)return fallback;
    const auto exact=match(option->id+(shield?"_shield":""));
    if(exact<bodies.size())return exact;
    const auto same_weapon=match(option->id+(shield?"":"_shield"));
    if(same_weapon<bodies.size())return same_weapon;
    if(desired_silver)if(const auto ordinary=find_option(false)) {
        const auto same_art=match(ordinary->id+(shield?"_shield":""));
        if(same_art<bodies.size())return same_art;
        const auto alternate=match(ordinary->id+(shield?"":"_shield"));
        if(alternate<bodies.size())return alternate;
    }
    return fallback;
}
}
