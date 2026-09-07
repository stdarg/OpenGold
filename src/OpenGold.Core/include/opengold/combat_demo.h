#ifndef OPENGOLD_COMBAT_DEMO_H
#define OPENGOLD_COMBAT_DEMO_H
#include "opengold/rules.h"
#include "opengold/ecl_machine.h"
#include "opengold/creature_catalog.h"
#include "opengold/formats.h"
#include "opengold/campaign_party.h"
namespace opengold {
struct CombatArt { rules::EntityId entity{}; Image image; };
// A bounded demonstration/campaign adapter. It depends on the rules interface,
// never on a specific edition. The application supplies the selected module.
class CombatDemo {
public:
    explicit CombatDemo(std::unique_ptr<rules::RulesModule> module);
    ~CombatDemo();
    void campaign_party(std::shared_ptr<CampaignParty> party);
    void training(std::uint64_t seed=42);
    void slums(const std::filesystem::path& game_directory,std::uint64_t seed=42);
    [[nodiscard]] const rules::CombatSession& combat() const;
    bool submit(const rules::Command& command);
    void continue_script();
    void revisit();
    [[nodiscard]] std::string save_combat() const;
    void restore_combat(std::string_view checkpoint);
    [[nodiscard]] const std::string& dialogue() const noexcept{return dialogue_;}
    [[nodiscard]] const std::string& status() const noexcept{return status_;}
    [[nodiscard]] bool waiting() const noexcept{return menu_ticket_!=0;}
    [[nodiscard]] bool has_combat() const noexcept{return combat_!=nullptr;}
    [[nodiscard]] bool is_slums() const noexcept{return vm_.has_value();}
    [[nodiscard]] bool script_complete() const noexcept{return vm_&&vm_->state()==por::EclState::completed;}
    [[nodiscard]] unsigned script_variable(std::uint16_t address) const;
    [[nodiscard]] const auto& art() const noexcept{return art_;}
private:
    std::unique_ptr<rules::RulesModule> module_;
    std::unique_ptr<rules::CombatSession> combat_;
    std::shared_ptr<CampaignParty> campaign_;
    bool owns_campaign_combat_{};
    void start_encounter(std::vector<rules::Participant> enemies);
    void synchronize_party();
    std::optional<por::EclMachine> vm_;
    std::optional<por::CreatureCatalog> creatures_;
    std::vector<rules::Participant> enemies_;
    std::vector<CombatArt> art_;
    std::filesystem::path game_directory_;
    std::string dialogue_,status_;
    std::uint64_t menu_ticket_{},combat_ticket_{},seed_{};
    unsigned encounters_{};
    void pump();
    void finish_combat();
};
// Demonstration AI consumes only public state/commands. No rolls or damage here.
[[nodiscard]] rules::Command choose_demo_command(const rules::CombatSession& session);
}
#endif
