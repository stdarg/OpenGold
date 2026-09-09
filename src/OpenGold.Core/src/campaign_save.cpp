#include "opengold/campaign_save.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace opengold {
namespace {
constexpr std::size_t limit=16*1024*1024;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::uint64_t fingerprint(std::string_view data){std::uint64_t n=14695981039346656037ULL;for(unsigned char c:data){n^=c;n*=1099511628211ULL;}return n;}
}
// Explicit field encoding: no pointers, native object layouts or derived sheets.
struct SaveCodec {
    bool reading{};
    std::stringstream stream;
    const rules::CharacterRules* creation{};
    const rules::RulesModule* module{};
    explicit SaveCodec(std::string_view bytes):reading(true),stream(std::string(bytes)){stream.imbue(std::locale::classic());}
    SaveCodec(){stream.imbue(std::locale::classic());}
    template<class... T> void fields(T&... value){(field(value),...);}
    template<class T> requires std::is_integral_v<T> void field(T& value){
        if(reading){
            if constexpr(std::is_signed_v<T>){std::int64_t n{};stream>>n;require(n>=std::numeric_limits<T>::min()&&n<=std::numeric_limits<T>::max(),"Save integer out of range");value=static_cast<T>(n);}
            else {std::string token;stream>>token;require(!token.empty()&&token.find_first_not_of("0123456789")==std::string::npos,"Invalid save integer");std::size_t used{};auto n=std::stoull(token,&used);require(used==token.size()&&n<=static_cast<std::uint64_t>(std::numeric_limits<T>::max()),"Save integer out of range");value=static_cast<T>(n);}
        }else if constexpr(std::is_signed_v<T>)stream<<static_cast<std::int64_t>(value)<<' ';
        else stream<<static_cast<std::uint64_t>(value)<<' ';
        require(bool(stream),"Truncated or invalid campaign save");
    }
    void field(std::string& value){if(reading)stream>>std::quoted(value);else stream<<std::quoted(value)<<' ';require(bool(stream)&&value.size()<=limit,"Invalid save string");}
    template<class T,std::size_t N>void field(std::array<T,N>& values){for(auto& v:values)field(v);}
    template<class T>void field(std::vector<T>& values){std::uint64_t count=values.size();field(count);require(count<=65536,"Save collection too large");if(reading){values.clear();for(std::uint64_t i=0;i<count;++i){T v{};field(v);values.push_back(std::move(v));}}else for(auto& v:values)field(v);}
    template<class K,class V>void field(std::map<K,V>& values){std::uint64_t count=values.size();field(count);require(count<=65536,"Save map too large");if(reading){values.clear();for(std::uint64_t i=0;i<count;++i){K k{};V v{};fields(k,v);require(values.emplace(k,std::move(v)).second,"Duplicate save map key");}}else for(auto& [key,v]:values){auto k=key;fields(k,v);}}
    template<class T>void field(std::optional<T>& value){bool present=value.has_value();field(present);if(reading){if(present)value.emplace();else value.reset();}if(present)field(*value);}
    void field(rules::Identity& v){fields(v.module,v.version,v.content);}
    void field(rules::AbilityRoll& v){fields(v.dice,v.discarded);}
    void field(rules::CharacterDraft& v){fields(v.race,v.gender,v.character_class,v.alignment,v.background,v.name,v.target_classes,v.rolls,v.assignment,v.adjustment,v.rolled);}
    void field(por::CharacterAppearance& v){fields(v.portrait_head,v.portrait_body,v.combat_head,v.combat_body,v.tall,v.colors);}
    void field(InventoryItem& v){fields(v.id,v.definition_id,v.name,v.quantity,v.original_type);}
    void field(Inventory& v){fields(v.next_id_,v.items_);if(reading){std::set<std::uint64_t> ids;require(v.next_id_!=0,"Invalid next inventory ID");for(const auto& i:v.items_)require(i.id&&i.id<v.next_id_&&ids.insert(i.id).second&&i.quantity&&!i.definition_id.empty()&&!i.name.empty(),"Invalid inventory entry");}}
    void field(por::DamageDice& v){fields(v.count,v.sides,v.modifier);}
    void field(por::ItemRecord& v){fields(v.raw,v.stored_name,v.type,v.name_components,v.magic_bonus,v.save_bonus,v.readied_raw,v.revealed_components,v.cursed_raw,v.weight,v.value,v.stack_size,v.effect_codes);}
    void field(por::ItemTemplate& v){fields(v.raw,v.worn_location,v.hands,v.rate_of_fire,v.protection_raw,v.damage_type,v.melee_flag,v.large_damage,v.small_medium_damage,v.range,v.class_restrictions,v.ammunition_type);}
    void field(por::EquipmentBonuses& v){fields(v.weapon_to_hit,v.weapon_damage,v.armor_base_ac,v.ac_adjustment,v.save_bonus);}
    void field(por::Equipment& v){fields(v.index,v.stored,v.base,v.bonuses);}
    void field(rules::VitalState& v){fields(v.hit_points,v.dead,v.resources,v.description);}
    void member(PartyMember& v){fields(v.id,v.npc_source,v.vitals,v.wealth,v.equipped,v.morale,v.experience,v.last_rest_minutes,v.item_sources,v.creation_source);}
    void field(PartyState& v){
        fields(v.slots,v.next_id,v.selected,v.time_minutes,v.random_state,v.claimed_rewards);
        std::uint64_t count=v.roster.size();field(count);require(count<=128,"Too many saved party members");
        if(reading){v.roster.clear();for(std::uint64_t i=0;i<count;++i){rules::CharacterDraft draft;por::CharacterAppearance appearance;int level{};fields(draft,appearance,level);require(level>=1&&level<=2,"Unsupported saved character level");Character character(*creation,std::move(draft),appearance);rules::VitalState scratch;while(character.sheet().level<level)require(character.advance(*module,scratch),"Unsupported saved advancement");field(character.inventory());PartyMember m{0,std::move(character)};member(m);v.roster.push_back(std::move(m));}}
        else for(auto& m:v.roster){auto draft=m.character.creation_data();auto appearance=m.character.appearance();auto level=m.character.sheet().level;fields(draft,appearance,level,m.character.inventory());member(m);}
    }
    void machine(por::EclMachine& v){
        require(v.state_==por::EclState::idle||v.state_==por::EclState::completed||reading,"Cannot save a pending script");
        fields(v.variables_,v.image_,v.instruction_spans_,v.conditions_,v.next_request_,v.total_instructions_);
        std::string rng;if(!reading){std::ostringstream out;out.imbue(std::locale::classic());out<<v.random_;rng=out.str();}field(rng);
        if(reading){require(v.image_.size()==v.program_->raw().size(),"Script image size mismatch");std::istringstream in(rng);in.imbue(std::locale::classic());in>>v.random_;require(bool(in),"Invalid saved script RNG");in>>std::ws;require(in.eof(),"Trailing script RNG data");v.state_=por::EclState::completed;v.pending_.reset();v.destination_.reset();v.menu_values_.clear();v.stack_.clear();v.trace_.clear();v.diagnostic_.clear();}
    }
    void town(por::RolfTourSession& v){
        require(reading||(v.can_leave()&&v.snapshot_.tour_finished&&!v.checkpoint_&&!v.pending_movement_),"Save only during idle town exploration");
        fields(v.current_script_,v.selected_character_,v.next_ticket_);
        if(reading){require(v.town_&&v.town_->programs.contains(v.current_script_)&&v.selected_character_<8,"Unsupported saved town context");v.machine_=por::EclMachine(v.town_->programs.at(v.current_script_));for(const std::uint8_t op:{12,13,14,45,49,58})v.machine_.enable_host(op);v.configure_town();}
        machine(v.machine_);
        auto& s=v.snapshot_;fields(s.dialogue,s.prompts,s.redraws,s.footsteps,s.event_runs);
        std::string visited=s.visited.to_string();field(visited);
        if(reading){require(visited.size()==256&&visited.find_first_not_of("01")==std::string::npos,"Invalid visited map");s.visited=std::bitset<256>(visited);s.phase=por::TourPhase::completed;s.tour_finished=true;s.script_id=v.current_script_;s.sprite_frame=-1;s.choices.clear();s.continue_ticket=0;s.diagnostic.clear();++s.picture_revision;v.picture_.reset();v.checkpoint_.reset();v.saved_campaign_.reset();v.pending_movement_.reset();v.treasure_.clear();v.who_slots_.clear();v.temple_targets_.clear();v.menu_request_=v.delayed_request_=v.who_request_=v.temple_request_=v.shop_request_=0;v.remaining_delay_=0;v.transition_=v.message_only_=false;v.event_stage_=0;v.publish_pose();v.campaign_.reset();}
    }
};

