#include "rolf_tour_view.h"
#include "opengold/exploration_view.h"
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <vector>
#include <queue>

using namespace godot;
using namespace opengold::por;
namespace {
// Numeric constructors are safe before Godot initializes the extension interface.
const Color background(18/255.f,26/255.f,32/255.f), panel(28/255.f,39/255.f,46/255.f),
    line(65/255.f,80/255.f,88/255.f), gold(215/255.f,180/255.f,121/255.f),
    party_color(121/255.f,214/255.f,212/255.f);
const std::array<Vector2,4> direction{Vector2(0,-1),Vector2(1,0),Vector2(0,1),Vector2(-1,0)};
const std::array<const char*,4> direction_name{"North","East","South","West"};

}

void RolfTourView::_bind_methods() {}
void RolfTourView::_notification(int what)
{
    if (what==NOTIFICATION_RESIZED && ready_) {layout();queue_redraw();}
}
void RolfTourView::_ready()
{
    ready_=true;
    set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
    // Child nodes are scene-owned; these lookups are temporary non-owning views.
    get_node<Button>("Continue")->connect("pressed",callable_mp(this,&RolfTourView::next));
    get_node<Button>("Restart")->connect("pressed",callable_mp(this,&RolfTourView::restart));
    get_node<Button>("Left")->connect("pressed",callable_mp(this,&RolfTourView::left));
    get_node<Button>("Right")->connect("pressed",callable_mp(this,&RolfTourView::right));
    get_node<Button>("Forward")->connect("pressed",callable_mp(this,&RolfTourView::forward));
    get_node<Button>("MapMode")->connect("pressed",callable_mp(this,&RolfTourView::map_mode));
    get_node<Button>("Look")->connect("pressed",callable_mp(this,&RolfTourView::look));
    get_node<Button>("Inventory")->connect("pressed",callable_mp(this,&RolfTourView::inventory));
    get_node<Button>("InventoryPanel/Close")->connect("pressed",callable_mp(this,&RolfTourView::inventory));
    get_node<Button>("LeaveShop")->connect("pressed",callable_mp(this,&RolfTourView::leave_shop));
    get_window()->set_min_size(Vector2i(960,720));
    layout();
    if (Engine::get_singleton()->is_editor_hint()) return;
    const auto args=OS::get_singleton()->get_cmdline_user_args();
    town_check_=args.has("--town-check");checking_=args.has("--tour-check")||town_check_;capture_=args.has("--capture");
    // An original OpenGold footstep cue, not extracted SSI sound data.
    footstep_.instantiate();footstep_->set_format(AudioStreamWAV::FORMAT_16_BITS);
    footstep_->set_mix_rate(22050);
    PackedByteArray samples; samples.resize(2205*2);
    std::uint32_t noise=7;
    for (int i=0;i<2205;++i) {
        noise=noise*1664525U+1013904223U;
        const double envelope=std::exp(-i/280.0);
        const auto sample=static_cast<std::int16_t>((static_cast<int>(noise>>16)-32768)*0.13*envelope);
        samples.set(i*2,static_cast<std::uint16_t>(sample)&255);
        samples.set(i*2+1,static_cast<std::uint16_t>(sample)>>8);
    }
    footstep_->set_data(samples);
    get_node<AudioStreamPlayer>("Footstep")->set_stream(footstep_);
    restart();
}

