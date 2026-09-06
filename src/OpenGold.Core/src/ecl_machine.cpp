#include "opengold/ecl_machine.h"
#include "opengold/formats.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace opengold::por {
EclCatalog EclCatalog::load(const std::filesystem::path& directory)
{
    try {
        std::map<std::string, std::filesystem::path> files;
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (!entry.is_regular_file()) continue;
            auto name = entry.path().filename().string();
            for (auto& c : name) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
            if (!name.starts_with("ECL") || !name.ends_with(".DAX")) continue;
            const auto digits = name.substr(3, name.size()-7);
            if (!std::all_of(digits.begin(), digits.end(), [](char c) { return c >= '0' && c <= '9'; })) continue;
            if (!files.emplace(name, entry.path()).second) throw EclError("Ambiguous script archive " + name);
        }
        EclCatalog catalog;
        for (const auto& [name, path] : files) {
            const auto size = std::filesystem::file_size(path);
            if (size > 32*1024*1024) throw EclError("Script archive too large: " + name);
            std::ifstream input(path, std::ios::binary);
            if (!input) throw EclError("Cannot open " + name);
            std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input), {}};
            if (input.bad() || size != bytes.size()) throw EclError("Cannot read " + name);
            auto dax = decode_dax_archive(bytes);
            if (!dax) throw EclError("Invalid DAX archive " + name);
            for (const auto& record : dax.records) {
                auto program = EclProgram::decode(record.bytes, name + ":" + std::to_string(record.id));
                catalog.programs_.emplace(ScriptId{name, record.id}, std::make_shared<const EclProgram>(std::move(program)));
            }
        }
        if (catalog.programs_.empty()) throw EclError("No ECL programs found in " + directory.string());
        return catalog;
    } catch (const std::filesystem::filesystem_error& e) { throw EclError(e.what()); }
}

std::shared_ptr<const EclProgram> EclCatalog::find(const ScriptId& id) const
{
    const auto found = programs_.find(id);
    return found == programs_.end() ? nullptr : found->second;
}

EclMachine::EclMachine(std::shared_ptr<const EclProgram> program) : program_(std::move(program))
{
    if (!program_) throw EclError("EclMachine requires a program");
}
void EclMachine::bind_variable(std::uint16_t address, std::uint16_t v)
{
    if (state_ != EclState::idle && state_ != EclState::completed) throw EclError("Cannot bind variables during execution");
    if (address >= EclProgram::origin && address < EclProgram::origin + program_->raw().size() - 2)
        throw EclError("Program addresses cannot be bound as variables");
    variables_[address] = v;
}
std::uint16_t EclMachine::variable(std::uint16_t address) const
{
    const auto it = variables_.find(address);
    if (it == variables_.end()) throw EclError("Unbound variable address " + std::to_string(address));
    return it->second;
}
std::uint16_t EclMachine::value(const EclOperand& a) const
{
    if (a.tag == 0 || a.tag == 2) return a.value;
    if (a.tag == 1 || a.tag == 3) return variable(a.value);
    throw EclError("Numeric operand required; string references are not implemented");
}
std::uint16_t EclMachine::destination(const EclOperand& a) const
{
    if (a.tag != 1 && a.tag != 3) throw EclError("Destination requires an address operand");
    (void)variable(a.value); // Validate before any mutation or request.
    return a.value;
}
std::string EclMachine::text(const EclOperand& a) const
{
    if (a.tag == 128) return a.text;
    return std::to_string(value(a));
}
EclInstruction EclMachine::decode(std::uint32_t address)
{
    if (address < program_->body_start()) throw EclError("Target points into the entry table");
    auto result = program_->instruction(address);
    for (const auto& [start, end] : instruction_spans_)
        if (start != address && address < end && result.next > start)
            throw EclError("Overlapping instruction or target inside operand data");
    instruction_spans_[address] = result.next;
    return result;
}
void EclMachine::jump(std::uint32_t address)
{
    (void)decode(address);
    pc_ = address;
}
bool EclMachine::start(std::size_t slot)
{
    if (slot >= program_->entries().size() || (state_ != EclState::idle && state_ != EclState::completed)) return false;
    pc_ = program_->entries()[slot];
    state_ = EclState::running;
    stack_.clear(); trace_.clear(); comparison_.reset(); pending_.reset(); destination_.reset();
    diagnostic_.clear(); total_instructions_ = 0;
    return true;
}

