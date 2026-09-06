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
    const auto bytes = read_archive(resolve_archive(directory, "SPRIT3.DAX"));
    std::array<opengold::Image, 3> sprites;
    for (std::uint8_t i = 0; i < sprites.size(); ++i) {
        auto result = decode_ega_sprite(bytes, 12, i);
        if (!result) throw EclError("Cannot decode Rolf encounter distance image " + std::to_string(i));
        sprites[i] = std::move(result.image);
    }
    return RolfTourSession(map->get(), program, std::move(sprites), 0xB071);
}

RolfTourSession::RolfTourSession(GeoMap map, std::shared_ptr<const EclProgram> program,
    std::array<opengold::Image, 3> sprites, std::uint32_t entry)
    : map_(std::move(map)), program_(std::move(program)), sprites_(std::move(sprites)),
      machine_(program_), entry_(entry)
{
    restart();
}

void RolfTourSession::restart()
{
    const auto revision = snapshot_.revision + 1;
    machine_ = EclMachine(program_);
    snapshot_ = {}; snapshot_.revision = revision;
    menu_request_ = delayed_request_ = 0; remaining_delay_ = 0;
    // Minimal explicit isolated state, not a fabricated whole party/world model.
    for (const std::uint16_t address : {0x03DE, 0x49C9, 0x49FD, 0x4AC5, 0x4A07, 0x4A0F,
            0x4A10, 0x4A11, 0x6DE1, 0x6E79, 0x6E7A, 0x6E7B, 0x6E7C, 0x6E7D,
            0x9801, 0xC04B, 0xC04C, 0xC04D, 0xC04E, 0xC04F})
        machine_.bind_variable(address, 0);
    machine_.bind_variable(0x49C9, 12); // Research fixture: midday.
    for (const std::uint8_t opcode : {12, 13, 14, 45, 49, 58}) machine_.enable_host(opcode);
    if (!machine_.start_at_for_inspection(entry_)) fail("Invalid isolated tour entry");
}

void RolfTourSession::fail(std::string diagnostic)
{
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
                snapshot_.phase = TourPhase::completed; publish_pose(); return;
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
                if (request.choices.size() != 1) throw EclError("Unexpected branching menu in Rolf tour");
                menu_request_ = request.id;
                snapshot_.continue_ticket = ++next_ticket_;
                snapshot_.phase = TourPhase::awaiting_continue;
                ++snapshot_.prompts; ++snapshot_.revision; return;
            } else if (request.kind == EclRequestKind::host) {
                handle_host(request);
                if (delayed_request_) return;
            } else throw EclError("Unexpected input request in Rolf tour");
        }
    } catch (const EclError& error) { fail(error.what()); }
}

bool RolfTourSession::continue_dialogue(std::uint64_t ticket)
{
    if (snapshot_.phase != TourPhase::awaiting_continue || ticket != snapshot_.continue_ticket) return false;
    if (!machine_.resume(menu_request_, 0)) return false;
    snapshot_.continue_ticket = 0; snapshot_.phase = TourPhase::running;
    ++snapshot_.revision; return true;
}

bool RolfTourSession::explore(ExplorationCommand command)
{
    if (snapshot_.phase != TourPhase::completed) return false;
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
