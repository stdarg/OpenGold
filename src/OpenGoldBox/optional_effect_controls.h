#ifndef OPENGOLDBOX_OPTIONAL_EFFECT_CONTROLS_H
#define OPENGOLDBOX_OPTIONAL_EFFECT_CONTROLS_H
#include "godot_nodes.h"
#include "localization.h"
#include "opengold/rules.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/window.hpp>
namespace presentation {
template<class Text>void setup_optional_effect(godot::Node& root,Text text,
    const godot::Callable& use,const godot::Callable& skip,const godot::Callable& input){
    using namespace godot;auto owned=make_node<Window>();owned->set_name("OptionalEffect");
    owned->set_size(Vector2i(640,300));owned->set_min_size(Vector2i(640,300));
    owned->set_flag(Window::FLAG_RESIZE_DISABLED,true);owned->set_transient(true);owned->set_exclusive(true);owned->hide();
    auto* w=attach_child(root,std::move(owned));w->connect("close_requested",skip);w->connect("window_input",input);
    auto* label=add_control<Label>(*w,"Text",Rect2(24,18,592,194));label->set("autowrap_mode",3);
    auto* no=add_control<Button>(*w,"Skip",Rect2(284,236,150,40));no->set_text(text(N_("Skip")));no->connect("pressed",skip);
    auto* yes=add_control<Button>(*w,"Use",Rect2(446,236,170,40));yes->set_text(text(N_("Use")));yes->connect("pressed",use);
}
template<class Render>void refresh_optional_effect(godot::Node& root,
    const std::optional<opengold::rules::OptionalEffectChoice>& choice,bool player,Render render){
    using namespace godot;auto* w=root.get_node<Window>("OptionalEffect");
    if(!player||!choice){if(w->is_visible()){w->hide();if(player)root.get_node<Button>("End")->grab_focus();}return;}
    w->set_title(render(choice->title));w->get_node<Label>("Text")->set_text(render(choice->description));
    if(!w->is_visible()){w->popup_centered();w->get_node<Button>("Use")->grab_focus();}
}
}
#endif
