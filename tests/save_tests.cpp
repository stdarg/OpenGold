#include "opengold/campaign_save.h"
#include "opengold/srd5.h"
#include "opengold/combat_demo.h"
#include <iostream>
#include <chrono>
#include <fstream>
using namespace opengold;
namespace {
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"Invalid save must reject");}
std::string changed_identity(const std::string& saved,const std::string& identity){auto start=saved.find('\n',saved.find('\n')+1)+1;auto body=saved.substr(start);auto position=body.find('"'+identity+'"');check(position!=body.npos,"Identity must be present");body.replace(position,identity.size()+2,"\"incompatible\"");std::uint64_t hash=14695981039346656037ULL;for(unsigned char c:body){hash^=c;hash*=1099511628211ULL;}return saved.substr(0,saved.find('\n')+1)+std::to_string(hash)+'\n'+body;}
auto module(){return srd5::load(std::filesystem::path(OPENGOLD_SOURCE_DIR)/"data/rules/srd-5.2.1/combat.rules");}
Character character(std::string klass){rules::CharacterDraft d;d.race="human";d.gender="female";d.character_class=klass;d.alignment="neutral_good";d.background="soldier";d.name="Save test "+klass;d.rolled=true;for(auto& r:d.rolls)r={{6,5,4,1},3};return Character(*srd5::character_rules(),d,{});}
auto prototype(){
    std::vector<std::uint8_t> bytes{0,0};for(int n=0;n<5;++n)bytes.insert(bytes.end(),{1,1,0x15,0x99});bytes.push_back(0);
    // Each LOOK increments a persistent cell, then completes.
    bytes.insert(bytes.end(),{4,1,0x10,0x98,0,1,1,0x10,0x98,0});
    auto p=std::make_shared<const por::EclProgram>(por::EclProgram::decode(bytes,"save fixture"));
    auto resources=std::make_shared<por::PhlanResources>();resources->programs[0]=p;
    return por::RolfTourSession({},p,{},0x9914,{},resources);
}
void settle(por::RolfTourSession& town){for(int i=0;i<100&&town.snapshot().phase==por::TourPhase::running;++i)town.advance(1);check(town.can_leave(),"Fixture must finish");}
void roundtrip(const std::filesystem::path& directory){
    auto party=std::make_shared<CampaignParty>(module());auto fighter=party->add_pc(character("fighter"));auto mage=party->add_pc(character("wizard"));auto reserve=party->add_pc(character("bard"));party->remove(reserve);
    party->set_wealth(fighter,{0,0,0,500,0,0,2});por::Equipment sword;sword.stored.type=36;sword.stored.stack_size=1;sword.stored.value=10;party->purchase(fighter,sword);party->equip(fighter,1);
    party->award_experience(300,"save:encounter");check(party->rest(),"Initial rest");
    auto state=party->checkpoint();state.roster[0].vitals.hit_points=1;state.roster[1].vitals.resources="SRD1 0 1 0 0 0";party->restore(state);party->temple_heal(fighter);
    auto town=prototype();town.campaign_party(party);settle(town);town.explore(por::ExplorationCommand::look);settle(town);
    auto path=directory/std::filesystem::u8path("named save ü.ogs");const auto saved=encode_campaign(*party,&town,"fixture-v1");write_campaign_file(path,saved);
    auto base=prototype();auto rules=module();auto loaded=decode_campaign(read_campaign_file(path),*srd5::character_rules(),*rules,"fixture-v1",&base);auto replacement=std::make_shared<CampaignParty>(module());replacement->restore(std::move(loaded.party));loaded.town->attach_restored_party(replacement);
    check(encode_campaign(*replacement,&*loaded.town,"fixture-v1")==saved,"Complete serialized state round trips");
    const auto encounter=[](const CampaignParty& p){rules::Encounter e{{8,8,std::vector<std::uint8_t>(64)},p.participants()};e.participants.push_back({99,"bandit","Bandit",1,{6,6}});return e;};
    auto combat_a=rules->create(encounter(*party),42),combat_b=rules->create(encounter(*replacement),42);
    for(int i=0;i<30&&combat_a->snapshot().outcome==rules::Outcome::ongoing;++i){auto command=choose_demo_command(*combat_a);check(combat_a->submit(command)&&combat_b->submit(command)&&combat_a->save()==combat_b->save(),"Next combat continues deterministically after disk reload");}
    check(replacement->member(fighter).wealth[3]==390,"Temple charge survives load");check(replacement->member(mage).vitals.resources=="SRD1 0 1 0 0 0","Spent caster slots survive load");
    replacement->award_experience(300,"save:encounter");check(replacement->member(fighter).experience==300,"No duplicate XP after reload");check(!replacement->rest(),"Reload must not reset rest timer");replacement->rejoin(reserve);check(!replacement->rest(),"Rejoin cannot bypass existing timers");replacement->remove(reserve);
    auto wounded=replacement->checkpoint();wounded.roster[0].vitals.hit_points=1;replacement->restore(wounded);party->restore(wounded);replacement->temple_heal(fighter);party->temple_heal(fighter);check(replacement->member(fighter).vitals==party->member(fighter).vitals&&replacement->state().random_state==party->state().random_state,"Service RNG continuation matches");
    loaded.town->explore(por::ExplorationCommand::look);town.explore(por::ExplorationCommand::look);settle(*loaded.town);settle(town);check(loaded.town->script_variable(0x9810)==town.script_variable(0x9810),"Script continuation matches");
    auto next=encode_campaign(*replacement,&*loaded.town,"fixture-v1");write_campaign_file(path,next);auto backup=path;backup+=".bak";check(read_campaign_file(backup)==saved,"Overwrite retains previous save");write_campaign_file(path,saved);check(read_campaign_file(backup)==next,"Second overwrite rotates previous save");
    auto truncated=saved.substr(0,saved.size()/2);write_campaign_file(directory/"truncated.ogs",truncated);rejects([&]{(void)decode_campaign(read_campaign_file(directory/"truncated.ogs"),*srd5::character_rules(),*rules,"fixture-v1",&base);});
    auto corrupt=saved;corrupt.back()^=1;rejects([&]{(void)decode_campaign(corrupt,*srd5::character_rules(),*rules,"fixture-v1",&base);});
    rejects([&]{(void)decode_campaign(saved,*srd5::character_rules(),*rules,"different-assets",&base);});
    for(const auto& identity:{rules->identity().module,rules->identity().version,rules->identity().content})rejects([&]{(void)decode_campaign(changed_identity(saved,identity),*srd5::character_rules(),*rules,"fixture-v1",&base);});
#ifdef _WIN32
    {std::ifstream held(path,std::ios::binary);rejects([&]{write_campaign_file(path,next);});check(read_campaign_file(path)==saved,"Failed file replacement preserves existing save");}
#endif
    auto version=saved;version[18]='9';rejects([&]{(void)decode_campaign(version,*srd5::character_rules(),*rules,"fixture-v1",&base);});
    auto invalid=party->checkpoint();invalid.roster[0].vitals.resources="SRD1 999 0 0 0 0";party->restore(invalid);auto malformed=encode_campaign(*party,nullptr,"fixture-v1");rejects([&]{(void)decode_campaign(malformed,*srd5::character_rules(),*rules,"fixture-v1",nullptr);});
    party->begin_combat();rejects([&]{(void)encode_campaign(*party,nullptr,"fixture-v1");});party->end_combat();
    auto busy=prototype();rejects([&]{(void)encode_campaign(*party,&busy,"fixture-v1");});
}
}
int main(){try{const auto directory=std::filesystem::temp_directory_path()/("opengold-save-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));roundtrip(directory);for(const auto& p:std::filesystem::directory_iterator(directory))std::filesystem::remove(p.path());std::filesystem::remove(directory);std::cout<<"Campaign file save tests passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
