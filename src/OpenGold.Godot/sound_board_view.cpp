#include "sound_board_view.h"
#include "opengold/speaker_audio.h"
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>
#include <filesystem>
#include <stdexcept>

using namespace godot;
using namespace opengold::por;
namespace {
String gs(std::string_view value){return String::utf8(value.data(),static_cast<int64_t>(value.size()));}
String button_name(int index){return "Sound"+String::num_int64(index+1);}
Ref<StyleBoxFlat> box(Color color,Color border,int width=1)
{
    Ref<StyleBoxFlat> result;result.instantiate();result->set_bg_color(color);
    result->set_border_color(border);result->set_border_width_all(width);
    result->set_corner_radius_all(6);result->set_content_margin_all(10);return result;
}
}
void SoundBoardView::_bind_methods(){}
void SoundBoardView::_notification(int what)
{if(what==NOTIFICATION_RESIZED&&ready_){layout();queue_redraw();}}
void SoundBoardView::_ready()
{
    ready_=true;get_window()->set_min_size(Vector2i(960,760));
    get_window()->set_title("OpenGold - Pool of Radiance Sound Board");
    // Node lookups are borrowed from the scene tree; resources use Godot Ref RAII.
    for(int i=0;i<21;++i) {
        auto* button=get_node<Button>(button_name(i));
        button->connect("pressed",callable_mp(this,&SoundBoardView::play).bind(i));
        button->add_theme_stylebox_override("hover",box(Color("30414b"),Color("d7b479")));
        button->add_theme_stylebox_override("pressed",box(Color("4b4334"),Color("edca8e"),2));
        button->add_theme_stylebox_override("focus",box(Color(0,0,0,0),Color("79d6d4"),2));
    }
    get_node<Button>("Stop")->connect("pressed",callable_mp(this,&SoundBoardView::stop));
    get_node<Button>("Mute")->connect("toggled",callable_mp(this,&SoundBoardView::mute));
    get_node<HSlider>("Volume")->connect("value_changed",callable_mp(this,&SoundBoardView::volume));
    get_node<AudioStreamPlayer>("Player")->connect("finished",callable_mp(this,&SoundBoardView::finished));
    layout();refresh_buttons();volume(get_node<HSlider>("Volume")->get_value());
    if(Engine::get_singleton()->is_editor_hint())return;
    const auto args=OS::get_singleton()->get_cmdline_user_args();
    checking_=args.has("--sound-check");capture_=args.has("--capture");
    try {
        auto directory=OS::get_singleton()->get_environment("OPENGOLD_GAME_DIR");
        if(directory.is_empty())directory=ProjectSettings::get_singleton()->get_setting("opengold/game_directory","");
        if(directory.is_empty())throw std::runtime_error("Set OPENGOLD_GAME_DIR to your Pool of Radiance folder, then restart.");
        effects_=load_sound_effects(std::filesystem::path(directory.utf16().get_data()));
        for(const auto& effect:effects_) {
            const auto pcm=render_speaker_audio(effect);
            PackedByteArray bytes;bytes.resize(static_cast<int64_t>(pcm.size()*2));
            for(std::size_t i=0;i<pcm.size();++i) {
                const auto value=static_cast<std::uint16_t>(pcm[i]);
                bytes.set(i*2,value&255);bytes.set(i*2+1,value>>8);
            }
            Ref<AudioStreamWAV> stream;stream.instantiate();
            stream->set_format(AudioStreamWAV::FORMAT_16_BITS);stream->set_mix_rate(speaker_sample_rate);
            stream->set_stereo(false);stream->set_data(bytes);streams_.push_back(stream);
            const auto i=static_cast<int>(effect.id-1);
            const auto detail=effect.audible() ? String::num(stream->get_length(),2)+" s" : "Silent control";
            get_node<Button>(button_name(i))->set_text(String::num_int64(effect.id).pad_zeros(2)+"   "+gs(effect.name)+"\n"+detail);
        }
        loaded_=true;refresh_buttons();
        get_node<Label>("Status")->set_text("Ready. Choose a sound to play.");
        get_node<Label>("Source")->set_tooltip_text(directory.path_join("START.EXE"));
    } catch(const std::exception& error) {
        get_node<Label>("Status")->set_text(gs(error.what()));
        if(checking_){UtilityFunctions::printerr(gs(error.what()));get_tree()->quit(1);}
    }
}
void SoundBoardView::layout()
{
    const auto width=get_size().x,height=get_size().y;
    const auto place=[&](const String& name,Rect2 rect) {
        auto* control=get_node<Control>(name);control->set_position(rect.position);control->set_size(rect.size);
    };
    place("Eyebrow",Rect2(28,18,width-56,22));
    place("Title",Rect2(28,43,width-56,44));
    place("Subtitle",Rect2(28,93,width-56,28));
    const double gap=12,tile_width=(width-56-gap*2)/3,tile_height=(height-278-gap*6)/7;
    for(int i=0;i<21;++i)place(button_name(i),Rect2(28+(i%3)*(tile_width+gap),140+(i/3)*(tile_height+gap),tile_width,tile_height));
    place("Status",Rect2(28,height-123,width-56,38));
    place("Stop",Rect2(28,height-70,108,38));
    place("Mute",Rect2(148,height-70,108,38));
    place("VolumeLabel",Rect2(width-372,height-66,105,30));
    place("Volume",Rect2(width-262,height-66,234,30));
    place("Source",Rect2(28,height-27,width-56,20));
}
void SoundBoardView::_draw()
{
    draw_rect(Rect2(Vector2(),get_size()),Color("121a20"));
    draw_line(Vector2(28,127),Vector2(get_size().x-28,127),Color("405058"));
}
void SoundBoardView::refresh_buttons()
{
    for(int i=0;i<21;++i) {
        auto* button=get_node<Button>(button_name(i));button->set_disabled(!loaded_);
        button->add_theme_stylebox_override("normal",box(Color(i==selected_?"3b382d":"1c272e"),Color(i==selected_?"d7b479":"405058")));
    }
}
void SoundBoardView::play(int index)
{
    if(!loaded_||index<0||static_cast<std::size_t>(index)>=streams_.size())return;
    auto* player=get_node<AudioStreamPlayer>("Player");player->stop();
    selected_=index;player->set_stream(streams_[index]);player->play();refresh_buttons();
    const auto& effect=effects_[index];
    get_node<Label>("Status")->set_text(effect.audible()?"Playing: "+gs(effect.name):gs(effect.name)+" - this entry contains no audible effect.");
}
void SoundBoardView::stop()
{
    get_node<AudioStreamPlayer>("Player")->stop();selected_=-1;refresh_buttons();
    if(loaded_)get_node<Label>("Status")->set_text("Stopped. Choose a sound to play.");
}
void SoundBoardView::finished()
{
    if(selected_>=0&&effects_[selected_].audible())
        get_node<Label>("Status")->set_text("Played: "+gs(effects_[selected_].name)+". Click to replay.");
    selected_=-1;refresh_buttons();
}
void SoundBoardView::volume(double value)
{
    const auto muted=get_node<Button>("Mute")->is_pressed();
    get_node<AudioStreamPlayer>("Player")->set_volume_db(muted||value<=0?-80:20*std::log10(value/100));
    get_node<Label>("VolumeLabel")->set_text("Volume "+String::num_int64(static_cast<int64_t>(value))+"%");
}
void SoundBoardView::mute(bool value)
{
    get_node<Button>("Mute")->set_text(value?"Unmute":"Mute");
    volume(get_node<HSlider>("Volume")->get_value());
}
void SoundBoardView::_process(double delta)
{
    ++frames_;
    if(capture_&&!captured_&&frames_>5) {
        const auto path=ProjectSettings::get_singleton()->globalize_path("res://../user-data/sound-board.png");
        get_viewport()->get_texture()->get_image()->save_png(path);captured_=true;
    }
    if(!checking_||!loaded_||frames_<8)return;
    check_elapsed_+=delta;
    auto* player=get_node<AudioStreamPlayer>("Player");
    if(check_elapsed_>20){UtilityFunctions::printerr("Sound playback timed out");get_tree()->quit(1);checking_=false;return;}
    if(player->is_playing())return;
    if(++check_index_==21) {
        get_node<HSlider>("Volume")->set_value(35);
        get_node<Button>("Mute")->set_pressed(true);
        const auto muted=player->get_volume_db()<=-80;
        get_node<Button>("Mute")->set_pressed(false);
        const auto restored=std::abs(player->get_volume_db()-20*std::log10(.35))<.01;
        get_node<Button>("Sound2")->emit_signal("pressed");
        get_node<Button>("Sound3")->emit_signal("pressed");
        const auto switched=player->get_stream()==streams_[2]&&player->is_playing();
        get_node<Button>("Stop")->emit_signal("pressed");
        const auto ok=muted&&restored&&switched&&!player->is_playing();
        UtilityFunctions::print(ok?"Sound board check passed: 21 buttons, playback completion, switching, stop, volume, mute":"Sound board control check failed");
        checking_=false;get_tree()->quit(ok?0:1);return;
    }
    check_elapsed_=0;
    get_node<Button>(button_name(check_index_))->emit_signal("pressed");
    if(player->get_stream()!=streams_[check_index_]||!player->is_playing()) {
        UtilityFunctions::printerr("Sound button did not start its stream");checking_=false;get_tree()->quit(1);
    }
}