void RolfTourView::layout()
{
    const auto width=get_size().x, height=get_size().y;
    const double margin=24, gutter=24, sidebar=std::min(std::clamp(width*.30,270.0,430.0),height-380.0);
    const double main_width=width-margin*2-gutter-sidebar;
    const bool shopping=session_&&session_->snapshot().phase==TourPhase::shopping;
    const double view_height=std::min(main_width*.625,height-(shopping?490.0:410.0));
    scene_rect_=Rect2(margin,102,main_width,view_height);
    map_rect_=Rect2(margin+main_width+gutter,102,sidebar,sidebar);
    dialogue_rect_=Rect2(margin,scene_rect_.get_end().y+18,main_width,height-scene_rect_.get_end().y-76);
    const auto place=[&](const char* name,Rect2 rect) {
        auto* node=get_node<Control>(name);node->set_position(rect.position);node->set_size(rect.size);
    };
    place("Title",Rect2(margin,20,main_width,34));
    place("Location",Rect2(margin,68,main_width,26));
    place("MapTitle",Rect2(map_rect_.position.x,68,sidebar-130,26));
    place("MapMode",Rect2(width-margin-122,64,122,30));
    place("Coordinates",Rect2(map_rect_.position.x,map_rect_.get_end().y+12,sidebar,28));
    place("Legend",Rect2(map_rect_.position.x,map_rect_.get_end().y+48,sidebar,50));
    place("Speaker",Rect2(dialogue_rect_.position+Vector2(18,12),Vector2(main_width-36,26)));
    place("Dialogue",Rect2(dialogue_rect_.position+Vector2(18,46),Vector2(main_width-36,dialogue_rect_.size.y-112)));
    place("Continue",Rect2(dialogue_rect_.get_end()-Vector2(182,54),Vector2(164,40)));
    place("Progress",Rect2(dialogue_rect_.position+Vector2(18,dialogue_rect_.size.y-48),Vector2(main_width-220,30)));
    place("Movement",Rect2(map_rect_.position.x,height-178,sidebar,26));
    const double button_width=(sidebar-12)/3;
    place("Left",Rect2(map_rect_.position.x,height-140,button_width,40));
    place("Forward",Rect2(map_rect_.position.x+button_width+6,height-140,button_width,40));
    place("Right",Rect2(map_rect_.position.x+(button_width+6)*2,height-140,button_width,40));
    place("Restart",Rect2(map_rect_.position.x,height-88,sidebar,34));
    place("Footer",Rect2(margin,height-36,width-2*margin,26));
    place("Party",Rect2(map_rect_.position.x,map_rect_.get_end().y+46,sidebar,50));
    get_node<Label>("Legend")->hide();
    place("Look",Rect2(map_rect_.position.x,height-226,sidebar*.48,34));
    place("Inventory",Rect2(map_rect_.position.x+sidebar*.52,height-226,sidebar*.48,34));
    place("Choices",Rect2(dialogue_rect_.position+Vector2(18,84),Vector2(main_width-36,dialogue_rect_.size.y-148)));
    place("Answer",Rect2(dialogue_rect_.position+Vector2(18,dialogue_rect_.size.y-100),Vector2(main_width-36,36)));
    place("LeaveShop",Rect2(dialogue_rect_.get_end()-Vector2(332,54),Vector2(140,40)));
    place("InventoryPanel",Rect2(margin+40,90,width-2*margin-80,height-160));
    auto* inventory_panel=get_node<Control>("InventoryPanel");
    get_node<Control>("InventoryPanel/Items")->set_position(Vector2(20,20));
    get_node<Control>("InventoryPanel/Items")->set_size(inventory_panel->get_size()-Vector2(40,90));
    get_node<Control>("InventoryPanel/Close")->set_position(Vector2(20,inventory_panel->get_size().y-54));
    get_node<Control>("InventoryPanel/Close")->set_size(Vector2(200,36));
}

