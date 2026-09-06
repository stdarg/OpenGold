#include "opengold/rolf_tour.h"
#include <algorithm>

namespace opengold::por {
namespace {
constexpr std::array<std::uint16_t,7> money{0x6BBB,0x6BBD,0x6BBF,0x6BC1,0x6BC3,0x6BC5,0x6BC7};
constexpr std::array<int,4> dx{0,1,0,-1}, dy{-1,0,1,0};
}

void RolfTourSession::configure_town()
{
    // Campaign flags, area-local flags, host registers and scratch strings have
    // explicit lifetimes. These are logical VM cells, never native addresses.
    for (unsigned a=0x49C3;a<=0x4AFF;++a) machine_.bind_variable(a,0);
    for (unsigned a=0x6B00;a<=0x6C1C;++a) machine_.bind_variable(a,0);
    for (unsigned a=0x6DA8;a<=0x6EFF;++a) machine_.bind_variable(a,0);
    for (unsigned a=0x9800;a<=0x98FF;++a) machine_.bind_variable(a,0);
    machine_.bind_variable(0xB8,0); machine_.bind_variable(0xB9,0);
    machine_.bind_variable(0x49C9,12); machine_.bind_variable(0x49CA,1);
    machine_.bind_variable(0x6E12,3); machine_.bind_variable(0x6E3E,1);
    for (const auto& w:character_reply(0).writes) machine_.bind_variable(w.address,w.value);
    for (const std::uint8_t op:{10,28,32,33,36,39,40,50,55,56,57}) machine_.enable_host(op);
}

EclHostReply RolfTourSession::character_reply(unsigned index) const
{
    EclHostReply reply;
    std::array<std::uint16_t,285> fields{};
    if (index==0) {
        for (std::size_t n=0;n<party_.name.size() && n<15;++n) fields[n]=party_.name[n];
        fields[0x18]=14; // Verified GBVM Constitution field; no guessed raw-record offsets.
        fields[0x100]=1; fields[0x119]=party_.hit_points;
        for (unsigned n=0;n<money.size();++n) fields[money[n]-0x6B00]=party_.wealth[n];
    }
    for (unsigned n=0;n<fields.size();++n) reply.writes.push_back({static_cast<std::uint16_t>(0x6B00+n),fields[n]});
    reply.writes.push_back({0x6DB1,static_cast<std::uint16_t>(index)});
    reply.writes.push_back({0x6DB4,static_cast<std::uint16_t>(index)});
    return reply;
}

void RolfTourSession::read_character()
{
    if (selected_character_!=0) return;
    for (unsigned n=0;n<money.size();++n) party_.wealth[n]=machine_.variable(money[n]);
    party_.hit_points=machine_.variable(0x6C19);
}

void RolfTourSession::bind_pose(PartyPose pose)
{
    machine_.bind_variable(0xC04B,pose.x); machine_.bind_variable(0xC04C,pose.y);
    machine_.bind_variable(0xC04D,pose.facing);
    const auto& cell=map_.at(pose.x,pose.y);
    machine_.bind_variable(0xC04E,cell.walls[pose.facing]); machine_.bind_variable(0xC04F,cell.event_raw);
    publish_pose();
}

bool RolfTourSession::move_party(ExplorationCommand command)
{
    auto pose=snapshot_.pose;
    if (command==ExplorationCommand::turn_left) pose.facing=(pose.facing+3)%4;
    else if (command==ExplorationCommand::turn_right) pose.facing=(pose.facing+1)%4;
    else if (command==ExplorationCommand::forward) {
        if (machine_.variable(0x6DC9)==255) return false;
        const int x=static_cast<int>(pose.x)+dx[pose.facing],y=static_cast<int>(pose.y)+dy[pose.facing];
        if (x<0 || y<0 || x>=16 || y>=16) return false;
        const auto& a=map_.at(pose.x,pose.y); const auto& b=map_.at(x,y);
        const auto side=pose.facing, other=(side+2)%4;
        // Ordinary doors are traversable. Locks retain their distinct codes.
        const auto blocked=[](unsigned wall,unsigned door){return door>1 || (wall && !door);};
        if (blocked(a.walls[side],a.doors[side]) || blocked(b.walls[other],b.doors[other])) return false;
        machine_.bind_variable(0x49F0,pose.x); machine_.bind_variable(0x49F1,pose.y);
        pose.x=x;pose.y=y;++snapshot_.footsteps;
    }
    bind_pose(pose);return true;
}

void RolfTourSession::begin_event(unsigned slot)
{
    checkpoint_=machine_; saved_party_=party_; saved_script_=current_script_;
    saved_selected_character_=selected_character_;
    event_stage_=slot==0?1:slot==2?4:2;
    machine_.bind_variable(0x6DC9,0);
    machine_.bind_variable(0x6DD5,0); // No off-map move is proposed by this bounded town host.
    machine_.bind_variable(0x6DCA,slot==1?2:0);
    if (!machine_.start(slot)) { fail("Unable to start town script");return; }
    ++snapshot_.event_runs; snapshot_.phase=TourPhase::running;
    snapshot_.diagnostic.clear(); snapshot_.choices.clear(); ++snapshot_.revision;
}

void RolfTourSession::finish_event()
{
    read_character(); publish_pose();
    if (transition_) {
        transition_=false; event_stage_=3;
        if (!machine_.start(4)) throw EclError("Cannot enter town building script");
        return;
    }
    if (event_stage_==1) {
        if (!move_party(*pending_movement_)) snapshot_.dialogue="The way is blocked.";
        pending_movement_.reset(); event_stage_=2;
        if (!machine_.start(1)) throw EclError("Cannot search destination");
        return;
    }
    if (event_stage_==3) {
        event_stage_=2;
        if (!machine_.start(1)) throw EclError("Cannot search building entrance");
        return;
    }
    if (event_stage_==4) {
        // The original pre-camp entry has run. A full SRD rest is not implied.
        snapshot_.dialogue += "\nCamp check complete. Rest and spell preparation are not implemented here.";
    }
    checkpoint_.reset();snapshot_.phase=TourPhase::completed;
    snapshot_.choices.clear(); snapshot_.continue_ticket=0; ++snapshot_.revision;
}

void RolfTourSession::notice(std::string message)
{
    message_only_=true; snapshot_.dialogue=std::move(message);
    snapshot_.phase=TourPhase::awaiting_continue; snapshot_.choices={"Continue"};
    snapshot_.continue_ticket=++next_ticket_; ++snapshot_.revision;
}

bool RolfTourSession::choose(std::uint64_t ticket,std::size_t choice)
{
    if (snapshot_.phase!=TourPhase::awaiting_continue || !ticket || ticket!=snapshot_.continue_ticket ||
        choice>=snapshot_.choices.size()) return false;
    if (message_only_) { message_only_=false;snapshot_.phase=TourPhase::completed; }
    else {
        if (!machine_.resume(menu_request_,choice)) return false;
        snapshot_.phase=TourPhase::running;
    }
    snapshot_.choices.clear();snapshot_.continue_ticket=0;++snapshot_.revision;return true;
}

bool RolfTourSession::input(std::uint64_t ticket,std::string_view value)
{
    if (snapshot_.phase!=TourPhase::awaiting_input || !ticket || ticket!=snapshot_.continue_ticket) return false;
    if(!machine_.resume_input(menu_request_,value)){
        snapshot_.diagnostic=snapshot_.number_input?"Enter a whole number from 0 to 65535.":"Enter up to 40 characters.";
        ++snapshot_.revision;return false;
    }
    snapshot_.diagnostic.clear();
    snapshot_.phase=TourPhase::running;snapshot_.continue_ticket=0;++snapshot_.revision;return true;
}

bool RolfTourSession::buy(std::uint64_t ticket,std::size_t item)
{
    if (snapshot_.phase!=TourPhase::shopping || !ticket || ticket!=snapshot_.continue_ticket || item>=treasure_.size()) return false;
    const auto& offered=treasure_[item];
    const auto price=offered.stored.value;
    if (party_.inventory.size()>=16 || party_.wealth[3]<price) {
        snapshot_.diagnostic=party_.inventory.size()>=16?"Inventory is full (16 items).":"Not enough gold.";
        ++snapshot_.revision;return false;
    }
    auto purchased=offered;purchased.index=party_.inventory.size();purchased.stored.readied_raw=0;
    party_.inventory.push_back(std::move(purchased)); // Allocate before charging.
    party_.wealth[3]-=price;
    snapshot_.diagnostic="Bought "+offered.label()+" for "+std::to_string(price)+" gp.";
    ++snapshot_.revision;return true;
}

bool RolfTourSession::leave_shop(std::uint64_t ticket)
{
    if (snapshot_.phase!=TourPhase::shopping || !ticket || ticket!=snapshot_.continue_ticket) return false;
    auto reply=character_reply(0); reply.writes.push_back({0x6E6C,0});
    if (!machine_.resume_host(shop_request_,reply)) return false;
    selected_character_=0;shop_request_=0;snapshot_.continue_ticket=0;
    snapshot_.phase=TourPhase::running;++snapshot_.revision;return true;
}

bool RolfTourSession::handle_town_host(const EclRequest& request)
{
    const auto op=request.instruction->opcode;
    EclHostReply reply;
    const auto arg=[&](unsigned n){return request.arguments.at(n).value;};
    switch (op) {
    case 10:
        if (arg(0)>=128) {
            if ((arg(0)&127)!=selected_character_) throw EclError("Selected character store mismatch");
            read_character();
        } else {
            read_character();selected_character_=arg(0);reply=character_reply(selected_character_);
        }
        break;
    case 57: read_character();selected_character_=0;reply=character_reply(0);break; // Single PC WHO.
    case 12: {
        // Encounter distance images use the already verified SPRIT3 format.
        std::array<Image,3> decoded;
        if(arg(1)>2)throw EclError("Unsupported encounter distance");
        for (unsigned n=0;n<3;++n) {
            auto image=decode_ega_sprite(town_->sprite_archive,arg(0),n);
            if (!image) throw EclError("Unsupported encounter sprite "+std::to_string(arg(0)));
            decoded[n]=std::move(image.image);
        }
        sprites_=std::move(decoded);snapshot_.sprite_frame=arg(1);snapshot_.sprite_id=arg(0);
        picture_.reset();++snapshot_.picture_revision;
        break;
    }
    case 14: {
        snapshot_.sprite_frame=-1;picture_.reset();++snapshot_.picture_revision;
        if(arg(0)==255)break;
        const auto head_id=machine_.variable(0x6DE1);
        if(head_id==255){
            const auto found=town_->pictures.find(arg(0));
            if(found==town_->pictures.end())throw EclError("Unsupported town picture "+std::to_string(arg(0)));
            picture_=found->second;
        }else{
            const auto head=town_->heads.find(head_id),body=town_->bodies.find(arg(0));
            if(head==town_->heads.end()||body==town_->bodies.end()||head->second.width!=88||body->second.width!=88||
                head->second.height!=40||body->second.height!=48)throw EclError("Unsupported town portrait composition");
            Image portrait;portrait.width=portrait.height=88;
            portrait.rgba=head->second.rgba;
            portrait.rgba.insert(portrait.rgba.end(),body->second.rgba.begin(),body->second.rgba.end());
            picture_=std::move(portrait);
        }
        break;
    }
    case 28: treasure_.clear();break;
    case 39: {
        for (unsigned n=0;n<7;++n) if (arg(n)) throw EclError("Non-shop treasure awards are not implemented");
        if (arg(7)==255) break;
        const auto found=town_->treasure.find(arg(7));
        if (found==town_->treasure.end()) throw EclError("Unsupported town treasure list");
        treasure_.insert(treasure_.end(),found->second.begin(),found->second.end());break;
    }
    case 36:
        if (machine_.variable(0x6E6C)==1) {
            read_character();shop_request_=request.id;
            snapshot_.phase=TourPhase::shopping;snapshot_.continue_ticket=++next_ticket_;
            snapshot_.choices.clear();snapshot_.diagnostic.clear();++snapshot_.revision;return true;
        }
        throw EclError(machine_.variable(0x6DE2)==1?"Temple healing service":"This town combat encounter");
    case 32: {
        const auto found=town_->programs.find(arg(0));
        if (found==town_->programs.end()) throw EclError("Travel outside New Phlan (script "+std::to_string(arg(0))+")");
        reply.next_program=found->second;
        reply.writes={{0x49F2,static_cast<std::uint16_t>(current_script_)},{0x6E12,3}};
        current_script_=arg(0);snapshot_.script_id=current_script_;transition_=true;break;
    }
    case 33:
        if (arg(0)!=0 || arg(1)!=0 || arg(2)!=0) throw EclError("Resources outside the New Phlan profile");
        break;
    case 55:
        if (arg(0)!=127 || arg(1)!=127 || arg(2)!=127) throw EclError("Unverified wall resource profile");
        break;
    case 50: {
        const bool found=std::any_of(party_.inventory.begin(),party_.inventory.end(),[&](const auto& i){return i.stored.type==arg(0);});
        reply.conditions=EclConditions{found,!found,false,false,false,false};break;
    }
    case 40: throw EclError("Pickpocket money and item removal");
    case 56: throw EclError("Training/camp service "+std::to_string(arg(0)));
    default: return false;
    }
    if (!machine_.resume_host(request.id,reply)) throw EclError("Town host reply rejected");
    ++snapshot_.revision;return true;
}
}
