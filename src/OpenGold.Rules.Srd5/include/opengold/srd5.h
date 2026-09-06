#ifndef OPENGOLD_SRD5_H
#define OPENGOLD_SRD5_H
#include "opengold/rules.h"
#include <filesystem>
namespace opengold::srd5 {
// Reads a pinned, curated content pack. Unknown definitions/mechanics fail.
[[nodiscard]] std::unique_ptr<rules::RulesModule> load(const std::filesystem::path& content_pack);
// Pure SRD arithmetic, also used by the deterministic combat resolver.
[[nodiscard]] int ability_modifier(int score) noexcept;
[[nodiscard]] bool attack_hits(int natural_roll, int bonus, int armor_class) noexcept;
}
#endif
