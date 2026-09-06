#ifndef OPENGOLD_ECL_MACHINE_H
#define OPENGOLD_ECL_MACHINE_H
#include "opengold/ecl.h"
#include <filesystem>
#include <compare>
#include <map>
#include <memory>
#include <optional>

namespace opengold::por {
struct ScriptId {
    std::string archive;
    std::uint8_t record{};
    auto operator<=>(const ScriptId&) const = default;
};
class EclCatalog {
public:
    [[nodiscard]] static EclCatalog load(const std::filesystem::path& directory);
    [[nodiscard]] const auto& all() const noexcept { return programs_; }
    [[nodiscard]] std::shared_ptr<const EclProgram> find(const ScriptId& id) const;
private:
    std::map<ScriptId, std::shared_ptr<const EclProgram>> programs_;
};

enum class EclState { idle, running, waiting, completed, faulted };
enum class EclRequestKind { text, menu };
struct EclRequest {
    std::uint64_t id{};
    EclRequestKind kind{};
    std::string text; // Text to display, or vertical-menu header/delay text operand.
    bool clear{}, vertical{};
    std::vector<std::string> choices;
};
struct EclRunResult {
    EclState state{};
    std::size_t instructions{}; // Running means the caller's instruction budget was exhausted.
    std::optional<EclRequest> request;
    std::string diagnostic;
};

// Research execution profile: unsigned 16-bit logical variables explicitly bound
// by the caller. No inferred DOS engine-memory mapping. Game operations not yet
// implemented fault, never act as no-ops. See docs/SCRIPTS.md for support limits.
class EclMachine {
public:
    explicit EclMachine(std::shared_ptr<const EclProgram> program);
    // Variables persist across invocations; binding is allowed only while idle/completed.
    void bind_variable(std::uint16_t address, std::uint16_t value);
    [[nodiscard]] std::uint16_t variable(std::uint16_t address) const;
    bool start(std::size_t entry_slot);
    [[nodiscard]] EclRunResult run(std::size_t budget = 1000);
    // Text expects no choice; menus expect a zero-based UI index. VM stores index+1.
    // Invalid/stale replies do not change machine state or spend the pending request.
    bool resume(std::uint64_t request_id, std::optional<std::size_t> choice = std::nullopt);
    [[nodiscard]] EclState state() const noexcept { return state_; }
    [[nodiscard]] std::uint32_t address() const noexcept { return pc_; }
    [[nodiscard]] const auto& trace() const noexcept { return trace_; }
private:
    std::shared_ptr<const EclProgram> program_;
    EclState state_{EclState::idle};
    std::uint32_t pc_{};
    std::map<std::uint16_t, std::uint16_t> variables_;
    std::map<std::uint32_t, std::uint32_t> instruction_spans_;
    std::vector<std::uint32_t> stack_, trace_;
    std::optional<int> comparison_;
    std::optional<EclRequest> pending_;
    std::optional<std::uint16_t> destination_;
    std::uint64_t next_request_{1}, total_instructions_{};
    std::string diagnostic_;
    [[nodiscard]] EclInstruction decode(std::uint32_t address);
    [[nodiscard]] std::uint16_t value(const EclOperand& arg) const;
    [[nodiscard]] std::uint16_t destination(const EclOperand& arg) const;
    [[nodiscard]] std::string text(const EclOperand& arg) const;
    void jump(std::uint32_t address);
    void execute(const EclInstruction& instruction);
};
}
#endif