void RolfTourView::restart()
{
    try {
        error_="";
        if (session_) session_->restart();
        else {
            auto directory=OS::get_singleton()->get_environment("OPENGOLD_GAME_DIR");
            if (directory.is_empty()) directory=ProjectSettings::get_singleton()->get_setting("opengold/game_directory","");
            session_.emplace(RolfTourSession::load(std::filesystem::u8path(directory.utf8().get_data())));
            for (unsigned i=0;i<sprites_.size();++i) {
                const auto& source=session_->sprites()[i];
                PackedByteArray pixels;pixels.resize(source.rgba.size());
                std::copy(source.rgba.begin(),source.rgba.end(),pixels.ptrw());
                const auto image=godot::Image::create_from_data(source.width,source.height,false,godot::Image::FORMAT_RGBA8,pixels);
                sprites_[i]=ImageTexture::create_from_image(image);
            }
        }
        rendered_pose_.reset();
        rendered_sprite_id_=999; displayed_ticket_=0;
        played_footsteps_=0; shown_revision_=0;
        session_->advance(0);
    } catch (const std::exception& error) {
        session_.reset();error_=String::utf8(error.what());
    }
    refresh();
}
void RolfTourView::next()
{
    if (!session_) return;
    const auto& s=session_->snapshot();
    const auto selected=get_node<ItemList>("Choices")->get_selected_items();
    if (s.phase==TourPhase::shopping) {
        if (!selected.is_empty()) session_->buy(s.continue_ticket,selected[0]);
    } else if (s.phase==TourPhase::awaiting_input) {
        session_->input(s.continue_ticket,get_node<LineEdit>("Answer")->get_text().utf8().get_data());
    } else session_->choose(s.continue_ticket,selected.is_empty()?0:selected[0]);
    refresh();
}
void RolfTourView::left(){movement(ExplorationCommand::turn_left);}
void RolfTourView::right(){movement(ExplorationCommand::turn_right);}
void RolfTourView::forward(){movement(ExplorationCommand::forward);}
void RolfTourView::look(){movement(ExplorationCommand::look);}
void RolfTourView::leave_shop(){if(session_){session_->leave_shop(session_->snapshot().continue_ticket);refresh();}}
void RolfTourView::inventory()
{
    auto* panel=get_node<Control>("InventoryPanel");
    if(panel->is_visible()){panel->hide();return;}
    if(!session_)return;
    const auto& p=session_->party();
    std::string text=p.name+" / Level "+std::to_string(p.level)+"\n"+std::to_string(p.wealth[3])+" gold pieces\n\n";
    if(p.inventory.empty())text+="Inventory is empty. Visit a shop to buy equipment.";
    for(const auto& item:p.inventory)text+=item.label()+"\n";
    get_node<RichTextLabel>("InventoryPanel/Items")->set_text(String::utf8(text.c_str()));panel->show();
}
void RolfTourView::movement(ExplorationCommand command)
{
    if (!session_) return;
    if (session_->explore(command)) refresh();
    else if (session_->snapshot().phase==TourPhase::completed)
        get_node<Label>("Movement")->set_text("The way is blocked");
}
void RolfTourView::map_mode(){full_map_=!full_map_;refresh();}

void RolfTourView::_input(const Ref<InputEvent>& event)
{
    const Ref<InputEventKey> key=event;
    if (key.is_null() || !key->is_pressed() || key->is_echo()) return;
    if(get_node<Control>("InventoryPanel")->is_visible())return;
    if(get_node<LineEdit>("Answer")->has_focus() && key->get_keycode()!=Key::KEY_ENTER)return;
    if(session_ && (session_->snapshot().choices.size()>1 || session_->snapshot().phase==TourPhase::shopping) && key->get_keycode()!=Key::KEY_ENTER)return;
    switch (key->get_keycode()) {
    case Key::KEY_ENTER: next();break;
    case Key::KEY_LEFT: left();break;
    case Key::KEY_RIGHT: right();break;
    case Key::KEY_UP: forward();break;
    case Key::KEY_L: look();break;
    default:return;
    }
    get_viewport()->set_input_as_handled();
}

void RolfTourView::_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint()) return;
    if (session_) {
        session_->advance(checking_?0.3:delta);
        if (session_->snapshot().footsteps!=played_footsteps_) {
            played_footsteps_=session_->snapshot().footsteps;
            if (!checking_) get_node<AudioStreamPlayer>("Footstep")->play();
        }
        if (session_->snapshot().revision!=shown_revision_) refresh();
    }
    if (checking_) check_run();
}

