#include "godot_images.h"
#include "application_settings.h"
#include "localization.h"
#include "game_resources.h"
#include "combat_view.h"
#include "combat_sprite_layout.h"
#include "godot_sound_output.h"
#include "opengold/srd5.h"
#include "opengold/character_art.h"
#include "opengold/formats.h"
#include "opengold/save_file.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/scroll_bar.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
using namespace godot;using namespace opengold;using namespace opengold::rules;
namespace {
String gs(std::string_view text){return String::utf8(text.data(),text.size());}
const std::array<std::pair<const char*,const char*>,11> action_buttons{{{"Melee","melee"},{"Ranged","ranged"},
    {"FireBolt","fire_bolt"},{"MagicMissile","magic_missile"},{"CureWounds","cure_wounds"},
    {"HealingWord","healing_word"},{"ScorchingRay","scorching_ray"},{"Blindness","blindness"},{"Dash","dash"},{"Dodge","dodge"},{"Disengage","disengage"}}};
std::string spell_verb(std::string verb,unsigned slot){if(slot==2&&(verb=="magic_missile"||verb=="cure_wounds"||verb=="healing_word"))verb+="_2";return verb;}
std::optional<Cell> movement_direction(Key key,bool shift)
{
    switch(key){
    case Key::KEY_UP:return shift?Cell{1,-1}:Cell{0,-1};
    case Key::KEY_RIGHT:return shift?Cell{1,1}:Cell{1,0};
    case Key::KEY_DOWN:return shift?Cell{-1,1}:Cell{0,1};
    case Key::KEY_LEFT:return shift?Cell{-1,-1}:Cell{-1,0};
    case Key::KEY_KP_7:return Cell{-1,-1};
    case Key::KEY_KP_8:return Cell{0,-1};
    case Key::KEY_KP_9:return Cell{1,-1};
    case Key::KEY_KP_4:return Cell{-1,0};
    case Key::KEY_KP_6:return Cell{1,0};
    case Key::KEY_KP_1:return Cell{-1,1};
    case Key::KEY_KP_2:return Cell{0,1};
    case Key::KEY_KP_3:return Cell{1,1};
    default:return std::nullopt;
    }
}
}
void CombatView::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("selected_character_id"),&CombatView::selected_character_id);
    ClassDB::bind_method(D_METHOD("selected_character_cell"),&CombatView::selected_character_cell);
    ClassDB::bind_method(D_METHOD("attack_pose_active","id"),&CombatView::attack_pose_active);
}
Vector2i CombatView::selected_character_cell() const
{
    if(!demo_||!demo_->has_combat())return {-1,-1};
    const auto state=demo_->combat().snapshot();
    const auto selected=std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==selected_;});
    return selected==state.combatants.end()?Vector2i(-1,-1):Vector2i(selected->cell.x,selected->cell.y);
}
void CombatView::prepare_combat()
{
    if(demo_)return;
    auto next=std::make_unique<CombatDemo>(srd5::load(std::filesystem::u8path(game_rules_file().utf8().get_data())));
    const bool demo_mode=settings::flag("--combat-demo");
    if(demo_mode){
        const auto directory=std::filesystem::u8path(settings::game_path().utf8().get_data());
        auto characters=srd5::character_rules();
        auto showcase=make_combat_demo(srd5::load(std::filesystem::u8path(game_rules_file().utf8().get_data())),*characters,directory);
        campaign_=std::move(showcase.party);
        encounter_=std::move(showcase.encounter);
    }
    if(campaign_)next->campaign_party(campaign_);
    if(encounter_)next->encounter(*encounter_,42);
    else if(OS::get_singleton()->get_cmdline_user_args().has("--slums"))
        next->slums(std::filesystem::u8path(settings::game_path().utf8().get_data()));
    else next->training(settings::flag("--conditions")?3:42,settings::flag("--conditions"));
    demo_=std::move(next);sync_art();
}
void CombatView::_notification(int what){if(what==NOTIFICATION_RESIZED&&ready_){layout();queue_redraw();}}
std::filesystem::path CombatView::local_path(const char* path) const
{return std::filesystem::u8path(ProjectSettings::get_singleton()->globalize_path(path).utf8().get_data());}
void CombatView::_ready()
{
    combat_zoom_=settings::combat_zoom_percent()/100.0;
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
    get_node<Button>("ZoomOut100")->connect("pressed",callable_mp(this,&CombatView::adjust_zoom).bind(-100));
    get_node<Button>("ZoomOut10")->connect("pressed",callable_mp(this,&CombatView::adjust_zoom).bind(-10));
    get_node<Button>("ZoomIn10")->connect("pressed",callable_mp(this,&CombatView::adjust_zoom).bind(10));
    get_node<Button>("ZoomIn100")->connect("pressed",callable_mp(this,&CombatView::adjust_zoom).bind(100));
    const auto args=OS::get_singleton()->get_cmdline_user_args();checking_=args.has("--combat-check");capture_=args.has("--capture");check_slums_=args.has("--slums");
    party_check_=campaign_&&args.has("--party-check");
    expedition_check_=campaign_&&args.has("--expedition-check");party_check_|=expedition_check_;
    defeat_check_=campaign_&&args.has("--defeat-check");
    try{prepare_combat();layout();refresh();
        const auto directory=std::filesystem::u8path(settings::game_path().utf8().get_data());
        if(std::filesystem::is_directory(directory)){
            attack_sound_=std::make_unique<por::SoundPlayer>(por::SoundBank::load(directory),
                std::make_unique<GodotSoundOutput>(*get_node<AudioStreamPlayer>("AttackAudio")));
            effect_sound_=std::make_unique<por::SoundPlayer>(por::SoundBank::load(directory),
                std::make_unique<GodotSoundOutput>(*get_node<AudioStreamPlayer>("EffectAudio")));
            attack_sound_->set_volume(0.125);
            effect_sound_->set_volume(0.125);
        }
        get_node<Label>("Help")->set_text(i18n::text(N_("Teal: party | Orange: enemies\nWheel: scroll | Shift+wheel: sideways\nMiddle-drag: pan | Scrollbars: navigate")));
        if(campaign_)for(const char* name:{"Training","Slums","Replay","Save","Load","Revisit"})get_node<Control>(name)->hide();
        get_node<Label>("Footer")->set_text(i18n::text(N_("Arrows/Numpad: move | Shift+arrow: diagonal | A: action | Space: use | Z: slot | Enter: end")));
        for(const char* name:{"Turn","Roster","Prompt","Help"})get_node<Control>(name)->hide();
        for(const char* name:{"Training","Slums","Replay","Move","Melee","Ranged","FireBolt","MagicMissile","CureWounds","HealingWord","ScorchingRay","Blindness","SpellSlot","SecondWind","Dash","Dodge","Disengage","End","Continue","React","Decline","Save","Load","Revisit"})get_node<Control>(name)->hide();
    }
    catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::layout()
{
    followed_.reset();
    const double width=get_size().x,height=get_size().y,sidebar=358,left_width=width-sidebar-72;
    const auto board=demo_&&demo_->has_combat()?demo_->combat().snapshot().battlefield:Battlefield{12,9,{}};
    const double battlefield_height=(height-180)*.85;
    base_tile_=std::max(left_width/board.width,battlefield_height/board.height);
    board_rect_=Rect2(24,16,left_width,battlefield_height);const double right=width-sidebar-24;
    auto* scroll=get_node<ScrollContainer>("BattlefieldScroll");
    for(int i=0;i<scroll->get_child_count(true);++i) {
        if(auto* bar=Object::cast_to<ScrollBar>(scroll->get_child(i,true)))bar->set_focus_mode(FOCUS_ALL);
    }
    scroll->set_position(board_rect_.position);scroll->set_size(board_rect_.size);
    get_node<Control>("BattlefieldScroll/Canvas")->set_custom_minimum_size(Vector2(base_tile_*board.width,base_tile_*board.height)*combat_zoom_);
    get_node<Control>("BattlefieldScroll/Canvas")->queue_redraw();
    const auto place=[&](const char* name,Rect2 rect){auto* node=get_node<Control>(name);node->set_position(rect.position);node->set_size(rect.size);};
    place("Training",Rect2(right,20,112,34));place("Slums",Rect2(right+120,20,112,34));place("Replay",Rect2(right+240,20,118,34));
    place("Turn",Rect2(right,70,sidebar,70));place("Roster",Rect2(right,148,sidebar,160));
    place("Prompt",Rect2(right,318,sidebar,46));
    unsigned index=0;
    for(const char* name:{"Move","Melee","Ranged","FireBolt","MagicMissile","CureWounds","HealingWord","ScorchingRay","Blindness","SpellSlot","SecondWind","Dash","Dodge","Disengage","End"}) {
        const unsigned row=index/3,column=index%3;place(name,Rect2(right+column*122,370+row*39,114,36));++index;
    }
    place("Continue",Rect2(right+244,526,114,36));
    place("React",Rect2(right,570,174,36));place("Decline",Rect2(right+184,570,174,36));
    place("Save",Rect2(right,614,112,34));place("Load",Rect2(right+122,614,112,34));place("Revisit",Rect2(right+244,614,114,34));
    place("ZoomLevel",Rect2(right,16,66,34));
    for(unsigned i=0;i<4;++i)place(std::array<const char*,4>{"ZoomOut100","ZoomOut10","ZoomIn10","ZoomIn100"}[i],Rect2(right+70+i*72,16,68,34));
    const int zoom_percent=static_cast<int>(std::lround(combat_zoom_*100));
    get_node<Label>("ZoomLevel")->set_text(String::num_int64(zoom_percent)+"%");
    get_node<Button>("ZoomOut100")->set_disabled(zoom_percent<=10);
    get_node<Button>("ZoomOut10")->set_disabled(zoom_percent<=10);
    get_node<Button>("ZoomIn10")->set_disabled(zoom_percent>=1000);
    get_node<Button>("ZoomIn100")->set_disabled(zoom_percent>=1000);
    place("Help",Rect2(right,700,sidebar,height-746));
    place("Log",Rect2(24,board_rect_.get_end().y+16,left_width,height-board_rect_.get_end().y-64));
    place("Footer",Rect2(24,height-34,width-48,24));
    layout_status();
}
void CombatView::layout_status()
{
    // Let the translated status summary determine its height. The roster keeps
    // the remaining space above the action prompt and scrolls when necessary.
    auto* turn=get_node<Label>("Turn");turn->set_size(Vector2(358,0));
    auto* roster=get_node<RichTextLabel>("Roster");
    const double top=std::max(148.0,double(turn->get_position().y+turn->get_size().y+8));
    roster->set_position(Vector2(turn->get_position().x,top));roster->set_size(Vector2(358,std::max(0.0,308-top)));
}
void CombatView::training(){try{error_.clear();demo_->training(settings::flag("--conditions")?3:42,settings::flag("--conditions"));sync_art();mode_="move";refresh();}catch(const std::exception& e){error_=e.what();refresh();}}
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
    art_.clear();portraits_.clear();terrain_art_.clear();skull_art_.unref();known_dead_.clear();skull_seconds_.clear();action_seconds_.clear();if(!demo_)return;
    const auto directory=std::filesystem::u8path(settings::game_path().utf8().get_data());
    if(std::filesystem::is_directory(directory)){
        for(const auto& file:std::filesystem::directory_iterator(directory)){
            auto name=file.path().filename().string();
            for(auto& c:name)if(c>='a'&&c<='z')c-=32;
            if(name!="COMSPR.DAX")continue;
            if(std::filesystem::file_size(file.path())>32*1024*1024)throw std::runtime_error("Combat effect archive exceeds limit");
            std::ifstream input(file.path(),std::ios::binary);
            std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input),{}};
            if(input.bad())throw std::runtime_error("Cannot read combat effect art");
            auto decoded=decode_ega_combat_icon(bytes,11,0);
            if(!decoded)throw std::runtime_error("Cannot decode original skull combat effect");
            skull_art_=ImageTexture::create_from_image(presentation::rgba_image(decoded.image));
            break;
        }
        if(skull_art_.is_null())throw std::runtime_error("Original skull combat effect is missing");
    }
    for(const auto& source:demo_->terrain_art()){
        terrain_art_.push_back(presentation::image_texture(source));
    }
    const auto install=[&](const CombatArt& source,bool goliath){
        const auto image=presentation::rgba_image(source.image);
        Ref<ImageTexture> action;
        if(source.action)action=ImageTexture::create_from_image(presentation::rgba_image(*source.action));
        art_[source.entity]={ImageTexture::create_from_image(image),action,image->get_used_rect(),goliath};
    };
    for(const auto& source:demo_->art())install(source,false);
    for(const auto& source:campaign_art_){
        bool goliath=false;
        if(campaign_)for(const auto& member:campaign_->state().roster)
            if(member.id==source.entity){goliath=member.character.creation_data().race=="goliath";break;}
        install(source,goliath);
    }
    if(campaign_){
    const auto legacy=por::CharacterArt::load(std::filesystem::u8path(settings::game_path().utf8().get_data()));
    Ref<JSON> catalog_json;catalog_json.instantiate();
    Dictionary catalog;
    if(catalog_json->parse(FileAccess::get_file_as_string("res://bin/portraits/portraits.json"))==OK&&catalog_json->get_data().get_type()==Variant::DICTIONARY)
        catalog=catalog_json->get_data();
    for(const auto id:campaign_->state().slots)if(id){
        const auto& member=campaign_->member(id);
        std::string filename=member.character.appearance().portrait;
        if(filename.empty()){
            int best=-1;
            for(const auto& key:catalog.keys()){
                const Dictionary entry=catalog[key];
                const auto& draft=member.character.creation_data();
                const int score=4*(String(entry.get("Race","")).to_lower()==gs(draft.race).to_lower())+
                    2*(String(entry.get("Gender","")).to_lower()==gs(draft.gender).to_lower())+
                    (String(entry.get("Class","")).to_lower()==gs(draft.character_class).to_lower());
                if(score>best){best=score;filename=String(key).utf8().get_data();}
            }
        }
        if(!filename.empty()){
            const Ref<Texture2D> portrait=ResourceLoader::get_singleton()->load(gs("res://bin/portraits/"+filename));
            if(portrait.is_valid()){portraits_.emplace(id,portrait);continue;}
        }
        const auto image=presentation::rgba_image(legacy.portrait(campaign_->member(id).character.appearance()));
        portraits_.emplace(id,ImageTexture::create_from_image(image));
    }
    }
}
void CombatView::save_game()
{
    try{write_save_file(local_path("user://checks/combat.save"),demo_->save_combat(),4*1024*1024);
        error_.clear();get_node<Label>("Prompt")->set_text(i18n::text(N_("Training combat saved.")));
    }catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::load_game()
{
    try{const auto bytes=read_save_file(local_path("user://checks/combat.save"),4*1024*1024);
        demo_->restore_combat(bytes);sync_art();error_.clear();refresh();
    }catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::select_mode(String verb)
{
    mode_=spell_verb(verb.utf8().get_data(),spell_slot_);if(mode_=="dash"||mode_=="dodge"||mode_=="disengage"){immediate(verb);return;}refresh();
}
void CombatView::spell_slot(){spell_slot_=spell_slot_==1?2:1;mode_="move";refresh();}
void CombatView::adjust_zoom(int percentage_points)
{
    const int current=static_cast<int>(std::lround(combat_zoom_*100));
    const int next=std::clamp(current+percentage_points,10,1000);
    if(next==current)return;
    combat_zoom_=next/100.0;
    layout();refresh();
    // ScrollContainer applies its new child bounds during the layout pass.
    zoom_center_frames_=2;
}
void CombatView::select_party(EntityId id)
{
    if(!demo_||!demo_->has_combat())return;
    const auto state=demo_->combat().snapshot();
    const auto selected=std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==id&&a.side==0;});
    if(selected==state.combatants.end())return;
    selected_=id;followed_.reset();center_on(selected->cell);refresh();
}
void CombatView::move_selected(Cell direction)
{
    if(!demo_||!demo_->has_combat())return;
    const auto state=demo_->combat().snapshot();
    if(state.outcome!=Outcome::ongoing||state.actor!=selected_||state.reaction_pending)return;
    const auto selected=std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==selected_&&a.side==0;});
    if(selected==state.combatants.end())return;
    const Cell destination{selected->cell.x+direction.x,selected->cell.y+direction.y};
    const auto offered=demo_->combat().legal_commands();
    const auto enemy=std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.side!=selected->side&&!a.dead&&a.cell==destination;});
    if(enemy!=state.combatants.end()) {
        const auto attack=std::find_if(offered.begin(),offered.end(),[&](const auto& c){return c.verb=="melee"&&c.actor==selected_&&c.target==enemy->id;});
        if(attack!=offered.end())act(*attack);
        return;
    }
    const auto move=std::find_if(offered.begin(),offered.end(),[&](const auto& c){return c.verb=="move"&&c.actor==selected_&&c.destination==destination;});
    if(move!=offered.end())act(*move);
}
void CombatView::immediate(String verb)
{
    if(!demo_||!demo_->has_combat())return;const auto wanted=std::string(verb.utf8().get_data());
    const auto state=demo_->combat().snapshot();
    if(std::none_of(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==state.actor&&a.side==0;}))return;
    for(const auto& c:demo_->combat().legal_commands())if(c.verb==wanted){act(c);return;}
}
void CombatView::act(const Command& command)
{
    try {
        const auto before=demo_->combat().snapshot();
        if(demo_->submit(command)){
            const auto after=demo_->combat().snapshot();
            unsigned sound=0;
            if(command.verb=="melee"||command.verb=="opportunity"){
                const auto previous=std::find_if(before.combatants.begin(),before.combatants.end(),[&](const auto& actor){return actor.id==command.target;});
                const auto current=std::find_if(after.combatants.begin(),after.combatants.end(),[&](const auto& actor){return actor.id==command.target;});
                sound=previous!=before.combatants.end()&&current!=after.combatants.end()&&current->hit_points<previous->hit_points?7:9;
            } else if(command.verb=="ranged")sound=6;
            else if(command.verb=="fire_bolt"||command.verb=="magic_missile"||command.verb=="magic_missile_2"||command.verb=="scorching_ray"||command.verb=="blindness")sound=2;
            if(sound){action_seconds_[command.actor]=1.0;if(attack_sound_)attack_sound_->play(sound);}
            bool moved=false,dead=false;
            for(const auto& actor:after.combatants){
                const auto previous=std::find_if(before.combatants.begin(),before.combatants.end(),[&](const auto& old){return old.id==actor.id;});
                if(previous==before.combatants.end())continue;
                moved=moved||actor.cell!=previous->cell;
                dead=dead||(actor.dead&&!previous->dead);
            }
            if(effect_sound_){if(dead)effect_sound_->play(5);else if(moved)effect_sound_->play(10);}
            mode_="move";ai_delay_=0;error_.clear();refresh();
        }
    }
    catch(const std::exception& e){error_=e.what();refresh();}
}
void CombatView::_input(const Ref<InputEvent>& event)
{
    if(!is_visible_in_tree()||!demo_||Engine::get_singleton()->is_editor_hint())return;
    const Ref<InputEventKey> key=event;
    if(key.is_valid()&&key->is_pressed()&&!key->is_echo()&&!key->is_ctrl_pressed()&&demo_->has_combat()){
        if(const auto direction=movement_direction(key->get_keycode(),key->is_shift_pressed())){
            move_selected(*direction);get_viewport()->set_input_as_handled();return;
        }
        if(key->get_keycode()==Key::KEY_A){
            std::vector<std::string> actions;
            for(const auto& command:demo_->combat().legal_commands())if(command.verb!="end"&&std::find(actions.begin(),actions.end(),command.verb)==actions.end())actions.push_back(command.verb);
            if(!actions.empty()){
                const auto current=std::find(actions.begin(),actions.end(),mode_);
                mode_=actions[current==actions.end()?0:(std::size_t(current-actions.begin())+1)%actions.size()];refresh();
            }
            get_viewport()->set_input_as_handled();return;
        }
        if(key->get_keycode()==Key::KEY_Z){spell_slot();get_viewport()->set_input_as_handled();return;}
        if(key->get_keycode()==Key::KEY_SPACE){
            for(const char* verb:{"second_wind","dash","dodge","disengage","opportunity","decline"})if(mode_==verb){immediate(gs(mode_));break;}
            const auto offered=demo_->combat().legal_commands();
            const auto target=std::find_if(offered.begin(),offered.end(),[&](const auto& c){return c.verb==mode_&&c.target==selected_;});
            if(target!=offered.end())act(*target);
            get_viewport()->set_input_as_handled();return;
        }
    }
    if(!defeated()&&key.is_valid()&&key->is_pressed()&&!key->is_echo()&&key->get_keycode()==Key::KEY_ENTER) {
        if(demo_->waiting())next();else immediate("end");get_viewport()->set_input_as_handled();return;
    }
    if(!demo_->has_combat())return;
    auto* scroll=get_node<ScrollContainer>("BattlefieldScroll");
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
    if(mouse->is_pressed()&&mouse->get_button_index()==MouseButton::MOUSE_BUTTON_LEFT&&campaign_){
        const double right=get_size().x-382,row_height=(get_size().y-120)/8.0;
        if(local.x>=right&&local.x<right+358&&local.y>=60&&local.y<60+8*row_height){
            const auto slot=static_cast<unsigned>((local.y-60)/row_height);
            if(const auto id=campaign_->state().slots[slot])select_party(id);
            get_viewport()->set_input_as_handled();return;
        }
    }
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
    const auto s=demo_->combat().snapshot();
    const auto canvas=get_node<Control>("BattlefieldScroll/Canvas")->get_global_transform_with_canvas().affine_inverse().xform(mouse->get_position());
    const auto relative=canvas/(combat_zoom_*base_tile_);
    const Cell cell{static_cast<int>(std::floor(relative.x)),static_cast<int>(std::floor(relative.y))};
    for(const auto& a:s.combatants)if(a.side==0&&a.cell==cell){select_party(a.id);get_viewport()->set_input_as_handled();return;}
    const auto current=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==s.actor;});
    if(current==s.combatants.end()||current->side!=0||selected_!=s.actor)return;
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
    if(loaded)for(const auto& actor:s.combatants){
        const auto [entry,first_seen]=known_dead_.emplace(actor.id,actor.dead);
        if(!first_seen){if(actor.dead&&!entry->second)skull_seconds_[actor.id]=1.0;entry->second=actor.dead;}
    }
    bool player=false;String turn=i18n::text(demo_?demo_->status():N_("Unable to load rules"));
    if(loaded&&s.outcome==Outcome::ongoing)for(const auto& a:s.combatants)if(a.id==s.actor) {
        player=a.side==0;
        turn=i18n::format(s.reaction_pending?N_("Round {round} / {name} reaction\nMove {feet} ft | {action}"):N_("Round {round} / {name} turn\nMove {feet} ft | {action}"),
            {{"round",s.round},{"name",gs(a.name)},{"feet",a.movement_feet},{"action",i18n::text(a.action?N_("Action ready"):N_("Action spent"))}});
        turn+="\n"+(a.status_messages.empty()?i18n::text(a.status):i18n::render(a.status_messages).replace("\n"," | "));
    }
    if(loaded&&s.outcome!=Outcome::ongoing)turn=i18n::text(s.outcome==Outcome::victory?N_("Victory"):N_("Party incapacitated / defeat"));
    if(player&&last_actor_&&last_actor_!=s.actor)selected_=s.actor;
    if(!selected_&&player)selected_=s.actor;
    if(loaded)last_actor_=s.actor;
    get_node<Label>("Turn")->set_text(turn);layout_status();
    String roster;for(const auto& a:s.combatants){
        roster+=String(a.id==s.actor?"> ":"  ")+i18n::format("{id} {name}  {current}/{maximum} HP  AC {ac}\n",
            {{"id",a.id},{"name",gs(a.name)},{"current",a.hit_points},{"maximum",a.max_hit_points},{"ac",a.armor_class}});
        if(!a.conditions.empty())roster+="    "+i18n::render(a.conditions)+"\n";
    }
    get_node<RichTextLabel>("Roster")->set_text(roster);
    get_node<Button>("Continue")->set_visible(demo_&&(demo_->waiting()||(loaded&&s.outcome!=Outcome::ongoing)));
    get_node<Button>("End")->set_visible(!get_node<Button>("Continue")->is_visible());
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
    if(loaded&&s.outcome==Outcome::ongoing&&!s.reaction_pending&&selected_&&selected_!=s.actor){
        const auto selected=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==selected_&&a.side==0;});
        if(selected!=s.combatants.end())get_node<Label>("Prompt")->set_text(i18n::format("It is not {name}'s turn.",{{"name",gs(selected->name)}}));
    }
    String log=turn+"\n"+get_node<Label>("Prompt")->get_text()+"\n"+i18n::text("A: next action | Space: use | Z: spell slot | Enter: end turn")+"\n\n";
    if(demo_)log+=i18n::campaign("por/combat/dialogue",demo_->dialogue())+"\n\n";
    if(s.log_messages.size()==s.log.size())for(const auto& entry:s.log_messages)log+=i18n::render(entry)+"\n";
    else for(const auto& entry:s.log)log+=i18n::text(entry)+"\n";
    if(!error_.empty())log+="\n"+i18n::text(error_);
    auto* log_view=get_node<RichTextLabel>("Log");
    auto* log_scroll=log_view->get_v_scroll_bar();
    const double previous_scroll=log_scroll->get_value();
    const bool follow_bottom=previous_scroll>=log_scroll->get_max()-log_scroll->get_page()-2;
    log_view->set_text(log);
    if(follow_bottom)log_view->scroll_to_line(std::max(0,log_view->get_line_count()-1));
    else log_scroll->set_value(previous_scroll);
    get_node<Button>("Continue")->hide();get_node<Button>("End")->hide();
    get_node<Control>("BattlefieldScroll/Canvas")->queue_redraw();
    queue_redraw();
}
void CombatView::center_on(Cell cell)
{
    auto* scroll=get_node<ScrollContainer>("BattlefieldScroll");
    const double tile=combat_zoom_*base_tile_;
    scroll->set_h_scroll(static_cast<int>((cell.x+.5)*tile-scroll->get_size().x*.5));
    scroll->set_v_scroll(static_cast<int>((cell.y+.5)*tile-scroll->get_size().y*.5));
}
void CombatView::_draw()
{
    draw_rect(Rect2(Vector2(),get_size()),Color("121a20"));draw_rect(board_rect_,Color("202d33"));
    if(!campaign_||!demo_||!demo_->has_combat())return;
    const auto snapshot=demo_->combat().snapshot();const auto font=get_theme_default_font();
    const double right=get_size().x-382,row_height=(get_size().y-120)/8.0;
    for(unsigned slot=0;slot<8;++slot){
        const auto id=campaign_->state().slots[slot];const double top=60+slot*row_height;
        const Rect2 row(right,top,358,row_height-4);
        draw_rect(row,id&&id==selected_?Color("344950"):Color("1b282e"));
        if(!id)continue;
        const auto& member=campaign_->member(id);
        const auto found=std::find_if(snapshot.combatants.begin(),snapshot.combatants.end(),[&](const auto& c){return c.id==id;});
        const int hp=found==snapshot.combatants.end()?member.vitals.hit_points:found->hit_points;
        const int maximum=found==snapshot.combatants.end()?member.character.sheet().hit_points:found->max_hit_points;
        const double size=std::min(64.0,row_height-18),portrait_y=top+4;
        const Rect2 image_rect(right+5,portrait_y,size,size);
        draw_rect(image_rect,Color("10171c"));
        if(const auto portrait=portraits_.find(id);portrait!=portraits_.end())draw_texture_rect(portrait->second,image_rect,false);
        else if(const auto sprite=art_.find(id);sprite!=art_.end())draw_texture_rect(sprite->second.texture,image_rect,false);
        draw_rect(Rect2(right+5,portrait_y+size+2,size,5),Color("37191d"));
        draw_rect(Rect2(right+5,portrait_y+size+2,size*std::clamp(double(hp)/std::max(1,maximum),0.0,1.0),5),Color("d6444b"));
        const double text_x=right+82;
        const auto line=[&](String value,double y,int size,Color color){
            auto cursor=Vector2(text_x,y);
            for(int i=0;i<value.length();++i)cursor.x+=font->draw_char(get_canvas_item(),cursor,value.unicode_at(i),size,color);
        };
        line(gs(member.character.sheet().name),top+27,17,Color("e2edf0"));
        const auto& sheet=member.character.sheet();
        line(gs(sheet.character_class).capitalize()+" / "+gs(sheet.race).capitalize()+" / "+gs(sheet.gender).capitalize(),top+49,13,Color("a8c1c7"));
        line(String::num_int64(hp)+" / "+String::num_int64(maximum)+" HP",top+68,15,Color("efb9bb"));
    }
}
void CombatView::draw_battlefield()
{
    if(!demo_||!demo_->has_combat())return;
    auto* canvas=get_node<Control>("BattlefieldScroll/Canvas");
    canvas->draw_set_transform(Vector2(),0,Vector2(combat_zoom_,combat_zoom_));
    const auto s=demo_->combat().snapshot();const double tile=base_tile_;const auto font=get_theme_default_font();
    for(int y=0;y<s.battlefield.height;++y)for(int x=0;x<s.battlefield.width;++x) {
        const Rect2 cell(Vector2(x*tile,y*tile),Vector2(tile,tile));
        const auto terrain=s.battlefield.at({x,y});canvas->draw_rect(cell,terrain==1?Color("64716d"):terrain==2?Color("665238"):((x+y)%2?Color("29373c"):Color("253137")));
        const auto index=y*s.battlefield.width+x;
        if(index<demo_->battlefield_tiles().size()&&demo_->battlefield_tiles()[index]<terrain_art_.size())canvas->draw_texture_rect(terrain_art_[demo_->battlefield_tiles()[index]],cell,false);
        else canvas->draw_rect(cell,Color("172228"),false);
    }
    const auto active=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==s.actor;});
    const auto selected=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==selected_&&a.side==0;});
    if(selected!=s.combatants.end()){
        if(selected_==s.actor)for(const auto p:demo_->combat().movement_reach(selected_))
            canvas->draw_rect(Rect2(Vector2(p.x*tile+1,p.y*tile+1),Vector2(tile-2,tile-2)),Color(1,1,1,.18));
        canvas->draw_rect(Rect2(Vector2(selected->cell.x*tile+1,selected->cell.y*tile+1),Vector2(tile-2,tile-2)),Color("e7c484"),false,2.0);
    }
    if(active!=s.combatants.end()&&active->side==0&&mode_!="move")for(const auto& c:demo_->combat().legal_commands())if(c.verb==mode_&&c.target){
        const auto target=std::find_if(s.combatants.begin(),s.combatants.end(),[&](const auto& a){return a.id==c.target;});
        if(target!=s.combatants.end())canvas->draw_rect(Rect2(Vector2(target->cell.x*tile+1,target->cell.y*tile+1),Vector2(tile-2,tile-2)),Color(.4,.8,.75,.23));
    }
    for(const auto index:presentation::combat_sprite_draw_order(s.combatants)) {
        const auto& a=s.combatants[index];
        const auto center=Vector2((a.cell.x+.5)*tile,(a.cell.y+.5)*tile);
        if(a.dead){
            if(skull_art_.is_valid()&&skull_seconds_.contains(a.id))
                canvas->draw_texture_rect(skull_art_,Rect2(Vector2(a.cell.x*tile,a.cell.y*tile),Vector2(tile,tile)),false);
            continue;
        }
        if(art_.contains(a.id)) {
            const auto& art=art_.at(a.id);
            const auto texture=action_seconds_.contains(a.id)&&art.action.is_valid()?art.action:art.texture;
            const auto rect=presentation::combat_sprite_rect(texture->get_size(),art.visible,
                Rect2(Vector2(a.cell.x*tile,a.cell.y*tile),Vector2(tile,tile)),art.goliath);
            canvas->draw_texture_rect(texture,rect,false,a.conscious?Color(1,1,1):Color(.5,.5,.5));
        } else {
            const auto number=std::to_string(a.id);
            auto cursor=center+Vector2(-5.5*number.size(),7);
            for(const char digit:number) {
                canvas->draw_char(font,cursor,gs(std::string(1,digit)),20,a.side==0?Color("79d6d4"):Color("dd9874"));
                cursor.x+=11;
            }
        }
    }
}
void CombatView::_process(double delta)
{
    if(Engine::get_singleton()->is_editor_hint())return;
    try {
        for(auto it=action_seconds_.begin();it!=action_seconds_.end();){
            it->second-=delta;
            if(it->second<=0)it=action_seconds_.erase(it);else ++it;
            get_node<Control>("BattlefieldScroll/Canvas")->queue_redraw();
        }
        for(auto it=skull_seconds_.begin();it!=skull_seconds_.end();){
            it->second-=delta;
            if(it->second<=0)it=skull_seconds_.erase(it);else ++it;
            get_node<Control>("BattlefieldScroll/Canvas")->queue_redraw();
        }
        if((checking_||expedition_check_)&&!error_.empty())throw std::runtime_error(error_);
        if(!demo_)return;
        if(checking_&&demo_->waiting()){next();return;}
        if(!demo_->has_combat())return;const auto s=demo_->combat().snapshot();
        if(zoom_center_frames_)--zoom_center_frames_;
        else for(const auto& a:s.combatants)if(a.id==(selected_?selected_:s.actor)) {
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
                mouse->set_position(get_node<Control>("BattlefieldScroll/Canvas")->get_global_transform_with_canvas().xform(Vector2(target->cell.x+.5,target->cell.y+.5)*(combat_zoom_*base_tile_)));
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
