#include "character_creation_view.h"
#include "opengold/srd5.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>

using namespace godot;
using namespace opengold;
using namespace opengold::rules;
namespace {
String gs(std::string_view s){return String::utf8(s.data(),static_cast<int64_t>(s.size()));}
const std::array<const char*,10> steps{"Race","Gender","Class","Alignment","Attributes","Hit points","Name","Portrait","Combat appearance","Character sheet"};
const std::array<const char*,6> abilities{"STR","DEX","CON","INT","WIS","CHA"};
const std::array<const char*,6> full_abilities{"Strength","Dexterity","Constitution","Intelligence","Wisdom","Charisma"};
const std::array<const char*,16> colors{"Black","Blue","Green","Cyan","Red","Magenta","Brown","Light gray","Dark gray","Light blue","Light green","Light cyan","Light red","Pink","Yellow","White"};
const std::array<const char*,6> parts{"Weapon","Body","Hair / Face","Shield","Arms","Legs"};
std::string signed_number(int n){return (n>=0?"+":"")+std::to_string(n);}
Color ega(unsigned index){const auto c=por::character_color(index);return Color(c[0]/255.f,c[1]/255.f,c[2]/255.f);}
Ref<StyleBoxFlat> box(Color color,Color border,int width=1)
{
    Ref<StyleBoxFlat> result;result.instantiate();result->set_bg_color(color);
    result->set_border_color(border);result->set_border_width_all(width);result->set_corner_radius_all(4);
    result->set_content_margin_all(8);return result;
}
std::string selection(const CharacterDraft& d,CreationField f)
{
    switch(f){case CreationField::race:return d.race;case CreationField::gender:return d.gender;
    case CreationField::character_class:return d.character_class;case CreationField::alignment:return d.alignment;
    case CreationField::background:return d.background;}return {};
}
}
void CharacterCreationView::_bind_methods(){}
void CharacterCreationView::_notification(int what)
{if(what==NOTIFICATION_RESIZED&&ready_){layout();queue_redraw();}}
void CharacterCreationView::_ready()
{
    ready_=true;get_window()->set_min_size(Vector2i(1120,800));set_texture_filter(TEXTURE_FILTER_NEAREST);
    // All node pointers here and below are borrowed from the owning scene tree.
    get_node<Button>("Next")->connect("pressed",callable_mp(this,&CharacterCreationView::next));
    get_node<Button>("Back")->connect("pressed",callable_mp(this,&CharacterCreationView::back));
    get_node<Button>("Restart")->connect("pressed",callable_mp(this,&CharacterCreationView::restart));
    get_node<Button>("Roll")->connect("pressed",callable_mp(this,&CharacterCreationView::roll));
    get_node<ItemList>("Choices")->connect("item_selected",callable_mp(this,&CharacterCreationView::choice_selected));
    get_node<OptionButton>("Background")->connect("item_selected",callable_mp(this,&CharacterCreationView::background_selected));
    get_node<OptionButton>("Bonus")->connect("item_selected",callable_mp(this,&CharacterCreationView::bonus_selected));
    get_node<OptionButton>("PortraitHead")->connect("item_selected",callable_mp(this,&CharacterCreationView::portrait_head_selected));
    get_node<LineEdit>("Name")->connect("text_changed",callable_mp(this,&CharacterCreationView::name_changed));
    for(int i=0;i<6;++i)get_node<Button>(gs("Ability"+std::to_string(i)))->connect("pressed",callable_mp(this,&CharacterCreationView::score_selected).bind(i));
    for(int direction:{-1,1}) {
        const auto suffix=direction<0?"Previous":"Next";
        get_node<Button>(gs(std::string("Head")+suffix))->connect("pressed",callable_mp(this,&CharacterCreationView::portrait_part).bind(0,direction));
        get_node<Button>(gs(std::string("Body")+suffix))->connect("pressed",callable_mp(this,&CharacterCreationView::portrait_part).bind(1,direction));
        get_node<Button>(gs(std::string("CombatHead")+suffix))->connect("pressed",callable_mp(this,&CharacterCreationView::combat_part).bind(0,direction));
        get_node<Button>(gs(std::string("Weapon")+suffix))->connect("pressed",callable_mp(this,&CharacterCreationView::combat_part).bind(1,direction));
    }
    get_node<Button>("Size")->connect("pressed",callable_mp(this,&CharacterCreationView::toggle_size));
    for(int bank=0;bank<2;++bank)for(int part=0;part<6;++part)
        get_node<Button>(gs("Color"+std::to_string(bank)+"_"+std::to_string(part)))->connect("pressed",callable_mp(this,&CharacterCreationView::color_selected).bind(bank,part));
    for(int i=0;i<16;++i) {
        auto* button=get_node<Button>(gs("Palette"+std::to_string(i)));
        button->connect("pressed",callable_mp(this,&CharacterCreationView::palette_selected).bind(i));
        button->set_tooltip_text(colors[i]);button->add_theme_stylebox_override("normal",box(ega(i),Color("667680")));
        button->add_theme_stylebox_override("hover",box(ega(i),Color("e6c28a"),3));
        button->add_theme_stylebox_override("focus",box(Color(0,0,0,0),Color("ffffff"),2));
    }
    get_node<ItemList>("Choices")->add_theme_stylebox_override("panel",box(Color("202d34"),Color("405058")));
    layout();
    if(Engine::get_singleton()->is_editor_hint())return;
    const auto args=OS::get_singleton()->get_cmdline_user_args();checking_=args.has("--character-check");capture_=args.has("--capture");
    try {
        auto directory=OS::get_singleton()->get_environment("OPENGOLD_GAME_DIR");
        if(directory.is_empty())directory=ProjectSettings::get_singleton()->get_setting("opengold/game_directory","");
        art_=por::CharacterArt::load(std::filesystem::u8path(directory.utf8().get_data()));
        load_additional_heads();
        const auto seed=checking_?42ULL:static_cast<std::uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        creator_=std::make_unique<CharacterCreator>(srd5::character_rules(),seed);recommend_head();refresh();
    } catch(const std::exception& e) {
        fatal_=true;error_=gs(e.what());get_node<Label>("Instructions")->set_text("Character art could not be loaded. Check OPENGOLD_GAME_DIR and run build-rolf.cmd, then review-character.cmd.");
        get_node<Label>("Status")->set_text(error_);get_node<Button>("Next")->set_disabled(true);
        for(int i=0;i<get_child_count();++i)if(auto* c=Object::cast_to<Control>(get_child(i)))
            if(c->get_name()!=StringName("Title")&&c->get_name()!=StringName("Instructions")&&c->get_name()!=StringName("Status"))c->hide();
    }
}
void CharacterCreationView::layout()
{
    const double w=get_size().x,h=get_size().y;
    page_rect_=Rect2(218,112,w-584,h-188);preview_rect_=Rect2(w-342,112,318,h-188);
    const auto place=[&](const String& name,Rect2 r){auto* c=get_node<Control>(name);c->set_position(r.position);c->set_size(r.size);};
    place("Title",Rect2(24,20,w-48,36));place("Subtitle",Rect2(24,64,w-48,26));
    place("Steps",Rect2(24,128,180,h-252));place("Restart",Rect2(24,h-110,166,36));
    const double x=page_rect_.position.x,y=page_rect_.position.y,pw=page_rect_.size.x,ph=page_rect_.size.y;
    place("PageTitle",Rect2(x+20,y+18,pw-40,36));place("Instructions",Rect2(x+20,y+60,pw-40,48));
    place("Choices",Rect2(x+20,y+116,pw-40,ph-272));place("Description",Rect2(x+20,y+ph-140,pw-40,120));
    place("BackgroundLabel",Rect2(x+20,y+118,106,32));place("Background",Rect2(x+126,y+114,pw-146,36));
    place("BonusLabel",Rect2(x+20,y+164,106,32));place("Bonus",Rect2(x+126,y+160,pw-146,36));
    place("Columns",Rect2(x+20,y+208,pw-40,24));
    place("DiceHeader",Rect2(x+104,y+208,175,24));
    place("BaseHeader",Rect2(x+288,y+208,56,24));place("BonusHeader",Rect2(x+348,y+208,64,24));place("TotalHeader",Rect2(x+416,y+208,60,24));
    for(int i=0;i<6;++i) {
        place(gs("Ability"+std::to_string(i)),Rect2(x+20,y+244+i*47,68,37));
        place(gs("Dice"+std::to_string(i)),Rect2(x+104,y+248+i*47,174,37));
        place(gs("Score"+std::to_string(i)),Rect2(x+288,y+248+i*47,56,37));
        place(gs("BonusScore"+std::to_string(i)),Rect2(x+348,y+248+i*47,64,37));
        place(gs("TotalScore"+std::to_string(i)),Rect2(x+416,y+248+i*47,60,37));
    }
    place("Roll",Rect2(x+20,y+ph-58,170,36));place("SwapHint",Rect2(x+202,y+ph-62,pw-222,46));
    place("Name",Rect2(x+20,y+138,pw-40,46));
    for(const auto& stem:{std::string("Head"),std::string("Body"),std::string("CombatHead"),std::string("Weapon")}) {
        const int row=stem=="Head"||stem=="CombatHead"?0:1;
        place(gs(stem+"Previous"),Rect2(x+20,y+126+row*48,110,36));
        if(stem=="Head")place("PortraitHead",Rect2(x+144,y+126,pw-290,36));
        else place(gs(stem+"Label"),Rect2(x+144,y+130+row*48,pw-290,30));
        place(gs(stem+"Next"),Rect2(x+pw-130,y+126+row*48,110,36));
    }
    place("Size",Rect2(x+20,y+222,150,34));place("ColorTitle",Rect2(x+20,y+264,pw-40,26));
    const double colorw=(pw-166)/2;
    place("Color1Title",Rect2(x+130,y+264,colorw,26));place("Color2Title",Rect2(x+138+colorw,y+264,colorw,26));
    for(int part=0;part<6;++part) {
        place(gs("Part"+std::to_string(part)),Rect2(x+20,y+300+part*35,105,28));
        for(int bank=0;bank<2;++bank)place(gs("Color"+std::to_string(bank)+"_"+std::to_string(part)),Rect2(x+130+bank*(colorw+8),y+294+part*35,colorw,30));
    }
    place("PaletteHint",Rect2(x+20,y+ph-106,pw-40,24));
    const double swatch=(pw-40-7*6)/8;
    for(int i=0;i<16;++i)place(gs("Palette"+std::to_string(i)),Rect2(x+20+(i%8)*(swatch+6),y+ph-76+(i/8)*30,swatch,25));
    const double px=preview_rect_.position.x,py=preview_rect_.position.y;
    place("PreviewTitle",Rect2(px+18,py+18,282,24));place("PreviewName",Rect2(px+18,py+50,282,36));
    portrait_rect_=Rect2(px+27,py+100,264,264);
    ready_rect_=Rect2(px+26,py+408,120,120);action_rect_=Rect2(px+172,py+408,120,120);
    place("ReadyLabel",Rect2(px+26,py+374,120,28));place("ActionLabel",Rect2(px+172,py+374,120,28));
    if(creator_&&creator_->step()==CreationStep::combat_icon) {
        ready_rect_=Rect2(px+63,py+112,192,192);action_rect_=Rect2(px+63,py+348,192,192);
        place("ReadyLabel",Rect2(px+63,py+88,192,24));place("ActionLabel",Rect2(px+63,py+324,192,24));
    }
    place("PreviewSummary",Rect2(px+18,py+550,282,std::max(42.0,ph-564)));
    place("Back",Rect2(x,h-60,150,38));place("Next",Rect2(x+pw-190,h-60,190,38));
    place("Status",Rect2(x+160,h-62,std::max(1.0,pw-360),44));
    place("Footer",Rect2(24,h-25,w-48,22));
    if(creator_&&(creator_->step()==CreationStep::sheet||creator_->step()==CreationStep::hit_points))
        place("Description",Rect2(x+20,y+124,pw-40,ph-148));
}
void CharacterCreationView::load_additional_heads()
{
    for(const auto& head:por::additional_portrait_heads()) {
        const auto path=gs("res://bin/portraits/"+std::string(head.filename));
        Ref<Texture2D> texture=ResourceLoader::get_singleton()->load(path);
        if(texture.is_null())throw std::runtime_error("Missing portrait: "+std::string(head.filename)+". Run build-rolf.cmd and review-character.cmd.");
        auto source=texture->get_image();
        if(source.is_null()||(source->is_compressed()&&source->decompress()!=OK))throw std::runtime_error("Cannot decode portrait: "+std::string(head.filename));
        source->convert(godot::Image::FORMAT_RGBA8);
        const auto pixels=source->get_data();opengold::Image decoded;
        decoded.width=source->get_width();decoded.height=source->get_height();
        decoded.rgba.assign(pixels.ptr(),pixels.ptr()+pixels.size());
        art_->add_portrait_head(head.id,por::prepare_portrait_head(decoded,head.id));
    }
}
void CharacterCreationView::recommend_head()
{
    if(portrait_chosen_)return;
    auto a=creator_->appearance();const auto& d=creator_->draft();
    a.portrait_head=por::matching_portrait_head(d.race,d.gender).value_or(1);
    creator_->appearance(a);
}
void CharacterCreationView::refresh_art()
{
    if(!creator_||!art_||rendered_==creator_->appearance())return;
    const auto& a=creator_->appearance();
    const std::array<opengold::Image,3> images{art_->portrait(a),art_->icon(a,false),art_->icon(a,true)};
    for(unsigned i=0;i<images.size();++i) {
        const auto& source=images[i];PackedByteArray pixels;pixels.resize(source.rgba.size());
        std::copy(source.rgba.begin(),source.rgba.end(),pixels.ptrw());
        images_[i]=ImageTexture::create_from_image(godot::Image::create_from_data(source.width,source.height,false,godot::Image::FORMAT_RGBA8,pixels));
    }
    rendered_=a;
}
void CharacterCreationView::refresh()
{
    if(!creator_)return;refreshing_=true;
    const auto step=creator_->step();const auto& d=creator_->draft();const auto& a=creator_->appearance();
    const auto show=[&](const char* node,bool visible){get_node<Control>(node)->set_visible(visible);};
    const bool choosing=step<=CreationStep::alignment,stats=step==CreationStep::attributes,portrait=step==CreationStep::portrait,icon=step==CreationStep::combat_icon;
    for(const auto* n:{"Choices"})show(n,choosing);
    show("Description",choosing||step==CreationStep::hit_points||step==CreationStep::sheet);
    for(const auto* n:{"BackgroundLabel","Background","BonusLabel","Bonus","Columns","DiceHeader","BaseHeader","BonusHeader","TotalHeader","Roll","SwapHint"})show(n,stats);
    show("Name",step==CreationStep::name);
    for(const auto* n:{"HeadPrevious","HeadNext","PortraitHead","BodyPrevious","BodyNext","BodyLabel"})show(n,portrait);
    for(const auto* n:{"CombatHeadPrevious","CombatHeadNext","CombatHeadLabel","WeaponPrevious","WeaponNext","WeaponLabel","Size","ColorTitle","Color1Title","Color2Title","PaletteHint"})show(n,icon);
    for(int i=0;i<6;++i) {
        for(const auto& stem:{std::string("Ability"),std::string("Dice"),std::string("Score"),std::string("BonusScore"),std::string("TotalScore")})get_node<Control>(gs(stem+std::to_string(i)))->set_visible(stats);
        get_node<Control>(gs("Part"+std::to_string(i)))->set_visible(icon);
        for(int bank=0;bank<2;++bank)get_node<Control>(gs("Color"+std::to_string(bank)+"_"+std::to_string(i)))->set_visible(icon);
    }
    for(int i=0;i<16;++i)get_node<Control>(gs("Palette"+std::to_string(i)))->set_visible(icon);
    get_node<Label>("PageTitle")->set_text(gs(steps[static_cast<unsigned>(step)]));
    std::string progress;
    for(unsigned i=0;i<steps.size();++i)progress+=(i==static_cast<unsigned>(step)?"> ":"  ")+std::to_string(i+1)+". "+(i==8?"Combat icon":steps[i])+"\n\n";
    get_node<Label>("Steps")->set_text(gs(progress));
    get_node<Button>("Back")->set_disabled(step==CreationStep::race);
    get_node<Button>("Back")->set_text(step==CreationStep::sheet?"Edit appearance":"Back");
    show("Next",step!=CreationStep::sheet);
    get_node<Button>("Next")->set_text(icon?"Show character sheet":"Next");
    get_node<Button>("Next")->set_disabled((stats&&!d.rolled)||(step==CreationStep::name&&d.name.empty()));
    get_node<Label>("Status")->set_text(error_);
    std::string instructions;
    if(choosing) {
        const auto field=static_cast<CreationField>(step);const auto choices=creator_->rules().choices(field);
        auto* list=get_node<ItemList>("Choices");list->clear();list->add_theme_constant_override("v_separation",18);
        for(unsigned i=0;i<choices.size();++i) {
            list->add_item(gs(choices[i].label));
            if(choices[i].id==selection(d,field)){list->select(i);get_node<RichTextLabel>("Description")->set_text(gs(choices[i].description));}
        }
        instructions=step==CreationStep::race?"Choose your race (called species in SRD 5.2.1).":"Select an option, then continue.";
    }
    if(stats) {
        instructions="Assign your rolls by selecting two attribute buttons to swap. Background bonuses stay with their attributes.";
        auto* background=get_node<OptionButton>("Background");background->clear();
        const auto choices=creator_->rules().choices(CreationField::background);
        for(unsigned i=0;i<choices.size();++i){background->add_item(gs(choices[i].label));if(choices[i].id==d.background)background->select(i);}
        auto* bonus=get_node<OptionButton>("Bonus");bonus->clear();
        for(const auto& option:creator_->rules().adjustments(d.background))bonus->add_item(gs(option.label));bonus->select(d.adjustment);
        get_node<Button>("Roll")->set_text(d.rolled?"Reroll all six":"Roll all six");
        get_node<Label>("SwapHint")->set_text(selected_score_<0?"Select two attributes to swap.":gs(std::string("Swap ")+abilities[selected_score_]+" with another attribute."));
    }
    std::optional<CharacterSheet> s;
    if(completed_)s=completed_->sheet();
    else if(d.rolled)s=creator_->sheet();
    if(stats)for(unsigned i=0;i<6;++i) {
        auto* b=get_node<Button>(gs("Ability"+std::to_string(i)));b->set_text(gs(std::string(selected_score_==i?"> ":"")+abilities[i]));b->set_disabled(!d.rolled);
        std::string dice="--   --   --   --";
        if(s) {
            dice.clear();const auto& r=d.rolls[d.assignment[i]];
            for(unsigned j=0;j<4;++j) {
                if(j==r.discarded)dice+="[color=#bd8585][s]";
                dice+=std::to_string(r.dice[j]);if(j==r.discarded)dice+="[/s][/color]";dice+="   ";
            }
        }
        get_node<RichTextLabel>(gs("Dice"+std::to_string(i)))->set_text(gs(dice));
        get_node<Label>(gs("Score"+std::to_string(i)))->set_text(s?gs(std::to_string(s->base[i])):String("--"));
        get_node<Label>(gs("BonusScore"+std::to_string(i)))->set_text(s?gs(signed_number(s->bonuses[i])):String("--"));
        get_node<Label>(gs("TotalScore"+std::to_string(i)))->set_text(s?gs(std::to_string(s->scores[i])):String("--"));
    }
    if(step==CreationStep::hit_points) {
        instructions="At level 1, take the maximum class Hit Die and add your Constitution modifier.";
        get_node<RichTextLabel>("Description")->set_text(gs("[font_size=48]"+std::to_string(s->hit_points)+" HP[/font_size]\n\n"+s->hp_explanation+"\n\nHit Dice: 1d"+std::to_string(s->hit_die)+"\n\nThese values update if you change class, race, or Constitution."));
    }
    if(step==CreationStep::name)instructions="Choose a name for your character (up to 40 characters).";
    if(portrait)instructions="Choose a head from the list or browse with the arrows, then choose a body. The preview updates immediately.";
    if(icon)instructions="Select a part's Color-1 or Color-2, then a swatch. Watch both poses change. Absent parts are disabled.";
    if(portrait) {
        auto* heads=get_node<OptionButton>("PortraitHead");heads->clear();
        for(const auto& [id,part]:art_->heads) {
            heads->add_item(gs(part.label.empty()?"Original head "+std::to_string(id):part.label),id);
            if(id==a.portrait_head)heads->select(heads->get_item_count()-1);
        }
    }
    get_node<Label>("BodyLabel")->set_text(gs("Body "+std::to_string(a.portrait_body)));
    get_node<Label>("CombatHeadLabel")->set_text(gs("Head "+std::to_string(a.combat_head+1)+" / 14"));
    get_node<Label>("WeaponLabel")->set_text(gs("Weapon "+std::to_string(a.combat_body+1)+" / 32"));
    get_node<Button>("Size")->set_text(a.tall?"Size: Tall":"Size: Short");
    if(icon) {
        const auto usage=art_->color_usage(a);
        if(!usage.contains(color_bank_,color_part_)) {
            for(unsigned i=0;i<12;++i)if(usage.contains(i/6,i%6)){color_bank_=i/6;color_part_=i%6;break;}
        }
        get_node<Label>("PaletteHint")->set_text(gs(std::string(parts[color_part_])+" / Color-"+std::to_string(color_bank_+1)+": choose a color"));
        for(int bank=0;bank<2;++bank)for(int part=0;part<6;++part) {
            auto* button=get_node<Button>(gs("Color"+std::to_string(bank)+"_"+std::to_string(part)));
            const bool selected=bank==color_bank_&&part==color_part_;const auto color=ega(a.colors[bank][part]);
            const bool present=usage.contains(bank,part);
            button->set_disabled(!present);
            button->set_tooltip_text(present?"Choose a swatch to recolor this part in the combat preview.":"This part is not present in either pose. Choose another head or weapon to use it.");
            button->set_text(present?gs(std::string(selected?"> ":"")+colors[a.colors[bank][part]]):String("Not present"));
            button->add_theme_stylebox_override("normal",box(color,selected?Color("f1d29c"):Color("62707a"),selected?3:1));
            button->add_theme_stylebox_override("hover",box(color,Color("ffffff"),2));
            button->add_theme_stylebox_override("disabled",box(Color("253038"),Color("405058")));
            const auto foreground=(color.r*.299+color.g*.587+color.b*.114)>.5?Color("101820"):Color("ffffff");
            button->add_theme_color_override("font_color",foreground);button->add_theme_color_override("font_hover_color",foreground);
        }
    }
    if(step==CreationStep::sheet) {
        instructions="Your character sheet. Use Back to adjust the appearance, or Start over to create another character.";
        std::string text="[font_size=24]Level 1 "+s->race+" "+s->character_class+"[/font_size]\n"+s->gender+" / "+s->alignment+"\nBackground: "+s->background+"\n\n";
        text+="[font_size=26]"+std::to_string(s->hit_points)+" HP[/font_size]     Hit Dice: 1d"+std::to_string(s->hit_die)+"\n\n";
        text+="[table=4][cell]Attribute     [/cell][cell]Score     [/cell][cell]Modifier     [/cell][cell]Base + bonus[/cell]";
        for(unsigned i=0;i<6;++i)text+="[cell]"+std::string(full_abilities[i])+"[/cell][cell]"+std::to_string(s->scores[i])+"[/cell][cell]"+signed_number(s->modifiers[i])+"[/cell][cell]"+std::to_string(s->base[i])+" "+signed_number(s->bonuses[i])+"[/cell]";
        text+="[/table]\n\n"+s->hp_explanation;
        text+="\n\n[b]Inventory[/b]";
        if(completed_->inventory().empty())text+="\nEmpty";
        else for(const auto& item:completed_->inventory().items())text+="\n"+item.name+" x"+std::to_string(item.quantity);
        get_node<RichTextLabel>("Description")->set_text(gs(text));
    }
    get_node<Label>("Instructions")->set_text(gs(instructions));
    get_node<Label>("PreviewTitle")->set_text(icon?"COMBAT PREVIEW":"CHARACTER PREVIEW");
    get_node<Label>("PreviewName")->set_text(d.name.empty()?"Unnamed character":gs(d.name));
    get_node<Label>("PreviewSummary")->set_text(s?gs(s->race+" / "+s->character_class+"\n"+s->alignment+" / "+std::to_string(s->hit_points)+" HP"):String("Choose your character's details."));
    refresh_art();layout();queue_redraw();refreshing_=false;
}
void CharacterCreationView::_draw()
{
    draw_rect(Rect2(Vector2(),get_size()),Color("121a20"));
    for(const auto& rect:{page_rect_,preview_rect_}){draw_rect(rect,Color("1c272e"));draw_rect(rect,Color("405058"),false);}
    for(const auto& rect:{ready_rect_,action_rect_})draw_rect(rect,Color("10171c"));
    if(!creator_||creator_->step()!=CreationStep::combat_icon) {
        draw_rect(portrait_rect_,Color("10171c"));
        if(images_[0].is_valid())draw_texture_rect(images_[0],portrait_rect_,false);
    }
    if(images_[1].is_valid())draw_texture_rect(images_[1],ready_rect_,false);
    if(images_[2].is_valid())draw_texture_rect(images_[2],action_rect_,false);
}
void CharacterCreationView::perform(const std::function<void()>& action)
{
    if(!creator_||fatal_)return;
    try{error_="";action();refresh();}
    catch(const std::exception& e){error_=gs(e.what());refreshing_=false;get_node<Label>("Status")->set_text(error_);}
}
void CharacterCreationView::next(){perform([&]{creator_->next();if(creator_->step()==CreationStep::sheet)completed_=creator_->create_character();selected_score_=-1;});}
void CharacterCreationView::back(){perform([&]{creator_->back();completed_.reset();selected_score_=-1;});}
void CharacterCreationView::restart(){perform([&]{creator_->restart();completed_.reset();portrait_chosen_=false;recommend_head();get_node<LineEdit>("Name")->set_text("");selected_score_=-1;});}
void CharacterCreationView::choice_selected(std::int64_t index)
{if(refreshing_)return;perform([&]{const auto f=static_cast<CreationField>(creator_->step());creator_->select(f,creator_->rules().choices(f).at(index).id);if(f==CreationField::race||f==CreationField::gender)recommend_head();});}
void CharacterCreationView::background_selected(std::int64_t index)
{if(refreshing_)return;perform([&]{creator_->select(CreationField::background,creator_->rules().choices(CreationField::background).at(index).id);});}
void CharacterCreationView::bonus_selected(std::int64_t index)
{if(refreshing_)return;perform([&]{creator_->select_adjustment(static_cast<unsigned>(index));});}
void CharacterCreationView::roll(){perform([&]{creator_->roll();selected_score_=-1;});}
void CharacterCreationView::score_selected(int index)
{perform([&]{if(selected_score_<0)selected_score_=index;else{creator_->swap_scores(selected_score_,index);selected_score_=-1;}});}
void CharacterCreationView::name_changed(String value){perform([&]{creator_->name(value.utf8().get_data());});}
void CharacterCreationView::portrait_part(int part,int direction)
{
    perform([&]{auto a=creator_->appearance();auto& id=part==0?a.portrait_head:a.portrait_body;const auto& parts=part==0?art_->heads:art_->bodies;
        auto it=parts.find(id);if(direction>0){if(++it==parts.end())it=parts.begin();}else{if(it==parts.begin())it=parts.end();--it;}
        id=it->first;creator_->appearance(a);if(part==0)portrait_chosen_=true;});
}
void CharacterCreationView::portrait_head_selected(std::int64_t index)
{
    if(refreshing_)return;
    perform([&]{auto* heads=get_node<OptionButton>("PortraitHead");
        if(index<0||index>=heads->get_item_count())throw std::runtime_error("Invalid portrait head selection");
        auto a=creator_->appearance();a.portrait_head=heads->get_item_id(index);art_->validate(a);
        creator_->appearance(a);portrait_chosen_=true;});
}
void CharacterCreationView::combat_part(int part,int direction)
{perform([&]{auto a=creator_->appearance();auto& id=part==0?a.combat_head:a.combat_body;const int count=part==0?14:32;id=(static_cast<int>(id)+direction+count)%count;creator_->appearance(a);});}
void CharacterCreationView::toggle_size(){perform([&]{auto a=creator_->appearance();a.tall=!a.tall;creator_->appearance(a);});}
void CharacterCreationView::color_selected(int bank,int part){perform([&]{color_bank_=bank;color_part_=part;});}
void CharacterCreationView::palette_selected(int index){perform([&]{auto a=creator_->appearance();a.colors[color_bank_][color_part_]=index;creator_->appearance(a);});}
void CharacterCreationView::capture(const char* name)
{
    if(!capture_)return;
    const auto path=std::filesystem::u8path(ProjectSettings::get_singleton()->globalize_path(gs(std::string("res://../user-data/")+name)).utf8().get_data());
    std::filesystem::create_directories(path.parent_path());const auto image=get_viewport()->get_texture()->get_image();
    if(image.is_null()||image->save_png(gs(path.generic_string()))!=OK)throw std::runtime_error("Character capture failed");
}
void CharacterCreationView::_process(double)
{
    if(!checking_||Engine::get_singleton()->is_editor_hint())return;
    try{if(fatal_||!error_.is_empty())throw std::runtime_error(error_.utf8().get_data());if(++check_frames_%4==0)check_run();
        if(check_frames_>400)throw std::runtime_error("Character UI check timed out");}
    catch(const std::exception& e){UtilityFunctions::printerr("Character UI check failed at stage ",check_stage_,": ",gs(e.what()));checking_=false;get_tree()->quit(1);}
}
void CharacterCreationView::capture_portrait_armor()
{
    if(!capture_)return;
    constexpr std::array<unsigned,5> heads{1,258,260,262,264};
    constexpr std::array<unsigned,3> bodies{1,18,26};
    PackedByteArray pixels;pixels.resize(440*264*4);
    for(unsigned row=0;row<bodies.size();++row)for(unsigned column=0;column<heads.size();++column) {
        por::CharacterAppearance a;a.portrait_head=heads[column];a.portrait_body=bodies[row];const auto portrait=art_->portrait(a);
        for(unsigned y=0;y<88;++y)std::copy_n(portrait.rgba.begin()+y*88*4,88*4,pixels.ptrw()+((row*88+y)*440+column*88)*4);
    }
    const auto image=godot::Image::create_from_data(440,264,false,godot::Image::FORMAT_RGBA8,pixels);
    image->resize(1760,1056,godot::Image::INTERPOLATE_NEAREST);
    const auto path=ProjectSettings::get_singleton()->globalize_path("res://../user-data/character-portrait-armor.png");
    if(image->save_png(path)!=OK)throw std::runtime_error("Portrait armor capture failed");
}
void CharacterCreationView::check_run()
{
    const auto click=[&](Vector2 position) {
        for(bool pressed:{true,false}){Ref<InputEventMouseButton> event;event.instantiate();event->set_position(position);event->set_global_position(position);
            event->set_button_index(MouseButton::MOUSE_BUTTON_LEFT);event->set_pressed(pressed);get_viewport()->push_input(event,true);}
    };
    const auto press=[&](const char* name){auto* button=get_node<Button>(name);if(!button->is_visible_in_tree()||button->is_disabled())throw std::runtime_error(std::string("Unavailable button: ")+name);click(button->get_global_rect().get_center());};
    const auto choose=[&](CreationField field,const char* id) {
        const auto choices=creator_->rules().choices(field);
        for(unsigned i=0;i<choices.size();++i)if(choices[i].id==id){auto* list=get_node<ItemList>("Choices");click(list->get_global_position()+list->get_item_rect(i).get_center());return;}
        throw std::runtime_error("Missing UI choice");
    };
    // ItemList lays out refreshed entries on the next frame. Keep race and
    // gender clicks on separate idle turns, as a player would see them.
    if(check_stage_==0&&check_default_<8) {
        if(check_default_<5) {
            const auto& head=por::additional_portrait_heads()[check_default_*2+1];
            choose(CreationField::race,std::string(head.race).c_str());
            if(creator_->appearance().portrait_head!=head.id)throw std::runtime_error("Race did not recommend its new head: "+std::string(head.race));
        } else if(check_default_==5) {choose(CreationField::race,"goliath");press("Next");}
        else if(check_default_==6) {
            choose(CreationField::gender,"male");if(creator_->appearance().portrait_head!=260)throw std::runtime_error("Male Goliath recommendation failed");
        } else {
            choose(CreationField::gender,"female");if(creator_->appearance().portrait_head!=261)throw std::runtime_error("Female Goliath recommendation failed");
            press("Back");
        }
        ++check_default_;return;
    }
    // Walk all ten new heads through the actual arrow buttons. Allow a draw
    // between selections so captures show the corresponding composed portrait.
    if(check_stage_==11&&check_head_<10) {
        const auto& head=por::additional_portrait_heads()[9-check_head_];
        const auto& a=creator_->appearance();
        if(a.portrait_head!=head.id||get_node<OptionButton>("PortraitHead")->get_selected_id()!=head.id)
            throw std::runtime_error("New head navigation or dropdown selection failed");
        const auto expected=art_->portrait(a);const auto pixels=images_[0]->get_image()->get_data();
        if(pixels.size()!=expected.rgba.size()||!std::equal(expected.rgba.begin(),expected.rgba.end(),pixels.ptr()))
            throw std::runtime_error("New portrait preview texture is stale");
        for(const auto& [body_id,body]:art_->bodies) {
            auto selection=a;selection.portrait_body=body_id;const auto joined=art_->portrait(selection);
            unsigned left=88,right=0;
            for(unsigned x=30;x<62;++x)if(body.image.rgba[x*4]==255&&body.image.rgba[x*4+1]==85&&body.image.rgba[x*4+2]==85){left=std::min(left,x);right=x+1;}
            if(right<=left)throw std::runtime_error("Original body has no neck opening");
            for(const auto x:{left,right-1}) {
                const auto p=(39*88+x)*4;
                if(std::max({joined.rgba[p],joined.rgba[p+1],joined.rgba[p+2]})<=24)
                    throw std::runtime_error("Portrait neck misses body opening: "+std::string(head.filename)+" / "+std::to_string(body_id));
            }
            if(!std::equal(body.image.rgba.begin(),body.image.rgba.end(),joined.rgba.begin()+88*40*4))
                throw std::runtime_error("Neck alignment changed the original body");
        }
        press("BodyNext");
        const auto changed_body=art_->portrait(creator_->appearance());const auto changed_pixels=images_[0]->get_image()->get_data();
        if(changed_pixels.size()!=changed_body.rgba.size()||!std::equal(changed_body.rgba.begin(),changed_body.rgba.end(),changed_pixels.ptr()))
            throw std::runtime_error("Changing bodies did not update neck alignment");
        press("BodyPrevious");
        if(head.race=="goliath") {
            unsigned colored=0;for(unsigned p=0;p<88*40*4;p+=4)
                if(std::max({pixels[p],pixels[p+1],pixels[p+2]})-std::min({pixels[p],pixels[p+1],pixels[p+2]})>20)++colored;
            if(colored<100)throw std::runtime_error("Goliath portrait lost its approved colors");
        }
        capture(("character-portrait-"+std::string(head.filename)).c_str());
        ++check_head_;press("HeadPrevious");return;
    }
    switch(check_stage_++) {
    case 0:choose(CreationField::race,"dwarf");press("Next");break;
    case 1:choose(CreationField::gender,"female");press("Next");break;
    case 2:choose(CreationField::character_class,"fighter");press("Next");break;
    case 3:choose(CreationField::alignment,"neutral_good");press("Next");break;
    case 4:if(!get_node<Button>("Next")->is_disabled())throw std::runtime_error("Unrolled scores accepted");press("Roll");break;
    case 5:{const auto old=creator_->draft().rolls;press("Roll");if(old==creator_->draft().rolls)throw std::runtime_error("Reroll did not replace dice");
        get_node<OptionButton>("Background")->emit_signal("item_selected",3);get_node<OptionButton>("Bonus")->emit_signal("item_selected",1);break;}
    case 6:{const auto old=creator_->draft().assignment;press("Ability0");press("Ability2");if(creator_->draft().assignment[0]!=old[2])throw std::runtime_error("UI swap failed");break;}
    case 7:capture("character-attributes.png");press("Next");break;
    case 8:if(creator_->sheet().hit_points!=11+creator_->sheet().modifiers[2])throw std::runtime_error("Wrong fighter/dwarf HP");press("Next");break;
    case 9:if(!get_node<Button>("Next")->is_disabled())throw std::runtime_error("Empty name accepted");
        get_node<LineEdit>("Name")->grab_focus();
        for(char c:std::string("Mira Stoneward"))for(bool pressed:{true,false}){Ref<InputEventKey> event;event.instantiate();event->set_unicode(c);
            event->set_keycode(static_cast<Key>(c>='a'&&c<='z'?c-32:c));event->set_pressed(pressed);get_viewport()->push_input(event,true);}
        break; // LineEdit publishes text_changed on the next idle turn.
    case 10:press("Next");press("BodyNext");
        get_node<OptionButton>("PortraitHead")->select(0);get_node<OptionButton>("PortraitHead")->emit_signal("item_selected",0);
        press("HeadPrevious");break;
    case 11:
        // Select a new head using the dropdown as well as the arrow buttons.
        {auto* heads=get_node<OptionButton>("PortraitHead");
        for(int i=0;i<heads->get_item_count();++i)if(heads->get_item_id(i)==261){heads->select(i);heads->emit_signal("item_selected",i);break;}}
        if(creator_->appearance().portrait_head!=261)throw std::runtime_error("Portrait dropdown input failed");
        press("Back");press("Next"); // Returning through Name must preserve a manual head choice.
        if(creator_->appearance().portrait_head!=261)throw std::runtime_error("Manual head selection was replaced on Back/Next");
        press("Next");
        for(int bank=0;bank<2;++bank)for(int part:{0,3})
            if(!get_node<Button>(gs("Color"+std::to_string(bank)+"_"+std::to_string(part)))->is_disabled())throw std::runtime_error("Absent weapon/shield control enabled");
        press("CombatHeadNext");for(int i=0;i<4;++i)press("WeaponNext");press("Size");break;
    case 12:
        capture("character-before-colors.png");
        for(int bank=0;bank<2;++bank)for(int part=0;part<6;++part) {
            const auto before=creator_->appearance();const auto usage=art_->color_usage(before);
            const auto chosen=(before.colors[bank][part]+3)%16;
            const auto portrait=images_[0]->get_image()->get_data();
            const std::array<PackedByteArray,2> old{images_[1]->get_image()->get_data(),images_[2]->get_image()->get_data()};
            press(("Color"+std::to_string(bank)+"_"+std::to_string(part)).c_str());press(("Palette"+std::to_string(chosen)).c_str());
            if(creator_->appearance().colors[bank][part]!=chosen)throw std::runtime_error("Color input did not change the requested region");
            if(images_[0]->get_image()->get_data()!=portrait)throw std::runtime_error("Combat colors changed the portrait");
            for(unsigned pose=0;pose<2;++pose) {
                const auto pixels=images_[pose+1]->get_image()->get_data();
                const auto expected=art_->icon(creator_->appearance(),pose!=0);
                if(pixels.size()!=expected.rgba.size()||!std::equal(expected.rgba.begin(),expected.rgba.end(),pixels.ptr()))
                    throw std::runtime_error("Preview texture is stale after palette input");
                unsigned changed=0;for(int64_t p=0;p<pixels.size();p+=4)
                    if(!std::equal(pixels.ptr()+p,pixels.ptr()+p+4,old[pose].ptr()+p))++changed;
                if(changed!=(pose?usage.action:usage.ready)[bank][part])throw std::runtime_error("Recolor changed the wrong number of visible pixels");
            }
        }break;
    case 13:capture("character-appearance.png");press("Next");break;
    case 14:if(creator_->step()!=CreationStep::sheet||!completed_||completed_->sheet().name!="Mira Stoneward"||!completed_->inventory().empty()||completed_->appearance()!=creator_->appearance()||completed_->appearance().portrait_head!=261)throw std::runtime_error("Character not completed with the selected new head");capture("character-sheet.png");press("Back");break;
    case 15:{const auto a=creator_->appearance();press("Size");press("CombatHeadNext");
        if(!get_node<Button>("Color0_2")->is_disabled())throw std::runtime_error("Helmet-covered hair control enabled");
        press("CombatHeadPrevious");press("Size");press("Next");
        if(a!=creator_->appearance()||a!=completed_->appearance())throw std::runtime_error("Appearance lost on part changes or review");press("Restart");break;}
    case 16:if(completed_||creator_->draft().rolled||!creator_->draft().name.empty()||creator_->step()!=CreationStep::race)throw std::runtime_error("Start over did not clear the character");
        if(creator_->appearance().portrait_head!=265)throw std::runtime_error("Restart did not restore the default race's recommended head");
        capture_portrait_armor();
        UtilityFunctions::print("Godot C++ character check passed: choices, dice, swaps, background, HP, name, ten new portrait heads fitted to all original bodies, race/gender defaults, twelve live texture recolors, character/inventory, sheet, edit, restart");checking_=false;get_tree()->quit(0);break;
    }
}
