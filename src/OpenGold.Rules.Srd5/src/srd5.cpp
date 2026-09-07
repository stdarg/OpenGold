#include "opengold/srd5.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>

namespace opengold::srd5 {
using namespace rules;
int ability_modifier(int score) noexcept { return static_cast<int>(std::floor((static_cast<double>(score)-10)/2.0)); }
bool attack_hits(int natural, int bonus, int ac) noexcept
{ return natural==20 || (natural!=1 && static_cast<std::int64_t>(natural)+bonus>=ac); }
namespace {
struct Dice { int count{}, sides{}, bonus{}; };
struct Definition {
    int ac{}, hp{}, initiative{}, speed{}, melee_bonus{};
    Dice melee;
    int ranged_bonus{};
    Dice ranged;
    int range{}, long_range{}, winds{}, slots{}, casting{}, level{}, spells{};
};
struct Content { Identity identity; std::map<std::string,Definition> definitions; };
struct Actor {
    Participant source;
    Definition definition;
    int hp{}, initiative{}, movement{}, winds{}, slots{}, successes{}, failures{};
    bool action{true}, bonus{true}, reaction{true}, dodge{}, disengaged{}, stable{}, dead{};
};
// Versioned, module-owned character recipe. Original item IDs never enter this layer.
Definition character_definition(std::string_view bytes)
{
    if(bytes.size()>1024)throw std::runtime_error("Character profile exceeds limit");
    std::istringstream in{std::string(bytes)};
    std::string magic,klass,race;std::array<int,6> scores{};unsigned count{};
    in>>magic>>std::quoted(klass)>>std::quoted(race);
    for(auto& score:scores)in>>score;
    in>>count;
    if(!in||magic!="PC1"||count>3||std::any_of(scores.begin(),scores.end(),[](int n){return n<3||n>20;})||
        (klass!="Fighter"&&klass!="Cleric"&&klass!="Wizard"))
        throw std::runtime_error("Campaign combat supports level-one Fighter, Cleric and Wizard profiles only");
    const auto races=character_rules()->choices(CreationField::race);
    if(std::none_of(races.begin(),races.end(),[&](const auto& r){return r.label==race;}))throw std::runtime_error("Unknown species");
    const int str=ability_modifier(scores[0]),dex=ability_modifier(scores[1]),con=ability_modifier(scores[2]);
    Definition d;d.hp=(klass=="Fighter"?10:klass=="Cleric"?8:6)+con+(race=="Dwarf"?1:0);
    d.ac=10+dex;d.initiative=dex;d.speed=race=="Goliath"?35:30;d.level=1;
    d.melee_bonus=2+str;d.melee={0,0,std::max(0,1+str)};
    d.winds=klass=="Fighter"?2:0;d.slots=klass=="Fighter"?0:2;
    d.casting=2+ability_modifier(scores[klass=="Cleric"?4:3]);d.spells=klass=="Cleric"?2:klass=="Wizard"?5:0;
    bool weapon=false,armor=false,shield=false;
    for(unsigned i=0;i<count;++i){std::string key;in>>std::quoted(key);
        if(key=="dagger"||key=="mace"||key=="longsword"||key=="quarterstaff"){
            if(weapon)throw std::runtime_error("Only one weapon may be equipped");weapon=true;
            if((key=="longsword"&&klass!="Fighter")||(key=="mace"&&klass=="Wizard"))throw std::runtime_error("Weapon proficiency is not supported for this class");
            const int modifier=key=="dagger"?std::max(str,dex):str;
            d.melee_bonus=2+modifier;d.melee={1,key=="dagger"?4:key=="longsword"?8:6,modifier};
        }else if(key=="leather"||key=="chain_mail"){
            if(armor||klass=="Wizard"||(key=="chain_mail"&&klass!="Fighter"))throw std::runtime_error("Unsupported armor training or duplicate armor");
            armor=true;d.ac=key=="leather"?11+dex:16;
            if(key=="chain_mail"&&scores[0]<13)d.speed-=10;
        }else if(key=="shield"){
            if(shield||klass=="Wizard")throw std::runtime_error("Unsupported shield training or duplicate shield");shield=true;
        }else throw std::runtime_error("Unsupported equipment conversion: "+key);
    }
    if(shield)d.ac+=2;
    in>>std::ws;if(!in.eof())throw std::runtime_error("Invalid character profile fields");return d;
}
void restore_vitals(Actor& a,const VitalState& state)
{
    a.hp=state.hit_points;a.dead=state.dead;
    if(!state.resources.empty()) {
        std::istringstream in(state.resources);std::string magic;
        in>>magic>>a.winds>>a.slots>>a.successes>>a.failures>>a.stable;
        if(!in||magic!="SRD1")throw std::runtime_error("Invalid character resource state");
        in>>std::ws;if(!in.eof())throw std::runtime_error("Trailing character resource state");
    }
    const auto& d=a.definition;
    if(a.hp<0||a.hp>d.hp||(a.dead&&a.hp!=0)||a.winds<0||a.winds>d.winds||a.slots<0||a.slots>d.slots||
        a.successes<0||a.successes>3||a.failures<0||a.failures>4)throw std::runtime_error("Invalid character vitals");
}
VitalState vitals(const Actor& a)
{
    std::ostringstream out;out<<"SRD1 "<<a.winds<<' '<<a.slots<<' '<<a.successes<<' '<<a.failures<<' '<<a.stable;
    std::string description;
    if(a.definition.slots)description="Level-one spell slots: "+std::to_string(a.slots)+" / "+std::to_string(a.definition.slots);
    if(a.definition.winds)description="Second Wind uses: "+std::to_string(a.winds)+" / "+std::to_string(a.definition.winds);
    if(a.hp==0)description+=(description.empty()?"":"\n")+std::string(a.dead?"Dead":a.stable?"Stable, unconscious":"Unconscious; death saves ")+(!a.dead&&!a.stable?std::to_string(a.successes)+" successes, "+std::to_string(a.failures)+" failures":"");
    return {a.hp,a.dead,out.str(),description};
}
int distance(Cell a,Cell b) { return std::max(std::abs(a.x-b.x),std::abs(a.y-b.y))*5; }
bool same_command(const Command& a,const Command& b)
{ return a.revision==b.revision && a.actor==b.actor && a.target==b.target && a.verb==b.verb && a.destination==b.destination; }
class Session final : public CombatSession {
public:
    Session(std::shared_ptr<const Content> content,Encounter encounter,std::uint64_t seed,bool restoring=false)
        : content_(std::move(content)), board_(std::move(encounter.battlefield)), rng_(seed)
    {
        if (board_.width<2 || board_.height<2 || board_.width>32 || board_.height>32 ||
            board_.terrain.size()!=static_cast<std::size_t>(board_.width*board_.height) ||
            std::any_of(board_.terrain.begin(),board_.terrain.end(),[](auto t){return t>2;}))
            throw std::runtime_error("Invalid battlefield");
        if (encounter.participants.size()<2 || encounter.participants.size()>64) throw std::runtime_error("Invalid encounter size");
        std::set<EntityId> ids;std::set<Cell> cells;std::set<unsigned> sides;
        for (auto& p:encounter.participants) {
            if (!p.id || !ids.insert(p.id).second || (!cells.insert(p.cell).second&&!restoring) || board_.at(p.cell)==1 ||
                p.side>1 || p.name.empty() || p.name.size()>160 || (p.character_profile.empty()&&!content_->definitions.contains(p.definition)))
                throw std::runtime_error("Invalid participant or unsupported rules definition: "+p.definition);
            sides.insert(p.side);
            const auto d=p.character_profile.empty()?content_->definitions.at(p.definition):character_definition(p.character_profile);
            Actor a; a.definition=d;a.source=std::move(p);a.hp=d.hp;a.winds=d.winds;a.slots=d.slots;
            if(a.source.state)restore_vitals(a,*a.source.state);
            a.initiative=roll(20)+d.initiative;a.movement=d.speed;
            actors_.push_back(std::move(a));
        }
        if (sides.size()!=2) throw std::runtime_error("Encounter needs both sides");
        // Fixed tie adjudication: descending initiative, then stable entity ID.
        std::stable_sort(actors_.begin(),actors_.end(),[](const Actor& a,const Actor& b){
            return a.initiative!=b.initiative?a.initiative>b.initiative:a.source.id<b.source.id;
        });
        log("Combat begins. Each square is 5 feet.");update_outcome();
        if(outcome_==Outcome::ongoing){if(actors_[turn_].hp<=0)end_turn();else begin_turn();}
    }
    Snapshot snapshot() const override;
    std::vector<Command> legal_commands() const override;
    bool submit(const Command& command) override;
    std::string save() const override;
    static std::unique_ptr<Session> restore(std::shared_ptr<const Content> content,std::string_view bytes);
private:
    std::shared_ptr<const Content> content_;
    Battlefield board_;
    std::vector<Actor> actors_;
    std::uint64_t rng_{}, revision_{1};
    unsigned turn_{}, round_{1};
    Outcome outcome_{Outcome::ongoing};
    std::vector<std::string> log_;
    std::vector<Cell> path_;
    std::size_t path_index_{};
    std::vector<EntityId> reactors_;
    std::size_t reactor_index_{};
    const Definition& def(const Actor& a) const {return a.definition;}
    Actor& actor(EntityId id) { return *std::find_if(actors_.begin(),actors_.end(),[&](const auto& a){return a.source.id==id;}); }
    std::uint64_t next_random() {
        // SplitMix64 with explicit integer arithmetic: stable across compilers.
        auto z=(rng_+=0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;
        z=(z^(z>>27))*0x94d049bb133111ebULL;return z^(z>>31);
    }
    int roll(int sides) {const auto n=static_cast<std::uint64_t>(sides), threshold=(-n)%n;
        auto value=next_random();while(value<threshold)value=next_random();return static_cast<int>(value%n)+1;}
    int dice(Dice d,bool critical=false) {int total=d.bonus;for(int i=0;i<d.count*(critical?2:1);++i)total+=roll(d.sides);return std::max(0,total);}
    void log(std::string message) {if(log_.size()==80)log_.erase(log_.begin());log_.push_back(std::move(message));}
    bool line_of_sight(Cell a,Cell b) const;
    std::vector<Cell> path_to(const Actor& a,Cell destination) const;
    bool occupied(Cell p) const {return std::any_of(actors_.begin(),actors_.end(),[&](const auto& a){return !a.dead && a.source.cell==p;});}
    EntityId pending() const {return reactor_index_<reactors_.size()?reactors_[reactor_index_]:0;}
    void attack(Actor& a,Actor& target,bool ranged,bool spell=false);
    void damage(Actor& target,int amount);
    void heal(Actor& target,int amount);
    void update_outcome();
    void begin_turn();
    void end_turn();
    void progress_movement();
};

bool Session::line_of_sight(Cell a,Cell b) const
{
    // Supercover traversal: solid cells and diagonal wall corners block sight.
    const int nx=std::abs(b.x-a.x),ny=std::abs(b.y-a.y),sx=b.x>a.x?1:-1,sy=b.y>a.y?1:-1;
    int ix=0,iy=0;Cell p=a;
    while(ix<nx || iy<ny) {
        const int cross=(1+2*ix)*ny-(1+2*iy)*nx;
        if(cross==0) {if(board_.at({p.x+sx,p.y})==1 || board_.at({p.x,p.y+sy})==1)return false;p.x+=sx;p.y+=sy;++ix;++iy;}
        else if(cross<0){p.x+=sx;++ix;}else{p.y+=sy;++iy;}
        if(board_.at(p)==1)return false;
    }
    return true;
}
std::vector<Cell> Session::path_to(const Actor& a,Cell destination) const
{
    if(!board_.contains(destination) || destination==a.source.cell || occupied(destination) || board_.at(destination)==1)return {};
    const auto index=[&](Cell p){return p.y*board_.width+p.x;};
    std::vector<int> cost(board_.terrain.size(),100000),previous(cost.size(),-1);
    using QueueItem=std::pair<int,int>;std::priority_queue<QueueItem,std::vector<QueueItem>,std::greater<QueueItem>> queue;
    cost[index(a.source.cell)]=0;queue.push({0,index(a.source.cell)});
    while(!queue.empty()) {
        const auto [spent,i]=queue.top();queue.pop();if(spent!=cost[i])continue;
        const Cell p{i%board_.width,i/board_.width};if(p==destination)break;
        for(int y=-1;y<=1;++y)for(int x=-1;x<=1;++x) {
            if(!x&&!y)continue;const Cell next{p.x+x,p.y+y};if(board_.at(next)==1)continue;
            if(x&&y&&(board_.at({p.x+x,p.y})==1||board_.at({p.x,p.y+y})==1))continue;
            bool ally=false,enemy=false;
            for(const auto& other:actors_)if(!other.dead&&other.source.cell==next) {
                if(other.source.side!=a.source.side)enemy=true;else ally=true;
            }
            if(enemy)continue;
            const int next_cost=spent+((ally||board_.at(next)==2)?10:5);
            if(next_cost>a.movement||next_cost>=cost[index(next)])continue;
            cost[index(next)]=next_cost;previous[index(next)]=i;queue.push({next_cost,index(next)});
        }
    }
    if(previous[index(destination)]<0)return {};
    std::vector<Cell> result;
    for(int i=index(destination);i!=index(a.source.cell);i=previous[i])result.push_back({i%board_.width,i/board_.width});
    std::reverse(result.begin(),result.end());return result;
}
Snapshot Session::snapshot() const
{
    Snapshot s;s.identity=content_->identity;s.revision=revision_;s.round=round_;s.outcome=outcome_;
    s.actor=pending()?pending():actors_[turn_].source.id;s.reaction_pending=pending()!=0;s.battlefield=board_;s.log=log_;
    for(const auto& a:actors_) {
        std::string status=a.dead?"Dead":a.hp==0?(a.stable?"Stable, unconscious":"Unconscious"):a.dodge?"Dodging":"Ready";
        if(def(a).slots)status+=" | slots "+std::to_string(a.slots);
        if(def(a).winds)status+=" | Second Wind "+std::to_string(a.winds);
        s.combatants.push_back({a.source.id,a.source.name,a.source.definition,a.source.side,a.source.cell,
            a.hp,def(a).hp,def(a).ac,a.initiative,a.movement,a.action,a.bonus,a.reaction,a.hp>0&&!a.dead,a.dead,status,vitals(a)});
    }
    return s;
}
std::vector<Command> Session::legal_commands() const
{
    std::vector<Command> commands;if(outcome_!=Outcome::ongoing)return commands;
    const auto add=[&](EntityId who,std::string verb,std::string label,EntityId target=0,Cell destination=Cell{}) {
        commands.push_back({revision_,who,target,std::move(verb),std::move(label),destination});
    };
    if(pending()) {
        add(pending(),"opportunity","Opportunity attack",actors_[turn_].source.id);
        add(pending(),"decline","Decline reaction");return commands;
    }
    const auto& a=actors_[turn_];if(a.hp<=0)return commands;const auto& d=def(a);const auto id=a.source.id;
    add(id,"end","End turn");
    if(a.bonus&&a.winds>0&&a.hp<d.hp)add(id,"second_wind","Second Wind",id);
    if(a.action) {
        add(id,"dash","Dash");add(id,"dodge","Dodge");add(id,"disengage","Disengage");
        for(const auto& other:actors_) {
            if(other.dead || !line_of_sight(a.source.cell,other.source.cell))continue;
            const int feet=distance(a.source.cell,other.source.cell);
            if(other.source.side!=a.source.side && other.hp>0) {
                if(feet<=5)add(id,"melee","Melee attack",other.source.id);
                if(feet<=d.long_range)add(id,"ranged","Ranged attack",other.source.id);
                if((d.spells&1)&&feet<=120)add(id,"fire_bolt","Fire Bolt",other.source.id);
                if((d.spells&4)&&a.slots>0&&feet<=120)add(id,"magic_missile","Magic Missile",other.source.id);
            } else if(other.source.side==a.source.side && other.hp<def(other).hp && feet<=5 && (d.spells&2)&&a.slots>0)
                add(id,"cure_wounds","Cure Wounds",other.source.id);
        }
    }
    if(a.movement>0)for(int y=0;y<board_.height;++y)for(int x=0;x<board_.width;++x)
        if(!path_to(a,{x,y}).empty())add(id,"move","Move",0,{x,y});
    return commands;
}
void Session::damage(Actor& target,int amount)
{
    const int remaining=amount-target.hp;target.hp=std::max(0,target.hp-amount);
    if(target.hp==0) {
        target.dodge=false;
        if(target.source.side==1 || remaining>=def(target).hp)target.dead=true;
        log(target.source.name+(target.dead?" is defeated.":" falls unconscious."));
    }
}
void Session::heal(Actor& target,int amount)
{
    const int restored=std::min(amount,def(target).hp-target.hp);target.hp+=restored;
    target.successes=target.failures=0;target.stable=false;log(target.source.name+" recovers "+std::to_string(restored)+" HP.");
}
void Session::attack(Actor& a,Actor& target,bool ranged,bool spell)
{
    const auto& d=def(a);bool disadvantaged=target.dodge;
    if(ranged) {
        if(!spell&&distance(a.source.cell,target.source.cell)>d.range)disadvantaged=true;
        for(const auto& other:actors_)if(other.source.side!=a.source.side&&other.hp>0&&distance(a.source.cell,other.source.cell)<=5&&line_of_sight(a.source.cell,other.source.cell))disadvantaged=true;
    }
    int natural=roll(20);if(disadvantaged)natural=std::min(natural,roll(20));
    const int bonus=spell?d.casting:ranged?d.ranged_bonus:d.melee_bonus;
    const bool hit=attack_hits(natural,bonus,def(target).ac);
    std::string message=a.source.name+" -> "+target.source.name+": d20 "+std::to_string(natural)+
        " + "+std::to_string(bonus)+" vs AC "+std::to_string(def(target).ac)+(disadvantaged?" (disadvantage)":"");
    if(!hit){log(message+" misses.");return;}
    const Dice damage_dice=spell?Dice{1,10,0}:ranged?d.ranged:d.melee;
    const int amount=dice(damage_dice,natural==20);
    log(message+(natural==20?" CRITICAL":" hits")+" for "+std::to_string(amount)+" damage.");damage(target,amount);
}
void Session::update_outcome()
{
    bool party=false,enemies=false;for(const auto& a:actors_)if(a.hp>0&&!a.dead)(a.source.side==0?party:enemies)=true;
    if(!party||!enemies) {
        outcome_=!party?Outcome::defeat:Outcome::victory;path_.clear();path_index_=0;reactors_.clear();reactor_index_=0;
        log(outcome_==Outcome::victory?"Victory.":"The party is incapacitated. Defeat.");
    }
}
void Session::begin_turn()
{
    auto& a=actors_[turn_];a.action=a.bonus=a.reaction=true;a.dodge=a.disengaged=false;a.movement=def(a).speed;
    log("Round "+std::to_string(round_)+": "+a.source.name+" acts.");
}
void Session::end_turn()
{
    for(std::size_t checked=0;checked<=actors_.size()*2;++checked) {
        turn_=(turn_+1)%actors_.size();if(turn_==0)++round_;
        auto& a=actors_[turn_];
        if(a.dead)continue;
        if(a.hp==0) {
            if(!a.stable) {
                const int result=roll(20);log(a.source.name+" death save: "+std::to_string(result));
                if(result==20){a.hp=1;a.successes=a.failures=0;}
                else if(result>=10)++a.successes;else a.failures+=result==1?2:1;
                if(a.failures>=3)a.dead=true;if(a.successes>=3)a.stable=true;
            }
            if(a.hp==0)continue;
        }
        begin_turn();return;
    }
    update_outcome();
}
void Session::progress_movement()
{
    auto& a=actors_[turn_];
    while(path_index_<path_.size()&&a.hp>0) {
        const auto destination=path_[path_index_];
        if(reactors_.empty()&&!a.disengaged)for(const auto& other:actors_)
            if(other.source.side!=a.source.side&&other.hp>0&&other.reaction&&
                distance(a.source.cell,other.source.cell)<=5&&distance(destination,other.source.cell)>5&&
                line_of_sight(a.source.cell,other.source.cell))reactors_.push_back(other.source.id);
        if(pending())return;
        const bool ally=std::any_of(actors_.begin(),actors_.end(),[&](const auto& other){return !other.dead&&other.source.cell==destination;});
        a.movement-=board_.at(destination)==2||ally?10:5;a.source.cell=destination;++path_index_;
        reactors_.clear();reactor_index_=0;
    }
    path_.clear();path_index_=0;reactors_.clear();reactor_index_=0;
}
bool Session::submit(const Command& command)
{
    const auto offered=legal_commands();
    if(std::none_of(offered.begin(),offered.end(),[&](const auto& c){return same_command(c,command);}))return false;
    auto& a=actor(command.actor);const auto& d=def(a);
    if(command.verb=="opportunity"||command.verb=="decline") {
        if(command.verb=="opportunity"){a.reaction=false;attack(a,actor(command.target),false);}
        ++reactor_index_;update_outcome();if(outcome_==Outcome::ongoing)progress_movement();
    } else if(command.verb=="move") {
        path_=path_to(a,command.destination);path_index_=0;progress_movement();
    } else if(command.verb=="end")end_turn();
    else if(command.verb=="second_wind") {a.bonus=false;--a.winds;heal(a,roll(10)+d.level);}
    else {
        a.action=false;
        if(command.verb=="dash"){a.movement+=d.speed;log(a.source.name+" dashes.");}
        else if(command.verb=="dodge"){a.dodge=true;log(a.source.name+" dodges.");}
        else if(command.verb=="disengage"){a.disengaged=true;log(a.source.name+" disengages.");}
        else if(command.verb=="cure_wounds"){--a.slots;heal(actor(command.target),dice({2,8,d.casting-2}));}
        else if(command.verb=="magic_missile") {
            --a.slots;int total=0;for(int dart=0;dart<3;++dart)total+=roll(4)+1;
            log(a.source.name+" casts Magic Missile for "+std::to_string(total)+" force damage.");damage(actor(command.target),total);
        } else attack(a,actor(command.target),command.verb!="melee",command.verb=="fire_bolt");
    }
    ++revision_;update_outcome();
    if(outcome_==Outcome::ongoing&&!pending()&&actors_[turn_].hp==0)end_turn();
    return true;
}

std::string Session::save() const
{
    // The module owns the checkpoint format, including RNG and pending reactions.
    std::ostringstream out;out<<"OGCOMBAT 2 "<<std::quoted(content_->identity.module)<<' '<<std::quoted(content_->identity.version)<<' '<<std::quoted(content_->identity.content)<<'\n';
    out<<board_.width<<' '<<board_.height<<'\n';for(auto cell:board_.terrain)out<<unsigned(cell)<<' ';out<<'\n';
    out<<rng_<<' '<<revision_<<' '<<turn_<<' '<<round_<<' '<<static_cast<int>(outcome_)<<' '<<actors_.size()<<'\n';
    for(const auto& a:actors_)out<<a.source.id<<' '<<std::quoted(a.source.definition)<<' '<<std::quoted(a.source.name)<<' '<<a.source.side<<' '<<a.source.cell.x<<' '<<a.source.cell.y<<' '
        <<a.hp<<' '<<a.initiative<<' '<<a.movement<<' '<<a.winds<<' '<<a.slots<<' '<<a.successes<<' '<<a.failures<<' '
        <<a.action<<' '<<a.bonus<<' '<<a.reaction<<' '<<a.dodge<<' '<<a.disengaged<<' '<<a.stable<<' '<<a.dead<<' '<<std::quoted(a.source.character_profile)<<'\n';
    out<<path_.size()<<' '<<path_index_<<'\n';for(auto p:path_)out<<p.x<<' '<<p.y<<' ';out<<'\n';
    out<<reactors_.size()<<' '<<reactor_index_<<'\n';for(auto id:reactors_)out<<id<<' ';out<<'\n';
    out<<log_.size()<<'\n';for(const auto& line:log_)out<<std::quoted(line)<<'\n';return out.str();
}
std::unique_ptr<Session> Session::restore(std::shared_ptr<const Content> content,std::string_view bytes)
{
    if(bytes.size()>65536)throw std::runtime_error("Combat checkpoint exceeds limit");
    std::istringstream in{std::string(bytes)};std::string magic;unsigned version;Identity id;
    in>>magic>>version>>std::quoted(id.module)>>std::quoted(id.version)>>std::quoted(id.content);
    if(!in||magic!="OGCOMBAT"||(version!=1&&version!=2)||id!=content->identity)throw std::runtime_error("Combat checkpoint rules/content version mismatch");
    Encounter e;in>>e.battlefield.width>>e.battlefield.height;
    if(!in||e.battlefield.width<2||e.battlefield.height<2||e.battlefield.width>32||e.battlefield.height>32)throw std::runtime_error("Invalid checkpoint board");
    for(int i=0;i<e.battlefield.width*e.battlefield.height;++i){unsigned t;in>>t;if(!in||t>2)throw std::runtime_error("Invalid checkpoint terrain");e.battlefield.terrain.push_back(t);}
    std::uint64_t rng,revision;unsigned turn,round,count,outcome;in>>rng>>revision>>turn>>round>>outcome>>count;
    if(!in||count<2||count>64||turn>=count||round==0||round>100000||outcome>2||!revision)throw std::runtime_error("Invalid checkpoint header");
    std::vector<Actor> actors;for(unsigned i=0;i<count;++i) {
        Actor a;in>>a.source.id>>std::quoted(a.source.definition)>>std::quoted(a.source.name)>>a.source.side>>a.source.cell.x>>a.source.cell.y
            >>a.hp>>a.initiative>>a.movement>>a.winds>>a.slots>>a.successes>>a.failures>>a.action>>a.bonus>>a.reaction>>a.dodge>>a.disengaged>>a.stable>>a.dead;
        if(version==2)in>>std::quoted(a.source.character_profile);
        if(!in||(a.source.character_profile.empty()&&!content->definitions.contains(a.source.definition)))throw std::runtime_error("Invalid checkpoint actor");
        a.definition=a.source.character_profile.empty()?content->definitions.at(a.source.definition):character_definition(a.source.character_profile);
        const auto& d=a.definition;
        if(a.hp<0||a.hp>d.hp||a.movement<0||a.movement>d.speed*2||a.winds<0||a.winds>d.winds||a.slots<0||a.slots>d.slots||
            a.successes<0||a.successes>3||a.failures<0||a.failures>4||(a.dead&&a.hp>0))throw std::runtime_error("Invalid checkpoint actor state");
        e.participants.push_back(a.source);actors.push_back(std::move(a));
    }
    // Constructor validates identities/geometry before mutable state is accepted.
    auto s=std::make_unique<Session>(content,std::move(e),0,true);s->actors_=std::move(actors);
    s->rng_=rng;s->revision_=revision;s->turn_=turn;s->round_=round;s->outcome_=static_cast<Outcome>(outcome);
    std::size_t n;in>>n>>s->path_index_;if(!in||n>1024||s->path_index_>n)throw std::runtime_error("Invalid checkpoint path");
    for(std::size_t i=0;i<n;++i){Cell p;in>>p.x>>p.y;if(!in||s->board_.at(p)==1)throw std::runtime_error("Invalid checkpoint path cell");s->path_.push_back(p);}
    in>>n>>s->reactor_index_;if(!in||n>64||s->reactor_index_>n)throw std::runtime_error("Invalid checkpoint reactions");
    std::set<EntityId> ids;for(std::size_t i=0;i<n;++i){EntityId reactor;in>>reactor;
        if(!in||!ids.insert(reactor).second||std::none_of(s->actors_.begin(),s->actors_.end(),[&](const auto& a){return a.source.id==reactor;}))throw std::runtime_error("Invalid checkpoint reactor");s->reactors_.push_back(reactor);}
    if(s->pending()&&(s->path_.empty()||s->path_index_>=s->path_.size()))throw std::runtime_error("Reaction without movement");
    bool party=false,enemies=false;
    const auto& mover=s->actors_[turn];
    for(const auto& a:s->actors_)if(a.hp>0&&!a.dead) {
        (a.source.side==0?party:enemies)=true;
        for(const auto& b:s->actors_)if(b.source.id>a.source.id&&b.hp>0&&a.source.cell==b.source.cell&&
            !(s->pending()&&a.source.side==b.source.side&&(a.source.id==mover.source.id||b.source.id==mover.source.id)))
            throw std::runtime_error("Overlapping active checkpoint actors");
    }
    const auto expected=!party?Outcome::defeat:!enemies?Outcome::victory:Outcome::ongoing;
    if(s->outcome_!=expected||(expected==Outcome::ongoing&&mover.hp==0))throw std::runtime_error("Invalid checkpoint outcome/turn");
    if(!s->pending()&&(!s->path_.empty()||!s->reactors_.empty()))throw std::runtime_error("Unpaused checkpoint movement");
    if(s->pending()) {
        if(expected!=Outcome::ongoing||mover.disengaged)throw std::runtime_error("Invalid pending movement");
        auto cell=mover.source.cell;int cost=0;
        for(auto i=s->path_index_;i<s->path_.size();++i) {
            const auto next=s->path_[i];
            if(distance(cell,next)!=5 || (cell.x!=next.x&&cell.y!=next.y&&
                (s->board_.at({cell.x,next.y})==1||s->board_.at({next.x,cell.y})==1)))throw std::runtime_error("Invalid checkpoint movement step");
            bool ally=false;
            for(const auto& a:s->actors_)if(!a.dead&&a.source.id!=mover.source.id&&a.source.cell==next) {
                if(a.source.side!=mover.source.side)throw std::runtime_error("Checkpoint path crosses enemy");ally=true;
            }
            cost+=ally||s->board_.at(next)==2?10:5;cell=next;
        }
        if(cost>mover.movement||s->occupied(cell))throw std::runtime_error("Invalid checkpoint movement budget/destination");
        for(auto i=s->reactor_index_;i<s->reactors_.size();++i) {
            const auto& a=s->actor(s->reactors_[i]);
            if(a.hp==0||!a.reaction||a.source.side==mover.source.side||distance(a.source.cell,mover.source.cell)>5||
                distance(a.source.cell,s->path_[s->path_index_])<=5||!s->line_of_sight(a.source.cell,mover.source.cell))
                throw std::runtime_error("Invalid checkpoint opportunity attack");
        }
    }
    in>>n;if(!in||n>80)throw std::runtime_error("Invalid checkpoint log");s->log_.clear();
    for(std::size_t i=0;i<n;++i){std::string line;in>>std::quoted(line);if(!in||line.size()>1000)throw std::runtime_error("Invalid checkpoint log line");s->log_.push_back(std::move(line));}
    in>>std::ws;if(!in.eof())throw std::runtime_error("Trailing checkpoint data");return s;
}
class Module final : public RulesModule {
public:
    explicit Module(Content content):content_(std::make_shared<const Content>(std::move(content))){}
    Identity identity() const override{return content_->identity;}
    std::vector<std::string> supported_features() const override{return {"initiative","movement","melee","ranged","critical_hits","dodge","dash","disengage","opportunity_attacks","death_saves","second_wind","fire_bolt","cure_wounds","magic_missile","checkpoint"};}
    std::unique_ptr<CombatSession> create(Encounter e,std::uint64_t seed) const override{return std::make_unique<Session>(content_,std::move(e),seed);}
    std::unique_ptr<CombatSession> restore(std::string_view checkpoint) const override{return Session::restore(content_,checkpoint);}
    CharacterProfile character_profile(const CharacterSheet& sheet,std::span<const std::string> gear) const override {
        if(sheet.identity!=character_rules()->identity()||sheet.level!=1)throw std::runtime_error("Unsupported character rules identity or level");
        std::ostringstream out;out<<"PC1 "<<std::quoted(sheet.character_class)<<' '<<std::quoted(sheet.race);
        for(auto score:sheet.scores)out<<' '<<score;
        out<<' '<<gear.size();for(const auto& item:gear)out<<' '<<std::quoted(item);
        const auto data=out.str();const auto d=character_definition(data);
        if(d.hp!=sheet.hit_points)throw std::runtime_error("Character HP does not match rules profile");
        return {data,d.hp,d.ac,"Level-one combat subset: Fighter (Second Wind), Cleric (Cure Wounds), Wizard (Fire Bolt, Magic Missile). Other class/species/background features and spell choices are not implemented.",d.speed,d.melee_bonus};
    }
private: std::shared_ptr<const Content> content_;
};
}
std::unique_ptr<RulesModule> load(const std::filesystem::path& file)
{
    if(std::filesystem::file_size(file)>65536)throw std::runtime_error("Rules content exceeds size limit");
    std::ifstream input(file,std::ios::binary);if(!input)throw std::runtime_error("Cannot open rules content: "+file.string());
    std::string bytes{std::istreambuf_iterator<char>(input),{}};
    if(bytes.size()>65536||input.bad())throw std::runtime_error("Invalid rules content size/read");
    // Git may translate line endings; identical content must keep its identity.
    bytes.erase(std::remove(bytes.begin(),bytes.end(),'\r'),bytes.end());
    std::uint64_t hash=14695981039346656037ULL;for(unsigned char c:bytes){hash^=c;hash*=1099511628211ULL;}
    std::istringstream lines(bytes);std::string line,magic,revision;unsigned version;
    std::getline(lines,line);std::istringstream header(line);header>>magic>>version>>revision;
    if(!header||magic!="OPENGOLD_SRD5"||version!=1)throw std::runtime_error("Unsupported rules content format");
    header>>std::ws;
    if(!header.eof()||revision.empty()||revision.size()>80)throw std::runtime_error("Invalid rules content header");
    Content content;content.identity={"opengold.srd5","0.2.0",revision+"/"+std::to_string(hash)};
    while(std::getline(lines,line)) {
        if(line.empty()||line[0]=='#'||line=="\r")continue;
        std::istringstream row(line);std::string tag,key;Definition d;row>>tag>>key>>d.ac>>d.hp>>d.initiative>>d.speed>>d.melee_bonus>>d.melee.count>>d.melee.sides>>d.melee.bonus
            >>d.ranged_bonus>>d.ranged.count>>d.ranged.sides>>d.ranged.bonus>>d.range>>d.long_range>>d.winds>>d.slots>>d.casting>>d.level>>d.spells;
        if(!row||tag!="creature"||key.size()>80||content.definitions.contains(key)||d.ac<1||d.ac>40||d.hp<1||d.hp>1000||
            d.initiative< -10||d.initiative>20||d.speed<5||d.speed>120||d.speed%5||d.melee.count<1||d.melee.count>10||d.melee.sides<2||d.melee.sides>20||
            d.ranged.count<1||d.ranged.count>10||d.ranged.sides<2||d.ranged.sides>20||d.range<5||d.long_range<d.range||d.long_range>600||
            d.winds<0||d.winds>10||d.slots<0||d.slots>20||d.level<1||d.level>4||d.spells<0||d.spells>7||
            d.melee_bonus< -10||d.melee_bonus>30||d.ranged_bonus< -10||d.ranged_bonus>30||d.casting< -10||d.casting>30||
            d.melee.bonus< -10||d.melee.bonus>30||d.ranged.bonus< -10||d.ranged.bonus>30)
            throw std::runtime_error("Invalid or unsupported creature definition: "+key);
        row>>std::ws;if(!row.eof())throw std::runtime_error("Unknown creature fields: "+key);
        content.definitions.emplace(std::move(key),d);
    }
    if(content.definitions.empty())throw std::runtime_error("Empty rules content");return std::make_unique<Module>(std::move(content));
}
}
