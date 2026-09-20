#ifndef OPENGOLD_COMBAT_BODY_CATALOG_H
#define OPENGOLD_COMBAT_BODY_CATALOG_H

#include <array>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace opengold::por {
// The review tool and game share stable look IDs, independent of translated labels.
enum class CombatLook { unreviewed, unarmed, unarmed_shield, dagger, dagger_shield, mace, mace_shield,
    sword, sword_shield, staff, staff_shield, bow, bow_shield };
[[nodiscard]] std::string_view combat_look_id(CombatLook look);
[[nodiscard]] CombatLook parse_combat_look(std::string_view id);
struct CombatBodyCatalog {
    std::array<CombatLook,32> bodies{};
    [[nodiscard]] static CombatBodyCatalog load(const std::filesystem::path& file);
    [[nodiscard]] unsigned choose(std::span<const std::string> equipped,unsigned fallback) const;
};
}
#endif
