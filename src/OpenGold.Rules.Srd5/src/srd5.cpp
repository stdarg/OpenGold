#include "dice.h"
#include "combat_grid.h"
#include "status_effects.h"
#include "weapons.h"
#include "opengold/srd5.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
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
std::string attack_ability(std::string_view key)
{
    const auto* item=detail::weapon(key);
    return item&&item->finesse?"higher of Strength or Dexterity":item&&item->ranged?"Dexterity":"Strength";
}
bool trained(std::string_view klass,std::string_view key)
{
    // Starting-class grants, SRD 5.2.1 pp. 49 and 61. Multiclass entry and
    // optional feature grants are separate, not yet implemented capabilities.
    if(const auto* weapon=detail::weapon(key))
        return !weapon->martial||klass=="Barbarian"||klass=="Fighter"||klass=="Paladin"||klass=="Ranger"||
            (klass=="Rogue"&&(weapon->finesse||weapon->light))||
            (klass=="Monk"&&weapon->light);
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
    if(item=="wand")text+=" Plain focus only; no charged wand spell is granted.";
    return text;
}
namespace {
struct Dice { int count{}, sides{}, bonus{}; };
struct Definition {
    int ac{}, hp{}, initiative{}, speed{}, melee_bonus{};
    Dice melee;
    int ranged_bonus{};
    int reach{5};
    Dice ranged;
    int range{}, long_range{}, winds{}, slots{}, casting{}, level{}, spells{},slots2{};
    bool str_dex_disadvantage{},savage{};
    std::array<int,6> saves{};
};
struct CombatDisplay {const char* type;const char* melee;const char* ranged;};
CombatDisplay combat_display(std::string_view definition)
{
    if(definition=="slums-kobold")return {"Kobold","Dagger",nullptr};
    if(definition=="slums-kobold-leader")return {"Kobold Leader","Short sword","Short bow"};
    if(definition=="slums-kobold-leader-sword")return {"Kobold Leader","Short sword",nullptr};
    if(definition=="bandit")return {"Bandit","Scimitar","Light crossbow"};
    if(definition=="slums-goblin")return {"Goblin Guard","Short sword",nullptr};
    if(definition=="slums-goblin-leader")return {"Goblin Leader","Short sword",nullptr};
    if(definition=="slums-orc")return {"Orc",nullptr,nullptr};
    if(definition=="slums-orc-leader")return {"Orc Leader",nullptr,nullptr};
    if(definition=="slums-bugbear")return {"Bugbear",nullptr,nullptr};
    return {nullptr,nullptr,nullptr};
}
struct Content { Identity identity; std::vector<Identity> previous_campaign_identities;std::map<std::string,Definition> definitions; };
struct Actor {
    Participant source;
    Definition definition;
    int hp{}, initiative{}, movement{}, winds{}, slots{}, successes{}, failures{},slots2{};
    bool action{true}, bonus{true}, reaction{true}, dodge{}, disengaged{}, stable{}, dead{};
    bool spent_slot{},savage_used{},facing_left{};
    detail::EffectState effects;
};
int maximum_hit_points(int die,bool dwarf,std::span<const int> modifiers)
{
    if(modifiers.empty()||modifiers.size()>4||
        std::any_of(modifiers.begin(),modifiers.end(),[](int n){return n<-4||n>5;}))
        throw std::runtime_error("Invalid HP advancement history");
    int hp=die+modifiers.front()+(dwarf?int(modifiers.size()):0);
    for(std::size_t i=1;i<modifiers.size();++i){
        const int increase=modifiers[i]-modifiers[i-1];
        if(increase<0||increase>1||(i!=3&&increase))throw std::runtime_error("Invalid Constitution advancement history");
        // SRD p. 23: gain HP first, then apply a new modifier per attained level.
        hp+=std::max(1,die/2+1+modifiers[i-1])+increase*int(i+1);
    }
    return hp;
}
// Versioned, module-owned character recipe. Original item IDs never enter this layer.
Definition character_definition(std::string_view bytes)
{
    if(bytes.size()>1024)throw std::runtime_error("Character profile exceeds limit");
    std::istringstream in{std::string(bytes)};
    std::string magic,klass,race;std::array<int,6> scores{};unsigned count{};
    unsigned level=1,features=0,selected_spells=0;in>>magic;
    const bool selected=magic=="PC3"||magic=="PC4";
    if(magic=="PC2"||selected)in>>level;
    if(selected)in>>features>>selected_spells;in>>std::quoted(klass)>>std::quoted(race);
    for(auto& score:scores)in>>score;
    if(!in||(magic!="PC1"&&magic!="PC2"&&!selected)||level<1||level>(selected?4u:2u)||features>3||selected_spells>63||std::any_of(scores.begin(),scores.end(),[](int n){return n<3||n>20;}))
        throw std::runtime_error("Invalid character profile");
    if(level>1&&klass!="Fighter"&&klass!="Cleric"&&klass!="Wizard")throw std::runtime_error("Advancement is unsupported for this class");
    const auto races=character_rules()->choices(CreationField::race);
    if(std::none_of(races.begin(),races.end(),[&](const auto& r){return r.label==race;}))throw std::runtime_error("Unknown species");
    const int str=ability_modifier(scores[0]),dex=ability_modifier(scores[1]),con=ability_modifier(scores[2]);
    std::vector<int> hp_modifiers(level,con);
    if(magic=="PC4")for(auto& modifier:hp_modifiers)in>>modifier;
    in>>count;
    if(!in||count>3||hp_modifiers.back()!=con||((features&1)&&hp_modifiers.front()!=con))
        throw std::runtime_error("Invalid character HP history or equipment count");
    const auto classes=character_rules()->choices(CreationField::character_class);
    if(std::none_of(classes.begin(),classes.end(),[&](const auto& c){return c.label==klass;}))throw std::runtime_error("Unknown class");
    const int die=klass=="Barbarian"?12:(klass=="Fighter"||klass=="Paladin"||klass=="Ranger")?10:(klass=="Wizard"||klass=="Sorcerer")?6:8;
    Definition d;d.hp=maximum_hit_points(die,race=="Dwarf",hp_modifiers);
    const auto trained_saves=detail::class_save_proficiencies(klass);
    for(unsigned i=0;i<6;++i)d.saves[i]=ability_modifier(scores[i])+((i==trained_saves[0]||i==trained_saves[1])?2:0);
    d.ac=10+dex;d.initiative=dex;d.speed=race=="Goliath"?35:30;d.level=level;
    d.melee_bonus=2+str;d.melee={0,0,std::max(0,1+str)};
    d.winds=klass=="Fighter"?(level==4?3:2):0;d.slots=(klass=="Cleric"||klass=="Wizard")?(level==1?2:level==2?3:4):0;
    d.slots2=(klass=="Cleric"||klass=="Wizard")&&level>=3?(level==3?2:3):0;
    d.casting=2+ability_modifier(scores[klass=="Cleric"?4:3]);d.spells=klass=="Cleric"?2:klass=="Wizard"?5:0;
    if(selected){
        const unsigned allowed=klass=="Cleric"?(level>=3?42:10):klass=="Wizard"?(level>=3?53:5):0;
        if(selected_spells&~allowed||(features&1)&&klass!="Fighter")throw std::runtime_error("Invalid prepared spells or feat prerequisites");
        d.spells=selected_spells;d.savage=(features&2)!=0;
    }
    bool weapon=false,armor=false,shield=false;unsigned hands=0;
    for(unsigned i=0;i<count;++i){std::string key;in>>std::quoted(key);
        if(const auto* item=detail::weapon(key)){
            if(weapon)throw std::runtime_error("Only one weapon may be equipped");weapon=true;
            hands+=item->hands;
            const int modifier=item->finesse?std::max(str,dex):item->ranged?dex:str;
            const int bonus=(trained(klass,key)?2:0)+modifier;
            if(item->dice&&!item->ranged){d.melee_bonus=bonus;d.melee={item->dice,item->sides,modifier};d.reach=item->reach;}
            if(item->range){d.ranged_bonus=bonus;d.ranged={item->dice,item->sides,modifier};d.range=item->range;d.long_range=item->long_range;}
        }else if(key=="leather"||key=="chain_mail"){
            if(armor)throw std::runtime_error("Only one armor may be equipped");
            if(!trained(klass,key)){d.str_dex_disadvantage=true;d.spells=0;}
            armor=true;d.ac=key=="leather"?11+dex:16;
            if(key=="chain_mail"&&scores[0]<13)d.speed-=10;
        }else if(key=="shield"){
            if(shield)throw std::runtime_error("Only one shield may be equipped");shield=true;++hands;
        }else throw std::runtime_error("Unsupported equipment conversion: "+key);
    }
    if(hands>2)throw std::runtime_error("Not enough free hands. Unequip the shield or two-handed weapon first.");
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
        in>>magic>>a.winds>>a.slots;if(magic=="SRD2"||magic=="SRD3")in>>a.slots2;
        in>>a.successes>>a.failures>>a.stable;
        if(!in||(magic!="SRD1"&&magic!="SRD2"&&magic!="SRD3"))throw std::runtime_error("Invalid character resource state");
        if(magic=="SRD3")a.effects=detail::read_effects(in);
        in>>std::ws;if(!in.eof())throw std::runtime_error("Trailing character resource state");
    }
    const auto& d=a.definition;
    if(a.hp<0||a.hp>d.hp||(a.dead&&a.hp!=0)||a.winds<0||a.winds>d.winds||a.slots<0||a.slots>d.slots||
        a.slots2<0||a.slots2>d.slots2||a.successes<0||a.successes>3||a.failures<0||a.failures>4)throw std::runtime_error("Invalid character vitals");
    // Earlier campaigns could retain completed counters after stabilization.
    if(a.stable)a.successes=a.failures=0;
}
VitalState vitals(const Actor& a)
{
    const bool effects=a.effects.next_id!=1;
    std::ostringstream out;out<<(effects?"SRD3 ":a.definition.slots2?"SRD2 ":"SRD1 ")<<a.winds<<' '<<a.slots<<' ';
    if(effects||a.definition.slots2)out<<a.slots2<<' ';out<<a.successes<<' '<<a.failures<<' '<<a.stable;
    if(effects){out<<' ';detail::write_effects(out,a.effects);}
    std::string description;
    if(a.definition.slots)description="Level-one spell slots: "+std::to_string(a.slots)+" / "+std::to_string(a.definition.slots);
    if(a.definition.slots2)description+="\nLevel-two spell slots: "+std::to_string(a.slots2)+" / "+std::to_string(a.definition.slots2);
    if(a.definition.winds)description="Second Wind uses: "+std::to_string(a.winds)+" / "+std::to_string(a.definition.winds);
    if(a.hp==0)description+=(description.empty()?"":"\n")+std::string(a.dead?"Dead":a.stable?"Stable, unconscious":"Unconscious; death saves ")+(!a.dead&&!a.stable?std::to_string(a.successes)+" successes, "+std::to_string(a.failures)+" failures":"");
    if(detail::blinded(a.effects))description+="\nBlinded";
    return {a.hp,a.dead,out.str(),description};
}
int distance(Cell a,Cell b) { return std::max(std::abs(a.x-b.x),std::abs(a.y-b.y))*5; }
bool same_command(const Command& a,const Command& b)
{ return a.revision==b.revision && a.actor==b.actor && a.target==b.target && a.verb==b.verb && a.destination==b.destination; }
bool turns_to_attack(std::string_view verb)
{
    return verb=="melee"||verb=="ranged"||verb=="fire_bolt"||verb=="magic_missile"||
        verb=="magic_missile_2"||verb=="scorching_ray"||verb=="blindness";
}
class Session final : public CombatSession {
public:
    Session(std::shared_ptr<const Content> content,Encounter encounter,std::uint64_t seed,bool restoring=false)
        : content_(std::move(content)), board_(std::move(encounter.battlefield)), rng_(seed), scope_(encounter.scope)
    {
        if(!scope_)throw std::runtime_error("Invalid encounter scope");
        detail::validate_battlefield(board_);
        if (encounter.participants.size()<2 || encounter.participants.size()>64) throw std::runtime_error("Invalid encounter size");
        std::set<EntityId> ids;std::set<Cell> cells;std::set<unsigned> sides;
        for (auto& p:encounter.participants) {
            if (!p.id || !ids.insert(p.id).second || (!cells.insert(p.cell).second&&!restoring) || board_.at(p.cell)==1 ||
                p.side>1 || p.name.empty() || p.name.size()>160 || (p.character_profile.empty()&&!content_->definitions.contains(p.definition)))
                throw std::runtime_error("Invalid participant or unsupported rules definition: "+p.definition);
            sides.insert(p.side);
            const auto d=p.character_profile.empty()?content_->definitions.at(p.definition):character_definition(p.character_profile);
            Actor a; a.definition=d;a.source=std::move(p);a.hp=d.hp;a.winds=d.winds;a.slots=d.slots;a.slots2=d.slots2;
            a.facing_left=a.source.facing_left;
            if(a.source.state)restore_vitals(a,*a.source.state);
            a.initiative=roll(20);if(d.str_dex_disadvantage||a.source.surprised)a.initiative=std::min(a.initiative,roll(20));a.initiative+=d.initiative;a.movement=d.speed;
            actors_.push_back(std::move(a));
        }
        if (sides.size()!=2) throw std::runtime_error("Encounter needs both sides");
        // Fixed tie adjudication: descending initiative, then stable entity ID.
        std::stable_sort(actors_.begin(),actors_.end(),[](const Actor& a,const Actor& b){
            return a.initiative!=b.initiative?a.initiative>b.initiative:a.source.id<b.source.id;
        });
        // Recovery checks follow the target's new initiative in a new encounter.
        // Exact timers in a restored encounter are installed after construction.
        for(std::size_t i=0;i<actors_.size();++i)
            for(auto& effect:actors_[i].effects.active)effect.save_in_ms=turn_end_ms(i);
        log("Combat begins. Each square is 5 feet.");update_outcome();
        if(!restoring&&outcome_==Outcome::ongoing&&!begin_turn())end_turn();
    }
    Snapshot snapshot() const override;
    std::vector<Command> legal_commands() const override;
    std::vector<Cell> movement_reach(EntityId actor) const override;
    bool submit(const Command& command) override;
    std::string save() const override;
    static std::unique_ptr<Session> restore(std::shared_ptr<const Content> content,std::string_view bytes);
private:
    std::shared_ptr<const Content> content_;
    Battlefield board_;
    std::vector<Actor> actors_;
    std::uint64_t rng_{}, revision_{1};
    std::uint64_t scope_{1}, elapsed_ms_{};
    unsigned turn_{}, round_{1};
    Outcome outcome_{Outcome::ongoing};
    std::vector<std::string> log_;
    std::vector<Message> log_messages_;
    std::vector<Cell> path_;
    std::size_t path_index_{};
    std::vector<EntityId> reactors_;
    std::size_t reactor_index_{};
    // A turn can provoke reactions after its attack, before the turn advances.
    // 0 = none, 1 = resume turn, 2 = end turn after reactions.
    unsigned turn_reaction_state_{};
    const Definition& def(const Actor& a) const {return a.definition;}
    Actor& actor(EntityId id) { return *std::find_if(actors_.begin(),actors_.end(),[&](const auto& a){return a.source.id==id;}); }
    int roll(int sides) {return roll_die(rng_,sides);}
    int dice(Dice d,bool critical=false) {int total=d.bonus;for(int i=0;i<d.count*(critical?2:1);++i)total+=roll(d.sides);return std::max(0,total);}
    void log(std::string english, Message message={}) {
        if(message.source.empty())message.source=english;
        if(log_.size()==80){log_.erase(log_.begin());log_messages_.erase(log_messages_.begin());}
        log_.push_back(std::move(english));log_messages_.push_back(std::move(message));
    }
    bool line_of_sight(Cell a,Cell b) const { return detail::has_line_of_sight(board_,a,b); }
    bool can_see(const Actor& a,const Actor& b) const {return !detail::blinded(a.effects)&&line_of_sight(a.source.cell,b.source.cell);}
    unsigned turn_end_ms(std::size_t index) const {return unsigned((index+1)*detail::round_ms/actors_.size());}
    unsigned next_save_ms(EntityId target) const;
    void advance_turn_time();
    void log_save(const Actor& target,const detail::SaveResult& result);
    detail::MovementGrid movement_grid(const Actor& mover) const;
    std::vector<Cell> path_to(const Actor& a,Cell destination) const;
    EntityId pending() const {return reactor_index_<reactors_.size()?reactors_[reactor_index_]:0;}
    void attack(Actor& a,Actor& target,bool ranged,bool spell=false,Dice spell_dice={1,10,0});
    void damage(Actor& target,int amount);
    void heal(Actor& target,int amount);
    void update_outcome();
    bool begin_turn();
    void end_turn();
    void progress_movement();
    void restore_movement(std::istream& input);
    void validate_restored_state() const;
    void validate_pending_movement() const;
    void validate_pending_turn_reaction() const;
    void restore_log(std::istream& input);
};