std::string encode_campaign(const CampaignParty& party,const por::RolfTourSession* town,std::string_view assets){
    require(!party.in_combat(),"Cannot save during combat");SaveCodec out;auto identity=party.identity();std::string asset(assets);auto state=party.checkpoint();out.fields(identity,asset,state);bool has_town=town!=nullptr;out.field(has_town);if(town){auto copy=*town;out.town(copy);}auto body=out.stream.str();require(body.size()<=limit,"Campaign save too large");return "OPENGOLD-CAMPAIGN 1\n"+std::to_string(fingerprint(body))+"\n"+body;
}
namespace {
void validate_saved_member(const PartyMember& member,const rules::RulesModule& module){
    module.validate_character_state(member.character.sheet(),member.vitals);
    for(const auto& [id,source]:member.item_sources){
        const auto item=member.character.inventory().find(id);
        require(item.has_value()&&item->get().definition_id==equipment_conversion(source)&&item->get().original_type==source.stored.type,"Saved item provenance mismatch");
    }
    for(const auto& item:member.character.inventory().items())if(!member.item_sources.contains(item.id)){
        const std::array<std::string,1> definition{item.definition_id};
        (void)module.character_profile(member.character.sheet(),definition);
    }
    std::vector<std::string> gear;for(auto id:member.equipped)gear.push_back(member.character.inventory().find(id)->get().definition_id);
    (void)module.character_profile(member.character.sheet(),gear);
}
}
SavedCampaign decode_campaign(std::string_view bytes,const rules::CharacterRules& creation,const rules::RulesModule& module,std::string_view assets,const por::RolfTourSession* town_template){
    require(bytes.size()<=limit,"Campaign save too large");constexpr std::string_view header="OPENGOLD-CAMPAIGN 1\n";require(bytes.starts_with(header),"Unsupported campaign save version");auto end=bytes.find('\n',header.size());require(end!=bytes.npos,"Truncated campaign save");auto body=bytes.substr(end+1);require(bytes.substr(header.size(),end-header.size())==std::to_string(fingerprint(body)),"Campaign save checksum mismatch");
    SaveCodec in(body);in.creation=&creation;in.module=&module;rules::Identity identity;std::string asset;in.fields(identity,asset);require(identity==module.identity(),"Campaign rules/content version mismatch");require(asset==assets,"Campaign original asset identity mismatch");SavedCampaign result;in.field(result.party);CampaignParty::validate(result.party);for(const auto& m:result.party.roster)validate_saved_member(m,module);bool has_town{};in.field(has_town);if(has_town){require(town_template!=nullptr,"This save requires town resources");result.town=*town_template;in.town(*result.town);}in.stream>>std::ws;require(in.stream.eof(),"Trailing campaign save data");return result;
}
std::string read_campaign_file(const std::filesystem::path& path){auto size=std::filesystem::file_size(path);require(size<=limit,"Campaign file too large");std::ifstream in(path,std::ios::binary);require(bool(in),"Cannot open campaign file");std::string data{std::istreambuf_iterator<char>(in),{}};require(!in.bad()&&data.size()==size,"Incomplete campaign file read");return data;}
std::string campaign_asset_identity(const std::filesystem::path& directory){
    std::map<std::string,std::uint64_t> files;for(const auto& entry:std::filesystem::directory_iterator(directory)){if(!entry.is_regular_file())continue;auto name=entry.path().filename().string();for(auto& c:name)if(c>='a'&&c<='z')c-=32;
        if(name.ends_with(".DAX")||name=="ITEMS"){require(!files.contains(name),"Ambiguous original asset filename");files.emplace(name,fingerprint(read_campaign_file(entry.path())));}}
    require(files.contains("ECL3.DAX")&&files.contains("GEO3.DAX")&&files.contains("ITEMS"),"Missing campaign assets");std::ostringstream out;for(auto& [name,hash]:files)out<<name<<':'<<hash<<';';return out.str();
}
void write_campaign_file(const std::filesystem::path& path,std::string_view bytes){
    require(bytes.size()<=limit,"Campaign file too large");std::filesystem::create_directories(path.parent_path());auto temp=path;temp+=".tmp";auto backup=path;backup+=".bak";
#ifdef _WIN32
    struct CloseHandleOwner{void operator()(void* h)const{CloseHandle(h);}};
    const auto raw=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);require(raw!=INVALID_HANDLE_VALUE,"Cannot create temporary campaign save");std::unique_ptr<void,CloseHandleOwner> handle(raw);DWORD written{};require(WriteFile(handle.get(),bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(handle.get()),"Cannot flush campaign save");handle.reset();
#else
    struct Descriptor{int fd;~Descriptor(){if(fd>=0)::close(fd);}} handle{::open(temp.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600)};require(handle.fd>=0,"Cannot create temporary campaign save");std::size_t pos=0;while(pos<bytes.size()){auto n=::write(handle.fd,bytes.data()+pos,bytes.size()-pos);require(n>0,"Cannot write campaign save");pos+=n;}require(::fsync(handle.fd)==0,"Cannot flush campaign save");
#endif
    require(read_campaign_file(temp)==bytes,"Campaign save verification failed");
#ifdef _WIN32
    if(std::filesystem::exists(path))require(ReplaceFileW(path.c_str(),temp.c_str(),backup.c_str(),0,nullptr,nullptr),"Cannot replace campaign save; previous file retained");else require(MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH),"Cannot install campaign save");
#else
    if(std::filesystem::exists(path))std::filesystem::copy_file(path,backup,std::filesystem::copy_options::overwrite_existing);std::filesystem::rename(temp,path);Descriptor directory{::open(path.parent_path().c_str(),O_RDONLY|O_DIRECTORY)};require(directory.fd>=0&&::fsync(directory.fd)==0,"Cannot flush save directory");
#endif
}
}
