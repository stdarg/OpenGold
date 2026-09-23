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

## Audit finding status

Status refers to the current implementation. The audit retains its original
commit-specific findings as historical evidence.

| Finding | Status | Primary work |
| --- | --- | --- |
| E1: Constitution/HP history | Open | [I03 #22](https://github.com/stdarg/OpenGold/issues/22) |
| E2: initial death save | Fixed | [I02 #21](https://github.com/stdarg/OpenGold/issues/21) |
| E3: stabilization counters | Fixed | [I02 #21](https://github.com/stdarg/OpenGold/issues/21) |
| E4: bonus provenance | Open | [I04 #23](https://github.com/stdarg/OpenGold/issues/23) |
| E5: Rogue/Monk weapon proficiency | Fixed for implemented weapons | [I01 #20](https://github.com/stdarg/OpenGold/issues/20) |
| G1: complete class features/advancement | Open; all twelve required | [Level-four milestone #8](https://github.com/stdarg/OpenGold/issues/8) and its class issues |
| G2: species/background grants | Open | [Species #51](https://github.com/stdarg/OpenGold/issues/51), [backgrounds #50](https://github.com/stdarg/OpenGold/issues/50) |
| G3: feat entitlements/choices | Open | [Feats #49](https://github.com/stdarg/OpenGold/issues/49) |
| G4: spell access/learning/preparation | Open | [F07 #36](https://github.com/stdarg/OpenGold/issues/36) and class-specific integration |
| G5: spells/shared casting mechanics | Open | [Spell inventory #165](https://github.com/stdarg/OpenGold/issues/165) and named spell/mechanic issues |
| G6: full progression/multiclassing | Open | [Higher levels #176](https://github.com/stdarg/OpenGold/issues/176), [multiclassing #179](https://github.com/stdarg/OpenGold/issues/179) and successors |
| G7: rests/recharge | Open | [F03 #30](https://github.com/stdarg/OpenGold/issues/30) |
| G8: death/recovery outside combat | Open | [F04 #31](https://github.com/stdarg/OpenGold/issues/31) |
| G9: equipment/properties | Open; E5 correction delivered | [Equipment #48](https://github.com/stdarg/OpenGold/issues/48), [Versatile I08 #27](https://github.com/stdarg/OpenGold/issues/27) |
| G10: conditions/effects/creature state | Open | [Conditions #35](https://github.com/stdarg/OpenGold/issues/35), [damage #33](https://github.com/stdarg/OpenGold/issues/33), [size #44](https://github.com/stdarg/OpenGold/issues/44) |
| G11: precise capability/documentation coverage | In progress: ledger and issue index established | Update with every increment; reconcile remaining support claims at [final closure #185](https://github.com/stdarg/OpenGold/issues/185) |
| G12: turn/facing/transit policies | SRD changes approved; implementation open | [I05 #24](https://github.com/stdarg/OpenGold/issues/24), [I06 #25](https://github.com/stdarg/OpenGold/issues/25), [I07 #26](https://github.com/stdarg/OpenGold/issues/26) |

Each subsequent delivery adds a feature row with the SRD source, actual class/
level/options supported, code/tests, save version and any reviewed adaptation.
Do not mark a tracking issue complete solely because its first child works.

SRD-derived rules use the attribution in
[NOTICE](../data/rules/srd-5.2.1/NOTICE.md); see the
[official SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).
