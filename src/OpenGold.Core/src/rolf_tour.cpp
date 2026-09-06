#include "opengold/rolf_tour.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

namespace opengold::por {
namespace {
std::filesystem::path resolve_archive(const std::filesystem::path& directory, std::string_view name)
{
    std::optional<std::filesystem::path> path;
    for (const auto& file : std::filesystem::directory_iterator(directory)) {
        auto candidate = file.path().filename().string();
        for (auto& c : candidate) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
        if (candidate != name || !file.is_regular_file()) continue;
        if (path) throw EclError("Ambiguous archive " + std::string(name));
        path = file.path();
    }
    if (!path) throw EclError("Missing " + std::string(name));
    return *path;
}
std::vector<std::uint8_t> read_archive(const std::filesystem::path& path)
{
    const auto size = std::filesystem::file_size(path);
    if (size > 32 * 1024 * 1024) throw EclError("Archive exceeds size limit");
    std::ifstream input(path, std::ios::binary);
    if (!input) throw EclError("Cannot open " + path.string());
    std::vector<std::uint8_t> data{std::istreambuf_iterator<char>(input), {}};
    if (input.bad() || data.size() != size) throw EclError("Incomplete archive read");
    return data;
}
constexpr std::array<int, 4> dx{0, 1, 0, -1}, dy{-1, 0, 1, 0};
}

RolfTourSession RolfTourSession::load(const std::filesystem::path& directory)
{
    const auto catalog = EclCatalog::load(directory);
    const auto program = catalog.find({"ECL3.DAX", 0});
    if (!program) throw EclError("Rolf tour requires ECL3.DAX record 0");
    // Guard this explicit entry profile. No pattern search through embedded data,
    // no modification of original bytes, and no assumption that an event ID is PC.
    const auto first = program->instruction(0xB071);
    const auto intro = program->instruction(0xB0B9);
    const auto setup = program->instruction(0xB0AD);
    if (first.opcode != 9 || first.operands[0].value != 1 || first.operands[1].value != 0x4AC5 ||
        intro.opcode != 18 || intro.operands[0].tag != 128 || setup.opcode != 12 ||
        setup.operands[0].value != 12)
        throw EclError("This ECL3:0 does not match the supported Rolf entry profile");
    const auto maps = MapCatalog::load(directory);
    const auto map = maps.find({"GEO3.DAX", 0});
    if (!map) throw EclError("Rolf tour requires GEO3.DAX record 0");
    // Explicit, verified Phlan profile. This does not implement general 127
    // operand semantics or assume that every map shares its GEO/art record ID.
    const auto pieces = program->instruction(0x9B11);
    if (pieces.opcode != 55 || pieces.operands.size() != 3 ||
        std::any_of(pieces.operands.begin(), pieces.operands.end(),
            [](const auto& a) { return a.tag != 0 || a.value != 127; }))
        throw EclError("Unsupported Phlan wall resource profile");
    const auto archive = [&](const char* name) {
        auto records = decode_dax_archive(read_archive(resolve_archive(directory, name)));
        if (!records) throw EclError(std::string("Invalid wall archive ") + name);
        return records;
    };
    const auto find_record = [](const DaxDecodeResult& records, unsigned id) -> const std::vector<std::uint8_t>& {
        for (const auto& record : records.records) if (record.id == id) return record.bytes;
        throw EclError("Missing required Phlan wall/tile record " + std::to_string(id));
    };
    const auto definitions = archive("WALLDEF3.DAX");
    const auto shared = archive("8X8D1.DAX");
    const auto local = archive("8X8D3.DAX");
    WallTiles tiles(1); // Blank tile slot zero.
    const auto append = [&](const std::vector<std::uint8_t>& record, unsigned expected_count) {
        auto decoded = decode_wall_tiles(record);
        if (!decoded || decoded->size() != expected_count) throw EclError("Unexpected Phlan wall tile layout");
        tiles.insert(tiles.end(), decoded->begin(), decoded->end());
    };
    append(find_record(shared, 203), 45);
    for (unsigned id : {101, 102, 103}) append(find_record(local, id), 70);
    auto wall_art = decode_wall_art(find_record(definitions, 0), tiles);
    if (!wall_art || wall_art->appearances.size() != 15) throw EclError("Invalid Phlan wall definitions");
    const auto bytes = read_archive(resolve_archive(directory, "SPRIT3.DAX"));
    std::array<opengold::Image, 3> sprites;
    for (std::uint8_t i = 0; i < sprites.size(); ++i) {
        auto result = decode_ega_sprite(bytes, 12, i);
        if (!result) throw EclError("Cannot decode Rolf encounter distance image " + std::to_string(i));
        sprites[i] = std::move(result.image);
    }
    auto town = std::make_shared<PhlanResources>();
    for (unsigned id : {0,8,11}) {
        auto p = catalog.find({"ECL3.DAX", static_cast<std::uint8_t>(id)});
        if (!p) throw EclError("Missing New Phlan building script");
        town->programs.emplace(id, std::move(p));
    }
    const auto templates = decode_item_templates(read_archive(resolve_archive(directory, "ITEMS")));
    if (!templates) throw EclError("Invalid item templates");
    for (const auto& record : archive("ITEM3.DAX").records) {
        const auto items = decode_items(record.bytes);
        if (!items) throw EclError("Invalid town treasure record");
        auto& stock = town->treasure[record.id];
        for (const auto& item : *items) {
            if (item.type >= templates->size()) throw EclError("Invalid town item type");
            Equipment equipment; equipment.index = stock.size();
            equipment.stored = item; equipment.base = (*templates)[item.type];
            stock.push_back(std::move(equipment));
        }
    }
    town->sprite_archive = bytes;
    const auto pictures=[&](const char* name,auto& destination){
        for(const auto& record:archive(name).records){
            auto image=decode_ega_picture(record.bytes);
            if(image)destination.emplace(record.id,std::move(image.image));
        }
    };
    pictures("HEAD3.DAX",town->heads);pictures("BODY3.DAX",town->bodies);pictures("PIC3.DAX",town->pictures);
    return RolfTourSession(map->get(), program, std::move(sprites), 0xB071, std::move(*wall_art), std::move(town));
}

RolfTourSession::RolfTourSession(GeoMap map, std::shared_ptr<const EclProgram> program,
    std::array<opengold::Image, 3> sprites, std::uint32_t entry, WallArtSet wall_art,
    std::shared_ptr<const PhlanResources> town)
    : map_(std::move(map)), program_(std::move(program)), sprites_(std::move(sprites)),
      wall_art_(std::move(wall_art)), machine_(program_), entry_(entry), town_(std::move(town))
{
    restart();
}

void RolfTourSession::restart()
{
    const auto revision = snapshot_.revision + 1;
    machine_ = EclMachine(program_);
    snapshot_ = {}; snapshot_.revision = revision;
    menu_request_ = delayed_request_ = 0; remaining_delay_ = 0;
    party_ = {}; treasure_.clear(); picture_.reset();checkpoint_.reset(); diagnostics_.clear();
    current_script_ = selected_character_ = event_stage_ = 0;
    pending_movement_.reset(); transition_ = message_only_ = false; shop_request_ = 0;
    if (town_ && !town_->sprite_archive.empty()) for (unsigned n=0;n<3;++n) {
        auto decoded=decode_ega_sprite(town_->sprite_archive,12,n);
        if (!decoded) throw EclError("Cannot restore Rolf sprite");
        sprites_[n]=std::move(decoded.image);
    }
    // Minimal explicit isolated state, not a fabricated whole party/world model.
    for (const std::uint16_t address : {0x03DE, 0x49C9, 0x49FD, 0x4AC5, 0x4A07, 0x4A0F,
            0x4A10, 0x4A11, 0x6DE1, 0x6E79, 0x6E7A, 0x6E7B, 0x6E7C, 0x6E7D,
            0x9801, 0xC04B, 0xC04C, 0xC04D, 0xC04E, 0xC04F})
        machine_.bind_variable(address, 0);
    machine_.bind_variable(0x49C9, 12); // Research fixture: midday.
    for (const std::uint8_t opcode : {12, 13, 14, 45, 49, 58}) machine_.enable_host(opcode);
    if (town_) configure_town();
    if (!machine_.start_at_for_inspection(entry_)) fail("Invalid isolated tour entry");
}

void RolfTourSession::fail(std::string diagnostic)
{
    if (snapshot_.tour_finished && checkpoint_) {
        diagnostics_.push_back(diagnostic);
        machine_ = std::move(*checkpoint_); checkpoint_.reset();
        party_ = saved_party_; current_script_ = saved_script_;
        selected_character_ = saved_selected_character_; pending_movement_.reset(); transition_ = false;
        delayed_request_ = shop_request_ = 0; treasure_.clear();
        snapshot_.sprite_frame = -1;picture_.reset();++snapshot_.picture_revision;publish_pose();
        snapshot_.script_id = current_script_;
        notice("This event is not supported yet: " + diagnostic +
            "\nThe event's changes were rolled back. You can continue exploring.");
        return;
    }
    snapshot_.phase = TourPhase::faulted; snapshot_.diagnostic = std::move(diagnostic);
    snapshot_.continue_ticket = 0; ++snapshot_.revision;
}

void RolfTourSession::publish_pose()
{
    const PartyPose next{machine_.variable(0xC04B), machine_.variable(0xC04C), machine_.variable(0xC04D)};
    if (next.x >= 16 || next.y >= 16 || next.facing >= 4) throw EclError("Tour wrote an invalid party pose");
    snapshot_.pose = next;
    snapshot_.visited.set(next.y * 16 + next.x);
    ++snapshot_.revision;
}

void RolfTourSession::handle_host(const EclRequest& request)
{
    if (town_ && snapshot_.tour_finished && handle_town_host(request)) return;
    EclHostReply reply;
    const auto opcode = request.instruction->opcode;
    switch (opcode) {
    case 12:
        if (request.arguments[0].value != 12 || request.arguments[1].value != 2 || request.arguments[2].value != 9)
            throw EclError("Unsupported encounter setup in Rolf tour");
        snapshot_.sprite_frame = 2; break;
    case 13: if (snapshot_.sprite_frame > 0) --snapshot_.sprite_frame; break;
    case 14:
        if (request.arguments[0].value != 255) throw EclError("Tour requested an unsupported picture");
        snapshot_.sprite_frame = -1; break;
    case 49: snapshot_.sprite_frame = -1; break;
    case 58:
        remaining_delay_ = 0.22; delayed_request_ = request.id; ++snapshot_.revision; return;
    case 45: {
        const auto service = request.arguments[0].value;
        if (service == 0x2C90) {
            const auto x = machine_.variable(0xC04B), y = machine_.variable(0xC04C), f = machine_.variable(0xC04D);
            if (x >= 16 || y >= 16 || f >= 4) throw EclError("Invalid redraw pose");
            const auto& cell = map_.at(x, y);
            reply.writes = {{0xC04E, cell.walls[f]}, {0xC04F, cell.event_raw}};
            ++snapshot_.redraws;
        } else if (service == 0xBA03) {
            if (machine_.variable(0x03DE) != 8) throw EclError("Unsupported tour sound selector");
            ++snapshot_.footsteps; // Presentation plays an original OpenGold footstep cue.
        } else throw EclError("Unsupported native service in tour: " + std::to_string(service));
        break;
    }
    default: throw EclError("Unsupported tour host request");
    }
    if (!machine_.resume_host(request.id, reply)) throw EclError("Tour host reply rejected");
    if (opcode == 45 && request.arguments[0].value == 0x2C90) publish_pose();
    ++snapshot_.revision;
}

void RolfTourSession::advance(double seconds)
{
    if (snapshot_.phase != TourPhase::running) return;
    if (!std::isfinite(seconds) || seconds < 0) return;
    try {
        if (delayed_request_) {
            remaining_delay_ -= std::min(seconds, 1.0);
            if (remaining_delay_ > 0) return;
            if (!machine_.resume_host(delayed_request_, {})) throw EclError("Delayed reply rejected");
            delayed_request_ = 0;
        }
        for (unsigned requests = 0; requests < 64; ++requests) {
            const auto result = machine_.run(256);
            if (result.state == EclState::faulted) { fail(result.diagnostic); return; }
            if (result.state == EclState::completed) {
                if (town_ && snapshot_.tour_finished) { finish_event(); return; }
                snapshot_.phase = TourPhase::completed;
                snapshot_.tour_finished = true; publish_pose(); return;
            }
            if (!result.request) return; // Instruction budget yield.
            const auto& request = *result.request;
            if (request.kind == EclRequestKind::text) {
                if (request.clear) snapshot_.dialogue.clear();
                snapshot_.dialogue += request.text;
                if (snapshot_.dialogue.size() > 8192) throw EclError("Tour dialogue exceeds limit");
                if (!machine_.resume(request.id)) throw EclError("Text acknowledgement rejected");
                ++snapshot_.revision;
            } else if (request.kind == EclRequestKind::menu) {
                if(request.vertical && !request.text.empty())snapshot_.dialogue += "\n"+request.text;
                snapshot_.choices = request.choices;
                menu_request_ = request.id;
                snapshot_.continue_ticket = ++next_ticket_;
                snapshot_.phase = TourPhase::awaiting_continue;
                ++snapshot_.prompts; ++snapshot_.revision; return;
            } else if (request.kind == EclRequestKind::host) {
                handle_host(request);
                if (delayed_request_ || snapshot_.phase != TourPhase::running) return;
            } else if (request.kind == EclRequestKind::input_number || request.kind == EclRequestKind::input_string) {
                menu_request_ = request.id; snapshot_.continue_ticket = ++next_ticket_;
                snapshot_.number_input = request.kind == EclRequestKind::input_number;
                snapshot_.phase = TourPhase::awaiting_input; ++snapshot_.revision; return;
            } else throw EclError("Unexpected input request");
        }
    } catch (const EclError& error) { fail(error.what()); }
}

bool RolfTourSession::continue_dialogue(std::uint64_t ticket)
{
    return choose(ticket, 0);
}

bool RolfTourSession::explore(ExplorationCommand command)
{
    if (snapshot_.phase != TourPhase::completed) return false;
    if (town_) {
        if (command == ExplorationCommand::forward) {
            pending_movement_ = command; begin_event(0);
        } else if (command == ExplorationCommand::look) begin_event(1);
        else if (command == ExplorationCommand::camp) begin_event(2);
        else { move_party(command); begin_event(1); }
        advance(0); return true;
    }
    auto pose = snapshot_.pose;
    if (command == ExplorationCommand::turn_left) pose.facing = (pose.facing + 3) % 4;
    else if (command == ExplorationCommand::turn_right) pose.facing = (pose.facing + 1) % 4;
    else {
        const auto& edge = map_.at(pose.x, pose.y);
        const int x = static_cast<int>(pose.x) + dx[pose.facing], y = static_cast<int>(pose.y) + dy[pose.facing];
        if (x < 0 || y < 0 || x >= 16 || y >= 16 || edge.walls[pose.facing] || edge.doors[pose.facing]) return false;
        const auto& other = map_.at(x, y);
        const auto reverse = (pose.facing + 2) % 4;
        if (other.walls[reverse] || other.doors[reverse]) return false;
        pose.x = x; pose.y = y;
    }
    machine_.bind_variable(0xC04B, pose.x); machine_.bind_variable(0xC04C, pose.y);
    machine_.bind_variable(0xC04D, pose.facing);
    const auto& cell = map_.at(pose.x, pose.y);
    machine_.bind_variable(0xC04E, cell.walls[pose.facing]); machine_.bind_variable(0xC04F, cell.event_raw);
    publish_pose(); return true;
}
}
