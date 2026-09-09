#include "character_creation_view.h"
#include "rolf_tour_view.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "opengold/campaign_save.h"
#include "opengold/srd5.h"
#include <algorithm>

using namespace godot;
namespace {
String gs(std::string_view s){return String::utf8(s.data(),s.size());}
struct DeleteNode {void operator()(Node* n)const{memdelete(n);}};
template<class T> T* control(Node* parent,const String& name,Rect2 rect){
    std::unique_ptr<T,DeleteNode> owned(memnew(T));owned->set_name(name);
    owned->set_position(rect.position);owned->set_size(rect.size);
    auto* borrowed=owned.get();parent->add_child(owned.get());owned.release();return borrowed;
}
}
void CharacterCreationView::setup_advancement(){
    get_node<ItemList>("PartyPanel/Roster")->add_theme_constant_override("v_separation",8);
    const auto args=OS::get_singleton()->get_cmdline_user_args();advancement_check_=args.has("--advancement-check");advancement_review_=args.has("--level-up-review");
    std::unique_ptr<Window,DeleteNode> owned(memnew(Window));owned->set_name("LevelUp");
    owned->set_title("Level up");owned->set_size(Vector2i(700,670));owned->set_min_size(Vector2i(700,670));
    owned->set_flag(Window::FLAG_RESIZE_DISABLED,true);owned->set_transient(true);owned->set_exclusive(true);
    owned->hide();auto* window=owned.get();add_child(owned.get());owned.release();
    window->connect("close_requested",callable_mp(this,&CharacterCreationView::close_advancement));
    auto* title=control<Label>(window,"Title",Rect2(24,20,652,40));title->add_theme_font_size_override("font_size",24);title->set_clip_text(true);
    control<Label>(window,"HP",Rect2(24,70,652,42));
    control<Label>(window,"FeatLabel",Rect2(24,122,652,28))->set_text("Feat or ability points");
    auto* feat=control<OptionButton>(window,"Feat",Rect2(24,155,652,38));
    feat->connect("item_selected",callable_mp(this,&CharacterCreationView::advancement_changed));
    const std::array<const char*,6> abilities{"STR","DEX","CON","INT","WIS","CHA"};
    for(unsigned i=0;i<6;++i){
        control<Label>(window,String("AbilityLabel")+String::num_uint64(i),Rect2(24+i*110,205,100,25))->set_text(abilities[i]);
        auto* points=control<OptionButton>(window,String("Ability")+String::num_uint64(i),Rect2(24+i*110,236,100,36));
        for(int n=0;n<=2;++n)points->add_item(String("+")+String::num_int64(n));
        points->connect("item_selected",callable_mp(this,&CharacterCreationView::advancement_changed));
    }
    control<Label>(window,"SpellLabel",Rect2(24,292,652,28))->set_text("Prepared spells");
    for(int i=0;i<4;++i){auto* spell=control<CheckBox>(window,String("Spell")+String::num_int64(i),Rect2(24,325+i*38,652,36));
        spell->connect("toggled",callable_mp(this,&CharacterCreationView::advancement_spell_changed).bind(i));}
    auto* note=control<Label>(window,"Note",Rect2(24,489,652,66));note->set_text("Fixed-average HP growth. Existing resource expenditure is preserved.\nAdditional class and subclass features are unavailable in this version.");note->add_theme_font_size_override("font_size",15);
    auto* error=control<Label>(window,"Error",Rect2(24,560,652,34));error->add_theme_font_size_override("font_size",15);
    auto* cancel=control<Button>(window,"Cancel",Rect2(386,610,136,40));cancel->set_text("Cancel");cancel->connect("pressed",callable_mp(this,&CharacterCreationView::close_advancement));
    auto* confirm=control<Button>(window,"Confirm",Rect2(536,610,140,40));confirm->set_text("Confirm");confirm->connect("pressed",callable_mp(this,&CharacterCreationView::confirm_advancement));
}
void CharacterCreationView::refresh_advancement_arrows(){
    auto* list=get_node<ItemList>("PartyPanel/Roster");if(!list->is_visible_in_tree())return;
    list->force_update_list_size();
    const auto& roster=campaign_->state().roster;
    for(std::size_t i=0;i<roster.size();++i){const auto& member=roster[i];const auto name=String("Advance")+String::num_uint64(member.id);
        auto* arrow=Object::cast_to<Button>(list->get_node_or_null(name));
        if(!arrow){arrow=control<Button>(list,name,Rect2(0,0,30,26));arrow->set_text(String::utf8("↑"));arrow->set_tooltip_text("Level up "+gs(member.character.sheet().name));
            arrow->add_theme_font_size_override("font_size",14);arrow->connect("pressed",callable_mp(this,&CharacterCreationView::open_advancement).bind(member.id));}
        arrow->set_tooltip_text("Level up "+gs(member.character.sheet().name));
        const auto rect=list->get_item_rect(i);const float y=rect.position.y-list->get_v_scroll_bar()->get_value();
        const auto font=list->get_theme_font("font");const float width=Vector2(font->call("get_string_size",gs(member.character.sheet().name),0,-1,list->get_theme_font_size("font_size"))).x;
        arrow->set_position(Vector2(std::min(width+16,list->get_size().x-52),y));
        arrow->set_visible(campaign_->can_advance(member.id)&&y>=0&&y+26<=list->get_size().y);
    }
    for(int n=0;n<list->get_child_count();++n)if(auto* arrow=Object::cast_to<Button>(list->get_child(n));arrow&&String(arrow->get_name()).begins_with("Advance")){
        const auto id=String(arrow->get_name()).substr(7).to_int();
        if(std::none_of(roster.begin(),roster.end(),[&](const auto& m){return m.id==id;}))arrow->hide();
    }
}
void CharacterCreationView::open_advancement(std::int64_t id){
    if(campaign_defeated_||!campaign_->can_advance(id))return;
    if(auto* town=Object::cast_to<RolfTourView>(get_node_or_null("CampaignTown"));town&&town->is_visible()&&!town->can_leave())return;
    advancing_=id;advancement_options_=campaign_->advancement_options(id);advancement_choice_=campaign_->default_advancement(id);
    advancement_refreshing_=true;auto* window=get_node<Window>("LevelUp");
    window->get_node<Label>("Title")->set_text(gs(campaign_->member(id).character.sheet().name)+" / Level "+String::num_uint64(advancement_options_.level));
    auto* feat=window->get_node<OptionButton>("Feat");feat->clear();
    if(advancement_options_.feats.empty())feat->add_item("No feat or ability increase at this level");
    for(unsigned i=0;i<advancement_options_.feats.size();++i){const auto& option=advancement_options_.feats[i];feat->add_item(gs(option.label)+(option.available?"":" (Unavailable)"));feat->set_item_disabled(i,!option.available);feat->set_item_tooltip(i,gs(option.description));if(option.id==advancement_choice_.feat)feat->select(i);}
    feat->set_disabled(advancement_options_.feats.empty());
    for(unsigned i=0;i<6;++i)window->get_node<OptionButton>(String("Ability")+String::num_uint64(i))->select(advancement_choice_.abilities[i]);
    window->get_node<Label>("SpellLabel")->set_text(advancement_options_.spells.empty()?"No spell choices for this class":"Prepared spells: select at least one");
    for(unsigned i=0;i<4;++i){auto* spell=window->get_node<CheckBox>(String("Spell")+String::num_uint64(i));spell->set_visible(i<advancement_options_.spells.size());if(i>=advancement_options_.spells.size())continue;
        const auto& option=advancement_options_.spells[i];spell->set_text(gs(option.label)+(option.available?"":" (Unavailable)"));spell->set_tooltip_text(gs(option.description));spell->set_disabled(!option.available);spell->set_pressed_no_signal(std::find(advancement_choice_.spells.begin(),advancement_choice_.spells.end(),option.id)!=advancement_choice_.spells.end());}
    advancement_refreshing_=false;advancement_changed();window->popup_centered();window->get_node<Button>("Cancel")->grab_focus();
}
void CharacterCreationView::advancement_spell_changed(bool,int){advancement_changed();}
void CharacterCreationView::advancement_changed(std::int64_t){
    if(advancement_refreshing_||!advancing_)return;auto* window=get_node<Window>("LevelUp");
    if(!advancement_options_.feats.empty())advancement_choice_.feat=advancement_options_.feats.at(window->get_node<OptionButton>("Feat")->get_selected()).id;
    const bool ability=advancement_choice_.feat=="ability_score_improvement";
    for(unsigned i=0;i<6;++i){auto* points=window->get_node<OptionButton>(String("Ability")+String::num_uint64(i));points->set_disabled(!ability);if(!ability)points->select(0);advancement_choice_.abilities[i]=ability?points->get_selected():0;
        const auto value=campaign_->member(advancing_).character.sheet().scores[i];const std::array<const char*,6> labels{"STR","DEX","CON","INT","WIS","CHA"};
        window->get_node<Label>(String("AbilityLabel")+String::num_uint64(i))->set_text(String(labels[i])+" "+String::num_int64(value)+String::utf8(" → ")+String::num_int64(value+advancement_choice_.abilities[i]));}
    advancement_choice_.spells.clear();for(unsigned i=0;i<advancement_options_.spells.size();++i)if(window->get_node<CheckBox>(String("Spell")+String::num_uint64(i))->is_pressed())advancement_choice_.spells.push_back(advancement_options_.spells[i].id);
    try{const auto preview=campaign_->preview_advancement(advancing_,advancement_choice_);const auto& old=campaign_->member(advancing_);
        window->get_node<Label>("HP")->set_text("Maximum HP: "+String::num_int64(old.character.sheet().hit_points)+String::utf8(" → ")+String::num_int64(preview.character.sheet().hit_points)+"   /   Current HP: "+String::num_int64(preview.vitals.hit_points));
        window->get_node<Label>("Error")->set_text("");window->get_node<Button>("Confirm")->set_disabled(false);
    }catch(const std::exception& e){window->get_node<Label>("HP")->set_text("Choose valid options to preview your new HP.");window->get_node<Label>("Error")->set_text(gs(e.what()));window->get_node<Button>("Confirm")->set_disabled(true);}
}
void CharacterCreationView::close_advancement(){get_node<Window>("LevelUp")->hide();advancing_=0;}
void CharacterCreationView::confirm_advancement(){
    if(!advancing_)return;
    try{campaign_->advance(advancing_,advancement_choice_);close_advancement();refresh_party();refresh_advancement_arrows();
        if(auto* town=Object::cast_to<RolfTourView>(get_node_or_null("CampaignTown")))town->resume_party();
    }catch(const std::exception& e){get_node<Label>("LevelUp/Error")->set_text(gs(e.what()));}
}
void CharacterCreationView::advancement_check(){
    if(advancement_frames_>6000)throw std::runtime_error("Advancement check timed out");
    if(++advancement_frames_%4)return;
    refresh_advancement_arrows();
    const auto press=[&](const String& path){auto* button=get_node<Button>(path);if(button->is_disabled())throw std::runtime_error("Disabled advancement check control");button->emit_signal("pressed");};
    const auto arrow=[](opengold::MemberId id){return String("PartyPanel/Roster/Advance")+String::num_uint64(id);};
    const auto capture_dialog=[&](const char* name){if(!capture_)return;const auto image=get_node<Window>("LevelUp")->get_texture()->get_image();
        if(image.is_null()||image->save_png(ProjectSettings::get_singleton()->globalize_path(String("res://../user-data/")+name))!=OK)throw std::runtime_error("Level-up capture failed");};
    const auto select=[&](const String& path,int index){auto* option=get_node<OptionButton>(path);option->select(index);option->emit_signal("item_selected",index);};
    const auto id=campaign_->state().roster.empty()?0:campaign_->state().slots[0];
    switch(advancement_stage_){
    case 0:{
        for(const char* klass:{"wizard","fighter","cleric"}){opengold::rules::CharacterDraft draft;draft.race="human";draft.gender="female";draft.character_class=klass;draft.alignment="neutral_good";draft.background="sage";draft.name=std::string(klass==std::string_view("wizard")?"Mira":klass==std::string_view("fighter")?"Tessa":"Lena")+" / "+klass;draft.rolled=true;for(auto& roll:draft.rolls)roll={{6,5,4,1},3};campaign_->add_pc(opengold::Character(*opengold::srd5::character_rules(),draft,{}));}
        campaign_->award_experience(2700,"fixture:level-up-review");const auto slots=campaign_->state().slots;
        for(const auto member:slots)if(member)for(unsigned level=2;level<=3;++level)campaign_->advance(member,campaign_->default_advancement(member));
        party_action(0);error_="Review party: each character is ready for level 4. Click the arrow beside a name.";refresh_party();refresh_advancement_arrows();
        if(advancement_review_){advancement_review_=false;return;}break;}
    case 1:{
        auto* list=get_node<ItemList>("PartyPanel/Roster");
        for(unsigned i=0;i<3;++i){auto* button=get_node<Button>(arrow(campaign_->state().slots[i]));
            if(list->get_item_at_position(button->get_position()+button->get_size()/2,true)!=i)throw std::runtime_error("Level-up arrow is not beside its own character row");}
        capture("level-up-arrows.png");press(arrow(id));break;}
    case 2:{capture_dialog("level-up-wizard.png");const auto before=opengold::encode_campaign(*campaign_,nullptr,"ui-check");press("LevelUp/Cancel");
        if(opengold::encode_campaign(*campaign_,nullptr,"ui-check")!=before)throw std::runtime_error("Cancel mutated campaign");
        press(arrow(id));for(int i=0;i<6;++i)select(String("LevelUp/Ability")+String::num_int64(i),0);
        if(!get_node<Button>("LevelUp/Confirm")->is_disabled())throw std::runtime_error("Incomplete points can be confirmed");
        select("LevelUp/Ability2",2);get_node<CheckBox>("LevelUp/Spell1")->set_pressed(true);break;}
    case 3:capture_dialog("level-up-choices.png");press("LevelUp/Confirm");
        if(campaign_->member(id).character.sheet().level!=4||campaign_->member(id).character.sheet().prepared_spells.size()!=2||get_node<Button>(arrow(id))->is_visible())throw std::runtime_error("Wizard confirmation did not apply choices and hide arrow");
        party_action(7);break;
    case 4:{auto* town=get_node<RolfTourView>("CampaignTown");if(!town->can_leave()){auto* next=town->get_node<Button>("Continue");if(next->is_visible()&&!next->is_disabled())next->emit_signal("pressed");return;}town->resume_party();auto* button=town->get_node<Button>("PartyList/Rows/Member1/Advance");if(!button->is_visible())throw std::runtime_error("Town level-up arrow is missing");button->emit_signal("pressed");if(!get_node<Window>("LevelUp")->is_visible())throw std::runtime_error("Town arrow did not open advancement");select("LevelUp/Feat",1);break;}
    case 5:capture_dialog("level-up-fighter.png");press("LevelUp/Confirm");if(campaign_->member(campaign_->state().slots[1]).character.sheet().feats!=std::vector<std::string>{"defense"})throw std::runtime_error("Fighter feat was not applied");
        get_node<RolfTourView>("CampaignTown")->get_node<Button>("PartyList/Rows/Member2/Advance")->emit_signal("pressed");get_node<CheckBox>("LevelUp/Spell1")->set_pressed(true);break;
    case 6:capture_dialog("level-up-cleric.png");press("LevelUp/Confirm");party_action(9);capture("level-up-complete.png");
        UtilityFunctions::print("Godot advancement passed: roster and town arrows, HP preview, Cancel rollback, invalid-point prevention, level-four feat and spell confirmations.");advancement_check_=false;get_tree()->quit(0);break;
    }
    ++advancement_stage_;
}