void RolfTourView::refresh()
{
    layout();
    if(session_ && rendered_sprite_id_!=session_->snapshot().sprite_id){
        for(unsigned n=0;n<3;++n){const auto& source=session_->sprites()[n];
            PackedByteArray pixels;pixels.resize(source.rgba.size());
            std::copy(source.rgba.begin(),source.rgba.end(),pixels.ptrw());
            sprites_[n]=ImageTexture::create_from_image(godot::Image::create_from_data(source.width,source.height,false,godot::Image::FORMAT_RGBA8,pixels));
        }
        rendered_sprite_id_=session_->snapshot().sprite_id;
    }
    if (session_ && (!rendered_pose_ || *rendered_pose_ != session_->snapshot().pose || rendered_picture_revision_!=session_->snapshot().picture_revision)) {
        try {
            const auto pose = session_->snapshot().pose;
            const auto source = session_->picture()?*session_->picture():compose_exploration_view(session_->map(), session_->wall_art(), pose.x, pose.y, pose.facing);
            PackedByteArray pixels; pixels.resize(source.rgba.size());
            std::copy(source.rgba.begin(), source.rgba.end(), pixels.ptrw());
            const auto image = godot::Image::create_from_data(source.width, source.height, false, godot::Image::FORMAT_RGBA8, pixels);
            if (wall_view_.is_null()) wall_view_ = ImageTexture::create_from_image(image);
            else wall_view_->set_image(image);
            rendered_pose_ = pose;
            rendered_picture_revision_=session_->snapshot().picture_revision;
        } catch (const std::exception& error) {
            session_.reset(); error_ = String::utf8(error.what());
        }
    }
    const bool loaded=session_.has_value();
    const TourSnapshot s=loaded?session_->snapshot():TourSnapshot{};
    shown_revision_=s.revision;
    const bool waiting=loaded&&s.phase==TourPhase::awaiting_continue;
    const bool completed=loaded&&s.phase==TourPhase::completed;
    const bool faulted=!loaded||s.phase==TourPhase::faulted;
    const bool shopping=loaded&&s.phase==TourPhase::shopping;
    const bool answer=loaded&&s.phase==TourPhase::awaiting_input;
    const bool multiple=waiting&&s.choices.size()>1;
    get_node<Label>("Location")->set_text("New Phlan  /  "+String(direction_name[s.pose.facing])+" view");
    get_node<Label>("Coordinates")->set_text("Party  ("+String::num_uint64(s.pose.x)+", "+String::num_uint64(s.pose.y)+")   "+String(direction_name[s.pose.facing]));
    get_node<Label>("Speaker")->set_text(faulted?"Unable to continue":shopping?"Shop / select an item":s.tour_finished?"New Phlan":"Rolf  /  Council guide");
    get_node<RichTextLabel>("Dialogue")->set_text(faulted?
        (loaded?String::utf8(s.diagnostic.c_str()):error_)+"\nSet OPENGOLD_GAME_DIR to your Pool of Radiance data folder, then restart.":
        s.dialogue.empty()?"Following Rolf...":String::utf8(s.dialogue.c_str()));
    get_node<Button>("Continue")->set_disabled(!waiting&&!shopping&&!answer);
    get_node<Button>("Continue")->set_text(shopping?"Buy [Enter]":multiple?"Choose [Enter]":answer?"Submit [Enter]":"Continue [Enter]");
    get_node<Button>("Continue")->set_visible(!completed);
    get_node<Label>("Progress")->set_text(faulted?"Stopped":s.tour_finished?"":waiting?"Pause "+String::num_uint64(s.prompts):"Following the guide");
    get_node<Label>("Movement")->set_text(completed?"Explore  /  arrow keys":"Movement paused");
    for (const char* name:{"Left","Forward","Right","Look"}) get_node<Button>(name)->set_disabled(!completed);
    get_node<Button>("LeaveShop")->set_visible(shopping);
    get_node<LineEdit>("Answer")->set_visible(answer);
    get_node<LineEdit>("Answer")->set_max_length(s.number_input?6:40);
    auto* choices=get_node<ItemList>("Choices");choices->set_visible(shopping||multiple);
    if(displayed_ticket_!=s.continue_ticket){
        choices->clear();
        if(shopping)for(const auto& item:session_->shop_stock())choices->add_item(String::utf8((item.label()+" — "+std::to_string(item.stored.value)+" gp").c_str()));
        else for(const auto& c:s.choices)choices->add_item(String::utf8(c.c_str()));
        if(choices->get_item_count())choices->select(0);
        if(shopping||multiple)choices->grab_focus();
        if(answer){get_node<LineEdit>("Answer")->clear();get_node<LineEdit>("Answer")->grab_focus();}
        displayed_ticket_=s.continue_ticket;
    }
    get_node<RichTextLabel>("Dialogue")->set_size(Vector2(dialogue_rect_.size.x-36,(shopping||multiple)?36:dialogue_rect_.size.y-(answer?160:112)));
    if(multiple){
        const double text_height=(dialogue_rect_.size.y-122)*.55;
        get_node<RichTextLabel>("Dialogue")->set_size(Vector2(dialogue_rect_.size.x-36,text_height));
        choices->set_position(dialogue_rect_.position+Vector2(18,56+text_height));
        choices->set_size(Vector2(dialogue_rect_.size.x-36,dialogue_rect_.size.y-120-text_height));
    } else {
        choices->set_position(dialogue_rect_.position+Vector2(18,84));
        choices->set_size(Vector2(dialogue_rect_.size.x-36,dialogue_rect_.size.y-148));
    }
    if(shopping)get_node<RichTextLabel>("Dialogue")->set_text(s.diagnostic.empty()?"Prices are per listed item or bundle.":String::utf8(s.diagnostic.c_str()));
    if(answer&&!s.diagnostic.empty())get_node<RichTextLabel>("Dialogue")->add_text("\n"+String::utf8(s.diagnostic.c_str()));
    if(loaded){const auto& p=session_->party();get_node<Label>("Party")->set_text("Fighter  /  Level 1  /  HP "+String::num_uint64(p.hit_points)+"/"+String::num_uint64(p.max_hit_points)+"\n"+String::num_uint64(p.wealth[3])+" gp  /  "+String::num_uint64(p.inventory.size())+" items");}
    get_node<Button>("MapMode")->set_text(full_map_?"Map: full":"Map: visited");
    queue_redraw();
}

