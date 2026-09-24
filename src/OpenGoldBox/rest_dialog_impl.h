// One implementation for the game and demo; the host supplies rest_text().
#include "godot_nodes.h"
#include <godot_cpp/classes/option_button.hpp>
#ifndef N_
#define N_(message) message
#endif
void RolfTourView::setup_rest()
{
    auto owned=presentation::make_node<Window>();owned->set_name("RestDialog");
    owned->set_title(rest_text(N_("Rest")));owned->set_size(Vector2i(720,640));owned->set_min_size(Vector2i(720,640));
    owned->set_flag(Window::FLAG_RESIZE_DISABLED,true);owned->set_transient(true);owned->set_exclusive(true);owned->hide();
    auto* w=presentation::attach_child(*this,std::move(owned));
    w->connect("close_requested",callable_mp(this,&RolfTourView::rest_finish));
    w->connect("window_input",callable_mp(this,&RolfTourView::rest_input));
    auto* label=presentation::add_control<Label>(*w,"KindLabel",Rect2(24,18,160,34));label->set_text(rest_text(N_("Rest type")));
    auto* kind=presentation::add_control<OptionButton>(*w,"Kind",Rect2(190,18,506,36));
    kind->add_item(rest_text(N_("Short Rest")),0);kind->add_item(rest_text(N_("Long Rest")),1);kind->select(1);
    kind->connect("item_selected",callable_mp(this,&RolfTourView::rest_selected));
    auto* list=presentation::add_control<ItemList>(*w,"Members",Rect2(24,68,672,192));
    list->connect("item_selected",callable_mp(this,&RolfTourView::rest_selected));
    auto* info=presentation::add_control<RichTextLabel>(*w,"Info",Rect2(24,274,672,208));info->set_scroll_active(true);
    auto* result=presentation::add_control<Label>(*w,"Result",Rect2(24,490,672,70));result->set("autowrap_mode",3);
    auto button=[&](const char* name,const char* text,Rect2 rect,Callable callback){
        auto* b=presentation::add_control<Button>(*w,name,rect);b->set_text(rest_text(text));b->connect("pressed",callback);return b;};
    button("Start",N_("Start"),Rect2(24,580,180,40),callable_mp(this,&RolfTourView::rest_start));
    button("Spend",N_("Spend 1 Hit Die"),Rect2(24,580,220,40),callable_mp(this,&RolfTourView::rest_spend));
    button("Resume",N_("Resume Long Rest"),Rect2(24,580,220,40),callable_mp(this,&RolfTourView::rest_resume));
    button("Save",N_("Save game"),Rect2(258,580,210,40),callable_mp(this,&RolfTourView::rest_save));
    button("Finish",N_("Cancel"),Rect2(482,580,214,40),callable_mp(this,&RolfTourView::rest_finish));
}
void RolfTourView::camp()
{
    if(!campaign_||!session_||!session_->can_leave()||campaign_->in_combat())return;
    rest_save_open_=false;rest_result_=String();refresh_rest();
    auto* w=get_node<Window>("RestDialog");w->popup_centered();w->get_node<OptionButton>("Kind")->grab_focus();
}
void RolfTourView::rest_selected(std::int64_t)
{
    auto* list=get_node<Window>("RestDialog")->get_node<ItemList>("Members");const auto selected=list->get_selected_items();
    if(!selected.is_empty())rest_member_=static_cast<unsigned>(static_cast<int64_t>(list->get_item_metadata(selected[0])));
    refresh_rest();
}
void RolfTourView::refresh_rest()
{
    auto* w=get_node<Window>("RestDialog");if(!campaign_||!session_||campaign_->in_combat()||!session_->can_leave()){w->hide();return;}
    const auto& state=campaign_->state();const bool spending=state.short_rest.has_value(),retained=state.rest_activity.has_value();
    auto* kind=w->get_node<OptionButton>("Kind");kind->set_disabled(spending||retained);
    if(retained)kind->select(1);else if(spending)kind->select(0);
    const auto selected_kind=kind->get_selected_id()==0?opengold::RestKind::short_rest:opengold::RestKind::long_rest;
    const auto infos=campaign_->rest_info(selected_kind);auto* list=w->get_node<ItemList>("Members");list->clear();
    if(!rest_member_||std::none_of(infos.begin(),infos.end(),[&](const auto& i){return i.id==rest_member_;}))rest_member_=infos.empty()?0:infos.front().id;
    String details;bool eligible=false,spendable=false;
    for(const auto& info:infos){
        const auto& m=campaign_->member(info.id);const auto& r=info.recovery;
        const bool earned=spending&&std::find(state.short_rest->members.begin(),state.short_rest->members.end(),info.id)!=state.short_rest->members.end();
        const bool can_start=info.denial==opengold::RestDenial::none;eligible|=can_start;
        auto row=String::utf8(m.character.sheet().name.c_str())+"   "+rest_text(N_("HP"))+" "+String::num_int64(m.vitals.hit_points)+"/"+String::num_int64(m.character.sheet().hit_points)+
            "   "+rest_text(N_("Hit Dice"))+" "+String::num_int64(r.hit_dice)+"/"+String::num_int64(r.hit_dice_max)+"d"+String::num_int64(r.hit_die);
        const int index=list->add_item(row);list->set_item_metadata(index,static_cast<int64_t>(info.id));
        if(info.id!=rest_member_)continue;list->select(index);
        details=String::utf8(m.character.sheet().name.c_str())+"\n";
        if(spending)details+=rest_text(earned?N_("Short Rest completed. Choose one die at a time, or finish."):N_("This member did not complete the rest."));
        else if(retained)details+=rest_text(N_("The Long Rest is interrupted. Resume after resolving the interruption, or end the rest."));
        else if(can_start)details+=rest_text(N_("Eligible to rest."));
        else if(info.denial==opengold::RestDenial::cooldown)details+=rest_text(N_("Long Rest cooldown remaining (minutes):"))+" "+String::num_int64((info.wait_milliseconds+59999)/60000);
        else details+=rest_text(N_("Rest requires at least 1 HP."));
        details+="\n\n"+rest_text(N_("Resource recovery"))+"\n";
        for(const auto& pool:r.resources){details+=rest_text(pool.label.source)+": "+String::num_int64(pool.remaining)+"/"+String::num_int64(pool.capacity)+"   ";
            details+=rest_text(N_("Short Rest"))+" +"+String::num_int64(std::min(pool.short_rest_recovery,pool.capacity-pool.remaining))+"; "+rest_text(N_("Long Rest"))+" "+String::num_int64(pool.capacity)+"\n";}
        spendable=earned&&r.can_rest&&r.hit_dice>0;
    }
    if(retained){const auto& a=*state.rest_activity;
        details=rest_text(N_("Rest progress (minutes):"))+" "+String::num_int64(a.elapsed_milliseconds/60000)+
            "\n"+rest_text(N_("Additional required time (minutes):"))+" "+String::num_int64(a.extension_milliseconds/60000)+
            "\n"+rest_text(N_("Remaining rest (minutes):"))+" "+String::num_int64((campaign_->remaining_rest_milliseconds()+59999)/60000)+"\n\n"+details;}
    w->get_node<RichTextLabel>("Info")->set_text(details);w->get_node<Label>("Result")->set_text(rest_result_);
    w->get_node<Button>("Start")->set_visible(!spending&&!retained);w->get_node<Button>("Start")->set_disabled(!eligible);
    w->get_node<Button>("Spend")->set_visible(spending);w->get_node<Button>("Spend")->set_disabled(!spendable);
    w->get_node<Button>("Resume")->set_visible(retained&&!spending);w->get_node<Button>("Resume")->set_disabled(!retained||!state.rest_activity->interrupted);
    w->get_node<Button>("Save")->set_visible(embedded_party_&&(spending||retained));
    w->get_node<Button>("Finish")->set_text(rest_text(spending?N_("Finish"):retained?N_("End Rest"):N_("Cancel")));
    if((spending||retained)&&!w->is_visible()&&!rest_save_open_&&is_visible_in_tree()){
        w->popup_centered();list->grab_focus();}
}
void RolfTourView::rest_start()
{
    try{auto* w=get_node<Window>("RestDialog");const auto kind=w->get_node<OptionButton>("Kind")->get_selected_id()==0?opengold::RestKind::short_rest:opengold::RestKind::long_rest;
        w->hide();rest_result_=String();if(session_)session_->camp(kind);refresh();}
    catch(const std::exception& e){rest_result_=rest_text(e.what());refresh_rest();get_node<Window>("RestDialog")->popup_centered();}
}
void RolfTourView::rest_spend()
{
    try{if(!campaign_||!campaign_->state().short_rest)return;
        const auto result=campaign_->spend_hit_die(campaign_->state().short_rest->ticket,rest_member_);
        rest_result_=rest_text(N_("Roll:"))+" "+String::num_int64(result.roll)+" + ("+String::num_int64(result.modifier)+")   "+rest_text(N_("HP restored:"))+" "+String::num_int64(result.healing)+"   "+rest_text(N_("Hit Dice remaining:"))+" "+String::num_int64(result.remaining);refresh_rest();}
    catch(const std::exception& e){rest_result_=rest_text(e.what());refresh_rest();}
}
void RolfTourView::rest_finish()
{
    try{if(campaign_){if(campaign_->state().short_rest)campaign_->finish_short_rest(campaign_->state().short_rest->ticket);
        else if(campaign_->state().rest_activity)campaign_->abandon_rest(campaign_->state().rest_activity->ticket);}
        get_node<Window>("RestDialog")->hide();rest_save_open_=false;rest_result_=String();refresh();}
    catch(const std::exception& e){rest_result_=rest_text(e.what());refresh_rest();}
}
void RolfTourView::rest_resume()
{
    try{get_node<Window>("RestDialog")->hide();rest_result_=rest_text(N_("Rest could not resume here. Resolve the interruption or end the rest."));if(session_)session_->resume_camp();refresh();}
    catch(const std::exception& e){rest_result_=rest_text(e.what());refresh_rest();}
}
void RolfTourView::rest_save()
{
    rest_save_open_=true;get_node<Window>("RestDialog")->hide();request_save(true);
}
void RolfTourView::rest_input(const Ref<InputEvent>& event)
{
    const Ref<InputEventKey> key=event;if(key.is_valid()&&key->is_pressed()&&key->get_keycode()==Key::KEY_ESCAPE){rest_finish();get_node<Window>("RestDialog")->set_input_as_handled();}
}

