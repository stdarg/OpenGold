#include "character_creation_view.h"
#include "opengold/srd5.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <algorithm>

using namespace godot;
using namespace opengold;
namespace {
const std::array<const char*,6> names{"Strength","Dexterity","Constitution","Intelligence","Wisdom","Charisma"};
std::string number(int n){return (n>=0?"+":"")+std::to_string(n);}
std::string literal(std::string_view value){std::string text;for(char c:value)text+=c=='['?"[lb]":std::string(1,c);return text;}
String gs(std::string_view text){return String::utf8(text.data(),text.size());}
struct DeleteNode {void operator()(Node* node) const {memdelete(node);}};
}
Variant CharacterCreationView::drag_roll(Vector2,int index)
{
    if(!creator_||party_open_||creator_->step()!=CreationStep::attributes||!creator_->draft().rolled||index<0||index>=6)return {};
    auto preview=std::unique_ptr<Label,DeleteNode>(memnew(Label));
    preview->set_text(gs("Roll "+std::to_string(index+1)+": "+std::to_string(creator_->draft().rolls[index].total())));
    get_node<Control>(gs("Dice"+std::to_string(index)))->set_drag_preview(preview.get());preview.release();
    Dictionary data;data["opengold_ability_roll"]=index;return data;
}
bool CharacterCreationView::can_drop_roll(Vector2,const Variant& data,int index)
{
    if(!creator_||party_open_||creator_->step()!=CreationStep::attributes||!creator_->draft().rolled||index<0||index>=6||data.get_type()!=Variant::DICTIONARY)return false;
    const Dictionary payload=data;const Variant roll=payload.get("opengold_ability_roll",Variant());
    return roll.get_type()==Variant::INT&&int(roll)>=0&&int(roll)<6;
}
void CharacterCreationView::drop_roll(Vector2 position,const Variant& data,int index)
{
    if(!can_drop_roll(position,data,index))return;
    const Dictionary payload=data;const unsigned roll=int(payload["opengold_ability_roll"]);
    perform([&]{const auto& assignments=creator_->draft().assignment;
        const auto source=std::find(assignments.begin(),assignments.end(),roll)-assignments.begin();
        creator_->swap_scores(source,index);selected_score_=-1;});
}
String CharacterCreationView::sheet_text(const Character& character,const PartyMember* member) const
{
    const auto& s=character.sheet();
    std::string text="[font_size=24]"+literal(s.name)+"[/font_size]\nLevel "+std::to_string(s.level)+" "+s.race+" "+s.gender+" "+s.character_class+"\n"+s.alignment+" / "+s.background+"\n\n";
    text+="[b]HP "+std::to_string(member?member->vitals.hit_points:s.hit_points)+" / "+std::to_string(s.hit_points)+"[/b]   Hit Dice: 1d"+std::to_string(s.hit_die);
    if(member)text+="   Gold "+std::to_string(member->wealth[3])+(member->vitals.dead?"   Dead":"");
    text+="\n\n[table=3][cell][b]Attribute     [/b][/cell][cell][b]Score     [/b][/cell][cell][b]Saving throw[/b][/cell]";
    for(unsigned i=0;i<6;++i)text+="[cell]"+std::string(names[i])+"[/cell][cell]"+std::to_string(s.scores[i])+"[/cell][cell]"+number(s.saving_throws[i])+(s.save_proficiencies[i]?" *":"")+"[/cell]";
    text+="[/table]\n* Proficient saving throw";
    if(member){
        try{const auto p=campaign_->profile(member->id);text+="\n\nAC "+std::to_string(p.armor_class)+"   Speed "+std::to_string(p.movement_feet)+" ft";}
        catch(const std::exception& e){text+="\n\n"+literal(e.what());}
        if(!member->vitals.description.empty())text+="\n"+literal(member->vitals.description);
    }
    text+="\n\n[b]Inventory[/b]";
    if(character.inventory().empty())text+="\nEmpty";
    for(const auto& item:character.inventory().items())text+="\n"+literal(item.name)+" x"+std::to_string(item.quantity);
    return gs(text);
}
void CharacterCreationView::show_modifiers()
{
    const PartyMember* member=nullptr; // Borrowed only for this synchronous rendering.
    if(party_open_&&!campaign_->state().roster.empty())member=&campaign_->state().roster.at(roster_index_);
    if(!member&&!completed_)return;
    const auto& s=member?member->character.sheet():completed_->sheet();
    std::string text="[b]Ability modifiers[/b]\n";
    for(unsigned i=0;i<6;++i)text+=std::string(names[i])+": "+number(s.modifiers[i])+"\n";
    text+="\n[b]Race / "+s.race+"[/b]\n"+s.racial_modifiers;
    text+="\n\n[b]Class / "+s.character_class+"[/b]\n"+s.class_modifiers;
    text+="\n\n[b]Background / "+s.background+"[/b]\n"+s.background_modifiers;
    try{
        const auto pack=std::filesystem::u8path(ProjectSettings::get_singleton()->globalize_path("res://../data/rules/srd-5.2.1/combat.rules").utf8().get_data());
        const auto profile=member?campaign_->profile(member->id):srd5::load(pack)->character_profile(s,{});
        text+="\n\n[b]Items[/b]\n"+profile.item_modifiers+"\n\n[b]Spells[/b]\n"+profile.spell_modifiers+"\n\n"+profile.description;
    }catch(const std::exception& e){
        text+="\n\n[b]Items[/b]\nNo equipped item modifiers.\n\n[b]Spells[/b]\nNo active spell modifiers. Persistent spell effects are not implemented.";
        text+="\n\n"+literal(e.what());
    }
    get_node<RichTextLabel>("ModifiersModal/Text")->set_text(gs(text));
    get_node<Label>("ModifiersModal/Title")->set_text(gs(s.name+" / Modifiers"));
    auto* modal=get_node<Window>("ModifiersModal");modal->popup_centered();
    get_node<Button>("ModifiersModal/Close")->grab_focus();
}
void CharacterCreationView::close_modifiers(){get_node<Window>("ModifiersModal")->hide();}
