#include "opengold/campaign_save.h"
#include "opengold/srd5.h"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace opengold;
using namespace opengold::rules;
namespace {
const auto root = std::filesystem::path(OPENGOLD_SOURCE_DIR);
const auto fixtures = root / "tests/fixtures";
constexpr std::array weapons{"dagger", "handaxe", "javelin", "light_hammer", "spear", "dart", "trident"};
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
auto module() { return srd5::load(root / "data/rules/srd-5.2.1/combat.rules"); }
std::string read(const char* name) {
    std::ifstream in(fixtures / name);
    check(bool(in), "Open historical thrown fixture");
    return {std::istreambuf_iterator<char>(in), {}};
}
void write(const char* name, const std::string& bytes) {
    std::ofstream out(fixtures / name);
    out << bytes;
    check(bool(out), "Write actual prior-writer fixture");
}
Command command(const CombatSession& combat, std::string_view verb, EntityId target = 0) {
    for (const auto& option : combat.legal_commands())
        if (option.verb == verb && (!target || option.target == target)) return option;
    throw std::runtime_error("Missing baseline command: " + std::string(verb));
}
void act(CombatSession& combat, std::string_view verb, EntityId target = 0) {
    check(combat.submit(command(combat, verb, target)), "Accept baseline legal command");
}
Character hero() {
    CharacterDraft draft;
    draft.race = "human"; draft.gender = "female"; draft.character_class = "fighter";
    draft.background = "soldier"; draft.alignment = "neutral_good"; draft.name = "Thrown baseline";
    draft.rolled = true;
    for (auto& roll : draft.rolls) roll = {{6, 5, 4, 1}, 3};
    return Character(*srd5::character_rules(), draft, {});
}
std::string current_identity(std::string bytes, const RulesModule& rules) {
    const auto position = bytes.find("0.6.46");
    check(position != bytes.npos, "Actual prior identity exists");
    bytes.replace(position, 6, rules.identity().version);
    return bytes;
}
void finish_second_throw(CombatSession& combat) {
    act(combat, "end"); // Decline the first critical's free movement.
    act(combat, "action_surge");
    act(combat, "ranged", 2);
    if (combat.snapshot().free_movement) act(combat, "end");
}
void capture_prior_writer() {
    auto rules = module();
    check(rules->identity().version == "0.6.46", "Capture only with the actual pre-Thrown writer");
    CampaignParty party(module());
    auto character = hero();
    for (const auto weapon : weapons) character.inventory().add(weapon, weapon, 3);
    const auto shield = character.inventory().add("shield", "Baseline shield");
    const auto id = party.add_pc(std::move(character));
    party.equip(id, 3); // Javelin in the ordered inventory above.
    party.equip(id, shield);
    party.award_experience(2700, "thrown-baseline");
    for (unsigned level = 2; level <= 4; ++level) party.advance(id, party.default_advancement(id));
    auto state = party.checkpoint();
    state.roster[0].vitals.hit_points -= 3;
    state.roster[0].wealth[3] = 37;
    state.time_minutes = 123; state.subminute_milliseconds = 456; state.random_state = 789;
    party.restore(state);
    write("campaign-v11-thrown-before.ogs", encode_campaign(party, nullptr, "thrown-before"));

    bool captured = false;
    for (unsigned seed = 0; seed < 400 && !captured; ++seed) {
        auto actors = party.participants();
        actors[0].cell = {1, 1};
        actors[0].ground_equipment = {1}; // Actual ground-item ledger; shield is already dropped.
        actors.push_back({2, "vanguard", "Target", 1, {5, 1}});
        actors.push_back({3, "vanguard", "Reserve", 1, {7, 7}});
        auto combat = rules->create({{10, 8, std::vector<std::uint8_t>(80)}, actors}, seed);
        while (combat->snapshot().actor != id) act(*combat, "end");
        act(*combat, "second_wind");
        const auto before = combat->save();
        // Capture continuation from an actual reload, including the old codec's
        // implicit format-16 movement flag, rather than unsaved constructor state.
        combat = rules->restore(before);
        check(combat->save() == before, "Prior reload is canonical before capture");
        act(*combat, "ranged", 2);
        const auto hit = combat->snapshot().savage_attack_choice;
        if (!hit || !hit->critical) continue;
        const auto pending = combat->save();
        act(*combat, "savage_use"); act(*combat, "savage_second");
        check(bool(combat->snapshot().free_movement), "Real thrown critical opens Champion movement");
        write("combat-v16-thrown-before.save", before);
        write("combat-v16-thrown-choice.save", pending);
        write("combat-v18-thrown-free.save", combat->save());
        finish_second_throw(*combat);
        write("combat-v16-thrown-continued.save", combat->save());
        captured = true;
    }
    check(captured, "Capture actual critical thrown attack sequence");
}
void prior_writer_continuation() {
    auto rules = module();
    auto combat = rules->restore(read("combat-v16-thrown-before.save"));
    check(combat->save() == current_identity(read("combat-v16-thrown-before.save"), *rules),
          "Prior held/ground items, spent Bonus Action and RNG round trip exactly");
    act(*combat, "ranged", 2);
    check(combat->save() == current_identity(read("combat-v16-thrown-choice.save"), *rules),
          "Prior thrown attack preserves pending Savage dice and resources");
    auto pending = rules->restore(combat->save());
    for (auto* session : {combat.get(), pending.get()}) {
        act(*session, "savage_use"); act(*session, "savage_second");
    }
    check(combat->save() == pending->save() &&
          combat->save() == current_identity(read("combat-v18-thrown-free.save"), *rules),
          "Prior critical preserves accepted damage and immediate Champion phase");
    auto free_move = rules->restore(combat->save());
    for (auto* session : {combat.get(), pending.get(), free_move.get()}) finish_second_throw(*session);
    check(combat->save() == pending->save() && combat->save() == free_move->save() &&
          combat->save() == current_identity(read("combat-v16-thrown-continued.save"), *rules),
          "Prior writer's second throw continues byte-exactly without changing its inventory semantics");

    const auto old = read("campaign-v11-thrown-before.ogs");
    CampaignParty party(module());
    party.restore(decode_campaign(old, *srd5::character_rules(), *rules, "thrown-before", nullptr).party);
    const auto& member = party.member(1);
    check(member.character.sheet().level == 4 && member.wealth[3] == 37 &&
          member.equipped == std::vector<std::uint64_t>{3, 8}, "Prior campaign retains level, wealth and equipped identities");
    for (unsigned index = 0; index < weapons.size(); ++index) {
        const auto item = member.character.inventory().find(index + 1);
        check(item && item->get().definition_id == weapons[index] && item->get().quantity == 3,
              "Each actual prior carried/held thrown stack retains its quantity and identity");
    }
    const auto saved = encode_campaign(party, nullptr, "thrown-before");
    const auto body = [](const std::string& bytes) { return bytes.substr(bytes.find('\n', bytes.find('\n') + 1) + 1); };
    check(body(saved) == body(current_identity(old, *rules)), "Prior campaign body changes only module identity");
    CampaignParty copy(module());
    copy.restore(decode_campaign(saved, *srd5::character_rules(), *rules, "thrown-before", nullptr).party);
    check(encode_campaign(copy, nullptr, "thrown-before") == saved, "Campaign migration is canonical");
}
}
int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string_view(argv[1]) == "--capture-prior-writer") capture_prior_writer();
        else { check(argc == 1, "Unknown test argument"); prior_writer_continuation(); }
        std::cout << "Thrown prior-writer checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
