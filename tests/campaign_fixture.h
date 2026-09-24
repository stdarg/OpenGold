#ifndef OPENGOLD_TEST_CAMPAIGN_FIXTURE_H
#define OPENGOLD_TEST_CAMPAIGN_FIXTURE_H
#include "opengold/character_rules.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace opengold::test {
// Independent expected migration for authored Wizard fixtures: their ledger
// begins with Spellcasting, optionally preceded by Soldier Savage Attacker. Add only the two known old preset
// selections, leaving all other campaign bytes untouched. No save decoder or
// current grant generator is used to construct this oracle.
inline std::string with_initial_wizard_spell_grants(std::string body){
    const std::string marker="\"feature:spellcasting\" \"class:wizard\" 1 0";
    std::size_t search=0;
    while(true){
        auto at=body.find(marker,search);if(at==std::string::npos)break;
        const std::string soldier="\"feat:savage_attacker\" \"background:soldier\" 1 0 ";
        if(at>=soldier.size()&&body.substr(at-soldier.size(),soldier.size())==soldier)at-=soldier.size();
        if(at<3||body[at-1]!=' ')throw std::runtime_error("Missing frozen grant count");
        auto begin=at-2;while(begin&&body[begin-1]>='0'&&body[begin-1]<='9')--begin;
        std::istringstream input(body.substr(begin));unsigned count{};input>>count;
        if(!input||count>32)throw std::runtime_error("Unexpected frozen grant ledger");
        std::vector<rules::FeatureGrant> grants;
        for(unsigned n=0;n<count;++n){rules::FeatureGrant g;unsigned choices{};input>>std::quoted(g.id)>>std::quoted(g.source_id)>>g.level>>choices;
            if(!input||choices>6)throw std::runtime_error("Malformed frozen grant");
            for(unsigned i=0;i<choices;++i){std::string k,v;input>>std::quoted(k)>>std::quoted(v);g.choices.emplace(k,v);}grants.push_back(std::move(g));}
        if(!input)throw std::runtime_error("Truncated frozen grant ledger");
        const auto length=static_cast<std::size_t>(input.tellg());
        grants.push_back({"spell:fire_bolt","class:wizard:spellcasting",1,{{"access","cantrip"}}});
        grants.push_back({"spell:magic_missile","class:wizard:spellcasting",1,{{"access","spellbook"}}});
        std::stable_sort(grants.begin(),grants.end(),[](const auto& a,const auto& b){return a.level<b.level;});
        std::ostringstream out;out<<grants.size();for(const auto& g:grants){out<<' '<<std::quoted(g.id)<<' '<<std::quoted(g.source_id)<<' '<<g.level<<' '<<g.choices.size();for(const auto& [k,v]:g.choices)out<<' '<<std::quoted(k)<<' '<<std::quoted(v);}
        const auto next=out.str();body.replace(begin,length,next);search=begin+next.size();
    }
    return body;
}
}
#endif
