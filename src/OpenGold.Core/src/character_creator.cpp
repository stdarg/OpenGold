#include "opengold/character_creator.h"
#include <algorithm>
#include <stdexcept>
namespace opengold {
using namespace rules;
CharacterCreator::CharacterCreator(std::unique_ptr<CharacterRules> rules,std::uint64_t seed)
    :rules_(std::move(rules)),random_(seed)
{if(!rules_)throw std::runtime_error("Character creator requires a rules module");restart();}
void CharacterCreator::restart()
{
    draft_={};appearance_={};step_=CreationStep::race;
    for(auto field:{CreationField::race,CreationField::gender,CreationField::character_class,CreationField::alignment,CreationField::background}) {
        const auto options=rules_->choices(field);
        if(options.empty())throw std::runtime_error("Rules module has no character choices");
        select(field,options.front().id);
    }
}
void CharacterCreator::require_editable() const
{if(step_==CreationStep::sheet)throw std::runtime_error("Return to editing before changing the character.");}
void CharacterCreator::select(CreationField field,std::string_view id)
{
    require_editable();const auto choices=rules_->choices(field);
    if(std::none_of(choices.begin(),choices.end(),[&](const auto& c){return c.id==id;}))throw std::runtime_error("Unknown character choice");
    switch(field) {
    case CreationField::race:draft_.race=id;break;
    case CreationField::gender:draft_.gender=id;break;
    case CreationField::character_class:draft_.character_class=id;break;
    case CreationField::alignment:draft_.alignment=id;break;
    case CreationField::background:draft_.background=id;draft_.adjustment=0;break;
    }
}
void CharacterCreator::select_adjustment(unsigned index)
{require_editable();if(index>=rules_->adjustments(draft_.background).size())throw std::runtime_error("Invalid score adjustment");draft_.adjustment=index;}
void CharacterCreator::roll()
{require_editable();draft_.rolls=rules_->roll(random_);draft_.rolled=true;draft_.assignment={0,1,2,3,4,5};}
void CharacterCreator::swap_scores(unsigned first,unsigned second)
{
    require_editable();if(!draft_.rolled||first>=6||second>=6)throw std::runtime_error("Invalid score swap");
    std::swap(draft_.assignment[first],draft_.assignment[second]);
}
void CharacterCreator::name(std::string text)
{
    require_editable();const auto first=text.find_first_not_of(" \t\r\n"),last=text.find_last_not_of(" \t\r\n");
    draft_.name=first==std::string::npos?"":text.substr(first,last-first+1);
}
void CharacterCreator::appearance(por::CharacterAppearance value)
{
    require_editable();por::validate_character_appearance(value);
    appearance_=value;
}
CharacterSheet CharacterCreator::sheet() const {return rules_->evaluate(draft_,step_>=CreationStep::portrait);}
Character CharacterCreator::create_character() const
{
    if(step_!=CreationStep::sheet)throw std::runtime_error("Finish character creation before exporting the character");
    return Character(*rules_,draft_,appearance_);
}
void CharacterCreator::next()
{
    if(step_==CreationStep::sheet)return;
    if(step_>=CreationStep::attributes)(void)rules_->evaluate(draft_,step_>=CreationStep::name);
    step_=static_cast<CreationStep>(static_cast<unsigned>(step_)+1);
}
void CharacterCreator::back()
{if(step_!=CreationStep::race)step_=static_cast<CreationStep>(static_cast<unsigned>(step_)-1);}
}