void RolfTourView::_draw()
{
    draw_rect(Rect2(Vector2(),get_size()),background);
    draw_line(Vector2(24,57),Vector2(get_size().x-24,57),line);
    draw_rect(dialogue_rect_,panel);draw_rect(dialogue_rect_,line,false);
    draw_line(dialogue_rect_.position,dialogue_rect_.position+Vector2(dialogue_rect_.size.x,0),gold,2);
    draw_rect(map_rect_,Color("162128"));
    draw_scene();draw_map();
    draw_rect(scene_rect_,line,false);draw_rect(map_rect_,line,false);
}

void RolfTourView::draw_scene()
{
    draw_rect(scene_rect_, panel);
    if (!session_ || wall_view_.is_null()) return;
    // Fit the complete original 88x88 view. DOS EGA pixels were displayed 6/5
    // as tall as wide; letterboxing preserves art and door framing on resize.
    const double scale = std::min(scene_rect_.size.x / 88.0, scene_rect_.size.y / 105.6);
    const Vector2 pixel_scale(scale, scale * 1.2), size(88 * pixel_scale.x, 88 * pixel_scale.y);
    const Rect2 view(scene_rect_.position + (scene_rect_.size - size) * .5, size);
    draw_texture_rect(wall_view_, view, false);
    const auto& state = session_->snapshot();
    if (state.sprite_frame >= 0 && sprites_[state.sprite_frame].is_valid()) {
        const auto& source = session_->sprites()[state.sprite_frame];
        const Vector2 sprite_size(source.width * pixel_scale.x, source.height * pixel_scale.y);
        draw_texture_rect(sprites_[state.sprite_frame],
            Rect2(view.position + Vector2((view.size.x - sprite_size.x) * .5, view.size.y - sprite_size.y), sprite_size), false);
    }
}