// Runtime acceptance drives the same signals and controls as a player.
#include "opengold/campaign_save.h"
void RolfTourView::check_rest_controls()
{
    try{
        auto check=[](bool ok,const char* message){if(!ok)throw std::runtime_error(message);};
        auto* w=get_node<Window>("RestDialog");
        if(rest_check_stage_==0){
            check(campaign_&&campaign_->selected(),"Rest check requires the real campaign party");
            const auto id=campaign_->selected();campaign_->award_experience(300,"rest-ui-check");
            if(campaign_->can_advance(id))campaign_->advance(id,campaign_->default_advancement(id));
            auto draft=campaign_->member(id).character.creation_data();draft.name="Fallen companion";
            const auto companion=campaign_->add_pc(opengold::Character(*opengold::srd5::character_rules(),draft,{}));
            auto state=campaign_->checkpoint();state.roster.front().vitals.hit_points=1;
            for(auto& m:state.roster)if(m.id==companion)m.vitals={0,true,"SRD1 0 0 0 3 0","Dead"};
            campaign_->restore(state);
            std::vector<std::uint8_t> bytes{0,0};for(int n=0;n<5;++n)bytes.insert(bytes.end(),{1,1,0x15,0x99});bytes.insert(bytes.end(),{0,0});
            auto p=std::make_shared<const opengold::por::EclProgram>(opengold::por::EclProgram::decode(bytes,"rest controls"));
            auto resources=std::make_shared<opengold::por::PhlanResources>();resources->programs[0]=p;
            session_.emplace(opengold::por::GeoMap{},p,std::array<opengold::Image,3>{},0x9914,opengold::por::WallArtSet{},resources);
            session_->campaign_party(campaign_);session_->advance(1);camp();
            check(w->is_visible(),"Camp opens the Rest picker");
            w->get_node<OptionButton>("Kind")->select(0);w->get_node<OptionButton>("Kind")->emit_signal("item_selected",0);
            check(!w->get_node<Button>("Start")->is_disabled(),"Eligible Short Rest enables Start");
        }else if(rest_check_stage_==12){
            w->get_node<Button>("Start")->emit_signal("pressed");
            check(campaign_->state().short_rest&&w->is_visible(),"Completing the hour opens the spending controls");
            check(campaign_->state().time_minutes==60,"Picker completes exactly one hour");
            auto* members=w->get_node<ItemList>("Members");members->select(1);members->emit_signal("item_selected",1);
            check(w->get_node<Button>("Spend")->is_disabled(),"Ineligible member cannot spend Hit Dice");
            members->select(0);members->emit_signal("item_selected",0);
            check(!w->get_node<Button>("Spend")->is_disabled(),"Eligible character can spend a die");
            w->get_node<Button>("Spend")->grab_focus();
            for(bool down:{true,false}){Ref<InputEventKey> key;key.instantiate();key->set_keycode(Key::KEY_SPACE);key->set_pressed(down);w->push_input(key,true);}
            check(!w->get_node<Label>("Result")->get_text().is_empty()&&campaign_->member(rest_member_).vitals.hit_points>1,"Committed die shows its healing result");
            auto rules=opengold::srd5::character_rules();
            const auto saved=opengold::encode_campaign(*campaign_,nullptr,"rest-ui");
            // Decode through the same campaign codec used by the save dialog.
            auto module=opengold::srd5::load(std::filesystem::path(rest_rules_path().utf8().get_data()));
            campaign_->restore(opengold::decode_campaign(saved,*rules,*module,"rest-ui",nullptr).party);
            refresh_rest();check(campaign_->state().short_rest.has_value(),"Reload retains pending Hit Die spending");
            w->get_node<Button>("Spend")->emit_signal("pressed");
            check(w->get_node<Button>("Spend")->is_disabled(),"After the second die no dice remain");
        }else if(rest_check_stage_==24){
            Ref<InputEventKey> escape;escape.instantiate();escape->set_keycode(Key::KEY_ESCAPE);escape->set_pressed(true);w->emit_signal("window_input",escape);
            check(!campaign_->state().short_rest&&!w->is_visible(),"Escape finishes committed spending and closes the window");
            const auto begin=campaign_->begin_rest(opengold::RestKind::long_rest);
            (void)campaign_->advance_rest(*begin,70*60000,opengold::RestWork::sleep);
            campaign_->interrupt_rest(campaign_->state().rest_activity->ticket,opengold::RestInterruption::initiative);
            campaign_->finish_short_rest(campaign_->state().short_rest->ticket);refresh();
            check(w->is_visible()&&w->get_node<Button>("Resume")->is_visible(),"An interrupted Long Rest displays Resume and End Rest");
        }else if(rest_check_stage_==36){
            w->get_node<Button>("Resume")->emit_signal("pressed");
            check(!campaign_->state().rest_activity&&campaign_->state().time_minutes==600,"Resume completes original permission check and remaining rest once");
            camp();w->get_node<OptionButton>("Kind")->select(1);w->get_node<OptionButton>("Kind")->emit_signal("item_selected",1);
            check(w->get_node<Button>("Start")->is_disabled(),"Long Rest cooldown disables Start");
            w->get_node<Button>("Finish")->emit_signal("pressed");
            UtilityFunctions::print("Godot rest controls passed: picker, sequential dice, continuation, finish, resume and cooldown.");get_tree()->quit();
        }
        if(rest_check_stage_==10||rest_check_stage_==22||rest_check_stage_==34){
            for(const auto& arg:OS::get_singleton()->get_cmdline_user_args())if(String(arg).begins_with("--rest-captures=")){
                const auto directory=String(arg).trim_prefix("--rest-captures=");
                std::filesystem::create_directories(std::filesystem::path(directory.utf8().get_data()));
                const auto image=get_viewport()->get_texture()->get_image();
                check(image.is_valid()&&image->save_png(directory.path_join(String::num_int64(rest_check_stage_)+".png"))==OK,"Rest screenshot saved");
            }
        }
        ++rest_check_stage_;
    }catch(const std::exception& e){UtilityFunctions::printerr("Rest control check failed: ",String::utf8(e.what()));get_tree()->quit(1);}
}