void EclMachine::execute(const EclInstruction& i)
{
    const auto& spec = ecl_opcode(i.opcode);
    if (!spec.executable) throw EclError("Unsupported execution: " + std::string(spec.name));
    const auto& a = i.operands;
    const auto branch_address = [](const EclOperand& arg) -> std::uint32_t {
        if (arg.tag >= 128) throw EclError("Branch requires encoded address");
        return arg.value; // Do not dereference address operands in branch roles.
    };
    switch (i.opcode) {
    case 0: state_ = EclState::completed; stack_.clear(); break;
    case 1: jump(branch_address(a[0])); return;
    case 2:
        if (stack_.size() >= 256) throw EclError("Subroutine stack limit exceeded");
        jump(branch_address(a[0])); stack_.push_back(i.next); return;
    case 19:
        if (stack_.empty()) throw EclError("RETURN without GOSUB");
        jump(stack_.back()); stack_.pop_back(); return;
    case 3: {
        if (a[0].tag == 128 && a[1].tag == 128) {
            comparison_ = a[0].text < a[1].text ? -1 : a[0].text > a[1].text ? 1 : 0;
        } else {
            const auto lhs = value(a[0]), rhs = value(a[1]);
            comparison_ = lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
        }
        break;
    }
    case 4: case 5: case 7: case 47: case 48: {
        const std::uint32_t lhs = value(a[0]), rhs = value(a[1]);
        const auto dest = destination(a[2]);
        std::uint32_t v = 0;
        if (i.opcode == 4) v = lhs + rhs;
        if (i.opcode == 5) v = rhs - lhs; // Reference SUBTRACT operand order.
        if (i.opcode == 7) v = lhs * rhs;
        if (i.opcode == 47) v = lhs & rhs;
        if (i.opcode == 48) v = lhs | rhs;
        variables_.at(dest) = static_cast<std::uint16_t>(v & 65535);
        break;
    }
    case 9: {
        const auto v = value(a[0]), dest = destination(a[1]);
        variables_.at(dest) = v; break;
    }
    case 17: case 18: {
        EclRequest request;
        request.kind = EclRequestKind::text; request.text = text(a[0]); request.clear = i.opcode == 18;
        request.id = next_request_++;
        pending_ = std::move(request); state_ = EclState::waiting; break;
    }
    case 21: case 43: {
        const auto dest = destination(a[0]);
        const auto count = a[spec.operands-1].value;
        if (!count) throw EclError("Empty menu is not supported");
        EclRequest request;
        request.kind = EclRequestKind::menu; request.vertical = i.opcode == 21;
        if (request.vertical) request.text = text(a[1]);
        for (std::size_t n = spec.operands; n < a.size(); ++n) request.choices.push_back(text(a[n]));
        request.id = next_request_++;
        pending_ = std::move(request); destination_ = dest; state_ = EclState::waiting; break;
    }
    case 22: case 23: case 24: case 25: case 26: case 27: {
        if (!comparison_) throw EclError("Conditional instruction before COMPARE");
        const int c = *comparison_;
        const std::array<bool, 6> pass{c == 0, c != 0, c < 0, c > 0, c <= 0, c >= 0};
        if (!pass[i.opcode-22]) { pc_ = decode(i.next).next; return; }
        break;
    }
    case 37: case 38: {
        const auto index = value(a[0]);
        if (index >= 1 && index <= a[1].value) {
            if (i.opcode == 38 && stack_.size() >= 256) throw EclError("Subroutine stack limit exceeded");
            jump(branch_address(a[1+index]));
            if (i.opcode == 38) stack_.push_back(i.next);
            return;
        }
        break; // One-based dispatch, out-of-range falls through in this profile.
    }
    default: throw EclError("Missing opcode implementation");
    }
    pc_ = i.next;
}

EclRunResult EclMachine::run(std::size_t budget)
{
    std::size_t count = 0;
    while (state_ == EclState::running && count < budget) {
        try {
            if (total_instructions_ >= 1000000) throw EclError("Invocation instruction limit exceeded");
            const auto instruction = decode(pc_);
            if (trace_.size() == 64) trace_.erase(trace_.begin());
            trace_.push_back(pc_);
            ++count; ++total_instructions_;
            execute(instruction);
        } catch (const EclError& error) {
            std::ostringstream message;
            message << program_->source() << " @ 0x" << std::hex << pc_
                    << " (record offset 0x" << (pc_ - EclProgram::origin + 2)
                    << ", stack depth " << std::dec << stack_.size() << "): " << error.what();
            diagnostic_ = message.str(); state_ = EclState::faulted;
        }
    }
    return {state_, count, pending_, diagnostic_};
}
bool EclMachine::resume(std::uint64_t id, std::optional<std::size_t> choice)
{
    if (state_ != EclState::waiting || !pending_ || pending_->id != id) return false;
    if (pending_->kind == EclRequestKind::menu) {
        if (!choice || *choice >= pending_->choices.size()) return false;
        variables_.at(*destination_) = static_cast<std::uint16_t>(*choice + 1);
    } else if (choice) return false;
    pending_.reset(); destination_.reset(); state_ = EclState::running;
    return true;
}
}
