#ifndef OPENGOLD_COMBAT_BODY_CATALOG_H
#define OPENGOLD_COMBAT_BODY_CATALOG_H

#include <array>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace opengold::por {
struct CombatLookOption {
    std::string id, label;
    int original_type{};
    bool silver{};
};
struct CombatEquipment {
    int original_type{-1};
    std::string name, definition_id;
};
// Body IDs and weapon labels are data, not compiled-in classifications of the art.
struct CombatBodyCatalog {
    std::array<std::string,32> bodies{};
    std::vector<CombatLookOption> options;
    [[nodiscard]] static CombatBodyCatalog load(const std::filesystem::path& assignments,
        const std::filesystem::path& options_file);
    [[nodiscard]] unsigned choose(std::span<const CombatEquipment> equipped,unsigned fallback) const;
};
}
#endif
