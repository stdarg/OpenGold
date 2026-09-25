#include "opengold/character.h"
#include <algorithm>
#include <stdexcept>

namespace opengold {
Character::Character(const rules::CharacterRules& rules,rules::CharacterDraft creation,por::CharacterAppearance appearance)
    :creation_(std::move(creation)),sheet_(rules.evaluate(creation_,true))
{this->appearance(appearance);}
void Character::appearance(por::CharacterAppearance value)
{por::validate_character_appearance(value);appearance_=value;}
bool Character::advance(const rules::RulesModule& rules, rules::VitalState& state)
{return advance(rules,state,rules.default_advancement(sheet_));}
bool Character::advance(const rules::RulesModule& rules,rules::VitalState& state,const rules::AdvancementChoice& choice)
{
    auto sheet=sheet_;auto vitals=state;auto history=advancements_;
    if(!rules.advance_character(sheet,vitals,choice))return false;
    history.push_back(choice);sheet_=std::move(sheet);state=std::move(vitals);advancements_=std::move(history);return true;
}
rules::TrainingChoices Character::training_choices() const {
    auto result=creation_.training;
    for(const auto& advancement:advancements_)for(const auto& [id,values]:advancement.training)result.emplace(id,values);
    return result;
}
Character Character::preview_training(const rules::CharacterRules& creation_rules,
    const rules::RulesModule& rules,const rules::TrainingChoices& choices,bool require_complete) const
{
    if(creation_rules.identity()!=sheet_.identity)throw std::runtime_error("Character creation rules do not match");
    if(require_complete&&sheet_.training.complete)throw std::runtime_error("This character has no pending training choices");
    for(const auto& [group,selected]:training_choices()){
        const auto found=choices.find(group);
        if(found==choices.end()||std::any_of(selected.begin(),selected.end(),[&](const auto& value){
            return std::find(found->second.begin(),found->second.end(),value)==found->second.end();
        }))throw std::runtime_error("Previously selected training cannot be replaced");
    }
    auto draft=creation_;draft.training=choices;
    const auto later=rules.training_options(sheet_);
    for(const auto& group:later)draft.training.erase(group.id);
    Character candidate(creation_rules,std::move(draft),appearance_);
    // Replay on an isolated healthy state, as during save reconstruction. The
    // campaign keeps the real wounds, effects and spent resources unchanged.
    rules::VitalState replay{candidate.sheet_.hit_points,false,{}};
    for(auto choice:advancements_){
        for(const auto& group:later)if(group.acquired_level==candidate.sheet_.level+1){
            choice.training.erase(group.id);
            if(const auto found=choices.find(group.id);found!=choices.end())choice.training.emplace(*found);
        }
        if(!candidate.advance(rules,replay,choice))throw std::runtime_error("Cannot reconstruct character advancement");
    }
    if(require_complete&&!candidate.sheet_.training.complete)throw std::runtime_error("Complete all pending training choices");
    candidate.inventory_=inventory_;
    (void)rules.character_profile(candidate.sheet_,{});
    return candidate;
}
}
