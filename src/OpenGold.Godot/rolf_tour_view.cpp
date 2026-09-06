#include "rolf_tour_view.h"
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/label.hpp>
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

using namespace godot;
using namespace opengold::por;
namespace {
// Numeric constructors are safe before Godot initializes the extension interface.
const Color background(18/255.f,26/255.f,32/255.f), panel(28/255.f,39/255.f,46/255.f),
    line(65/255.f,80/255.f,88/255.f), gold(215/255.f,180/255.f,121/255.f),
    party_color(121/255.f,214/255.f,212/255.f);
const std::array<Vector2,4> direction{Vector2(0,-1),Vector2(1,0),Vector2(0,1),Vector2(-1,0)};
const std::array<const char*,4> direction_name{"North","East","South","West"};

// Clip perspective wall quads to the view rectangle. Prevent close side walls
// from drawing into the map or dialogue; no Godot nodes/resources are allocated.
PackedVector2Array clipped(std::vector<Vector2> points, const Rect2& r)
{
    for (int side=0; side<4; ++side) {
        if (points.empty()) break;
        const bool x_axis=side<2;
        const double limit=side==0?r.position.x:side==1?r.get_end().x:side==2?r.position.y:r.get_end().y;
        const auto value=[&](Vector2 p){return x_axis?p.x:p.y;};
        const auto inside=[&](Vector2 p){return side%2==0?value(p)>=limit:value(p)<=limit;};
        std::vector<Vector2> output;
        auto previous=points.back(); bool was_inside=inside(previous);
        for (const auto point:points) {
            const bool is_inside=inside(point);
            if (is_inside!=was_inside) {
                const auto fraction=(limit-value(previous))/(value(point)-value(previous));
                output.push_back(previous+(point-previous)*fraction);
            }
            if (is_inside) output.push_back(point);
            previous=point;was_inside=is_inside;
        }
        points=std::move(output);
    }
    PackedVector2Array result;
    for (const auto point:points) result.push_back(point);
    return result;
}
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
    get_window()->set_min_size(Vector2i(960,720));
    layout();
    if (Engine::get_singleton()->is_editor_hint()) return;
    const auto args=OS::get_singleton()->get_cmdline_user_args();
    checking_=args.has("--tour-check");capture_=args.has("--capture");
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
    const double view_height=std::min(main_width*.625,height-410.0);
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
        played_footsteps_=0; shown_revision_=0;
        session_->advance(0);
    } catch (const std::exception& error) {
        session_.reset();error_=String::utf8(error.what());
    }
    refresh();
}
void RolfTourView::next()
{
    if (session_ && session_->continue_dialogue(session_->snapshot().continue_ticket)) refresh();
}
void RolfTourView::left(){movement(ExplorationCommand::turn_left);}
void RolfTourView::right(){movement(ExplorationCommand::turn_right);}
void RolfTourView::forward(){movement(ExplorationCommand::forward);}
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
    switch (key->get_keycode()) {
    case Key::KEY_ENTER: next();break;
    case Key::KEY_LEFT: left();break;
    case Key::KEY_RIGHT: right();break;
    case Key::KEY_UP: forward();break;
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
    const bool loaded=session_.has_value();
    const TourSnapshot s=loaded?session_->snapshot():TourSnapshot{};
    shown_revision_=s.revision;
    const bool waiting=loaded&&s.phase==TourPhase::awaiting_continue;
    const bool completed=loaded&&s.phase==TourPhase::completed;
    const bool faulted=!loaded||s.phase==TourPhase::faulted;
    get_node<Label>("Location")->set_text("New Phlan  /  "+String(direction_name[s.pose.facing])+" view");
    get_node<Label>("Coordinates")->set_text("Party  ("+String::num_uint64(s.pose.x)+", "+String::num_uint64(s.pose.y)+")   "+String(direction_name[s.pose.facing]));
    get_node<Label>("Speaker")->set_text(faulted?"Unable to continue":completed?"Tour complete":"Rolf  /  Council guide");
    get_node<RichTextLabel>("Dialogue")->set_text(faulted?
        (loaded?String::utf8(s.diagnostic.c_str()):error_)+"\nSet OPENGOLD_GAME_DIR to your Pool of Radiance data folder, then restart.":
        s.dialogue.empty()?"Following Rolf...":String::utf8(s.dialogue.c_str()));
    get_node<Button>("Continue")->set_disabled(!waiting);
    get_node<Button>("Continue")->set_text(completed?"Tour finished":"Continue  [Enter]");
    get_node<Label>("Progress")->set_text(faulted?"Stopped":completed?"Explore the map or replay":waiting?"Pause "+String::num_uint64(s.prompts):"Following the guide");
    get_node<Label>("Movement")->set_text(completed?"Explore  /  arrow keys":"Movement paused during tour");
    for (const char* name:{"Left","Forward","Right"}) get_node<Button>(name)->set_disabled(!completed);
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
    const auto& r=scene_rect_;
    draw_rect(r,Color("718b95"));
    draw_rect(Rect2(r.position+Vector2(0,r.size.y*.49),Vector2(r.size.x,r.size.y*.51)),Color("585c55"));
    if (!session_) return;
    const auto& s=session_->snapshot();
    const Vector2 forward=direction[s.pose.facing], right(-forward.y,forward.x);
    // Set the eye back within the occupied cell to leave room around nearby doors.
    const Vector2 camera=Vector2(s.pose.x+.5,s.pose.y+.5)-forward*.25;
    struct Wall {Vector2 a,b;double depth;unsigned material,door,side;};
    std::vector<Wall> walls;
    const auto relative=[&](Vector2 p){const auto d=p-camera;return Vector2(d.dot(right),d.dot(forward));};
    for (unsigned y=0;y<16;++y) for (unsigned x=0;x<16;++x) {
        const auto& cell=session_->map().at(x,y);
        const std::array<Vector2,4> corners{Vector2(x,y),Vector2(x+1,y),Vector2(x+1,y+1),Vector2(x,y+1)};
        for (unsigned side=0;side<4;++side) {
            if (!cell.walls[side]&&!cell.doors[side]) continue;
            auto a=corners[side],b=corners[(side+1)%4];
            if ((camera-(a+b)*.5).dot(direction[side])>=0) continue;
            a=relative(a);b=relative(b);
            if (std::max(a.y,b.y)<.06||std::min(a.y,b.y)>4.5) continue;
            if (a.y<.06) a=a+(b-a)*((.06-a.y)/(b.y-a.y));
            if (b.y<.06) b=b+(a-b)*((.06-b.y)/(a.y-b.y));
            walls.push_back({a,b,(a.y+b.y)*.5,cell.walls[side],cell.doors[side],side});
        }
    }
    std::stable_sort(walls.begin(),walls.end(),[](const Wall& a,const Wall& b){return a.depth>b.depth;});
    // A front wall is now 0.75 cells away. Fit its full height, including the
    // door's lintel and threshold, even when the view becomes wide and shallow.
    const double focal_length=std::min(r.size.x*.68,r.size.y*.60);
    const auto project=[&](Vector2 p,double height){return r.position+Vector2(r.size.x*.5+p.x/p.y*focal_length,r.size.y*.49-height/p.y*focal_length);};
    const auto face=[&](Vector2 a,Vector2 b,double top,double bottom,Color color){
        const auto polygon=clipped({project(a,top),project(b,top),project(b,bottom),project(a,bottom)},r);
        if (polygon.size()>=3) draw_colored_polygon(polygon,color);
    };
    for (const auto& wall:walls) {
        const std::array<Color,4> colors{Color("88938d"),Color("998b72"),Color("9b7561"),Color("a5a69a")};
        auto color=colors[wall.material%colors.size()].darkened(std::min(.55,wall.depth*.075)+(wall.side%2?0.06:0.0));
        face(wall.a,wall.b,.56,-.5,color);
        // Schematic masonry guides; original WALLDEF perspective art is pending.
        for (unsigned row=0;row<5;++row) {
            const double h=.56-row*.22;
            face(wall.a,wall.b,h,h-.012,color.darkened(.28));
        }
        const auto mid=wall.a+(wall.b-wall.a)*.5;
        face(mid,mid+(wall.b-wall.a)*.012,.56,-.5,color.darkened(.22));
        if (wall.door) {
            const auto a=wall.a.lerp(wall.b,.22),b=wall.a.lerp(wall.b,.78);
            face(a,b,.30,-.5,Color("504137").darkened(std::min(.4,wall.depth*.06)));
            face(a,b,.04,.02,gold.darkened(.45));
            // A brass knob on the right, slightly below the door's midpoint.
            // Project it in the door plane so side views keep their perspective.
            const auto knob=a.lerp(b,.83), along=(b-a).normalized();
            for (unsigned layer=0;layer<2;++layer) {
                const double radius=layer==0?.025:.018;
                std::vector<Vector2> outline;
                for (unsigned point=0;point<12;++point) {
                    const double angle=point*6.283185307179586/12;
                    outline.push_back(project(knob+along*(std::cos(angle)*radius),-.15+std::sin(angle)*radius));
                }
                const auto polygon=clipped(std::move(outline),r);
                if (polygon.size()>=3) draw_colored_polygon(polygon,
                    layer==0?Color("30261b"):gold.darkened(std::min(.4,wall.depth*.06)));
            }
        }
    }
    if (s.sprite_frame>=0 && sprites_[s.sprite_frame].is_valid()) {
        const auto& image=session_->sprites()[s.sprite_frame];
        // Preserve source pixels and padding; distance selects a stored image.
        const double scale=std::max(1.0,std::floor(r.size.y/95.0));
        const Vector2 size(image.width*scale,image.height*scale);
        draw_texture_rect(sprites_[s.sprite_frame],Rect2(r.position+Vector2((r.size.x-size.x)*.5,r.size.y-size.y-8),size),false);
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
