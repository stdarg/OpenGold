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
Character Character::preview_training(const rules::CharacterRules& creation_rules,
    const rules::RulesModule& rules,const rules::TrainingChoices& choices) const
{
    if(creation_rules.identity()!=sheet_.identity)throw std::runtime_error("Character creation rules do not match");
    if(sheet_.training.complete)throw std::runtime_error("This character has no pending training choices");
    for(const auto& [group,selected]:creation_.training){
        const auto found=choices.find(group);
        if(found==choices.end()||std::any_of(selected.begin(),selected.end(),[&](const auto& value){
            return std::find(found->second.begin(),found->second.end(),value)==found->second.end();
        }))throw std::runtime_error("Previously selected training cannot be replaced");
    }
    auto draft=creation_;draft.training=choices;
    Character candidate(creation_rules,std::move(draft),appearance_);
    if(!candidate.sheet_.training.complete)throw std::runtime_error("Complete all pending training choices");
    // Replay on an isolated healthy state, as during save reconstruction. The
    // campaign keeps the real wounds, effects and spent resources unchanged.
    rules::VitalState replay{candidate.sheet_.hit_points,false,{}};
    for(const auto& choice:advancements_)
        if(!candidate.advance(rules,replay,choice))throw std::runtime_error("Cannot reconstruct character advancement");
    candidate.inventory_=inventory_;
    (void)rules.character_profile(candidate.sheet_,{});
    return candidate;
}
}