void RolfTourView::draw_map()
{
    if (!session_) return;
    const auto& s=session_->snapshot();
    const double cell=map_rect_.size.x/16;
    for (unsigned y=0;y<16;++y) for (unsigned x=0;x<16;++x) {
        const auto origin=map_rect_.position+Vector2(x*cell,y*cell);
        if (!full_map_&&!s.visited.test(y*16+x)) continue;
        draw_rect(Rect2(origin,Vector2(cell,cell)),s.visited.test(y*16+x)?Color("304747"):Color("253038"));
        draw_rect(Rect2(origin,Vector2(cell,cell)),Color("1b252b"),false);
        const auto& c=session_->map().at(x,y);
        const double inset=1.3;
        const std::array<Vector2,4> corners{origin+Vector2(inset,inset),origin+Vector2(cell-inset,inset),origin+Vector2(cell-inset,cell-inset),origin+Vector2(inset,cell-inset)};
        for (unsigned d=0;d<4;++d) {
            if (c.walls[d]) draw_line(corners[d],corners[(d+1)%4],Color("9ca8a7"),1.5);
            if (c.doors[d]) draw_line(corners[d].lerp(corners[(d+1)%4],.27),corners[d].lerp(corners[(d+1)%4],.73),gold,3);
        }
    }
    const auto center=map_rect_.position+Vector2((s.pose.x+.5)*cell,(s.pose.y+.5)*cell);
    const auto forward=direction[s.pose.facing],right=Vector2(-forward.y,forward.x);
    PackedVector2Array arrow;arrow.push_back(center+forward*cell*.43);
    arrow.push_back(center-forward*cell*.3+right*cell*.32);
    arrow.push_back(center-forward*cell*.16);
    arrow.push_back(center-forward*cell*.3-right*cell*.32);
    draw_circle(center,cell*.45,background);draw_colored_polygon(arrow,party_color);
}

void RolfTourView::capture_frame(const String& name)
{
    if (!capture_) return;
    const auto directory=ProjectSettings::get_singleton()->globalize_path("res://../user-data");
    std::filesystem::create_directories(std::filesystem::u8path(directory.utf8().get_data()));
    const auto path=directory.path_join(name+String(".png"));
    const auto image=get_viewport()->get_texture()->get_image();
    if (image.is_null() || image->save_png(path)!=OK) throw std::runtime_error("Failed to capture tour scene");
    UtilityFunctions::print("Screenshot: ",path);
}
void RolfTourView::check_run()
{
    try {
        const auto press_key=[&](Key code, bool echo=false) {
            Ref<InputEventKey> key;key.instantiate();
            key->set_keycode(code);key->set_pressed(true);key->set_echo(echo);
            get_viewport()->push_input(key,true);
            key->set_pressed(false);key->set_echo(false);
            get_viewport()->push_input(key,true);
        };
        if (!session_ || session_->snapshot().phase==TourPhase::faulted)
            throw std::runtime_error(session_?session_->snapshot().diagnostic:error_.utf8().get_data());
        if (++check_frames_>2000) throw std::runtime_error("Tour integration check timed out");
        const auto& s=session_->snapshot();
        if(town_check_ && s.tour_finished){check_town();return;}
        if (s.phase==TourPhase::awaiting_continue) {
            if (s.continue_ticket!=checked_ticket_) {
                checked_ticket_=s.continue_ticket;++check_prompts_;capture_pending_=true;return;
            }
            if (capture_pending_) {
                capture_frame("rolf-tour-"+String::num_uint64(check_prompts_));capture_pending_=false;
                if (get_node<Button>("Continue")->is_disabled() || !get_node<Button>("Forward")->is_disabled())
                    throw std::runtime_error("Incorrect input lock at tour prompt");
                UtilityFunctions::print("Tour pause ",check_prompts_," at ",s.pose.x,",",s.pose.y," facing ",s.pose.facing);
                get_node<Button>("MapMode")->grab_focus();
                const auto pose=s.pose;
                press_key(Key::KEY_RIGHT);
                if (session_->snapshot().pose!=pose) throw std::runtime_error("Keyboard bypassed tour movement lock");
                press_key(Key::KEY_ENTER,true);
                if (session_->snapshot().phase!=TourPhase::awaiting_continue)
                    throw std::runtime_error("Held Enter skipped a prompt");
                press_key(Key::KEY_ENTER);
                if (session_->snapshot().phase!=TourPhase::running)
                    throw std::runtime_error("Enter did not continue with a different button focused");
            }
        } else if (s.phase==TourPhase::completed) {
            if (check_prompts_!=8) throw std::runtime_error("Expected all eight original dialogue pauses");
            get_node<Button>("Restart")->grab_focus();
            const auto facing=s.pose.facing;press_key(Key::KEY_RIGHT);
            if (session_->snapshot().pose.facing!=(facing+1)%4) throw std::runtime_error("Exploration turn did not update native state");
            UtilityFunctions::print("Godot C++ tour integration passed: ",check_prompts_," pauses.");
            get_tree()->quit(0);checking_=false;
        }
    } catch (const std::exception& error) {
        UtilityFunctions::push_error(String::utf8(error.what()));get_tree()->quit(1);checking_=false;
    }
}

