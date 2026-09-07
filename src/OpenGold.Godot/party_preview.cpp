#include "character_creation_view.h"
#include "combat_view.h"
#include "rolf_tour_view.h"
#include "opengold/srd5.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <stdexcept>

using namespace godot;
using namespace opengold;
namespace {
String gs(std::string_view text){return String::utf8(text.data(),text.size());}
struct DeleteNode {void operator()(Node* node) const {memdelete(node);}};
std::unique_ptr<Node,DeleteNode> scene(const char* path)
{
    Ref<PackedScene> packed=ResourceLoader::get_singleton()->load(path);
    if(packed.is_null())throw std::runtime_error(std::string("Missing scene: ")+path);
    return std::unique_ptr<Node,DeleteNode>(packed->instantiate());
}
Character preview_guard()
{
    auto rules=srd5::character_rules();rules::CharacterDraft draft;
    draft.race="human";draft.gender="male";draft.character_class="fighter";draft.alignment="lawful_good";
    draft.background="soldier";draft.name="Preview guard";draft.rolled=true;
    for(auto& roll:draft.rolls)roll={{6,5,4,1},3};
    return Character(*rules,std::move(draft),{});
}
}
void CharacterCreationView::setup_party()
{
    const auto pack=std::filesystem::u8path(ProjectSettings::get_singleton()->globalize_path("res://../data/rules/srd-5.2.1/combat.rules").utf8().get_data());
    campaign_=std::make_shared<CampaignParty>(srd5::load(pack));
    auto panel=scene("res://scenes/party_panel.tscn");add_child(panel.get());panel.release();
    get_node<Control>("PartyPanel")->hide();
    get_node<Button>("Party")->connect("pressed",callable_mp(this,&CharacterCreationView::party_action).bind(0));
    get_node<Button>("AddParty")->connect("pressed",callable_mp(this,&CharacterCreationView::party_action).bind(1));
    get_node<Button>("ReturnParty")->connect("pressed",callable_mp(this,&CharacterCreationView::party_action).bind(9));
    const std::array<const char*,9> buttons{"Create","Remove","Rejoin","Recruit","Equip","Unequip","Explore","Combat","Close"};
    const std::array<int,9> actions{2,3,4,5,6,10,7,8,11};
    for(unsigned i=0;i<buttons.size();++i)get_node<Button>(gs(std::string("PartyPanel/")+buttons[i]))->connect("pressed",callable_mp(this,&CharacterCreationView::party_action).bind(actions[i]));
    get_node<ItemList>("PartyPanel/Roster")->connect("item_selected",callable_mp(this,&CharacterCreationView::party_selected));
    get_node<Button>("PartyPanel/Modifiers")->connect("pressed",callable_mp(this,&CharacterCreationView::show_modifiers));
    get_node<RichTextLabel>("PartyPanel/Sheet")->set_use_bbcode(true);
    party_check_=OS::get_singleton()->get_cmdline_user_args().has("--party-check");party_layout();
}
void CharacterCreationView::party_layout()
{
    const auto w=get_size().x,h=get_size().y;
    get_node<Control>("PartyPanel")->set_size(get_size());
    const auto place=[&](const char* name,Rect2 rect){auto* node=get_node<Control>(name);node->set_position(rect.position);node->set_size(rect.size);};
    place("Party",Rect2(24,h-158,166,36));place("AddParty",Rect2(218+page_rect_.size.x-190,h-60,190,38));
    place("ReturnParty",Rect2(w-218,20,190,36));
    place("PartyPanel/Title",Rect2(24,22,w-48,40));place("PartyPanel/Roster",Rect2(24,90,300,h-300));
    place("PartyPanel/Sheet",Rect2(350,90,w-650,h-380));
    place("PartyPanel/Portrait",Rect2(w-284,90,264,264));
    place("PartyPanel/Inventory",Rect2(350,h-280,w-374,140));
    const std::array<const char*,9> buttons{"Create","Remove","Rejoin","Recruit","Equip","Unequip","Explore","Combat","Close"};
    const double bw=(w-64)/5;
    for(unsigned i=0;i<buttons.size();++i)place((std::string("PartyPanel/")+buttons[i]).c_str(),Rect2(24+(i%5)*(bw+4),h-125+(i/5)*44,bw,36));
    place("PartyPanel/Status",Rect2(24,h-39,w-48,32));
    place("PartyPanel/Modifiers",Rect2(24+4*(bw+4),h-81,bw,36));
    for(const auto* name:{"CampaignTown","CampaignCombat"})if(auto* child=Object::cast_to<Control>(get_node_or_null(name)))child->set_size(get_size());
}
void CharacterCreationView::party_selected(std::int64_t index)
{
    if(index<0||static_cast<std::size_t>(index)>=campaign_->state().roster.size())return;
    roster_index_=index;const auto id=campaign_->state().roster[index].id;
    for(unsigned slot=0;slot<8;++slot)if(campaign_->state().slots[slot]==id)campaign_->select(slot);
    refresh_party();
}
void CharacterCreationView::refresh_party()
{
    auto* list=get_node<ItemList>("PartyPanel/Roster");list->clear();
    const auto& state=campaign_->state();
    for(const auto& m:state.roster){
        auto slot=std::find(state.slots.begin(),state.slots.end(),m.id);
        list->add_item(gs(m.character.sheet().name+(slot==state.slots.end()?" (Reserve)":"")));
    }
    auto* items=get_node<ItemList>("PartyPanel/Inventory");items->clear();
    std::string sheet="Create a character, finish its sheet, then Add to party.\n\nSix PC positions and two NPC positions. Removed members remain in the roster.";
    if(!state.roster.empty()){
        roster_index_=std::min(roster_index_,state.roster.size()-1);list->select(roster_index_);
        const auto& m=state.roster[roster_index_];
        sheet=sheet_text(m.character,&m).utf8().get_data();
        for(const auto& item:m.character.inventory().items())items->add_item(gs(std::string(std::find(m.equipped.begin(),m.equipped.end(),item.id)!=m.equipped.end()?"Equipped / ":"")+item.name+" x"+std::to_string(item.quantity)));
        const auto image=art_->portrait(m.character.appearance());PackedByteArray pixels;pixels.resize(image.rgba.size());std::copy(image.rgba.begin(),image.rgba.end(),pixels.ptrw());
        get_node<TextureRect>("PartyPanel/Portrait")->set_texture(ImageTexture::create_from_image(godot::Image::create_from_data(image.width,image.height,false,godot::Image::FORMAT_RGBA8,pixels)));
    }
    get_node<RichTextLabel>("PartyPanel/Sheet")->set_text(gs(sheet));
    get_node<Label>("PartyPanel/Status")->set_text(error_.is_empty()?"Session preview / New PCs receive 250 gp / Progress is not saved yet.":error_);
    for(const char* name:{"Remove","Rejoin","Equip","Unequip","Explore","Combat","Modifiers"})get_node<Button>(gs(std::string("PartyPanel/")+name))->set_disabled(state.roster.empty());
}
void CharacterCreationView::party_action(int action)
{
    try {
        error_="";
        const auto id=campaign_->state().roster.empty()?0:campaign_->state().roster.at(roster_index_).id;
        if(action==1){if(!completed_||added_to_party_)return;const auto added=campaign_->add_pc(*completed_);campaign_->set_wealth(added,{0,0,0,250,0,0,0});
            added_to_party_=true;roster_index_=campaign_->state().roster.size()-1;get_node<Button>("AddParty")->hide();}
        if(action==2||action==11){party_open_=false;get_node<Control>("PartyPanel")->hide();if(action==2)restart();return;}
        if(action==3)campaign_->remove(id);
        if(action==4)campaign_->rejoin(id);
        if(action==5){const auto recruited=campaign_->recruit("preview:guard",preview_guard());
            const auto& roster=campaign_->state().roster;roster_index_=std::find_if(roster.begin(),roster.end(),[&](const auto& m){return m.id==recruited;})-roster.begin();}
        if(action==6||action==10){const auto selection=get_node<ItemList>("PartyPanel/Inventory")->get_selected_items();
            if(selection.is_empty())throw std::runtime_error("Select an inventory item first");
            const auto items=campaign_->member(id).character.inventory().items();const auto selected=items[selection[0]].id;
            if(action==6)campaign_->equip(id,selected);else campaign_->unequip(id,selected);}
        if(action==7||action==8){
            if(!campaign_->selected())throw std::runtime_error("Add a party member first");
            if(action==7){auto* town=Object::cast_to<RolfTourView>(get_node_or_null("CampaignTown"));
                if(!town){auto owned=scene("res://scenes/rolf_tour.tscn");town=Object::cast_to<RolfTourView>(owned.get());if(!town)throw std::runtime_error("Invalid exploration scene");
                    town->set_name("CampaignTown");town->campaign_party(campaign_);add_child(owned.get());owned.release();}
                town->show();town->set_process(true);town->set_process_input(true);town->resume_party();
            }else{
                const auto participants=campaign_->participants();std::vector<CombatArt> images;
                for(const auto& participant:participants)images.push_back({participant.id,art_->icon(campaign_->member(participant.id).character.appearance(),false)});
                auto owned=scene("res://scenes/combat_demo.tscn");auto* combat=Object::cast_to<CombatView>(owned.get());if(!combat)throw std::runtime_error("Invalid combat scene");
                combat->set_name("CampaignCombat");combat->campaign_party(campaign_,std::move(images));add_child(owned.get());owned.release();
            }
            get_node<Control>("PartyPanel")->hide();auto* back=get_node<Button>("ReturnParty");move_child(back,get_child_count()-1);back->show();party_layout();return;
        }
        if(action==9){
            if(auto* combat=Object::cast_to<CombatView>(get_node_or_null("CampaignCombat"))){if(!combat->can_leave())throw std::runtime_error("Finish the fight before returning to the party");
                remove_child(combat);std::unique_ptr<Node,DeleteNode> released(combat);}
            if(auto* town=Object::cast_to<RolfTourView>(get_node_or_null("CampaignTown"))){if(town->is_visible()&&!town->can_leave())throw std::runtime_error("Finish the dialogue or leave the shop first");
                town->hide();town->set_process(false);town->set_process_input(false);}
            get_node<Button>("ReturnParty")->hide();
        }
        party_open_=true;auto* panel=get_node<Control>("PartyPanel");move_child(panel,get_child_count()-1);panel->show();refresh_party();
    }catch(const std::exception& e){error_=gs(e.what());get_node<Label>("Status")->set_text(error_);get_node<Label>("PartyPanel/Status")->set_text(error_);
        get_node<Button>("ReturnParty")->set_tooltip_text(error_);}
}
void CharacterCreationView::party_check()
{
    const auto press=[&](const char* node){get_node<Button>(node)->emit_signal("pressed");if(!error_.is_empty())throw std::runtime_error(error_.utf8().get_data());};
    switch(party_check_stage_){
    case 0:
        creator_->select(rules::CreationField::race,"human");creator_->select(rules::CreationField::character_class,"fighter");
        recommend_head();creator_->roll();creator_->name("Party check fighter");while(creator_->step()!=CreationStep::sheet)next();
        press("AddParty");if(campaign_->state().slots[0]==0)throw std::runtime_error("Add party callback failed");
        press("PartyPanel/Recruit");if(!campaign_->state().slots[6])throw std::runtime_error("Recruit callback failed");
        press("PartyPanel/Remove");press("PartyPanel/Rejoin");
        party_selected(0);capture("party-roster.png");++party_check_stage_;break;
    case 1:press("PartyPanel/Explore");++party_check_stage_;break;
    case 2:if(!get_node<RolfTourView>("CampaignTown")->party_route_checked())return;
        press("ReturnParty");party_selected(0);
        if(campaign_->member(campaign_->selected()).character.inventory().items().size()!=1)throw std::runtime_error("Town purchase missing from party inventory");
        get_node<ItemList>("PartyPanel/Inventory")->select(0);press("PartyPanel/Equip");++party_check_stage_;break;
    case 3:
        if(!get_node<RichTextLabel>("PartyPanel/Sheet")->get_text().contains("Saving throw"))throw std::runtime_error("Party selection did not display the character sheet");
        press("PartyPanel/Modifiers");
        if(!get_node<RichTextLabel>("ModifiersModal/Text")->get_text().contains("Shield: +2 AC"))throw std::runtime_error("Party modifiers omitted equipped shield");
        press("ModifiersModal/Close");
        capture("party-equipped.png");press("PartyPanel/Combat");++party_check_stage_;break;
    case 4:{auto* fight=get_node<CombatView>("CampaignCombat");
        if(!fight->can_leave())return;press("ReturnParty");capture("party-after-combat.png");
        if(campaign_->in_combat()||campaign_->member(campaign_->state().slots[0]).vitals.resources.empty())throw std::runtime_error("Combat state was not returned");
        ++party_check_stage_;break;}
    case 5:press("PartyPanel/Explore");++party_check_stage_;break;
    case 6:if(!get_node<RolfTourView>("CampaignTown")->can_leave())return;
        press("ReturnParty");++party_check_stage_;break;
    case 7:capture("party-after-combat.png");
        UtilityFunctions::print("Godot party check passed: creation, party sheets/inventory, NPC remove/rejoin, original town shop, equipment, combat and return to exploration");party_check_=false;get_tree()->quit(0);break;
    }
}
void CharacterCreationView::update_party_navigation()
{
    bool allowed=true;
    if(auto* fight=Object::cast_to<CombatView>(get_node_or_null("CampaignCombat")))allowed=fight->can_leave();
    if(auto* town=Object::cast_to<RolfTourView>(get_node_or_null("CampaignTown")))if(town->is_visible())allowed=town->can_leave();
    auto* button=get_node<Button>("ReturnParty");button->set_disabled(!allowed);
    button->set_tooltip_text(allowed?"Inspect your party and equipment.":"Finish combat, dialogue or shopping before returning to the party.");
}
