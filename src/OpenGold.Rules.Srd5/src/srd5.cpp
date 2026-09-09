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
int minimum_save_roll(int dc, int bonus) noexcept
{ return static_cast<int>(std::clamp(static_cast<long long>(dc)-bonus,1LL,21LL)); }
bool attack_hits(int natural, int bonus, int ac) noexcept
{ return natural==20 || (natural!=1 && static_cast<std::int64_t>(natural)+bonus>=ac); }
namespace {
bool trained(std::string_view klass,std::string_view key)
{
    if(key=="dagger"||key=="mace"||key=="quarterstaff")return true;
    if(key=="longsword")return klass=="Barbarian"||klass=="Fighter"||klass=="Paladin"||klass=="Ranger";
    if(key=="chain_mail")return klass=="Fighter"||klass=="Paladin";
    if(key=="leather")return klass!="Monk"&&klass!="Sorcerer"&&klass!="Wizard";
    if(key=="shield")return klass=="Barbarian"||klass=="Cleric"||klass=="Druid"||klass=="Fighter"||klass=="Paladin"||klass=="Ranger";
    throw std::runtime_error("Unsupported equipment conversion: "+std::string(key));
}
}
std::string equipment_note(const CharacterSheet& sheet,std::string_view item)
{
    std::string text;
    if(trained(sheet.character_class,item))text="Class training: no untrained-use penalty.";
    else if(item=="shield")text="Untrained shield: no AC bonus.";
    else if(item=="leather"||item=="chain_mail")text="Untrained armor: disadvantage on Strength/Dexterity attacks, checks (including initiative), and saves; cannot cast spells.";
    else text="Untrained weapon: no proficiency bonus on attack rolls (no +2 at level 1).";
    if(item=="chain_mail"&&sheet.scores[0]<13)text+=" Chain mail requires Strength 13: speed reduced by 10 feet.";
    return text;
}
namespace {
struct Dice { int count{}, sides{}, bonus{}; };
struct Definition {
    int ac{}, hp{}, initiative{}, speed{}, melee_bonus{};
    Dice melee;
    int ranged_bonus{};
    Dice ranged;
    int range{}, long_range{}, winds{}, slots{}, casting{}, level{}, spells{},slots2{};
    bool str_dex_disadvantage{},savage{};
};
struct Content { Identity identity; std::optional<Identity> previous_campaign_identity;std::map<std::string,Definition> definitions; };
struct Actor {
    Participant source;
    Definition definition;
    int hp{}, initiative{}, movement{}, winds{}, slots{}, successes{}, failures{},slots2{};
    bool action{true}, bonus{true}, reaction{true}, dodge{}, disengaged{}, stable{}, dead{};
    bool spent_slot{},savage_used{};
};
// Versioned, module-owned character recipe. Original item IDs never enter this layer.
Definition character_definition(std::string_view bytes,bool combat=true)
{
    if(bytes.size()>1024)throw std::runtime_error("Character profile exceeds limit");
    std::istringstream in{std::string(bytes)};
    std::string magic,klass,race;std::array<int,6> scores{};unsigned count{};
    unsigned level=1,features=0,selected_spells=0;in>>magic;if(magic=="PC2"||magic=="PC3")in>>level;
    if(magic=="PC3")in>>features>>selected_spells;in>>std::quoted(klass)>>std::quoted(race);
    for(auto& score:scores)in>>score;
    in>>count;
    if(!in||(magic!="PC1"&&magic!="PC2"&&magic!="PC3")||level<1||level>(magic=="PC3"?4u:2u)||features>3||selected_spells>31||count>3||std::any_of(scores.begin(),scores.end(),[](int n){return n<3||n>20;}))
        throw std::runtime_error("Invalid character profile");
    if(combat&&klass!="Fighter"&&klass!="Cleric"&&klass!="Wizard")
        throw std::runtime_error("Campaign combat supports Fighter, Cleric and Wizard subsets only");
    if(level>1&&klass!="Fighter"&&klass!="Cleric"&&klass!="Wizard")throw std::runtime_error("Advancement is unsupported for this class");
    const auto races=character_rules()->choices(CreationField::race);
    if(std::none_of(races.begin(),races.end(),[&](const auto& r){return r.label==race;}))throw std::runtime_error("Unknown species");
    const int str=ability_modifier(scores[0]),dex=ability_modifier(scores[1]),con=ability_modifier(scores[2]);
    const auto classes=character_rules()->choices(CreationField::character_class);
    if(std::none_of(classes.begin(),classes.end(),[&](const auto& c){return c.label==klass;}))throw std::runtime_error("Unknown class");
    const int die=klass=="Barbarian"?12:(klass=="Fighter"||klass=="Paladin"||klass=="Ranger")?10:(klass=="Wizard"||klass=="Sorcerer")?6:8;
    Definition d;d.hp=die+con+(race=="Dwarf"?int(level):0)+(level-1)*std::max(1,die/2+1+con);
    d.ac=10+dex;d.initiative=dex;d.speed=race=="Goliath"?35:30;d.level=level;
    d.melee_bonus=2+str;d.melee={0,0,std::max(0,1+str)};
    d.winds=klass=="Fighter"?(level==4?3:2):0;d.slots=(klass=="Cleric"||klass=="Wizard")?(level==1?2:level==2?3:4):0;
    d.slots2=(klass=="Cleric"||klass=="Wizard")&&level>=3?(level==3?2:3):0;
    d.casting=2+ability_modifier(scores[klass=="Cleric"?4:3]);d.spells=klass=="Cleric"?2:klass=="Wizard"?5:0;
    if(magic=="PC3"){
        const unsigned allowed=klass=="Cleric"?10:klass=="Wizard"?(level>=3?21:5):0;
        if(selected_spells&~allowed||(features&1)&&klass!="Fighter")throw std::runtime_error("Invalid prepared spells or feat prerequisites");
        d.spells=selected_spells;d.savage=(features&2)!=0;
    }
    bool weapon=false,armor=false,shield=false;
    for(unsigned i=0;i<count;++i){std::string key;in>>std::quoted(key);
        if(key=="dagger"||key=="mace"||key=="longsword"||key=="quarterstaff"){
            if(weapon)throw std::runtime_error("Only one weapon may be equipped");weapon=true;
            const int modifier=key=="dagger"?std::max(str,dex):str;
            d.melee_bonus=(trained(klass,key)?2:0)+modifier;d.melee={1,key=="dagger"?4:key=="longsword"?8:6,modifier};
        }else if(key=="leather"||key=="chain_mail"){
            if(armor)throw std::runtime_error("Only one armor may be equipped");
            if(!trained(klass,key)){d.str_dex_disadvantage=true;d.spells=0;}
            armor=true;d.ac=key=="leather"?11+dex:16;
            if(key=="chain_mail"&&scores[0]<13)d.speed-=10;
        }else if(key=="shield"){
            if(shield)throw std::runtime_error("Only one shield may be equipped");shield=true;
        }else throw std::runtime_error("Unsupported equipment conversion: "+key);
    }
    if(!armor&&klass=="Barbarian")d.ac=std::max(d.ac,10+dex+con);
    if(!armor&&!shield&&klass=="Monk")d.ac=std::max(d.ac,10+dex+ability_modifier(scores[4]));
    if(shield&&trained(klass,"shield"))d.ac+=2;
    if(armor&&(features&1))++d.ac;
    in>>std::ws;if(!in.eof())throw std::runtime_error("Invalid character profile fields");return d;
}
void restore_vitals(Actor& a,const VitalState& state)
{
    a.hp=state.hit_points;a.dead=state.dead;
    if(!state.resources.empty()) {
        std::istringstream in(state.resources);std::string magic;
        in>>magic>>a.winds>>a.slots;if(magic=="SRD2")in>>a.slots2;
        in>>a.successes>>a.failures>>a.stable;
        if(!in||(magic!="SRD1"&&magic!="SRD2"))throw std::runtime_error("Invalid character resource state");
        in>>std::ws;if(!in.eof())throw std::runtime_error("Trailing character resource state");
    }
    const auto& d=a.definition;
    if(a.hp<0||a.hp>d.hp||(a.dead&&a.hp!=0)||a.winds<0||a.winds>d.winds||a.slots<0||a.slots>d.slots||
        a.slots2<0||a.slots2>d.slots2||a.successes<0||a.successes>3||a.failures<0||a.failures>4)throw std::runtime_error("Invalid character vitals");
}
VitalState vitals(const Actor& a)
{
    std::ostringstream out;out<<(a.definition.slots2?"SRD2 ":"SRD1 ")<<a.winds<<' '<<a.slots<<' ';
    if(a.definition.slots2)out<<a.slots2<<' ';out<<a.successes<<' '<<a.failures<<' '<<a.stable;
    std::string description;
    if(a.definition.slots)description="Level-one spell slots: "+std::to_string(a.slots)+" / "+std::to_string(a.definition.slots);
    if(a.definition.slots2)description+="\nLevel-two spell slots: "+std::to_string(a.slots2)+" / "+std::to_string(a.definition.slots2);
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
        if (board_.width<2 || board_.height<2 || board_.width>64 || board_.height>64 ||
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
            Actor a; a.definition=d;a.source=std::move(p);a.hp=d.hp;a.winds=d.winds;a.slots=d.slots;a.slots2=d.slots2;
            if(a.source.state)restore_vitals(a,*a.source.state);
            a.initiative=roll(20);if(d.str_dex_disadvantage||a.source.surprised)a.initiative=std::min(a.initiative,roll(20));a.initiative+=d.initiative;a.movement=d.speed;
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
    void attack(Actor& a,Actor& target,bool ranged,bool spell=false,Dice spell_dice={1,10,0});
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
    const auto spell=[&](const char* verb,const char* label,EntityId target){
        if(a.spent_slot)return;
        if(a.slots>0)add(id,verb,label,target);
        if(a.slots2>0)add(id,std::string(verb)+"_2",std::string(label)+" (level 2 slot)",target);
    };
    if(a.bonus&&(d.spells&8))for(const auto& other:actors_)
        if(!other.dead&&other.source.side==a.source.side&&other.hp<def(other).hp&&distance(a.source.cell,other.source.cell)<=60&&line_of_sight(a.source.cell,other.source.cell))
            spell("healing_word","Healing Word",other.source.id);
    if(a.action) {
        add(id,"dash","Dash");add(id,"dodge","Dodge");add(id,"disengage","Disengage");
        for(const auto& other:actors_) {
            if(other.dead || !line_of_sight(a.source.cell,other.source.cell))continue;
            const int feet=distance(a.source.cell,other.source.cell);
            if(other.source.side!=a.source.side && other.hp>0) {
                if(feet<=5)add(id,"melee","Melee attack",other.source.id);
                if(feet<=d.long_range)add(id,"ranged","Ranged attack",other.source.id);
                if((d.spells&1)&&feet<=120)add(id,"fire_bolt","Fire Bolt",other.source.id);
                if((d.spells&4)&&feet<=120)spell("magic_missile","Magic Missile",other.source.id);
                if((d.spells&16)&&a.slots2>0&&!a.spent_slot&&feet<=120)add(id,"scorching_ray","Scorching Ray",other.source.id);
            } else if(other.source.side==a.source.side && other.hp<def(other).hp && feet<=5 && (d.spells&2))
                spell("cure_wounds","Cure Wounds",other.source.id);
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
void Session::attack(Actor& a,Actor& target,bool ranged,bool spell,Dice spell_dice)
{
    const auto& d=def(a);bool disadvantaged=target.dodge||(!spell&&d.str_dex_disadvantage);
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
    const Dice damage_dice=spell?spell_dice:ranged?d.ranged:d.melee;
    int amount=dice(damage_dice,natural==20);
    if(!spell&&damage_dice.count&&d.savage&&!a.savage_used){amount=std::max(amount,dice(damage_dice,natural==20));a.savage_used=true;message+=" (Savage Attacker)";}
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
    for(auto& actor:actors_)actor.savage_used=false;
    auto& a=actors_[turn_];a.action=a.bonus=a.reaction=true;a.dodge=a.disengaged=false;a.movement=def(a).speed;
    a.spent_slot=false;
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
    const bool second=command.verb.ends_with("_2");
    const auto spend=[&]{if(second||command.verb=="scorching_ray")--a.slots2;else --a.slots;a.spent_slot=true;};
    if(command.verb=="opportunity"||command.verb=="decline") {
        if(command.verb=="opportunity"){a.reaction=false;attack(a,actor(command.target),false);}
        ++reactor_index_;update_outcome();if(outcome_==Outcome::ongoing)progress_movement();
    } else if(command.verb=="move") {
        path_=path_to(a,command.destination);path_index_=0;progress_movement();
    } else if(command.verb=="end")end_turn();
    else if(command.verb=="second_wind") {a.bonus=false;--a.winds;heal(a,roll(10)+d.level);}
    else if(command.verb=="healing_word"||command.verb=="healing_word_2"){a.bonus=false;spend();heal(actor(command.target),dice({second?4:2,4,d.casting-2}));}
    else {
        a.action=false;
        if(command.verb=="dash"){a.movement+=d.speed;log(a.source.name+" dashes.");}
        else if(command.verb=="dodge"){a.dodge=true;log(a.source.name+" dodges.");}
        else if(command.verb=="disengage"){a.disengaged=true;log(a.source.name+" disengages.");}
        else if(command.verb=="cure_wounds"||command.verb=="cure_wounds_2"){spend();heal(actor(command.target),dice({second?4:2,8,d.casting-2}));}
        else if(command.verb=="magic_missile"||command.verb=="magic_missile_2") {
            spend();int total=0;for(int dart=0;dart<(second?4:3);++dart)total+=roll(4)+1;
            log(a.source.name+" casts Magic Missile for "+std::to_string(total)+" force damage.");damage(actor(command.target),total);
        } else if(command.verb=="scorching_ray"){
            spend();for(unsigned ray=0;ray<3&&actor(command.target).hp>0;++ray)attack(a,actor(command.target),true,true,{2,6,0});
        }else attack(a,actor(command.target),command.verb!="melee",command.verb=="fire_bolt");
    }
    ++revision_;update_outcome();
    if(outcome_==Outcome::ongoing&&!pending()&&actors_[turn_].hp==0)end_turn();
    return true;
}

std::string Session::save() const
{
    // The module owns the checkpoint format, including RNG and pending reactions.
    std::ostringstream out;out<<"OGCOMBAT 3 "<<std::quoted(content_->identity.module)<<' '<<std::quoted(content_->identity.version)<<' '<<std::quoted(content_->identity.content)<<'\n';
    out<<board_.width<<' '<<board_.height<<'\n';for(auto cell:board_.terrain)out<<unsigned(cell)<<' ';out<<'\n';
    out<<rng_<<' '<<revision_<<' '<<turn_<<' '<<round_<<' '<<static_cast<int>(outcome_)<<' '<<actors_.size()<<'\n';
    for(const auto& a:actors_)out<<a.source.id<<' '<<std::quoted(a.source.definition)<<' '<<std::quoted(a.source.name)<<' '<<a.source.side<<' '<<a.source.cell.x<<' '<<a.source.cell.y<<' '
        <<a.hp<<' '<<a.initiative<<' '<<a.movement<<' '<<a.winds<<' '<<a.slots<<' '<<a.successes<<' '<<a.failures<<' '
        <<a.action<<' '<<a.bonus<<' '<<a.reaction<<' '<<a.dodge<<' '<<a.disengaged<<' '<<a.stable<<' '<<a.dead<<' '<<std::quoted(a.source.character_profile)<<' '<<a.slots2<<' '<<a.spent_slot<<' '<<a.savage_used<<'\n';
    out<<path_.size()<<' '<<path_index_<<'\n';for(auto p:path_)out<<p.x<<' '<<p.y<<' ';out<<'\n';
    out<<reactors_.size()<<' '<<reactor_index_<<'\n';for(auto id:reactors_)out<<id<<' ';out<<'\n';
    out<<log_.size()<<'\n';for(const auto& line:log_)out<<std::quoted(line)<<'\n';return out.str();
}
std::unique_ptr<Session> Session::restore(std::shared_ptr<const Content> content,std::string_view bytes)
{
    if(bytes.size()>65536)throw std::runtime_error("Combat checkpoint exceeds limit");
    std::istringstream in{std::string(bytes)};std::string magic;unsigned version;Identity id;
    in>>magic>>version>>std::quoted(id.module)>>std::quoted(id.version)>>std::quoted(id.content);
    if(!in||magic!="OGCOMBAT"||(version<1||version>3)||id!=content->identity)throw std::runtime_error("Combat checkpoint rules/content version mismatch");
    Encounter e;in>>e.battlefield.width>>e.battlefield.height;
    if(!in||e.battlefield.width<2||e.battlefield.height<2||e.battlefield.width>64||e.battlefield.height>64)throw std::runtime_error("Invalid checkpoint board");
    for(int i=0;i<e.battlefield.width*e.battlefield.height;++i){unsigned t;in>>t;if(!in||t>2)throw std::runtime_error("Invalid checkpoint terrain");e.battlefield.terrain.push_back(t);}
    std::uint64_t rng,revision;unsigned turn,round,count,outcome;in>>rng>>revision>>turn>>round>>outcome>>count;
    if(!in||count<2||count>64||turn>=count||round==0||round>100000||outcome>2||!revision)throw std::runtime_error("Invalid checkpoint header");
    std::vector<Actor> actors;for(unsigned i=0;i<count;++i) {
        Actor a;in>>a.source.id>>std::quoted(a.source.definition)>>std::quoted(a.source.name)>>a.source.side>>a.source.cell.x>>a.source.cell.y
            >>a.hp>>a.initiative>>a.movement>>a.winds>>a.slots>>a.successes>>a.failures>>a.action>>a.bonus>>a.reaction>>a.dodge>>a.disengaged>>a.stable>>a.dead;
        if(version>=2)in>>std::quoted(a.source.character_profile);
        if(version>=3)in>>a.slots2>>a.spent_slot>>a.savage_used;
        if(!in||(a.source.character_profile.empty()&&!content->definitions.contains(a.source.definition)))throw std::runtime_error("Invalid checkpoint actor");
        a.definition=a.source.character_profile.empty()?content->definitions.at(a.source.definition):character_definition(a.source.character_profile);
        const auto& d=a.definition;
        if(a.hp<0||a.hp>d.hp||a.movement<0||a.movement>d.speed*2||a.winds<0||a.winds>d.winds||a.slots<0||a.slots>d.slots||
            a.slots2<0||a.slots2>d.slots2||a.successes<0||a.successes>3||a.failures<0||a.failures>4||(a.dead&&a.hp>0))throw std::runtime_error("Invalid checkpoint actor state");
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
    bool accepts_campaign_identity(const Identity& saved) const override {
        if(saved.version!=content_->identity.version&&saved.version!="0.3.0")return false;
        auto compatible=saved;compatible.version=content_->identity.version;
        return compatible==content_->identity||(content_->previous_campaign_identity&&compatible==*content_->previous_campaign_identity);
    }
    std::vector<std::string> supported_features() const override{return {"initiative","movement","melee","ranged","critical_hits","dodge","dash","disengage","opportunity_attacks","death_saves","second_wind","fire_bolt","cure_wounds","magic_missile","healing_word","scorching_ray","level_two_slots","manual_advancement","ability_score_improvement","defense","savage_attacker","checkpoint"};}
    std::unique_ptr<CombatSession> create(Encounter e,std::uint64_t seed) const override{return std::make_unique<Session>(content_,std::move(e),seed);}
    std::unique_ptr<CombatSession> restore(std::string_view checkpoint) const override{return Session::restore(content_,checkpoint);}
    unsigned experience_for_level(unsigned level) const override
    {static constexpr unsigned thresholds[]{0,0,300,900,2700};if(level<1||level>4)throw std::runtime_error("Unsupported character level");return thresholds[level];}
    bool advance_character(CharacterSheet& sheet,VitalState& state) const override
    {return advance_character(sheet,state,default_advancement(sheet));}
    AdvancementOptions advancement_options(const CharacterSheet& sheet) const override
    {
        if(sheet.level>=4||(sheet.character_class!="Fighter"&&sheet.character_class!="Cleric"&&sheet.character_class!="Wizard"))return {};
        AdvancementOptions result;result.level=sheet.level+1;
        result.description="Fixed-average HP growth. Spendable resources gain only their new capacity; existing expenditure remains.";
        if(result.level==4)result.feats={
            {"ability_score_improvement","Ability points","Add 2 to one ability or 1 to two abilities; maximum 20."},
            {"defense","Defense","+1 AC while wearing armor. Requires the Fighter's Fighting Style feature.",sheet.character_class=="Fighter"},
            {"savage_attacker","Savage Attacker","Roll weapon damage twice on the first weapon hit each turn; use the higher result.",sheet.background!="Soldier"},
            {"grappler","Grappler","Unavailable: grappling is not implemented.",false},
            {"magic_initiate","Magic Initiate","Unavailable: its complete spell-selection feature is not implemented.",false}};
        if(sheet.character_class=="Cleric")result.spells={
            {"cure_wounds","Cure Wounds","Action; touch; heals 2d8 + Wisdom modifier."},
            {"healing_word","Healing Word","Bonus action; 60 feet; heals 2d4 + Wisdom modifier."},
            {"bless","Bless","Unavailable: concentration is not implemented.",false}};
        if(sheet.character_class=="Wizard")result.spells={
            {"magic_missile","Magic Missile","Action; 120 feet; three darts at one target."},
            {"scorching_ray","Scorching Ray","Action; 120 feet; three spell attacks at one target. Requires level 3.",result.level>=3},
            {"shield","Shield","Unavailable: spell reactions are not implemented.",false}};
        return result;
    }
    AdvancementChoice default_advancement(const CharacterSheet& sheet) const override
    {
        AdvancementChoice choice;const auto options=advancement_options(sheet);if(!options.level)return choice;
        choice.spells=sheet.prepared_spells;
        if(choice.spells.empty()){if(sheet.character_class=="Cleric")choice.spells={"cure_wounds"};else if(sheet.character_class=="Wizard")choice.spells={"magic_missile"};}
        if(options.level==4){
            choice.feat="ability_score_improvement";const unsigned primary=sheet.character_class=="Fighter"?0:sheet.character_class=="Cleric"?4:3;
            unsigned remaining=2;for(unsigned n=0;n<6&&remaining;++n){const auto index=(primary+n)%6;
                choice.abilities[index]=std::min(remaining,unsigned(std::max(0,20-sheet.scores[index])));remaining-=choice.abilities[index];}
        }
        return choice;
    }
    bool advance_character(CharacterSheet& sheet,VitalState& state,const AdvancementChoice& choice) const override
    {
        const auto options=advancement_options(sheet);if(!options.level)return false;
        const auto old=character_definition(character_profile(sheet,{}).data);
        unsigned points=0;for(auto n:choice.abilities){if(n>2)throw std::runtime_error("An ability increase cannot exceed 2");points+=n;}
        if(options.level==4){
            const auto feat=std::find_if(options.feats.begin(),options.feats.end(),[&](const auto& f){return f.id==choice.feat&&f.available;});
            if(feat==options.feats.end())throw std::runtime_error("Choose an available feat or ability points");
            if(points!=(choice.feat=="ability_score_improvement"?2u:0u))throw std::runtime_error("Assign exactly two ability points, or choose a feat");
        }else if(!choice.feat.empty()||points)throw std::runtime_error("Feats and ability points are available at level 4");
        std::set<std::string> selected;
        for(const auto& spell:choice.spells){if(!selected.insert(spell).second||std::none_of(options.spells.begin(),options.spells.end(),[&](const auto& s){return s.id==spell&&s.available;}))throw std::runtime_error("Choose only available, distinct spells");}
        if(!options.spells.empty()&&choice.spells.empty())throw std::runtime_error("Choose at least one supported spell");
        Actor actor;actor.definition=old;actor.winds=old.winds;actor.slots=old.slots;actor.slots2=old.slots2;restore_vitals(actor,state);
        auto next=sheet;++next.level;
        for(unsigned n=0;n<6;++n){next.scores[n]+=choice.abilities[n];next.bonuses[n]+=choice.abilities[n];
            if(next.scores[n]>20)throw std::runtime_error("Ability scores cannot exceed 20");
            next.modifiers[n]=ability_modifier(next.scores[n]);next.saving_throws[n]=next.modifiers[n]+(next.save_proficiencies[n]?2:0);}
        if(!choice.feat.empty())next.feats.push_back(choice.feat);next.prepared_spells=choice.spells;
        const int con=next.modifiers[2];next.hit_points=next.hit_die+con+(next.race=="Dwarf"?next.level:0)+(next.level-1)*std::max(1,next.hit_die/2+1+con);
        const int growth=next.hit_points-sheet.hit_points;
        next.hp_explanation="Level "+std::to_string(next.level)+": "+std::to_string(next.hit_points)+" maximum HP; gain "+std::to_string(growth)+". Fixed-average Hit Die growth includes Constitution and any retroactive Constitution increase.";
        next.class_modifiers+="\nLevel "+std::to_string(next.level)+": HP and spell-slot advancement applied. Additional class and subclass features remain unavailable.";
        actor.definition=character_definition(character_profile(next,{}).data);
        if(actor.hp>0)actor.hp+=growth;
        actor.slots+=actor.definition.slots-old.slots;actor.slots2+=actor.definition.slots2-old.slots2;actor.winds+=actor.definition.winds-old.winds;
        auto continuation=vitals(actor);sheet=std::move(next);state=std::move(continuation);
        return true;
    }
    void validate_character_state(const CharacterSheet& sheet,const VitalState& state) const override
    {
        Actor actor;actor.definition=character_definition(character_profile(sheet,{}).data,false);
        actor.winds=actor.definition.winds;actor.slots=actor.definition.slots;actor.slots2=actor.definition.slots2;restore_vitals(actor,state);
    }
    RestPolicy long_rest_policy() const override {return {480,960};}
    void recover(VitalState& state,const CharacterSheet& sheet) const override
    {
        const auto d=character_definition(character_profile(sheet,{}).data,false);
        Actor actor;actor.definition=d;actor.winds=d.winds;actor.slots=d.slots;actor.slots2=d.slots2;restore_vitals(actor,state);
        if(actor.dead||actor.hp<1)throw std::runtime_error("Long rest requires at least one HP at its start");
        actor.hp=d.hp;actor.winds=d.winds;actor.slots=d.slots;actor.slots2=d.slots2;actor.successes=actor.failures=0;actor.stable=false;
        state=vitals(actor);
    }
    void temple_heal(VitalState& state,const CharacterSheet& sheet,std::uint64_t& random_state) const override
    {
        const auto d=character_definition(character_profile(sheet,{}).data,false);
        Actor actor;actor.definition=d;actor.winds=d.winds;actor.slots=d.slots;actor.slots2=d.slots2;restore_vitals(actor,state);
        if(actor.dead||actor.hp>=d.hp)throw std::runtime_error("Cure Wounds requires a wounded living member");
        // Authored temple caster: Cure Wounds, Wisdom +3. Same SplitMix64 as combat.
        auto rng=random_state;int amount=3;
        for(int i=0;i<2;++i){auto z=(rng+=0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;
            z=(z^(z>>27))*0x94d049bb133111ebULL;amount+=int((z^(z>>31))%8)+1;}
        actor.hp=std::min(d.hp,actor.hp+amount);actor.successes=actor.failures=0;actor.stable=false;
        auto next=vitals(actor);state=std::move(next);random_state=rng;
    }
    CharacterProfile character_profile(const CharacterSheet& sheet,std::span<const std::string> gear) const override {
        if(sheet.identity!=character_rules()->identity()||sheet.level<1||sheet.level>4)throw std::runtime_error("Unsupported character rules identity or level");
        unsigned features=sheet.background=="Soldier"?2:0;
        if(sheet.feats.size()!=(sheet.level==4?1u:0u))throw std::runtime_error("Invalid advancement feat count");
        for(const auto& feat:sheet.feats){
            if(feat=="defense"&&sheet.character_class=="Fighter")features|=1;
            else if(feat=="savage_attacker"&&sheet.background!="Soldier")features|=2;
            else if(feat!="ability_score_improvement")throw std::runtime_error("Invalid advancement feat");
        }
        unsigned spells=sheet.character_class=="Wizard"?1:0;std::set<std::string> selected;
        if(sheet.prepared_spells.empty()){if(sheet.character_class=="Wizard")spells|=4;else if(sheet.character_class=="Cleric")spells|=2;}
        for(const auto& spell:sheet.prepared_spells){
            if(!selected.insert(spell).second)throw std::runtime_error("Duplicate prepared spell");
            if(spell=="cure_wounds"&&sheet.character_class=="Cleric")spells|=2;
            else if(spell=="healing_word"&&sheet.character_class=="Cleric")spells|=8;
            else if(spell=="magic_missile"&&sheet.character_class=="Wizard")spells|=4;
            else if(spell=="scorching_ray"&&sheet.character_class=="Wizard"&&sheet.level>=3)spells|=16;
            else throw std::runtime_error("Unsupported prepared spell");
        }
        std::ostringstream out;out<<"PC3 "<<sheet.level<<' '<<features<<' '<<spells<<' '<<std::quoted(sheet.character_class)<<' '<<std::quoted(sheet.race);
        for(auto score:sheet.scores)out<<' '<<score;
        out<<' '<<gear.size();for(const auto& item:gear)out<<' '<<std::quoted(item);
        const auto data=out.str();const auto d=character_definition(data,false);
        if(d.hp!=sheet.hit_points)throw std::runtime_error("Character HP does not match rules profile");
        CharacterProfile result{data,d.hp,d.ac,"Level 1-4 subset: HP, selected feats, supported prepared spells and level-one/two slots. Additional class/subclass and species features remain unavailable.",d.speed,d.melee_bonus};
        result.strength_dexterity_disadvantage=d.str_dex_disadvantage;
        for(const auto& key:gear){
            if(key=="shield")result.item_modifiers+=trained(sheet.character_class,key)?"Source: equipped Shield: +2 AC.\n":"Source: equipped Shield: +0 AC (untrained).\n";
            else if(key=="leather")result.item_modifiers+="Source: equipped Leather armor and Dexterity score "+std::to_string(sheet.scores[1])+". AC becomes 11 + Dexterity modifier ("+std::to_string(sheet.modifiers[1])+").\n";
            else if(key=="chain_mail")result.item_modifiers+="Source: equipped Chain mail. AC becomes 16; speed -10 feet below Strength 13 (current Strength "+std::to_string(sheet.scores[0])+").\n";
            else result.item_modifiers+="Source: equipped "+key+" and "+sheet.character_class+" weapon proficiency. Melee attack uses "+(key=="dagger"?std::string("higher of Strength or Dexterity"):std::string("Strength"))+" modifier"+(trained(sheet.character_class,key)?" +2 class proficiency":" without proficiency")+"; damage adds that ability modifier.\n";
        }
        for(const auto& key:gear)result.item_modifiers+="Source: equipped "+key+". "+equipment_note(sheet,key)+"\n";
        if(features&1)result.item_modifiers+="Defense feat: +1 AC while wearing armor.\n";
        if(features&2)result.item_modifiers+="Savage Attacker: higher of two weapon-damage rolls on the first weapon hit each turn.\n";
        if(gear.empty())result.item_modifiers="No equipment modifiers. Source: unarmed strike rules and Strength score "+std::to_string(sheet.scores[0])+". Attack uses Strength modifier +2 level-one proficiency; damage is 1 + Strength modifier (minimum 0).";
        result.spell_modifiers="No active spell modifiers. Persistent spell effects are not implemented.";
        if(sheet.character_class=="Wizard")result.spell_modifiers="Source: Fire Bolt and Wizard spellcasting, Intelligence score "+std::to_string(sheet.scores[3])+". Attack: Intelligence modifier +2 level-one proficiency = "+std::to_string(d.casting)+". Magic Missile has no ability modifier to damage.\n"+result.spell_modifiers;
        if(sheet.character_class=="Cleric")result.spell_modifiers="Source: Cure Wounds and Cleric spellcasting, Wisdom score "+std::to_string(sheet.scores[4])+". Healing: 2d8 + Wisdom modifier ("+std::to_string(d.casting-2)+").\n"+result.spell_modifiers;
        if(d.str_dex_disadvantage)result.spell_modifiers="Cannot cast spells while wearing untrained armor.\n"+result.spell_modifiers;
        return result;
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
    Content content;content.identity={"opengold.srd5","0.4.0",revision+"/"+std::to_string(hash)};
    // Additive roaming profiles do not invalidate existing saved characters.
    // Reconstruct the exact preceding pack identity; modifications to any old
    // definition still produce a mismatch. Combat checkpoints remain strict.
    std::istringstream previous_lines(bytes);std::string previous,line_before;
    const std::array<std::string_view,6> additions{"slums-kobold","slums-goblin","slums-kobold-leader","slums-goblin-leader","slums-orc-leader","slums-bugbear"};
    while(std::getline(previous_lines,line_before)){
        std::istringstream row(line_before);std::string tag,key;row>>tag>>key;
        if(tag=="creature"&&std::find(additions.begin(),additions.end(),key)!=additions.end())continue;
        previous+=line_before+'\n';
    }
    std::uint64_t previous_hash=14695981039346656037ULL;for(unsigned char c:previous){previous_hash^=c;previous_hash*=1099511628211ULL;}
    if(previous_hash!=hash)content.previous_campaign_identity=Identity{"opengold.srd5","0.4.0",revision+"/"+std::to_string(previous_hash)};
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
