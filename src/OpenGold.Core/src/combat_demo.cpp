#include "opengold/combat_demo.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <queue>
#include <stdexcept>
namespace opengold {
using namespace rules;using namespace por;
namespace {
Battlefield arena() {
    Battlefield b{12,9,std::vector<std::uint8_t>(108,0)};
    for(int y=2;y<=6;++y)if(y!=4)b.terrain[y*12+6]=1;
    b.terrain[4*12+5]=2;b.terrain[4*12+6]=2;return b;
}
std::vector<Participant> party() {return {{1,"vanguard","Vanguard",0,{2,2}},{2,"scout","Scout",0,{2,4}},
    {3,"adept","Adept",0,{1,3}},{4,"healer","Healer",0,{1,5}}};}
std::optional<Image> original_icon(const std::filesystem::path& directory,unsigned record) {
    for(const auto& file:std::filesystem::directory_iterator(directory)) {
        auto name=file.path().filename().string();for(auto& c:name)if(c>='a'&&c<='z')c-=32;
        if(name!="CPIC2.DAX")continue;
        if(std::filesystem::file_size(file.path())>32*1024*1024)throw std::runtime_error("Combat art archive exceeds limit");
        std::ifstream input(file.path(),std::ios::binary);std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input),{}};
        if(input.bad())throw std::runtime_error("Cannot read combat art");
        auto decoded=decode_ega_combat_icon(bytes,static_cast<std::uint8_t>(record),0);
        if(!decoded)throw std::runtime_error("Cannot decode the script-selected combat icon");return std::move(decoded.image);
    }
    return std::nullopt;
}
}
CombatDemo::CombatDemo(std::unique_ptr<RulesModule> module):module_(std::move(module))
{if(!module_)throw std::runtime_error("A combat rules module is required");}
CombatDemo::~CombatDemo(){if(campaign_&&owns_campaign_combat_)campaign_->end_combat();}
void CombatDemo::campaign_party(std::shared_ptr<CampaignParty> party)
{if(combat_)throw std::runtime_error("Attach party before starting combat");campaign_=std::move(party);}
void CombatDemo::synchronize_party()
{
    if(campaign_&&combat_){campaign_->apply_combat(combat_->snapshot());
        if(combat_->snapshot().outcome!=Outcome::ongoing){campaign_->end_combat();owns_campaign_combat_=false;}}
}
void CombatDemo::start_encounter(std::vector<Participant> enemies)
{
    auto participants=campaign_?campaign_->participants():party();
    participants.insert(participants.end(),enemies.begin(),enemies.end());
    auto next=module_->create({arena(),std::move(participants)},seed_);
    if(campaign_){if(campaign_->identity()!=module_->identity())throw std::runtime_error("Party and combat rules differ");
        campaign_->begin_combat();owns_campaign_combat_=true;}combat_=std::move(next);synchronize_party();
}
const CombatSession& CombatDemo::combat() const
{if(!combat_)throw std::runtime_error("No active combat");return *combat_;}
void CombatDemo::training(std::uint64_t seed)
{
    if(campaign_){seed_=seed;start_encounter({{1000,"bandit","Bandit",1,{9,4}}});
        vm_.reset();art_.clear();dialogue_="Party combat preview. HP and spent resources carry back to the party.";status_="Party training";return;}
    auto next=module_->create({arena(),{{1,"vanguard","Vanguard",0,{2,4}},{10,"bandit","Bandit",1,{9,4}}}},seed);
    combat_=std::move(next);vm_.reset();creatures_.reset();art_.clear();enemies_.clear();
    menu_ticket_=combat_ticket_=0;encounters_=0;seed_=seed;
    dialogue_="Training encounter: one martial test profile and one SRD Bandit.";
    status_="Training arena";
}
void CombatDemo::slums(const std::filesystem::path& directory,std::uint64_t seed)
{
    const auto catalog=EclCatalog::load(directory);const auto program=catalog.find({"ECL2.DAX",20});
    if(!program)throw std::runtime_error("Slums profile requires ECL2.DAX:20");
    auto creatures=CreatureCatalog::load(directory);
    for(auto record:{4,13})if(!creatures.find({2,static_cast<std::uint8_t>(record)}))throw std::runtime_error("Missing Slums creature record");
    EclMachine vm(program);
    for(auto [first,last]:std::array<std::array<unsigned,2>,3>{{{0x4900,0x4cff},{0x6b00,0x6eff},{0x9700,0x98ff}}})
        for(unsigned a=first;a<=last;++a)vm.bind_variable(static_cast<std::uint16_t>(a),0);
    vm.bind_variable(0xC04F,1);
    for(std::uint8_t opcode:{11,12,13,14,28,36})vm.enable_host(opcode);
    if(!vm.start(1))throw std::runtime_error("Cannot enter Slums event 1");
    vm_=std::move(vm);creatures_=std::move(creatures);combat_.reset();art_.clear();enemies_.clear();
    menu_ticket_=combat_ticket_=0;seed_=seed;encounters_=0;game_directory_=directory;dialogue_.clear();status_="Slums event 1";pump();
}
void CombatDemo::pump()
{
    if(!vm_||menu_ticket_||combat_ticket_)return;
    for(unsigned step=0;step<64;++step) {
        const auto result=vm_->run(1000);
        if(result.state==EclState::faulted)throw std::runtime_error(result.diagnostic);
        if(result.state==EclState::completed){status_="Script complete; event flag "+std::to_string(vm_->variable(0x4ACA))+", fight count "+std::to_string(vm_->variable(0x4ABB));return;}
        if(!result.request)continue;
        const auto& request=*result.request;
        if(request.kind==EclRequestKind::text) {
            if(request.clear)dialogue_.clear();dialogue_+=request.text;
            if(dialogue_.size()>8192)throw std::runtime_error("Slums dialogue exceeds limit");
            if(!vm_->resume(request.id))throw std::runtime_error("Text acknowledgement rejected");
        } else if(request.kind==EclRequestKind::menu) {
            if(request.choices.size()!=1)throw std::runtime_error("Unsupported Slums menu");menu_ticket_=request.id;return;
        } else if(request.kind==EclRequestKind::host) {
            const auto opcode=request.instruction->opcode;
            if(opcode==11) {
                const auto& a=request.arguments;
                const bool first=enemies_.empty();const unsigned record=first?13:4,count=first?1:3;
                if(a.size()!=3||a[0].value!=record||a[1].value!=count||a[2].value!=4||enemies_.size()>1)
                    throw std::runtime_error("Unrecognized Slums creature/count/icon profile");
                const auto& creature=creatures_->find({2,static_cast<std::uint8_t>(record)})->get();
                const auto icon=original_icon(game_directory_,a[2].value);
                for(unsigned i=0;i<count;++i) {
                    const auto id=static_cast<EntityId>(1000+enemies_.size());
                    enemies_.push_back({id,"slums-orc",creature.stored.name+" "+std::to_string(enemies_.size()+1),1,{9,2+static_cast<int>(enemies_.size())}});
                    if(icon)art_.push_back({id,*icon});
                }
            } else if(opcode==36) {
                if(enemies_.size()!=4||encounters_!=0||vm_->variable(0x6DC6)!=99||vm_->variable(0x6DCB)!=0)
                    throw std::runtime_error("Unsupported Slums combat context");
                start_encounter(enemies_);combat_ticket_=request.id;++encounters_;
                status_="Slums combat: original four-orc group, authored 5e conversion and arena";return;
            } else if(opcode==28) {
                enemies_.clear();art_.clear(); // CLEAR MONSTERS resets the staged encounter.
            } else if(opcode!=12&&opcode!=13&&opcode!=14)throw std::runtime_error("Unsupported Slums presentation service");
            if(!vm_->resume_host(request.id,{}))throw std::runtime_error("Slums host acknowledgement rejected");
        } else throw std::runtime_error("Unsupported Slums input");
    }
    throw std::runtime_error("Slums script exceeded request budget");
}
void CombatDemo::continue_script()
{if(!vm_||!menu_ticket_)return;if(!vm_->resume(menu_ticket_,0))throw std::runtime_error("Slums menu acknowledgement rejected");menu_ticket_=0;pump();}
void CombatDemo::finish_combat()
{
    if(!vm_||!combat_ticket_||!combat_)return;const auto state=combat_->snapshot();if(state.outcome==Outcome::ongoing)return;
    unsigned defeated=0;for(const auto& unit:state.combatants)if(unit.side==1&&unit.hit_points==0)++defeated;
    EclHostReply reply;reply.writes={{0x6DC7,static_cast<std::uint16_t>(state.outcome==Outcome::victory?0:128)},
        {0x6DC8,static_cast<std::uint16_t>(defeated)},{0x6DCB,0},{0x6DE3,0},{0x6E70,0},{0x6E71,0},{0x6E72,0}};
    if(!vm_->resume_host(combat_ticket_,reply))throw std::runtime_error("Combat outcome rejected by ECL");combat_ticket_=0;pump();
}
bool CombatDemo::submit(const Command& command)
{if(!combat_||!combat_->submit(command))return false;synchronize_party();finish_combat();return true;}
void CombatDemo::revisit()
{
    if(!script_complete()||!combat_||combat_->snapshot().outcome!=Outcome::victory)throw std::runtime_error("Revisit requires completed victory");
    if(!vm_->start(1))throw std::runtime_error("Slums revisit rejected");dialogue_.clear();pump();
}
std::string CombatDemo::save_combat() const
{if(vm_||campaign_)throw std::runtime_error("Campaign checkpoints are pending; save/load currently supports training combat only");return combat().save();}
void CombatDemo::restore_combat(std::string_view checkpoint)
{if(vm_||campaign_)throw std::runtime_error("Cannot replace a campaign combat with a training checkpoint");auto restored=module_->restore(checkpoint);combat_=std::move(restored);}
unsigned CombatDemo::script_variable(std::uint16_t address) const
{if(!vm_)throw std::runtime_error("No active campaign script");return vm_->variable(address);}
Command choose_demo_command(const CombatSession& session)
{
    const auto state=session.snapshot();const auto offered=session.legal_commands();if(offered.empty())throw std::runtime_error("No legal combat command");
    const auto& active=*std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==state.actor;});
    // Rank offered destinations by a geometric route around obstacles. Straight
    // distance alone can strand both sides on opposite corners of a wall.
    // This is an AI heuristic; legal movement and its costs remain module-owned.
    const auto& board=state.battlefield;
    const auto index=[&](Cell p){return p.y*board.width+p.x;};
    std::vector<int> routes(board.terrain.size(),10000);std::queue<Cell> frontier;
    for(const auto& a:state.combatants)if(a.side!=active.side&&a.conscious){routes[index(a.cell)]=0;frontier.push(a.cell);}
    while(!frontier.empty()) {
        const auto p=frontier.front();frontier.pop();
        for(int y=-1;y<=1;++y)for(int x=-1;x<=1;++x) {
            const Cell next{p.x+x,p.y+y};if((!x&&!y)||board.at(next)==1)continue;
            if(x&&y&&(board.at({p.x+x,p.y})==1||board.at({p.x,p.y+y})==1))continue;
            if(routes[index(next)]<=routes[index(p)]+1)continue;
            routes[index(next)]=routes[index(p)]+1;frontier.push(next);
        }
    }
    const auto nearest=[&](Cell p){return routes[index(p)];};
    for(const auto& command:offered)if(command.verb=="opportunity"||command.verb=="second_wind")return command;
    for(const auto& command:offered)if(command.verb=="cure_wounds") {
        const auto& target=*std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==command.target;});
        if(target.hit_points*2<target.max_hit_points)return command;
    }
    for(const auto verb:{"magic_missile","melee","fire_bolt","ranged"}) {
        const Command* best=nullptr;int hp=100000; // Borrowed view into local offered commands.
        for(const auto& command:offered)if(command.verb==verb) {
            const auto& target=*std::find_if(state.combatants.begin(),state.combatants.end(),[&](const auto& a){return a.id==command.target;});
            if(target.hit_points<hp){best=&command;hp=target.hit_points;}
        }
        if(best)return *best;
    }
    const Command* move=nullptr;int closest=nearest(active.cell);
    for(const auto& command:offered)if(command.verb=="move"&&nearest(command.destination)<closest){move=&command;closest=nearest(command.destination);}
    if(move)return *move;
    for(const auto& command:offered)if(command.verb=="end")return command;
    return offered.front();
}
}
