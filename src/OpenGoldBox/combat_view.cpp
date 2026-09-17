#include "godot_images.h"
#include "application_settings.h"
#include "localization.h"
#include "game_resources.h"
#include "combat_view.h"
#include "opengold/srd5.h"
#include "opengold/save_file.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/scroll_bar.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <array>
#include <cmath>
using namespace godot;using namespace opengold;using namespace opengold::rules;
namespace {
constexpr double combat_zoom=3.0;
String gs(std::string_view text){return String::utf8(text.data(),text.size());}
const std::array<std::pair<const char*,const char*>,10> action_buttons{{{"Melee","melee"},{"Ranged","ranged"},
    {"FireBolt","fire_bolt"},{"MagicMissile","magic_missile"},{"CureWounds","cure_wounds"},
    {"HealingWord","healing_word"},{"ScorchingRay","scorching_ray"},{"Dash","dash"},{"Dodge","dodge"},{"Disengage","disengage"}}};
std::string spell_verb(std::string verb,unsigned slot){if(slot==2&&(verb=="magic_missile"||verb=="cure_wounds"||verb=="healing_word"))verb+="_2";return verb;}
}
void CombatView::_bind_methods(){}
void CombatView::_notification(int what){if(what==NOTIFICATION_RESIZED&&ready_){layout();queue_redraw();}}
std::filesystem::path CombatView::local_path(const char* path) const
{return std::filesystem::u8path(ProjectSettings::get_singleton()->globalize_path(path).utf8().get_data());}
void CombatView::_ready()
{
    i18n::prepare_ui(*this);
    get_node<Control>("BattlefieldScroll/Canvas")->connect("draw",callable_mp(this,&CombatView::draw_battlefield));
    ready_=true;get_window()->set_min_size(Vector2i(1120,800));set_texture_filter(TEXTURE_FILTER_NEAREST);layout();
    if(Engine::get_singleton()->is_editor_hint())return;
    for(const auto& [node,verb]:action_buttons)
        get_node<Button>(node)->connect("pressed",callable_mp(this,&CombatView::select_mode).bind(String(verb)));
    get_node<Button>("Move")->connect("pressed",callable_mp(this,&CombatView::select_mode).bind(String("move")));
    get_node<Button>("SpellSlot")->connect("pressed",callable_mp(this,&CombatView::spell_slot));
    get_node<Button>("SecondWind")->connect("pressed",callable_mp(this,&CombatView::immediate).bind(String("second_wind")));
    get_node<Button>("End")->connect("pressed",callable_mp(this,&CombatView::immediate).bind(String("end")));
    get_node<Button>("React")->connect("pressed",callable_mp(this,&CombatView::immediate).bind(String("opportunity")));
    get_node<Button>("Decline")->connect("pressed",callable_mp(this,&CombatView::immediate).bind(String("decline")));
    get_node<Button>("Training")->connect("pressed",callable_mp(this,&CombatView::training));
    get_node<Button>("Slums")->connect("pressed",callable_mp(this,&CombatView::slums));
    get_node<Button>("Replay")->connect("pressed",callable_mp(this,&CombatView::replay));
    get_node<Button>("Continue")->connect("pressed",callable_mp(this,&CombatView::next));
    get_node<Button>("Revisit")->connect("pressed",callable_mp(this,&CombatView::revisit));
    get_node<Button>("Save")->connect("pressed",callable_mp(this,&CombatView::save_game));
    get_node<Button>("Load")->connect("pressed",callable_mp(this,&CombatView::load_game));
    const auto args=OS::get_singleton()->get_cmdline_user_args();checking_=args.has("--combat-check");capture_=args.has("--capture");check_slums_=args.has("--slums");
    party_check_=campaign_&&args.has("--party-check");
    expedition_check_=campaign_&&args.has("--expedition-check");party_check_|=expedition_check_;
    defeat_check_=campaign_&&args.has("--defeat-check");
    try{demo_=std::make_unique<CombatDemo>(srd5::load(std::filesystem::u8path(game_rules_file().utf8().get_data())));if(campaign_)demo_->campaign_party(campaign_);if(encounter_)demo_->encounter(*encounter_,42);else if(check_slums_)slums();else training();sync_art();layout();refresh();
        get_node<Label>("Help")->set_text(i18n::text(N_("Teal: party | Orange: enemies\nWheel: scroll | Shift+wheel: sideways\nMiddle-drag: pan | Scrollbars: navigate")));
        if(campaign_)for(const char* name:{"Training","Slums","Replay","Save","Load","Revisit"})get_node<Control>(name)->hide();
        if(encounter_){get_node<Label>("Title")->set_text(i18n::text(N_("SLUMS / Combat")));get_node<Label>("Subtitle")->set_text(i18n::text(N_("Choose an action, then click its target. Enter ends your turn.")));
            get_node<Label>("Footer")->set_text(i18n::text(N_("Each square is 5 feet. Victory returns your party to exploration.")));}
    }
    catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::layout()
{
    followed_.reset();
    const double width=get_size().x,height=get_size().y,sidebar=358,left_width=width-sidebar-72;
    const auto board=demo_&&demo_->has_combat()?demo_->combat().snapshot().battlefield:Battlefield{12,9,{}};
    const double tile=std::min(left_width/board.width,(height-280)/board.height);
    board_rect_=Rect2(24,116,tile*board.width,tile*board.height);const double right=width-sidebar-24;
    auto* scroll=get_node<ScrollContainer>("BattlefieldScroll");
    for(int i=0;i<scroll->get_child_count(true);++i) {
        if(auto* bar=Object::cast_to<ScrollBar>(scroll->get_child(i,true)))bar->set_focus_mode(FOCUS_ALL);
    }
    scroll->set_position(board_rect_.position);scroll->set_size(board_rect_.size);
    get_node<Control>("BattlefieldScroll/Canvas")->set_custom_minimum_size(board_rect_.size*combat_zoom);
    get_node<Control>("BattlefieldScroll/Canvas")->queue_redraw();
    const auto place=[&](const char* name,Rect2 rect){auto* node=get_node<Control>(name);node->set_position(rect.position);node->set_size(rect.size);};
    place("Title",Rect2(24,18,left_width,34));place("Subtitle",Rect2(24,62,left_width,45));
    place("Training",Rect2(right,20,112,34));place("Slums",Rect2(right+120,20,112,34));place("Replay",Rect2(right+240,20,118,34));
    place("Turn",Rect2(right,70,sidebar,70));place("Roster",Rect2(right,148,sidebar,160));
    place("Prompt",Rect2(right,318,sidebar,46));
    unsigned index=0;
    for(const char* name:{"Move","Melee","Ranged","FireBolt","MagicMissile","CureWounds","HealingWord","ScorchingRay","SpellSlot","SecondWind","Dash","Dodge","Disengage","End","Continue"}) {
        const unsigned row=index/3,column=index%3;place(name,Rect2(right+column*122,370+row*43,114,36));++index;
    }
    place("React",Rect2(right,590,174,36));place("Decline",Rect2(right+184,590,174,36));
    place("Save",Rect2(right,639,112,34));place("Load",Rect2(right+122,639,112,34));place("Revisit",Rect2(right+244,639,114,34));
    place("Help",Rect2(right,686,sidebar,height-732));
    place("Log",Rect2(24,board_rect_.get_end().y+16,left_width,height-board_rect_.get_end().y-64));
    place("Footer",Rect2(24,height-34,width-48,24));
}
void CombatView::training(){try{error_.clear();demo_->training();art_.clear();mode_="move";refresh();}catch(const std::exception& e){error_=e.what();refresh();}}
void CombatView::slums()
{
    try{error_.clear();const auto directory=settings::game_path();
        demo_->slums(std::filesystem::u8path(directory.utf8().get_data()));mode_="move";sync_art();refresh();
    }catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::replay(){if(demo_&&demo_->is_slums())slums();else if(demo_)training();}
void CombatView::next(){try{if(demo_){demo_->continue_script();sync_art();refresh();}}catch(const std::exception& e){error_=e.what();refresh();}}
void CombatView::revisit(){try{demo_->revisit();refresh();}catch(const std::exception& e){error_=e.what();refresh();}}
void CombatView::sync_art()
{
    art_.clear();terrain_art_.clear();if(!demo_)return;
    for(const auto& source:demo_->terrain_art()){
        terrain_art_.push_back(presentation::image_texture(source));
    }
    for(const auto& source:demo_->art()) {
        art_[source.entity]=presentation::image_texture(source.image);
    }
    for(const auto& source:campaign_art_){
        art_[source.entity]=presentation::image_texture(source.image);}
}
void CombatView::save_game()
{
    try{write_save_file(local_path("user://checks/combat.save"),demo_->save_combat(),65536);
        error_.clear();get_node<Label>("Prompt")->set_text(i18n::text(N_("Training combat saved.")));
    }catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::load_game()
{
    try{const auto bytes=read_save_file(local_path("user://checks/combat.save"),65536);
        demo_->restore_combat(bytes);error_.clear();refresh();
    }catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::select_mode(String verb)
{
    mode_=spell_verb(verb.utf8().get_data(),spell_slot_);if(mode_=="dash"||mode_=="dodge"||mode_=="disengage"){immediate(verb);return;}refresh();
}
void CombatView::spell_slot(){spell_slot_=spell_slot_==1?2:1;mode_="move";refresh();}
void CombatView::immediate(String verb)
{
    if(!demo_||!demo_->has_combat())return;const auto wanted=std::string(verb.utf8().get_data());
    const auto state=demo_->combat().snapshot();
    if(std::none_of(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==state.actor&&a.side==0;}))return;
    for(const auto& c:demo_->combat().legal_commands())if(c.verb==wanted){act(c);return;}
}
void CombatView::act(const Command& command)
{
    try{if(demo_->submit(command)){mode_="move";ai_delay_=0;error_.clear();refresh();}}
    catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::_input(const Ref<InputEvent>& event)
{
    if(!is_visible_in_tree()||!demo_||Engine::get_singleton()->is_editor_hint())return;
    const Ref<InputEventKey> key=event;
    if(!defeated()&&key.is_valid()&&key->is_pressed()&&!key->is_echo()&&key->get_keycode()==Key::KEY_ENTER) {
        if(demo_->waiting())next();else immediate("end");get_viewport()->set_input_as_handled();return;
    }
    if(!demo_->has_combat())return;
    auto* scroll=get_node<ScrollContainer>("BattlefieldScroll");
    if(key.is_valid()&&key->is_pressed()) {
        bool focused=scroll->has_focus();
        for(int i=0;i<scroll->get_child_count(true);++i) {
            if(auto* bar=Object::cast_to<ScrollBar>(scroll->get_child(i,true)))focused|=bar->has_focus();
        }
        if(focused) {
            const int step=std::max(1,static_cast<int>(combat_zoom*board_rect_.size.x/demo_->combat().snapshot().battlefield.width));
            switch(key->get_keycode()) {
            case Key::KEY_LEFT:scroll->set_h_scroll(scroll->get_h_scroll()-step);break;
            case Key::KEY_RIGHT:scroll->set_h_scroll(scroll->get_h_scroll()+step);break;
            case Key::KEY_UP:scroll->set_v_scroll(scroll->get_v_scroll()-step);break;
            case Key::KEY_DOWN:scroll->set_v_scroll(scroll->get_v_scroll()+step);break;
            default:return;
            }
            get_viewport()->set_input_as_handled();return;
        }
    }
    const Ref<InputEventMouseMotion> motion=event;
    if(motion.is_valid()&&panning_) {
        if(!motion->get_button_mask().has_flag(MouseButtonMask::MOUSE_BUTTON_MASK_MIDDLE)){panning_=false;return;}
        const auto inverse=get_global_transform_with_canvas().affine_inverse();
        const auto delta=inverse.basis_xform(motion->get_relative());
        scroll->set_h_scroll(scroll->get_h_scroll()-static_cast<int>(delta.x));
        scroll->set_v_scroll(scroll->get_v_scroll()-static_cast<int>(delta.y));
        get_viewport()->set_input_as_handled();return;
    }
    const Ref<InputEventMouseButton> mouse=event;
    if(mouse.is_null())return;
    if(mouse->get_button_index()==MouseButton::MOUSE_BUTTON_MIDDLE&&!mouse->is_pressed()){panning_=false;return;}
    const auto local=get_global_transform_with_canvas().affine_inverse().xform(mouse->get_position());
    // Keep clicks on the scrollbars out of combat targeting.
    if(!board_rect_.has_point(local))return;
    for(int i=0;i<scroll->get_child_count(true);++i) {
        auto* bar=Object::cast_to<ScrollBar>(scroll->get_child(i,true));
        if(bar&&bar->is_visible()&&Rect2(Vector2(),bar->get_size()).has_point(bar->get_global_transform_with_canvas().affine_inverse().xform(mouse->get_position())))return;
    }
    if(mouse->get_button_index()==MouseButton::MOUSE_BUTTON_MIDDLE&&mouse->is_pressed()) {
        panning_=true;scroll->grab_focus();get_viewport()->set_input_as_handled();return;
    }
    if(defeated()||!mouse->is_pressed()||mouse->get_button_index()!=MouseButton::MOUSE_BUTTON_LEFT)return;
    const auto s=demo_->combat().snapshot();const auto current=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==s.actor;});
    if(current==s.combatants.end()||current->side!=0)return;
    const auto canvas=get_node<Control>("BattlefieldScroll/Canvas")->get_global_transform_with_canvas().affine_inverse().xform(mouse->get_position());
    const auto relative=canvas/(combat_zoom*board_rect_.size.x/s.battlefield.width);
    const Cell cell{static_cast<int>(std::floor(relative.x)),static_cast<int>(std::floor(relative.y))};
    for(const auto& c:demo_->combat().legal_commands())if(c.verb==mode_) {
        if(c.verb=="move"&&c.destination==cell){act(c);break;}
        if(c.target) {const auto target=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==c.target;});if(target!=s.combatants.end()&&target->cell==cell){act(c);break;}}
    }
    get_viewport()->set_input_as_handled();
}
void CombatView::refresh()
{
    if(!ready_)return;
    const bool loaded=demo_&&demo_->has_combat();Snapshot s;if(loaded)s=demo_->combat().snapshot();
    bool player=false;String turn=i18n::text(demo_?demo_->status():N_("Unable to load rules"));
    if(loaded&&s.outcome==Outcome::ongoing)for(const auto& a:s.combatants)if(a.id==s.actor) {
        player=a.side==0;
        turn=i18n::format(s.reaction_pending?N_("Round {round} / {name} reaction\nMove {feet} ft | {action}"):N_("Round {round} / {name} turn\nMove {feet} ft | {action}"),
            {{"round",s.round},{"name",gs(a.name)},{"feet",a.movement_feet},{"action",i18n::text(a.action?N_("Action ready"):N_("Action spent"))}});
        turn+="\n"+(a.status_messages.empty()?i18n::text(a.status):i18n::render(a.status_messages));
    }
    if(loaded&&s.outcome!=Outcome::ongoing)turn=i18n::text(s.outcome==Outcome::victory?N_("Victory"):N_("Party incapacitated / defeat"));
    get_node<Label>("Turn")->set_text(turn);
    String roster;for(const auto& a:s.combatants)roster+=String(a.id==s.actor?"> ":"  ")+i18n::format("{id} {name}  {current}/{maximum} HP  AC {ac}\n",
        {{"id",a.id},{"name",gs(a.name)},{"current",a.hit_points},{"maximum",a.max_hit_points},{"ac",a.armor_class}});
    get_node<RichTextLabel>("Roster")->set_text(roster);
    const auto offered=loaded?demo_->combat().legal_commands():std::vector<Command>{};
    const auto enabled=[&](std::string_view verb){return player&&std::any_of(offered.begin(),offered.end(),[&](const auto& c){return c.verb==verb;});};
    for(const auto& [node,verb]:action_buttons)get_node<Button>(node)->set_disabled(!enabled(spell_verb(verb,spell_slot_)));
    get_node<Button>("SpellSlot")->set_text(i18n::format("Slot level {level}",{{"level",spell_slot_}}));
    get_node<Button>("SpellSlot")->set_disabled(!enabled("magic_missile")&&!enabled("magic_missile_2")&&!enabled("cure_wounds")&&!enabled("cure_wounds_2")&&!enabled("healing_word")&&!enabled("healing_word_2"));
    for(const auto& [node,verb]:std::array<std::pair<const char*,const char*>,5>{{{"Move","move"},{"End","end"},{"SecondWind","second_wind"},{"React","opportunity"},{"Decline","decline"}}})
        get_node<Button>(node)->set_disabled(!enabled(verb));
    get_node<Button>("Continue")->set_disabled(!demo_||!demo_->waiting());
    get_node<Button>("Save")->set_disabled(!loaded||demo_->is_slums());get_node<Button>("Load")->set_disabled(!loaded||demo_->is_slums());
    get_node<Button>("Revisit")->set_disabled(!loaded||!demo_->script_complete()||s.outcome!=Outcome::victory);
    String action=i18n::text("Move");
    for(const auto& command:offered)if(command.verb==mode_){action=i18n::text(command.label);break;}
    get_node<Label>("Prompt")->set_text(!error_.empty()?i18n::text(error_):demo_&&demo_->waiting()?i18n::text("Read the encounter text, then Continue."):
        loaded&&s.outcome!=Outcome::ongoing?i18n::text(demo_->status()):s.reaction_pending?i18n::text("Use or decline the opportunity attack."):
        player?i18n::format("Selected: {action}. Click a highlighted square.",{{"action",action}}):i18n::text("Enemy turn"));
    String log=demo_?i18n::campaign("por/combat/dialogue",demo_->dialogue())+"\n\n":String();
    if(s.log_messages.size()==s.log.size())for(const auto& entry:s.log_messages)log+=i18n::render(entry)+"\n";
    else for(const auto& entry:s.log)log+=i18n::text(entry)+"\n";
    if(!error_.empty())log+="\n"+i18n::text(error_);
    get_node<RichTextLabel>("Log")->set_text(log);get_node<RichTextLabel>("Log")->scroll_to_line(std::max(0,get_node<RichTextLabel>("Log")->get_line_count()-1));
    get_node<Control>("BattlefieldScroll/Canvas")->queue_redraw();
    queue_redraw();
}
void CombatView::center_on(Cell cell)
{
    auto* scroll=get_node<ScrollContainer>("BattlefieldScroll");
    const double tile=combat_zoom*board_rect_.size.x/demo_->combat().snapshot().battlefield.width;
    scroll->set_h_scroll(static_cast<int>((cell.x+.5)*tile-scroll->get_size().x*.5));
    scroll->set_v_scroll(static_cast<int>((cell.y+.5)*tile-scroll->get_size().y*.5));
}
void CombatView::_draw()
{
    draw_rect(Rect2(Vector2(),get_size()),Color("121a20"));draw_rect(board_rect_,Color("202d33"));
}
void CombatView::draw_battlefield()
{
    if(!demo_||!demo_->has_combat())return;
    auto* canvas=get_node<Control>("BattlefieldScroll/Canvas");
    canvas->draw_set_transform(Vector2(),0,Vector2(combat_zoom,combat_zoom));
    const auto s=demo_->combat().snapshot();const double tile=board_rect_.size.x/s.battlefield.width;const auto font=get_theme_default_font();
    for(int y=0;y<s.battlefield.height;++y)for(int x=0;x<s.battlefield.width;++x) {
        const Rect2 cell(Vector2(x*tile,y*tile),Vector2(tile,tile));
        const auto terrain=s.battlefield.at({x,y});canvas->draw_rect(cell,terrain==1?Color("64716d"):terrain==2?Color("665238"):((x+y)%2?Color("29373c"):Color("253137")));
        const auto index=y*s.battlefield.width+x;
        if(index<demo_->battlefield_tiles().size()&&demo_->battlefield_tiles()[index]<terrain_art_.size())canvas->draw_texture_rect(terrain_art_[demo_->battlefield_tiles()[index]],cell,false);
        else canvas->draw_rect(cell,Color("172228"),false);
    }
    const auto active=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==s.actor;});
    if(active!=s.combatants.end()&&active->side==0)for(const auto& c:demo_->combat().legal_commands())if(c.verb==mode_) {
        auto p=c.destination;if(c.target){const auto target=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==c.target;});if(target==s.combatants.end())continue;p=target->cell;}
        canvas->draw_rect(Rect2(Vector2(p.x*tile+2,p.y*tile+2),Vector2(tile-4,tile-4)),Color(.4,.8,.75,.17));
    }
    for(const auto& a:s.combatants) {
        const auto center=Vector2((a.cell.x+.5)*tile,(a.cell.y+.5)*tile);
        const auto color=a.side==0?Color("79d6d4"):Color("dd9874");
        canvas->draw_circle(center,tile*.34,a.conscious?color:Color("51595b"));
        if(a.id==s.actor&&s.outcome==Outcome::ongoing)canvas->draw_arc(center,tile*.42,0,6.283185,32,Color("e6c28a"),2);
        if(art_.contains(a.id)) {
            const auto texture=art_.at(a.id);const double scale=tile*.9/std::max(texture->get_width(),texture->get_height());
            const Vector2 size(texture->get_width()*scale,texture->get_height()*scale);canvas->draw_texture_rect(texture,Rect2(center-size*.5,size),false,a.conscious?Color(1,1,1):Color(.5,.5,.5));
        } else {
            const auto number=std::to_string(a.id);
            auto cursor=center+Vector2(-5.5*number.size(),7);
            for(const char digit:number) {
                canvas->draw_char(font,cursor,gs(std::string(1,digit)),20,Color("142027"));
                cursor.x+=11;
            }
        }
        canvas->draw_rect(Rect2(center+Vector2(-tile*.35,tile*.38),Vector2(tile*.7,4)),Color("101719"));
        canvas->draw_rect(Rect2(center+Vector2(-tile*.35,tile*.38),Vector2(tile*.7*a.hit_points/a.max_hit_points,4)),color);
    }
}
void CombatView::_process(double delta)
{
    if(Engine::get_singleton()->is_editor_hint())return;
    try {
        if((checking_||expedition_check_)&&!error_.empty())throw std::runtime_error(error_);
        if(!demo_)return;
        if(checking_&&demo_->waiting()){next();return;}
        if(!demo_->has_combat())return;const auto s=demo_->combat().snapshot();
        for(const auto& a:s.combatants)if(a.id==s.actor) {
            const auto current=std::pair{a.id,a.cell};
            if(followed_!=current){center_on(a.cell);followed_=current;}
        }
        if((checking_||expedition_check_)&&capture_&&!captured_) {
            if(++completion_frames_<3)return;completion_frames_=0;
            const auto file=local_path(expedition_check_?"user://checks/slums-battlefield.png":check_slums_?"user://checks/slums-combat.png":"user://checks/training-combat.png");std::filesystem::create_directories(file.parent_path());
            const auto image=get_viewport()->get_texture()->get_image();if(image.is_null()||image->save_png(gs(file.generic_string()))!=OK)throw std::runtime_error("Combat capture failed");captured_=true;
        }
        if(s.outcome!=Outcome::ongoing) {
            if(checking_&&++completion_frames_>2) {
                if(check_slums_){if(!demo_->script_complete())throw std::runtime_error("Original ECL did not finish");if(s.outcome==Outcome::victory)demo_->revisit();}
                UtilityFunctions::print("Godot C++ combat check passed: ",check_slums_?"Slums":"training",", commands ",check_steps_,", outcome ",s.outcome==Outcome::victory?"victory":"defeat");checking_=false;get_tree()->quit(0);
            }return;
        }
        const auto active=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==s.actor;});
        if(checking_&&active->side==1) {
            Ref<InputEventKey> key;key.instantiate();key->set_keycode(Key::KEY_ENTER);key->set_pressed(true);_input(key);
            if(demo_->combat().snapshot().revision!=s.revision)throw std::runtime_error("Keyboard skipped enemy turn");
        }
        if(checking_&&!checked_input_&&active->side==0&&!s.reaction_pending) {
            const auto command=choose_demo_command(demo_->combat());
            if(command.target)for(const auto& [node,verb]:action_buttons)if(command.verb==verb) {
                get_node<Button>(node)->emit_signal("pressed");
                const auto target=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==command.target;});
                Ref<InputEventMouseButton> mouse;mouse.instantiate();mouse->set_button_index(MouseButton::MOUSE_BUTTON_LEFT);mouse->set_pressed(true);
                if(!check_target_centered_) {
                    center_on(target->cell);check_target_centered_=true;
                    return; // ScrollContainer applies the canvas offset during its layout pass.
                }
                mouse->set_position(get_node<Control>("BattlefieldScroll/Canvas")->get_global_transform_with_canvas().xform(Vector2(target->cell.x+.5,target->cell.y+.5)*(combat_zoom*board_rect_.size.x/s.battlefield.width)));
                get_viewport()->push_input(mouse,true);
                if(demo_->combat().snapshot().revision!=s.revision+1)throw std::runtime_error("Action button/target click did not submit command");
                checked_input_=true;++check_steps_;return;
            }
        }
        if(checking_||party_check_||defeat_check_||active->side==1) {
            ai_delay_+=delta;if(!checking_&&!party_check_&&!defeat_check_&&ai_delay_<.65)return;ai_delay_=0;
            if(defeat_check_){
                if(++check_steps_>2000)throw std::runtime_error("Defeat check command limit exceeded");
                if(active->side==0){
                    const auto offered=demo_->combat().legal_commands();
                    const auto pass=std::find_if(offered.begin(),offered.end(),[&](const auto& c){return c.verb==(s.reaction_pending?"decline":"end");});
                    if(pass==offered.end())throw std::runtime_error("Defeat check cannot pass party turn");
                    act(*pass);return;
                }
            }
            if(checking_&&++check_steps_>1000) {
                std::string details="Combat check command limit exceeded";
                for(const auto& line:s.log)details+="\n"+line;
                throw std::runtime_error(details);
            }
            act(choose_demo_command(demo_->combat()));
        }
    }catch(const std::exception& e){error_=e.what();refresh();if(checking_||defeat_check_||expedition_check_){UtilityFunctions::push_error(gs(error_));checking_=false;get_tree()->quit(1);}}
}