void RolfTourView::check_town()
{
    const auto& s=session_->snapshot();
    if(check_prompts_!=8)throw std::runtime_error("Town started before the full tour");
    if(shop_check_stage_==3 && s.phase==TourPhase::completed){
        if(session_->party().inventory.size()!=1 || session_->script_variable(0x6BC1)!=session_->party().wealth[3])
            throw std::runtime_error("Shop results did not persist after leaving");
        UtilityFunctions::print("Godot town integration passed: tour, doors, shop purchase, inventory and return to exploration.");
        get_tree()->quit(0);checking_=false;return;
    }
    if(s.phase==TourPhase::shopping){
        if(shop_check_stage_==0){shop_check_stage_=1;return;}
        if(shop_check_stage_==1){
            capture_frame("phlan-shop");
            const auto gold_before=session_->party().wealth[3];
            const auto price=session_->shop_stock()[0].stored.value;
            if(get_node<ItemList>("Choices")->get_item_count()!=57 || get_node<Button>("Continue")->is_disabled())
                throw std::runtime_error("Arms shop list is not available in Godot");
            get_node<Button>("Continue")->emit_signal("pressed");
            if(session_->party().wealth[3]!=gold_before-price || session_->party().inventory.size()!=1)
                throw std::runtime_error("Godot purchase did not debit the purse and add inventory");
            get_node<Button>("Inventory")->emit_signal("pressed");shop_check_stage_=2;return;
        }
        capture_frame("phlan-inventory");
        get_node<Button>("InventoryPanel/Close")->emit_signal("pressed");
        get_node<Button>("LeaveShop")->emit_signal("pressed");shop_check_stage_=3;return;
    }
    if(s.phase==TourPhase::awaiting_continue){
        std::size_t selection=s.choices.size()-1;
        for(const auto* safe:{"NO","LEAVE","RUN","GO","NONE","EXIT"})
            for(std::size_t n=0;n<s.choices.size();++n)if(s.choices[n]==safe)selection=n;
        if(s.pose.x==13 && s.pose.y==8 && s.dialogue.find("SHOP")!=std::string::npos)selection=0;
        get_node<ItemList>("Choices")->select(selection);get_node<Button>("Continue")->emit_signal("pressed");return;
    }
    if(s.phase==TourPhase::awaiting_input){get_node<LineEdit>("Answer")->set_text("0");next();return;}
    if(s.phase!=TourPhase::completed)return;
    // Exercise actual movement callbacks to the original arms shop at (13,8).
    constexpr int dx[]{0,1,0,-1},dy[]{-1,0,1,0};
    std::array<int,256> previous;previous.fill(-1);std::queue<int> cells;
    const int origin=s.pose.y*16+s.pose.x;previous[origin]=origin;cells.push(origin);
    while(!cells.empty()){
        const int cell=cells.front();cells.pop();
        for(int d=0;d<4;++d){const int x=cell%16+dx[d],y=cell/16+dy[d];if(x<0||y<0||x>=16||y>=16)continue;
            const auto& a=session_->map().at(cell%16,cell/16);const auto& b=session_->map().at(x,y);const int r=(d+2)%4;
            if(a.doors[d]>1||b.doors[r]>1||(a.walls[d]&&!a.doors[d])||(b.walls[r]&&!b.doors[r]))continue;
            const int next=y*16+x;if(previous[next]>=0)continue;previous[next]=cell;cells.push(next);
        }
    }
    int next=8*16+13;
    if(next==origin){look();return;}
    if(previous[next]<0)throw std::runtime_error("No route to arms shop");
    while(previous[next]!=origin)next=previous[next];
    const unsigned facing=next%16>int(s.pose.x)?1:next%16<int(s.pose.x)?3:next/16>int(s.pose.y)?2:0;
    get_node<Button>(s.pose.facing==facing?"Forward":"Right")->emit_signal("pressed");
}