detail::MovementGrid Session::movement_grid(const Actor& mover) const
{
    std::vector<detail::Occupant> occupants;
    for (const auto& other : actors_) {
        // Unconscious actors still occupy space; corpses do not. The mover's
        // current cell is the path origin, not an obstacle.
        if (!other.dead && other.source.id != mover.source.id)
            occupants.push_back({other.source.cell, other.source.side != mover.source.side});
    }
    return {board_, mover.source.cell, occupants};
}

std::vector<Cell> Session::path_to(const Actor& actor, Cell destination) const
{
    return movement_grid(actor).reachable(actor.movement).path_to(destination);
}

Snapshot Session::snapshot() const
{
    Snapshot s;s.identity=content_->identity;s.revision=revision_;s.round=round_;s.outcome=outcome_;
    s.elapsed_milliseconds=elapsed_ms_;
    s.actor=pending()?pending():actors_[turn_].source.id;s.reaction_pending=pending()!=0;s.battlefield=board_;s.log=log_;s.log_messages=log_messages_;
    for(const auto& a:actors_) {
        std::string status=a.dead?"Dead":a.hp==0?(a.stable?"Stable, unconscious":"Unconscious"):a.dodge?"Dodging":"Ready";
        if(def(a).slots)status+=" | slots "+std::to_string(a.slots);
        if(def(a).slots2)status+=" | L2 slots "+std::to_string(a.slots2);
        if(def(a).winds)status+=" | Second Wind "+std::to_string(a.winds);
        s.combatants.push_back({a.source.id,a.source.name,a.source.definition,a.source.side,a.source.cell,
            a.hp,def(a).hp,def(a).ac,a.initiative,a.movement,a.action,a.bonus,a.reaction,a.hp>0&&!a.dead,a.dead,a.facing_left,status,vitals(a)});
        const auto display=combat_display(a.source.definition);
        auto& view=s.combatants.back();
        if(display.type)view.type_name=display.type;
        if(display.melee)view.melee_weapon=display.melee;
        if(display.ranged)view.ranged_weapon=display.ranged;
        view.ranged_attack_available=def(a).range>0;
        auto& messages=s.combatants.back().status_messages;
        messages.push_back({a.dead?"Dead":a.hp==0?(a.stable?"Stable, unconscious":"Unconscious"):a.dodge?"Dodging":"Ready",{}});
        if(def(a).slots)messages.push_back({"Spell slots: {count}",{{"count",std::to_string(a.slots)}}});
        if(def(a).slots2)messages.push_back({"L2 slots: {count}",{{"count",std::to_string(a.slots2)}}});
        if(def(a).winds)messages.push_back({"Second Wind: {count}",{{"count",std::to_string(a.winds)}}});
        if(detail::blinded(a.effects)){
            messages.push_back({"Blinded",{}});s.combatants.back().status+=" | Blinded";
            s.combatants.back().conditions.push_back({"Blinded",{}});
        }
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
        if(!other.dead&&other.source.side==a.source.side&&other.hp<def(other).hp&&distance(a.source.cell,other.source.cell)<=60&&can_see(a,other))
            spell("healing_word","Healing Word",other.source.id);
    if(a.action) {
        add(id,"dash","Dash");add(id,"dodge","Dodge");add(id,"disengage","Disengage");
        for(const auto& other:actors_) {
            if(other.dead || !line_of_sight(a.source.cell,other.source.cell))continue;
            const int feet=distance(a.source.cell,other.source.cell);
            if(other.source.side!=a.source.side && other.hp>0) {
                if(feet<=d.reach)add(id,"melee",a.source.definition=="slums-kobold"?"Dagger attack":
                    a.source.definition=="slums-kobold-leader"||a.source.definition=="slums-kobold-leader-sword"?"Short sword attack":"Melee attack",other.source.id);
                if(d.range>0&&feet<=d.long_range)add(id,"ranged",
                    a.source.definition=="slums-kobold-leader"?"Short bow attack":"Ranged attack",other.source.id);
                if((d.spells&1)&&feet<=120)add(id,"fire_bolt","Fire Bolt",other.source.id);
                if((d.spells&4)&&feet<=120&&can_see(a,other))spell("magic_missile","Magic Missile",other.source.id);
                if((d.spells&16)&&a.slots2>0&&!a.spent_slot&&feet<=120)add(id,"scorching_ray","Scorching Ray",other.source.id);
                if((d.spells&32)&&a.slots2>0&&!a.spent_slot&&feet<=120&&can_see(a,other)&&detail::can_apply(other.effects))
                    add(id,"blindness","Blindness",other.source.id);
            } else if(other.source.side==a.source.side && other.hp<def(other).hp && feet<=5 && (d.spells&2))
                spell("cure_wounds","Cure Wounds",other.source.id);
        }
    }
    for(const auto cell:movement_reach(id))add(id,"move","Move",0,cell);
    return commands;
}
std::vector<Cell> Session::movement_reach(EntityId id) const
{
    std::vector<Cell> cells;
    if(outcome_!=Outcome::ongoing||pending())return cells;
    const auto actor=std::find_if(actors_.begin(),actors_.end(),[&](const auto& a){return a.source.id==id;});
    if(actor==actors_.end()||actor->hp<=0||actor->dead||actor->movement<=0)return cells;
    const auto reachable=movement_grid(*actor).reachable(actor->movement);
    for(int y=0;y<board_.height;++y)for(int x=0;x<board_.width;++x)
        if(reachable.cost_to({x,y}))cells.push_back({x,y});
    return cells;
}
void Session::damage(Actor& target,int amount)
{
    const int remaining=amount-target.hp;target.hp=std::max(0,target.hp-amount);
    if(target.hp==0) {
        target.dodge=false;
        if(target.source.side==1 || remaining>=def(target).hp)target.dead=true;
        log(target.source.name+(target.dead?" is defeated.":" falls unconscious."),
            {target.dead?"{name} is defeated.":"{name} falls unconscious.",{{"name",target.source.name}}});
    }
}
void Session::heal(Actor& target,int amount)
{
    const int restored=std::min(amount,def(target).hp-target.hp);target.hp+=restored;
    target.successes=target.failures=0;target.stable=false;log(target.source.name+" recovers "+std::to_string(restored)+" HP.",
        {"{name} recovers {hp} HP.",{{"name",target.source.name},{"hp",std::to_string(restored)}}});
}
void Session::attack(Actor& a,Actor& target,bool ranged,bool spell,Dice spell_dice)
{
    if(target.source.cell.x!=a.source.cell.x)a.facing_left=target.source.cell.x<a.source.cell.x;
    const auto& d=def(a);bool disadvantaged=!spell&&d.str_dex_disadvantage;
    if(ranged) {
        if(!spell&&distance(a.source.cell,target.source.cell)>d.range)disadvantaged=true;
        for(const auto& other:actors_)if(other.source.side!=a.source.side&&other.hp>0&&distance(a.source.cell,other.source.cell)<=5&&can_see(other,a))disadvantaged=true;
    }
    const auto modifiers=detail::attack_modifiers(detail::blinded(a.effects),detail::blinded(target.effects),target.dodge,disadvantaged);
    const int natural=detail::d20(modifiers,rng_);
    const std::string modifier_label=modifiers.mode()<0?" (disadvantage)":modifiers.mode()>0?" (advantage)":"";
    const int bonus=spell?d.casting:ranged?d.ranged_bonus:d.melee_bonus;
    const bool hit=attack_hits(natural,bonus,def(target).ac);
    std::string message=a.source.name+" -> "+target.source.name+": d20 "+std::to_string(natural)+
        " + "+std::to_string(bonus)+" vs AC "+std::to_string(def(target).ac)+modifier_label;
    std::vector<MessageArgument> arguments{{"actor",a.source.name},{"target",target.source.name},{"roll",std::to_string(natural)},
        {"bonus",std::to_string(bonus)},{"ac",std::to_string(def(target).ac)},{"disadvantage",modifier_label,true}};
    if(!hit){log(message+" misses.",{"{actor} -> {target}: d20 {roll} + {bonus} vs AC {ac}{disadvantage} misses.",arguments});return;}
    const Dice damage_dice=spell?spell_dice:ranged?d.ranged:d.melee;
    int amount=dice(damage_dice,natural==20);
    bool savage=false;
    if(!spell&&damage_dice.count&&d.savage&&!a.savage_used){amount=std::max(amount,dice(damage_dice,natural==20));a.savage_used=true;savage=true;message+=" (Savage Attacker)";}
    arguments.push_back({"savage",savage?" (Savage Attacker)":"",true});
    arguments.push_back({"hit",natural==20?"CRITICAL":"hits",true});arguments.push_back({"damage",std::to_string(amount)});
    log(message+(natural==20?" CRITICAL":" hits")+" for "+std::to_string(amount)+" damage.",
        {"{actor} -> {target}: d20 {roll} + {bonus} vs AC {ac}{disadvantage}{savage} {hit} for {damage} damage.",arguments});damage(target,amount);
}
void Session::update_outcome()
{
    bool party=false,enemies=false;for(const auto& a:actors_)if(a.hp>0&&!a.dead)(a.source.side==0?party:enemies)=true;
    if(!party||!enemies) {
        outcome_=!party?Outcome::defeat:Outcome::victory;path_.clear();path_index_=0;reactors_.clear();reactor_index_=0;turn_reaction_state_=0;
        log(outcome_==Outcome::victory?"Victory.":"The party is incapacitated. Defeat.");
    }
}
bool Session::begin_turn()
{
    auto& a=actors_[turn_];
    if(a.dead)return false;
    if(a.hp==0){
        if(!a.stable){
            const int result=roll(20);
            log(a.source.name+" death save: "+std::to_string(result),
                {"{name} death save: {roll}",{{"name",a.source.name},{"roll",std::to_string(result)}}});
            if(result==20){a.hp=1;a.successes=a.failures=0;}
            else if(result>=10)++a.successes;
            else a.failures+=result==1?2:1;
            if(a.failures>=3)a.dead=true;
            else if(a.successes>=3){a.stable=true;a.successes=a.failures=0;}
        }
        if(a.hp==0)return false;
    }
    for(auto& actor:actors_)actor.savage_used=false;
    a.action=a.bonus=a.reaction=true;a.dodge=a.disengaged=false;a.movement=def(a).speed;
    a.spent_slot=false;
    log("Round "+std::to_string(round_)+": "+a.source.name+" acts.",{"Round {round}: {name} acts.",{{"round",std::to_string(round_)},{"name",a.source.name}}});
    return true;
}
unsigned Session::next_save_ms(EntityId target) const
{
    const auto found=std::find_if(actors_.begin(),actors_.end(),[&](const auto& a){return a.source.id==target;});
    const auto end=turn_end_ms(found-actors_.begin());
    const auto start=turn_?turn_end_ms(turn_-1):0;
    return end>start?end-start:detail::round_ms-start+end;
}
void Session::log_save(const Actor& target,const detail::SaveResult& result)
{
    const std::string outcome=result.success?"success":"failure";
    log(target.source.name+" Constitution save: d20 "+std::to_string(result.natural)+" + "+std::to_string(result.bonus)+
        " vs DC "+std::to_string(result.dc)+" ("+outcome+").",
        {"{name} Constitution save: d20 {roll} + {bonus} vs DC {dc} ({result}).",
         {{"name",target.source.name},{"roll",std::to_string(result.natural)},{"bonus",std::to_string(result.bonus)},
          {"dc",std::to_string(result.dc)},{"result",outcome,true}}});
}
void Session::advance_turn_time()
{
    // Partition one six-second round across its fixed initiative slots. Integer
    // boundaries telescope to exactly 6000 ms, even with 7 or 64 participants.
    // Dead/unconscious slots still pass time; menus and repeated snapshots don't.
    const unsigned delta=turn_end_ms(turn_)-(turn_?turn_end_ms(turn_-1):0);
    std::vector<detail::EffectSubject> subjects;
    for(auto& a:actors_)subjects.push_back({a.source.id,a.effects,def(a).saves,a.dead,def(a).str_dex_disadvantage,a.dodge});
    detail::elapse_effects(subjects,delta,rng_,[&](const detail::EffectEvent& event){
        const auto& target=actor(event.target);
        if(event.save)log_save(target,*event.save);
        if(event.removed)log(target.source.name+" recovers from a blindness effect.",
            {"{name} recovers from a blindness effect.",{{"name",target.source.name}}});
    });
    elapsed_ms_+=std::min<std::uint64_t>(delta,std::numeric_limits<std::uint64_t>::max()-elapsed_ms_);
}
void Session::end_turn()
{
    for(std::size_t checked=0;checked<=actors_.size()*2;++checked) {
        advance_turn_time();
        turn_=(turn_+1)%actors_.size();
        // The round is a display counter. Saturation avoids wrapping it to
        // zero while keeping an extremely long (or edited) combat playable.
        if(turn_==0 && round_<std::numeric_limits<unsigned>::max())++round_;
        if(begin_turn())return;
    }
    update_outcome();
}
void Session::progress_movement()
{
    auto& a=actors_[turn_];
    const auto grid = movement_grid(a);
    while(path_index_<path_.size()&&a.hp>0) {
        const auto destination=path_[path_index_];
        if(reactors_.empty()&&!a.disengaged)for(const auto& other:actors_)
            if(other.source.side!=a.source.side&&other.hp>0&&other.reaction&&
                distance(a.source.cell,other.source.cell)<=def(other).reach&&distance(destination,other.source.cell)>def(other).reach&&
                can_see(other,a))reactors_.push_back(other.source.id);
        if(pending())return;
        const auto cost = grid.step_cost(a.source.cell, destination);
        if (!cost || *cost > a.movement) throw std::logic_error("Invalid accepted movement path");
        a.movement -= *cost;
        a.source.cell = destination;
        ++path_index_;
        reactors_.clear();reactor_index_=0;
    }
    path_.clear();path_index_=0;reactors_.clear();reactor_index_=0;
}
bool Session::submit(const Command& command)
{
    const auto offered=legal_commands();
    if(std::none_of(offered.begin(),offered.end(),[&](const auto& c){return same_command(c,command);}))return false;
    auto& a=actor(command.actor);const auto& d=def(a);
    const bool attack_turn=command.verb=="melee"||command.verb=="ranged"||command.verb=="fire_bolt"||
        command.verb=="magic_missile"||command.verb=="magic_missile_2"||command.verb=="scorching_ray";
    std::vector<EntityId> turn_reactors;
    if(turns_to_attack(command.verb)&&command.target){
        const auto& target=actor(command.target);
        if(target.source.cell.x!=a.source.cell.x){
            const bool new_left=target.source.cell.x<a.source.cell.x;
            if(new_left!=a.facing_left){
                const bool old_left=a.facing_left;
                a.facing_left=new_left;
                log(a.source.name+(new_left?" turns left.":" turns right."));
                for(const auto& other:actors_)
                    if(other.source.side!=a.source.side&&other.hp>0&&other.reaction&&
                        (old_left?other.source.cell.x<a.source.cell.x:other.source.cell.x>a.source.cell.x)&&
                        distance(other.source.cell,a.source.cell)<=def(other).reach&&can_see(other,a))
                        turn_reactors.push_back(other.source.id);
            }
        }
    }
    const bool second=command.verb.ends_with("_2");
    const auto spend=[&]{if(second||command.verb=="scorching_ray"||command.verb=="blindness")--a.slots2;else --a.slots;a.spent_slot=true;};
    if(command.verb=="opportunity"||command.verb=="decline") {
        if(command.verb=="opportunity"){a.reaction=false;attack(a,actor(command.target),false);}
        ++reactor_index_;update_outcome();
        if(outcome_==Outcome::ongoing&&turn_reaction_state_&&actors_[turn_].hp==0){
            turn_reaction_state_=0;reactors_.clear();reactor_index_=0;end_turn();
        }else if(outcome_==Outcome::ongoing&&!pending()){
            if(turn_reaction_state_){
                const bool end_after=turn_reaction_state_==2;
                turn_reaction_state_=0;reactors_.clear();reactor_index_=0;
                if(end_after)end_turn();
            }else progress_movement();
        }
    } else if(command.verb=="move") {
        path_=path_to(a,command.destination);path_index_=0;progress_movement();
    } else if(command.verb=="end")end_turn();
    else if(command.verb=="second_wind") {a.bonus=false;--a.winds;heal(a,roll(10)+d.level);}
    else if(command.verb=="healing_word"||command.verb=="healing_word_2"){a.bonus=false;spend();heal(actor(command.target),dice({second?4:2,4,d.casting-2}));}
    else {
        a.action=false;
        if(command.verb=="dash"){a.movement+=d.speed;log(a.source.name+" dashes.",{"{name} dashes.",{{"name",a.source.name}}});}
        else if(command.verb=="dodge"){a.dodge=true;log(a.source.name+" dodges.",{"{name} dodges.",{{"name",a.source.name}}});}
        else if(command.verb=="disengage"){a.disengaged=true;log(a.source.name+" disengages.",{"{name} disengages.",{{"name",a.source.name}}});}
        else if(command.verb=="cure_wounds"||command.verb=="cure_wounds_2"){spend();heal(actor(command.target),dice({second?4:2,8,d.casting-2}));}
        else if(command.verb=="magic_missile"||command.verb=="magic_missile_2") {
            spend();int total=0;for(int dart=0;dart<(second?4:3);++dart)total+=roll(4)+1;
            log(a.source.name+" casts Magic Missile for "+std::to_string(total)+" force damage.",
                {"{name} casts Magic Missile for {damage} force damage.",{{"name",a.source.name},{"damage",std::to_string(total)}}});damage(actor(command.target),total);
        } else if(command.verb=="blindness"){
            spend();auto& target=actor(command.target);
            const auto result=detail::saving_throw(detail::Ability::constitution,def(target).saves[2],8+d.casting,detail::saving_modifiers(detail::Ability::constitution,def(target).str_dex_disadvantage,target.dodge),rng_);
            log_save(target,result);
            if(!result.success){
                detail::apply_blindness(target.effects,scope_,a.source.id,a.source.name,result.dc,next_save_ms(target.source.id));
                log(target.source.name+" is Blinded.",{"{name} is Blinded.",{{"name",target.source.name}}});
            }
        } else if(command.verb=="scorching_ray"){
            spend();for(unsigned ray=0;ray<3&&actor(command.target).hp>0;++ray)attack(a,actor(command.target),true,true,{2,6,0});
        }else attack(a,actor(command.target),command.verb!="melee",command.verb=="fire_bolt");
    }
    if(!turn_reactors.empty()&&outcome_==Outcome::ongoing){
        reactors_=std::move(turn_reactors);reactor_index_=0;
        turn_reaction_state_=attack_turn?2:1;
    }
    // Revisions are command tickets; zero is reserved for invalid commands.
    // Unsigned wrap is defined, but must skip that reserved value.
    if (++revision_ == 0) revision_ = 1;
    update_outcome();
    if(outcome_==Outcome::ongoing&&!pending()&&(attack_turn||actors_[turn_].hp==0))end_turn();
    if(outcome_!=Outcome::ongoing)advance_turn_time();
    return true;
}

std::string Session::save() const
{
    // The module owns the checkpoint format, including RNG and pending reactions.
    std::ostringstream out;out<<"OGCOMBAT 5 "<<std::quoted(content_->identity.module)<<' '<<std::quoted(content_->identity.version)<<' '<<std::quoted(content_->identity.content)<<'\n';
    out<<board_.width<<' '<<board_.height<<'\n';for(auto cell:board_.terrain)out<<unsigned(cell)<<' ';out<<'\n';
    out<<rng_<<' '<<revision_<<' '<<turn_<<' '<<round_<<' '<<static_cast<int>(outcome_)<<' '<<actors_.size()<<'\n';
    for(const auto& a:actors_)out<<a.source.id<<' '<<std::quoted(a.source.definition)<<' '<<std::quoted(a.source.name)<<' '<<a.source.side<<' '<<a.source.cell.x<<' '<<a.source.cell.y<<' '
        <<a.hp<<' '<<a.initiative<<' '<<a.movement<<' '<<a.winds<<' '<<a.slots<<' '<<a.successes<<' '<<a.failures<<' '
        <<a.action<<' '<<a.bonus<<' '<<a.reaction<<' '<<a.dodge<<' '<<a.disengaged<<' '<<a.stable<<' '<<a.dead<<' '<<std::quoted(a.source.character_profile)<<' '<<a.slots2<<' '<<a.spent_slot<<' '<<a.savage_used<<' '<<a.facing_left<<'\n';
    out<<path_.size()<<' '<<path_index_<<'\n';for(auto p:path_)out<<p.x<<' '<<p.y<<' ';out<<'\n';
    out<<reactors_.size()<<' '<<reactor_index_<<'\n';for(auto id:reactors_)out<<id<<' ';out<<'\n';
    out<<log_.size()<<'\n';for(const auto& line:log_)out<<std::quoted(line)<<'\n';
    out<<scope_<<' '<<elapsed_ms_<<' '<<actors_.size()<<'\n';
    for(const auto& a:actors_){detail::write_effects(out,a.effects);out<<'\n';}
    out<<turn_reaction_state_<<'\n';
    return out.str();
}
// Parse one actor independently of session mutation. Old checkpoint versions
// omit later fields; Actor's value initializers supply their original defaults.
Actor read_checkpoint_actor(std::istream& input, unsigned version, const Content& content)
{
    Actor actor;
    auto& source = actor.source;
    input >> source.id >> std::quoted(source.definition) >> std::quoted(source.name)
          >> source.side >> source.cell.x >> source.cell.y
          >> actor.hp >> actor.initiative >> actor.movement >> actor.winds >> actor.slots
          >> actor.successes >> actor.failures >> actor.action >> actor.bonus >> actor.reaction
          >> actor.dodge >> actor.disengaged >> actor.stable >> actor.dead;
    if (version >= 2) input >> std::quoted(source.character_profile);
    if (version >= 3) input >> actor.slots2 >> actor.spent_slot >> actor.savage_used;
    if (version >= 5) input >> actor.facing_left;
    if (!input || (source.character_profile.empty() && !content.definitions.contains(source.definition)))
        throw std::runtime_error("Invalid checkpoint actor");
    actor.definition = source.character_profile.empty()
        ? content.definitions.at(source.definition) : character_definition(source.character_profile);
    const auto& definition = actor.definition;
    if (actor.hp < 0 || actor.hp > definition.hp || (actor.dead && actor.hp > 0) ||
        // Dash spends the action before adding a second movement allowance.
        // Accepting both extra movement and an unused action lets a later Dash
        // create a state outside the checkpoint's own movement bounds.
        actor.movement < 0 || actor.movement > definition.speed*(actor.action ? 1 : 2) ||
        actor.winds < 0 || actor.winds > definition.winds ||
        actor.slots < 0 || actor.slots > definition.slots ||
        actor.slots2 < 0 || actor.slots2 > definition.slots2 ||
        actor.successes < 0 || actor.successes > 3 || actor.failures < 0 || actor.failures > 4)
        throw std::runtime_error("Invalid checkpoint actor state");
    return actor;
}

Battlefield read_checkpoint_board(std::istream& input)
{
    Battlefield board;
    input >> board.width >> board.height;
    // Bound dimensions before multiplication or allocation, even for truncated
    // input. No serialized count is allowed to control an unbounded allocation.
    if (!input || board.width < 2 || board.height < 2 || board.width > 64 || board.height > 64)
        throw std::runtime_error("Invalid checkpoint board");
    for (int i = 0; i < board.width*board.height; ++i) {
        unsigned terrain{};
        input >> terrain;
        if (!input || terrain > 2) throw std::runtime_error("Invalid checkpoint terrain");
        board.terrain.push_back(static_cast<std::uint8_t>(terrain));
    }
    return board;
}

void Session::restore_movement(std::istream& input)
{
    std::size_t count{};
    input >> count >> path_index_;
    if (!input || count > 1024 || path_index_ > count)
        throw std::runtime_error("Invalid checkpoint path");
    for (std::size_t i = 0; i < count; ++i) {
        Cell cell;
        input >> cell.x >> cell.y;
        if (!input || board_.at(cell) == 1) throw std::runtime_error("Invalid checkpoint path cell");
        path_.push_back(cell);
    }
    input >> count >> reactor_index_;
    if (!input || count > 64 || reactor_index_ > count)
        throw std::runtime_error("Invalid checkpoint reactions");
    std::set<EntityId> seen;
    for (std::size_t i = 0; i < count; ++i) {
        EntityId id{};
        input >> id;
        const auto exists = std::any_of(actors_.begin(), actors_.end(),
                                       [id](const auto& actor) { return actor.source.id == id; });
        if (!input || !seen.insert(id).second || !exists)
            throw std::runtime_error("Invalid checkpoint reactor");
        reactors_.push_back(id);
    }
}

void Session::validate_restored_state() const
{
    if (pending() && !turn_reaction_state_ && path_index_ >= path_.size())
        throw std::runtime_error("Reaction without movement");
    const auto& mover = actors_[turn_];
    bool party = false, enemies = false;
    for (const auto& actor : actors_) {
        if (actor.hp <= 0 || actor.dead) continue;
        (actor.source.side == 0 ? party : enemies) = true;
        for (const auto& other : actors_) {
            if (other.source.id <= actor.source.id || other.hp <= 0 || actor.source.cell != other.source.cell)
                continue;
            throw std::runtime_error("Overlapping active checkpoint actors");
        }
    }
    const auto expected = !party ? Outcome::defeat : !enemies ? Outcome::victory : Outcome::ongoing;
    if (outcome_ != expected || (expected == Outcome::ongoing && mover.hp == 0))
        throw std::runtime_error("Invalid checkpoint outcome/turn");
    if (!pending() && (!path_.empty() || !reactors_.empty() || turn_reaction_state_))
        throw std::runtime_error("Unpaused checkpoint movement");
    if (turn_reaction_state_) validate_pending_turn_reaction();
    else if (pending()) validate_pending_movement();
}

void Session::validate_pending_turn_reaction() const
{
    const auto& attacker=actors_[turn_];
    if(!pending()||!path_.empty()||path_index_||attacker.hp<=0||attacker.action)
        throw std::runtime_error("Invalid pending turn reaction");
    for(auto i=reactor_index_;i<reactors_.size();++i){
        const auto found=std::find_if(actors_.begin(),actors_.end(),[&](const auto& a){return a.source.id==reactors_[i];});
        const auto& reactor=*found;
        if(reactor.hp<=0||!reactor.reaction||reactor.source.side==attacker.source.side||
            (attacker.facing_left?reactor.source.cell.x<=attacker.source.cell.x:reactor.source.cell.x>=attacker.source.cell.x)||
            distance(reactor.source.cell,attacker.source.cell)>def(reactor).reach||!can_see(reactor,attacker))
            throw std::runtime_error("Invalid pending turn opportunity attack");
    }
}

void Session::validate_pending_movement() const
{
    const auto& mover = actors_[turn_];
    if (outcome_ != Outcome::ongoing || mover.disengaged)
        throw std::runtime_error("Invalid pending movement");
    const auto grid = movement_grid(mover);
    auto cell = mover.source.cell;
    int remaining = mover.movement;
    // Only the suffix remains to be travelled. The prefix is history and has
    // already spent its movement budget; charging for it again breaks restores.
    for (auto i = path_index_; i < path_.size(); ++i) {
        const auto next = path_[i];
        const auto cost = grid.step_cost(cell, next);
        if (!cost || *cost > remaining) throw std::runtime_error("Invalid checkpoint movement step/budget");
        remaining -= *cost;
        cell = next;
    }
    if (!grid.can_stop_at(cell)) throw std::runtime_error("Invalid checkpoint movement destination");
    for (auto i = reactor_index_; i < reactors_.size(); ++i) {
        const auto reactor = std::find_if(actors_.begin(), actors_.end(),
            [&](const auto& actor) { return actor.source.id == reactors_[i]; });
        // restore_movement already established that every reactor ID exists.
        const auto& actor = *reactor;
        if (actor.hp == 0 || !actor.reaction || actor.source.side == mover.source.side ||
            distance(actor.source.cell, mover.source.cell) > def(actor).reach ||
            distance(actor.source.cell, path_[path_index_]) <= def(actor).reach ||
            !can_see(actor,mover))
            throw std::runtime_error("Invalid checkpoint opportunity attack");
    }
}

void Session::restore_log(std::istream& input)
{
    std::size_t count{};
    input >> count;
    if (!input || count > 80) throw std::runtime_error("Invalid checkpoint log");
    log_.clear();
    log_messages_.clear();
    for (std::size_t i = 0; i < count; ++i) {
        std::string line;
        input >> std::quoted(line);
        if (!input || line.size() > 1000) throw std::runtime_error("Invalid checkpoint log line");
        log_messages_.push_back({line,{}});
        log_.push_back(std::move(line));
    }
}

std::unique_ptr<Session> Session::restore(std::shared_ptr<const Content> content, std::string_view bytes)
{
    if (bytes.size() > 4*1024*1024) throw std::runtime_error("Combat checkpoint exceeds limit");
    std::istringstream input{std::string(bytes)};
    std::string magic;
    unsigned version{};
    Identity identity;
    input >> magic >> version >> std::quoted(identity.module)
          >> std::quoted(identity.version) >> std::quoted(identity.content);
    if (!input || magic != "OGCOMBAT" || version < 1 || version > 5 || identity != content->identity)
        throw std::runtime_error("Combat checkpoint rules/content version mismatch");
    Encounter encounter;
    encounter.battlefield = read_checkpoint_board(input);
    std::uint64_t rng{}, revision{};
    unsigned turn{}, round{}, outcome{}, count{};
    input >> rng >> revision >> turn >> round >> outcome >> count;
    if (!input || count < 2 || count > 64 || turn >= count || round == 0 || outcome > 2 || !revision)
        throw std::runtime_error("Invalid checkpoint header");
    std::vector<Actor> actors;
    for (unsigned i = 0; i < count; ++i) {
        auto actor = read_checkpoint_actor(input, version, *content);
        encounter.participants.push_back(actor.source);
        actors.push_back(std::move(actor));
    }
    // Build and validate a separate owned candidate. Any failure destroys it;
    // callers never receive a partially restored session or lose a live one.
    // The constructor checks identities/geometry; saved order and RNG then
    // replace its fresh initiative state before relational checks run.
    auto session = std::make_unique<Session>(content, std::move(encounter), 0, true);
    session->actors_ = std::move(actors);
    session->rng_ = rng;
    session->revision_ = revision;
    session->turn_ = turn;
    session->round_ = round;
    session->outcome_ = static_cast<Outcome>(outcome);
    session->restore_movement(input);
    session->restore_log(input);
    if(version>=4){
        unsigned effects_count{};input>>session->scope_>>session->elapsed_ms_>>effects_count;
        if(!input||!session->scope_||effects_count!=session->actors_.size())throw std::runtime_error("Invalid checkpoint effect header");
        for(auto& a:session->actors_)a.effects=detail::read_effects(input);
    }
    if(version>=5){
        input>>session->turn_reaction_state_;
        if(!input||session->turn_reaction_state_>2)throw std::runtime_error("Invalid checkpoint turn reaction");
    }
    session->validate_restored_state();
    input >> std::ws;
    if (!input.eof()) throw std::runtime_error("Trailing checkpoint data");
    return session;
}
class Module final : public RulesModule {
public:
    explicit Module(Content content):content_(std::make_shared<const Content>(std::move(content))){}
    Identity identity() const override{return content_->identity;}
    bool accepts_campaign_identity(const Identity& saved) const override {
        if(saved.version!=content_->identity.version&&saved.version!="0.3.0"&&saved.version!="0.4.0"&&saved.version!="0.5.0"&&saved.version!="0.6.0"&&saved.version!="0.6.1"&&saved.version!="0.6.2")return false;
        auto compatible=saved;compatible.version=content_->identity.version;
        return compatible==content_->identity||std::find(content_->previous_campaign_identities.begin(),content_->previous_campaign_identities.end(),compatible)!=content_->previous_campaign_identities.end();
    }
    std::vector<std::string> supported_features() const override{return {"initiative","movement","melee","ranged","critical_hits","dodge","dash","disengage","opportunity_attacks","facing","turn_opportunity_attacks","death_saves","second_wind","fire_bolt","cure_wounds","magic_missile","healing_word","scorching_ray","level_two_slots","manual_advancement","ability_score_improvement","defense","savage_attacker","saving_throws","blinded","blindness","timed_effects","checkpoint"};}
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
            {"blindness","Blindness","Blindness/Deafness (blindness option): Constitution save; repeat at end of turn; up to 1 minute. Level 2 slot.",result.level>=3},
            {"bless","Bless","Unavailable: concentration is not implemented.",false}};
        if(sheet.character_class=="Wizard")result.spells={
            {"magic_missile","Magic Missile","Action; 120 feet; three darts at one target."},
            {"scorching_ray","Scorching Ray","Action; 120 feet; three spell attacks at one target. Requires level 3.",result.level>=3},
            {"blindness","Blindness","Blindness/Deafness (blindness option): Constitution save; repeat at end of turn; up to 1 minute. Level 2 slot.",result.level>=3},
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
        next.hit_point_modifiers.push_back(next.modifiers[2]);
        next.hit_points=maximum_hit_points(next.hit_die,next.race=="Dwarf",next.hit_point_modifiers);
        const int growth=next.hit_points-sheet.hit_points;
        next.hp_explanation="Level "+std::to_string(next.level)+": "+std::to_string(next.hit_points)+" maximum HP; gain "+std::to_string(growth)+". Fixed-average Hit Die growth includes Constitution and any retroactive Constitution increase.";
        next.hp_messages={{"Level {level}: {hp} maximum HP; gain {growth}. Fixed-average Hit Die growth includes Constitution and any retroactive Constitution increase.",
            {{"level",std::to_string(next.level)},{"hp",std::to_string(next.hit_points)},{"growth",std::to_string(growth)}}}};
        next.class_modifiers+="\nLevel "+std::to_string(next.level)+": HP and spell-slot advancement applied. Additional class and subclass features remain unavailable.";
        next.class_messages.push_back({"Level {level}: HP and spell-slot advancement applied. Additional class and subclass features remain unavailable.",{{"level",std::to_string(next.level)}}});
        actor.definition=character_definition(character_profile(next,{}).data);
        if(actor.hp>0)actor.hp+=growth;
        actor.slots+=actor.definition.slots-old.slots;actor.slots2+=actor.definition.slots2-old.slots2;actor.winds+=actor.definition.winds-old.winds;
        auto continuation=vitals(actor);sheet=std::move(next);state=std::move(continuation);
        return true;
    }
    void validate_character_state(const CharacterSheet& sheet,const VitalState& state) const override
    {
        Actor actor;actor.definition=character_definition(character_profile(sheet,{}).data);
        actor.winds=actor.definition.winds;actor.slots=actor.definition.slots;actor.slots2=actor.definition.slots2;restore_vitals(actor,state);
    }
    void migrate_character_state(const Identity& saved,const CharacterSheet& sheet,VitalState& state) const override
    {
        if(!accepts_campaign_identity(saved))throw std::runtime_error("Unsupported campaign migration");
        auto definition=character_definition(character_profile(sheet,{}).data);
        const bool old_hp=saved.version=="0.3.0"||saved.version=="0.4.0"||saved.version=="0.5.0"||
            saved.version=="0.6.0"||saved.version=="0.6.1"||saved.version=="0.6.2";
        if(old_hp){
            // Prior modules rebuilt every gain with the final modifier. Validate
            // against that old maximum before restoring the missing HP to a
            // conscious member; zero HP/death and all resources remain intact.
            const int con=ability_modifier(sheet.scores[2]);
            definition.hp=sheet.hit_die+con+(sheet.race=="Dwarf"?sheet.level:0)+
                (sheet.level-1)*std::max(1,sheet.hit_die/2+1+con);
        }
        Actor actor;actor.definition=definition;actor.winds=definition.winds;
        actor.slots=definition.slots;actor.slots2=definition.slots2;restore_vitals(actor,state);
        auto next=state;if(next.hit_points>0)next.hit_points+=sheet.hit_points-definition.hp;
        validate_character_state(sheet,next);state=std::move(next);
    }
    RestPolicy long_rest_policy() const override {return {480,960};}
    void elapse(std::span<Participant> participants,std::uint64_t milliseconds,std::uint64_t& random_state) const override
    {
        // Work on owned candidates so malformed state cannot partly advance a
        // party or consume its RNG. No Godot or campaign data enters the rules.
        std::vector<Actor> actors;actors.reserve(participants.size());
        for(const auto& p:participants){
            Actor a;a.source=p;
            a.definition=p.character_profile.empty()?content_->definitions.at(p.definition):character_definition(p.character_profile);
            a.hp=a.definition.hp;a.winds=a.definition.winds;a.slots=a.definition.slots;a.slots2=a.definition.slots2;
            if(p.state)restore_vitals(a,*p.state);
            actors.push_back(std::move(a));
        }
        std::vector<detail::EffectSubject> subjects;
        for(auto& a:actors)subjects.push_back({a.source.id,a.effects,a.definition.saves,a.dead,a.definition.str_dex_disadvantage,false});
        auto rng=random_state;detail::elapse_effects(subjects,milliseconds,rng);
        std::vector<VitalState> next;next.reserve(actors.size());
        for(const auto& a:actors)next.push_back(vitals(a));
        for(std::size_t i=0;i<participants.size();++i)
            if(participants[i].state&&participants[i].state->resources.starts_with("SRD3 "))participants[i].state=std::move(next[i]);
        random_state=rng;
    }
    void recover(VitalState& state,const CharacterSheet& sheet) const override
    {
        const auto d=character_definition(character_profile(sheet,{}).data);
        Actor actor;actor.definition=d;actor.winds=d.winds;actor.slots=d.slots;actor.slots2=d.slots2;restore_vitals(actor,state);
        if(actor.dead||actor.hp<1)throw std::runtime_error("Long rest requires at least one HP at its start");
        actor.hp=d.hp;actor.winds=d.winds;actor.slots=d.slots;actor.slots2=d.slots2;actor.successes=actor.failures=0;actor.stable=false;
        state=vitals(actor);
    }
    void temple_heal(VitalState& state,const CharacterSheet& sheet,std::uint64_t& random_state) const override
    {
        const auto d=character_definition(character_profile(sheet,{}).data);
        Actor actor;actor.definition=d;actor.winds=d.winds;actor.slots=d.slots;actor.slots2=d.slots2;restore_vitals(actor,state);
        if(actor.dead||actor.hp>=d.hp)throw std::runtime_error("Cure Wounds requires a wounded living member");
        // Authored temple caster: Cure Wounds, Wisdom +3. Same SplitMix64 as combat.
        auto rng=random_state;int amount=3;
        for(int i=0;i<2;++i)amount+=roll_die(rng,8);
        actor.hp=std::min(d.hp,actor.hp+amount);actor.successes=actor.failures=0;actor.stable=false;
        auto next=vitals(actor);state=std::move(next);random_state=rng;
    }
    EquipmentInfo equipment_info(std::string_view key) const override {
        if(const auto* item=detail::weapon(key))return {EquipmentSlot::weapon,item->hands};
        if(key=="shield")return {EquipmentSlot::shield,1};
        if(key=="leather"||key=="chain_mail")return {EquipmentSlot::armor,0};
        return {};
    }
    CharacterProfile character_profile(const CharacterSheet& sheet,std::span<const std::string> gear) const override {
        if(sheet.identity!=character_rules()->identity()||sheet.level<1||sheet.level>4)throw std::runtime_error("Unsupported character rules identity or level");
        unsigned features=sheet.background=="Soldier"?2:0;
        if(sheet.feats.size()!=(sheet.level==4?1u:0u))throw std::runtime_error("Invalid advancement feat count");
        if(sheet.hit_point_modifiers.size()!=sheet.level||
            (sheet.level==4&&sheet.feats.front()!="ability_score_improvement"&&
                sheet.hit_point_modifiers.front()!=sheet.hit_point_modifiers.back()))
            throw std::runtime_error("HP history does not match character advancement");
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
            else if(spell=="blindness"&&(sheet.character_class=="Wizard"||sheet.character_class=="Cleric")&&sheet.level>=3)spells|=32;
            else throw std::runtime_error("Unsupported prepared spell");
        }
        std::ostringstream out;out<<"PC4 "<<sheet.level<<' '<<features<<' '<<spells<<' '<<std::quoted(sheet.character_class)<<' '<<std::quoted(sheet.race);
        for(auto score:sheet.scores)out<<' '<<score;
        for(auto modifier:sheet.hit_point_modifiers)out<<' '<<modifier;
        out<<' '<<gear.size();for(const auto& item:gear)out<<' '<<std::quoted(item);
        const auto data=out.str();const auto d=character_definition(data);
        if(d.hp!=sheet.hit_points)throw std::runtime_error("Character HP does not match rules profile");
        CharacterProfile result{data,d.hp,d.ac,"Level 1-4 subset: HP, selected feats, supported prepared spells and level-one/two slots. Additional class/subclass and species features remain unavailable.",d.speed,d.melee_bonus};
        result.strength_dexterity_disadvantage=d.str_dex_disadvantage;
        for(const auto& key:gear){
            if(key=="shield")result.item_modifiers+=trained(sheet.character_class,key)?"Source: equipped Shield: +2 AC.\n":"Source: equipped Shield: +0 AC (untrained).\n";
            else if(key=="leather")result.item_modifiers+="Source: equipped Leather armor and Dexterity score "+std::to_string(sheet.scores[1])+". AC becomes 11 + Dexterity modifier ("+std::to_string(sheet.modifiers[1])+").\n";
            else if(key=="chain_mail")result.item_modifiers+="Source: equipped Chain mail. AC becomes 16; speed -10 feet below Strength 13 (current Strength "+std::to_string(sheet.scores[0])+").\n";
            else if(key=="wand")result.item_modifiers+="Source: equipped Wand. Held focus; melee uses unarmed strike.\n";
            else result.item_modifiers+="Source: equipped "+key+" and "+sheet.character_class+" weapon proficiency. Weapon attack uses "+attack_ability(key)+" modifier"+(trained(sheet.character_class,key)?" +2 class proficiency":" without proficiency")+"; damage adds that ability modifier.\n";
        }
        for(const auto& key:gear)result.item_modifiers+="Source: equipped "+key+". "+equipment_note(sheet,key)+"\n";
        if(features&1)result.item_modifiers+="Defense feat: +1 AC while wearing armor.\n";
        if(features&2)result.item_modifiers+="Savage Attacker: higher of two weapon-damage rolls on the first weapon hit each turn.\n";
        if(gear.empty())result.item_modifiers="No equipment modifiers. Source: unarmed strike rules and Strength score "+std::to_string(sheet.scores[0])+". Attack uses Strength modifier +2 level-one proficiency; damage is 1 + Strength modifier (minimum 0).";
        result.spell_modifiers="Active conditions are shown in the character status.";
        if(sheet.character_class=="Wizard")result.spell_modifiers="Source: Fire Bolt and Wizard spellcasting, Intelligence score "+std::to_string(sheet.scores[3])+". Attack: Intelligence modifier +2 level-one proficiency = "+std::to_string(d.casting)+". Magic Missile has no ability modifier to damage.\n"+result.spell_modifiers;
        if(sheet.character_class=="Cleric")result.spell_modifiers="Source: Cure Wounds and Cleric spellcasting, Wisdom score "+std::to_string(sheet.scores[4])+". Healing: 2d8 + Wisdom modifier ("+std::to_string(d.casting-2)+").\n"+result.spell_modifiers;
        if(d.str_dex_disadvantage)result.spell_modifiers="Cannot cast spells while wearing untrained armor.\n"+result.spell_modifiers;
        // Language-independent presentation of the same computed rule results.
        for(const auto& key:gear){
            if(key=="shield")result.item_messages.push_back({trained(sheet.character_class,key)?"Source: equipped Shield: +2 AC.":"Source: equipped Shield: +0 AC (untrained).",{}});
            else if(key=="leather")result.item_messages.push_back({"Source: equipped Leather armor and Dexterity score {score}. AC becomes 11 + Dexterity modifier ({modifier}).",{{"score",std::to_string(sheet.scores[1])},{"modifier",std::to_string(sheet.modifiers[1])}}});
            else if(key=="chain_mail")result.item_messages.push_back({"Source: equipped Chain mail. AC becomes 16; speed -10 feet below Strength 13 (current Strength {score}).",{{"score",std::to_string(sheet.scores[0])}}});
            else if(key=="wand")result.item_messages.push_back({"Source: equipped Wand. Held focus; melee uses unarmed strike.",{}});
            else result.item_messages.push_back({"Source: equipped {item} and {class} weapon proficiency. Weapon attack uses {ability} modifier {proficiency}; damage adds that ability modifier.",
                {{"item",key,true},{"class",sheet.character_class,true},{"ability",attack_ability(key),true},
                 {"proficiency",trained(sheet.character_class,key)?"+2 class proficiency":"without proficiency",true}}});
            result.item_messages.push_back({equipment_note(sheet,key),{}});
        }
        if(features&1)result.item_messages.push_back({"Defense feat: +1 AC while wearing armor.",{}});
        if(features&2)result.item_messages.push_back({"Savage Attacker: higher of two weapon-damage rolls on the first weapon hit each turn.",{}});
        if(gear.empty())result.item_messages.push_back({"No equipment modifiers. Source: unarmed strike rules and Strength score {score}. Attack uses Strength modifier +2 level-one proficiency; damage is 1 + Strength modifier (minimum 0).",{{"score",std::to_string(sheet.scores[0])}}});
        if(d.str_dex_disadvantage)result.spell_messages.push_back({"Cannot cast spells while wearing untrained armor.",{}});
        if(sheet.character_class=="Wizard")result.spell_messages.push_back({"Source: Fire Bolt and Wizard spellcasting, Intelligence score {score}. Attack: Intelligence modifier +2 level-one proficiency = {attack}. Magic Missile has no ability modifier to damage.",{{"score",std::to_string(sheet.scores[3])},{"attack",std::to_string(d.casting)}}});
        if(sheet.character_class=="Cleric")result.spell_messages.push_back({"Source: Cure Wounds and Cleric spellcasting, Wisdom score {score}. Healing: 2d8 + Wisdom modifier ({modifier}).",{{"score",std::to_string(sheet.scores[4])},{"modifier",std::to_string(d.casting-2)}}});
        result.spell_messages.push_back({"Active conditions are shown in the character status.",{}});
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
    return parse_content(bytes);
}
std::unique_ptr<RulesModule> parse_content(std::string_view content_bytes)
{
    if(content_bytes.size()>65536)throw std::runtime_error("Rules content exceeds size limit");
    std::string bytes(content_bytes);
    // Git may translate line endings; identical content must keep its identity.
    bytes.erase(std::remove(bytes.begin(),bytes.end(),'\r'),bytes.end());
    std::uint64_t hash=14695981039346656037ULL;for(unsigned char c:bytes){hash^=c;hash*=1099511628211ULL;}
    std::istringstream lines(bytes);std::string line,magic,revision;unsigned version;
    std::getline(lines,line);std::istringstream header(line);header>>magic>>version>>revision;
    if(!header||magic!="OPENGOLD_SRD5"||version!=1)throw std::runtime_error("Unsupported rules content format");
    header>>std::ws;
    if(!header.eof()||revision.empty()||revision.size()>80)throw std::runtime_error("Invalid rules content header");
    Content content;content.identity={"opengold.srd5","0.6.3",revision+"/"+std::to_string(hash)};
    // Preserve campaign saves from the preceding pack and the frozen v1/v2 fixtures.
    if(revision=="srd-5.2.1-demo.1")for(const auto fingerprint:
        {"15286736505479635800","1436083463150607054","4820123901484423331"})
        content.previous_campaign_identities.push_back({"opengold.srd5",content.identity.version,revision+"/"+fingerprint});
    // These additive rows introduce saves and an isolated casting fixture. Old
    // campaign sheets can migrate; combat checkpoints still require exact rules.
    // Reconstruct both supported historical packs without guessing fingerprints.
    for(bool remove_roaming:{false,true}){
        std::istringstream previous_lines(bytes);std::string previous,line_before;
        const std::array<std::string_view,7> additions{"slums-kobold","slums-goblin","slums-kobold-leader","slums-kobold-leader-sword","slums-goblin-leader","slums-orc-leader","slums-bugbear"};
        while(std::getline(previous_lines,line_before)){
            std::istringstream row(line_before);std::string tag,key;row>>tag>>key;
            if(tag=="saves"||tag=="spellcasting"||(tag=="creature"&&key=="blindness-adept"))continue;
            if(remove_roaming&&tag=="creature"&&std::find(additions.begin(),additions.end(),key)!=additions.end())continue;
            previous+=line_before+'\n';
        }
        std::uint64_t previous_hash=14695981039346656037ULL;for(unsigned char c:previous){previous_hash^=c;previous_hash*=1099511628211ULL;}
        if(previous_hash!=hash)content.previous_campaign_identities.push_back({"opengold.srd5",content.identity.version,revision+"/"+std::to_string(previous_hash)});
    }
    std::set<std::string> save_rows,casting_rows;
    while(std::getline(lines,line)) {
        if(line.empty()||line[0]=='#'||line=="\r")continue;
        std::istringstream row(line);std::string tag,key;Definition d;row>>tag>>key;
        if(tag=="saves"||tag=="spellcasting"){
            const auto found=content.definitions.find(key);
            auto& seen=tag=="saves"?save_rows:casting_rows;
            if(found==content.definitions.end()||!seen.insert(key).second)throw std::runtime_error("Invalid supplemental creature definition: "+key);
            auto& definition=found->second;
            if(tag=="saves"){
                for(auto& bonus:definition.saves)row>>bonus;
                if(std::any_of(definition.saves.begin(),definition.saves.end(),[](int n){return n< -10||n>30;}))throw std::runtime_error("Invalid saving throw bonus");
            }else{
                row>>definition.slots2>>definition.spells;
                if(definition.level<3||definition.slots2<0||definition.slots2>20||definition.spells<0||definition.spells>63)throw std::runtime_error("Invalid supplemental spellcasting");
            }
            if(!row)throw std::runtime_error("Truncated supplemental creature definition");
            row>>std::ws;if(!row.eof())throw std::runtime_error("Unknown supplemental creature fields");
            continue;
        }
        row>>d.ac>>d.hp>>d.initiative>>d.speed>>d.melee_bonus>>d.melee.count>>d.melee.sides>>d.melee.bonus
            >>d.ranged_bonus>>d.ranged.count>>d.ranged.sides>>d.ranged.bonus>>d.range>>d.long_range>>d.winds>>d.slots>>d.casting>>d.level>>d.spells;
        const bool ranged_none=d.range==0&&d.long_range==0&&d.ranged_bonus==0&&
            d.ranged.count==0&&d.ranged.sides==0&&d.ranged.bonus==0;
        const bool ranged_weapon=d.range>=5&&d.long_range>=d.range&&d.long_range<=600&&
            d.ranged_bonus>= -10&&d.ranged_bonus<=30&&d.ranged.count>=1&&d.ranged.count<=10&&
            d.ranged.sides>=2&&d.ranged.sides<=20&&d.ranged.bonus>= -10&&d.ranged.bonus<=30;
        if(!row||tag!="creature"||key.size()>80||content.definitions.contains(key)||d.ac<1||d.ac>40||d.hp<1||d.hp>1000||
            d.initiative< -10||d.initiative>20||d.speed<5||d.speed>120||d.speed%5||d.melee.count<1||d.melee.count>10||d.melee.sides<2||d.melee.sides>20||
            (!ranged_none&&!ranged_weapon)||
            d.winds<0||d.winds>10||d.slots<0||d.slots>20||d.level<1||d.level>4||d.spells<0||d.spells>7||
            d.melee_bonus< -10||d.melee_bonus>30||d.casting< -10||d.casting>30||
            d.melee.bonus< -10||d.melee.bonus>30)
            throw std::runtime_error("Invalid or unsupported creature definition: "+key);
        row>>std::ws;if(!row.eof())throw std::runtime_error("Unknown creature fields: "+key);
        content.definitions.emplace(std::move(key),d);
    }
    if(content.definitions.empty())throw std::runtime_error("Empty rules content");return std::make_unique<Module>(std::move(content));
}
}
