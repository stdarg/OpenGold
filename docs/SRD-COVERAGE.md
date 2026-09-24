# SRD coverage ledger

This ledger tracks the [audit](audits/srd-5.2.1-rules.md) against the
[incremental plan](SRD-IMPLEMENTATION.md). The
[GitHub issue index](https://github.com/stdarg/OpenGold/issues/186) links every
planned increment and recurring work queue. All use `SRD_improvements`.
The all-twelve-class level-four milestone is
[#8](https://github.com/stdarg/OpenGold/issues/8).

## Delivered increments

| Increment / feature | Authority and supported scope | Implementation and verification | Persistence |
| --- | --- | --- | --- |
| [I01 / E5](https://github.com/stdarg/OpenGold/issues/20): starting-class weapon proficiency | SRD 5.2.1 pp. 49, 61, 91. Rogue gains martial Finesse or Light; Monk gains martial Light. Applies to the implemented catalog, including Shortsword/Scimitar. Other starting-class grants remain intact. | [weapons.h](../src/OpenGold.Rules.Srd5/src/weapons.h) records Light; [srd5.cpp](../src/OpenGold.Rules.Srd5/src/srd5.cpp) uses the same training query for profiles, equipment notes and combat. [party_tests.cpp](../tests/party_tests.cpp), `class_weapon_proficiency`, verifies all twelve classes, actual +5 attacks for Dexterity-16 Rogue/Monk, equipment explanations and deterministic campaign/combat reload. The regression failed before the fix. | Module 0.6.1; campaign schema unchanged. Existing 0.6.0 and earlier supported campaigns reconstruct the corrected bonuses without losing equipment or wounds. [save_tests.cpp](../tests/save_tests.cpp) verifies 0.6.0 campaign upgrade. Standalone combat checkpoints require exact module identity. |
| [I02 / E2–E3](https://github.com/stdarg/OpenGold/issues/21): death-save turn entry and stabilization | SRD 5.2.1 pp. 17–18. An unstable actor at 0 HP rolls once on turn entry, including the initial initiative slot. A natural 20 permits the recovered actor's turn. Stabilization clears both counters. | [srd5.cpp](../src/OpenGold.Rules.Srd5/src/srd5.cpp) shares one turn-entry path. [rules_tests.cpp](../tests/rules_tests.cpp), `death_save_turn_entry_tests`, covers initial/later entry, all four outcomes, stable/dead skipping and deterministic checkpoint continuation. The regression failed before the fix. [party_tests.cpp](../tests/party_tests.cpp), `stabilization_handoff`, verifies campaign handoff and reload with spent resources preserved. | Module 0.6.2; campaign schema unchanged. Supports 0.6.1 and earlier supported campaigns. Legacy stable counters normalize when rules hydrate vitals. [save_tests.cpp](../tests/save_tests.cpp) verifies 0.6.0/0.6.1 campaign upgrades. Checkpoint restoration performs no turn-entry roll; standalone combat checkpoints still require exact module identity. |
| [I03 / E1](https://github.com/stdarg/OpenGold/issues/22): Constitution and HP history | SRD 5.2.1 p. 23. Gain fixed HP with the previous modifier before applying the Constitution increase per attained level. Prior minimum-one gains persist; Dwarven Toughness remains additive. | [character_rules.cpp](../src/OpenGold.Rules.Srd5/src/character_rules.cpp) initializes modifier history; [srd5.cpp](../src/OpenGold.Rules.Srd5/src/srd5.cpp) replays and validates it for advancement and combat profiles. [advancement_tests.cpp](../tests/advancement_tests.cpp), `hp_history`, verifies the 9-HP Wizard, odd/even modifier boundaries, Dwarves, wounds, unconsciousness, resource expenditure and campaign/combat reconstruction. The regression failed before the fix. | Module 0.6.3 and PC4 combat profiles; campaign format remains 6. Existing saved choices reconstruct history. A rules-owned migration preserves living HP deficits and zero-HP/dead state. [save_tests.cpp](../tests/save_tests.cpp), `hp_migration`, uses a [frozen 0.6.2 save](../tests/fixtures/README.md) to verify correction and reload without double application. Prior-module standalone combat saves still reject. |
| [I04 / E4](https://github.com/stdarg/OpenGold/issues/23): ability-bonus provenance | SRD 5.2.1 pp. 23, 83, 87. Background allocations and level-four Ability Score Improvement are distinct grants. | [character_rules.h](../src/OpenGold.Rules/include/opengold/character_rules.h) exposes source IDs, acquisition levels and individual amounts. Creation/advancement emit separate records; game and demo modifier dialogs render each contribution. [advancement_tests.cpp](../tests/advancement_tests.cpp), `ability_sources`, verifies Soldier +2 and feat +2, grants outside the background list, totals, preview/rejection and reload. Both Godot advancement checks verify actual dialog text and reconstruction; the new game regression failed before the fix. | Module 0.6.3, PC4 and campaign format 6 unchanged. Sources are derived by replaying existing saved choices, including the frozen prior-module fixture in [save_tests.cpp](../tests/save_tests.cpp). Numeric scores, vitals and combat continuation are unchanged. |
| [I05 / G12](https://github.com/stdarg/OpenGold/issues/24): remaining turn resources | SRD 5.2.1 pp. 13–14. Existing melee/ranged attacks and damaging spells preserve unused movement and Bonus Actions. Explicit End Turn advances initiative; AI uses available recovery and then ends. | [srd5.cpp](../src/OpenGold.Rules.Srd5/src/srd5.cpp) retains the turn after attacks/reaction resolution. [rules_tests.cpp](../tests/rules_tests.cpp), `turn_budget_tests`, covers all six offensive verbs, pre/post-attack movement, Second Wind, spell slots, spent-action rejection, ordered reactions and enemy completion. The regression failed before the fix. [party_tests.cpp](../tests/party_tests.cpp), `remaining_turn_handoff`, checks campaign time and spent recovery through completed combat and reload. [combat_view_tests.gd](../tests/combat_view_tests.gd) exercises actual ranged input, keyboard movement, End Turn and AI return. | Module 0.6.4; checkpoint 5 and campaign 6 unchanged. Post-attack/reaction checkpoints continue deterministically with no resource refund. Supported 0.6.3 and earlier campaigns migrate; prior-module combat checkpoints require the exact identity. |
| [I06 / G12](https://github.com/stdarg/OpenGold/issues/25): opportunity triggers and presentation facing | [SRD 5.2.1 p. 15](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf). Turning toward a weapon/spell target creates no reaction. Leave-reach movement still pauses before the step, respecting sight, Disengage and the reaction budget. Incapacitating the mover stops movement and the queue. | [srd5.cpp](../src/OpenGold.Rules.Srd5/src/srd5.cpp) removes runtime facing queues. [rules_tests.cpp](../tests/rules_tests.cpp) verifies left/right turns, party/enemy actors, every targeted offensive verb, ordered movement reactions, spent-reaction limits, interruption, atomic rejection and deterministic migration. The no-reaction regression failed before the fix. Existing status-effect tests verify Blinded visibility. [opportunity_view_tests.gd](../tests/opportunity_view_tests.gd) uses real Load/Save and keyboard controls to verify migrated facing, remaining movement and pending queues. | Module 0.6.5 / combat format 6. [Frozen 0.6.4 fixtures](../tests/fixtures/README.md) prove migration cancels only valid obsolete facing queues, invalidates their command tickets and preserves resources/time/RNG. Real movement retains partial queue position and matches the prior writer after resolution. Unrelated combat identities reject; supported campaigns through 0.6.4 upgrade with campaign format 6 unchanged. |
| [I07 / G12](https://github.com/stdarg/OpenGold/issues/26): allied transit | [SRD 5.2.1 p. 14](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf). Allied spaces are transit cells at normal terrain cost; every selected endpoint must be free. Hostile, size-dependent and general Incapacitated transit remain F13. | Shared [combat_grid.cpp](../src/OpenGold.Rules.Srd5/src/combat_grid.cpp) governs planning, execution and restore. The corridor regression failed before the fix. [combat_grid_tests.cpp](../tests/combat_grid_tests.cpp) checks normal/difficult allied costs, budget boundaries, walls and 9,686 exhaustive layouts. [rules_tests.cpp](../tests/rules_tests.cpp) covers both sides, successive allies, atomic occupied-stop rejection, shared-cell reactions, malformed overlap, knockdown, healing, death and natural-20 recovery. [party_tests.cpp](../tests/party_tests.cpp) verifies a real campaign fight and save/reload; the rendered Godot demo clicks across two allies and rejects an occupied arrow destination. | Module 0.6.6 / combat format 7 retains an involuntary-overlap marker so interrupted movement and later recovery reload correctly. Accepted paused routes validate their remaining budget and free endpoint. Supported 0.6.4/0.6.5 combat and campaign saves upgrade without refilling resources. Prone and size-dependent co-occupancy effects remain explicitly tracked in #35/#44; the marker is persistence state, not a claim that Prone is implemented. |

| [I08 / G9](https://github.com/stdarg/OpenGold/issues/27): Versatile grip | [SRD 5.2.1 pp. 90–91](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf). Quarterstaff/Spear use 1d6/1d8; Battleaxe/Longsword/Trident/Warhammer/War Pick use 1d8/1d10. Two hands affect melee only and require no shield. | Rules supply grip choices and validated commands; campaign inventory and Godot combat dropdowns persist selection. [versatile_tests.cpp](../tests/versatile_tests.cpp) checks all seven weapons, fixed normal/critical/miss dice, thrown attacks, atomic rejection, resource preservation, pending reactions and campaign handoff. [grip_view_tests.gd](../tests/grip_view_tests.gd) exercises layout, keyboard, dice and save/load; inventory integration checks cover shields and both grip choices. | Module 0.6.7 / PC5 / combat 8 / campaign 7 store selected hands separately from resources. [Frozen 0.6.6 files](../tests/fixtures/README.md) preserve the old four forced grips and other one-handed grips; migrated and new saves resume deterministically. Weapon/shield artwork remains the existing catalog; new poses and remaining equipment properties are outside I08. |

Light extra attacks, Monk Martial Arts, optional feature-granted proficiency,
multiclass-entry proficiency and missing catalog weapons remain their own
issues. Add Rapier and other new-weapon conformance examples with the catalog
increment. I01 does not establish complete Rogue or Monk support.

Validation for I01: rebuilt and passed the five native suites for party, save,
rules, character and content behavior. No Godot layout or control code changed;
equipment explanation checks exercise the shared rules output.

Validation for I02: rebuilt and passed the rules, party, save, status-effect and
advancement native suites. No Godot layout or control code changed. Death saves
and natural recovery outside combat remain [F04 #31](https://github.com/stdarg/OpenGold/issues/31);
this increment preserves the existing all-unconscious-party defeat policy.

Validation for I03: the rules, party, save, status-effect, advancement, character
and content native suites pass. The Godot GDExtension is rebuilt against the
updated shared character data. No UI layout or controls changed. Higher levels
and additional classes retain their separate advancement issues.

Validation for I04: advancement, save, character and party native suites pass;
game and demo Godot advancement checks pass with the installed original assets.
English/Spanish catalogs and the localization UI check pass. Existing dialogs
retain their layout and controls. The general feature/feat grant ledger remains
[F01 #28](https://github.com/stdarg/OpenGold/issues/28).

## Audit finding status

Status refers to the current implementation. The audit retains its original
commit-specific findings as historical evidence.

| Finding | Status | Primary work |
| --- | --- | --- |
| E1: Constitution/HP history | Fixed | [I03 #22](https://github.com/stdarg/OpenGold/issues/22) |
| E2: initial death save | Fixed | [I02 #21](https://github.com/stdarg/OpenGold/issues/21) |
| E3: stabilization counters | Fixed | [I02 #21](https://github.com/stdarg/OpenGold/issues/21) |
| E4: bonus provenance | Fixed | [I04 #23](https://github.com/stdarg/OpenGold/issues/23) |
| E5: Rogue/Monk weapon proficiency | Fixed for implemented weapons | [I01 #20](https://github.com/stdarg/OpenGold/issues/20) |
| G1: complete class features/advancement | Open; all twelve required | [Level-four milestone #8](https://github.com/stdarg/OpenGold/issues/8) and its class issues |
| G2: species/background grants | Open | [Species #51](https://github.com/stdarg/OpenGold/issues/51), [backgrounds #50](https://github.com/stdarg/OpenGold/issues/50) |
| G3: feat entitlements/choices | Open | [Feats #49](https://github.com/stdarg/OpenGold/issues/49) |
| G4: spell access/learning/preparation | Open | [F07 #36](https://github.com/stdarg/OpenGold/issues/36) and class-specific integration |
| G5: spells/shared casting mechanics | Open | [Spell inventory #165](https://github.com/stdarg/OpenGold/issues/165) and named spell/mechanic issues |
| G6: full progression/multiclassing | Open | [Higher levels #176](https://github.com/stdarg/OpenGold/issues/176), [multiclassing #179](https://github.com/stdarg/OpenGold/issues/179) and successors |
| G7: rests/recharge | Open | [F03 #30](https://github.com/stdarg/OpenGold/issues/30) |
| G8: death/recovery outside combat | Open | [F04 #31](https://github.com/stdarg/OpenGold/issues/31) |
| G9: equipment/properties | Open; E5 and Versatile corrections delivered | [Equipment #48](https://github.com/stdarg/OpenGold/issues/48), [Versatile I08 #27](https://github.com/stdarg/OpenGold/issues/27) |
| G10: conditions/effects/creature state | Open | [Conditions #35](https://github.com/stdarg/OpenGold/issues/35), [damage #33](https://github.com/stdarg/OpenGold/issues/33), [size #44](https://github.com/stdarg/OpenGold/issues/44) |
| G11: precise capability/documentation coverage | In progress: ledger and issue index established | Update with every increment; reconcile remaining support claims at [final closure #185](https://github.com/stdarg/OpenGold/issues/185) |
| G12: turn/facing/transit policies | I05–I07 delivered; remaining creature-state rules are tracked separately | [I05 #24](https://github.com/stdarg/OpenGold/issues/24), [I06 #25](https://github.com/stdarg/OpenGold/issues/25), [I07 #26](https://github.com/stdarg/OpenGold/issues/26) |

Each subsequent delivery adds a feature row with the SRD source, actual class/
level/options supported, code/tests, save version and any reviewed adaptation.
Do not mark a tracking issue complete solely because its first child works.

SRD-derived rules use the attribution in
[NOTICE](../data/rules/srd-5.2.1/NOTICE.md); see the
[official SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).
