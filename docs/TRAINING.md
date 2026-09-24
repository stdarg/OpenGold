# Skill, tool and language grants

F02 is split into the [rules/persistence layer (#187)](https://github.com/stdarg/OpenGold/issues/187),
[creation controls (#188)](https://github.com/stdarg/OpenGold/issues/188), and
[completion of missing saved choices (#189)](https://github.com/stdarg/OpenGold/issues/189).
The parent [#29](https://github.com/stdarg/OpenGold/issues/29) remains open until
all three are delivered. This document describes the shared rules layer;
player-facing training selection is not yet integrated.

## First supported package

- All eighteen skills have their ordinary governing abilities and derived bonuses.
- Rogue chooses four skills from its SRD list and two proficient skills for
  Expertise. Its fixed Thieves' Tools and Thieves' Cant grants retain class sources.
- Criminal grants Sleight of Hand, Stealth and Thieves' Tools with background sources.
- Every character knows Common and chooses two distinct other standard languages.
  Rogue chooses one additional distinct language from the standard or rare tables.
  Known Common and Thieves' Cant are excluded from the additional selections.
- Partial selections remain explicitly incomplete. Unknown groups/options,
  duplicate selections within a group, excessive selections and Expertise without
  proficiency reject. Both sources of an overlapping class/background proficiency
  persist; overlap supplies no second bonus or automatic replacement choice.

Other class/background proficiency packages, feats (including Criminal's Alert),
starting equipment, higher-level Rogue features and campaign uses of skills remain
in their respective plan issues. A completed training selection means only that
the choices supported by this increment are filled, not that the class is complete.
The project target remains all twelve SRD classes.

The rules follow [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
pp. 8–9 (proficiency and skills), 20 (languages), 61–62 (Rogue), 83 (Criminal),
and the Expertise glossary entry. Proficiency contributes once; Expertise doubles
it once. A check involving both a proficient skill and a proficient tool gains
Advantage without adding proficiency again. Tool checks can use the governing
ability required by the situation. The shared query reports the numeric modifier,
Expertise, tool-derived Advantage and all contributing sources; it does not roll
dice, advance time, consume resources or resolve campaign interactions.

## Data and validation

`CharacterDraft::training` records named choice groups and ordered selections.
`CharacterRules::training_options` supplies valid options and required counts.
The implementation uses the F01 `FeatureGrant` records with `skill:`, `tool:`,
`expertise:` and `language:` IDs, distinct source IDs and acquisition levels.
`CharacterSheet::training` contains derived skill totals, known tools/languages,
their sources and a completeness flag. Advancement refreshes the derived totals
when an ability modifier changes. `CharacterRules::ability_check` calculates a
check from validated grants and scores rather than trusting cached display totals.

Rules stay in the C++20 SRD library. Core serialization stores the records and
asks the module to validate its older grant format; Core does not interpret SRD
skill names, proficiency math or grant prefixes. There is no new runtime or UI stack.

## Persistence and pending choices

Rules **0.6.9** introduced **PC7** character recipes and campaign format **9** to
carry training provenance. The campaign stores the original selections as well as the acquired
grant records, which must agree on load. PC7 validates grant entitlements and
choices before creating an actor. PC8 additionally validates the Dwarf resistance
grant, with no training-choice changes. Current rules **0.6.15** use **OGCOMBAT 12**, SRD4 for spent Hit Dice and
SRD5 for [mortality recovery clocks](RECOVERY-CLOCKS.md); [rest resources](REST-RESOURCES.md) describes
that extension. Training completion preserves those resources too.

Per the user's approved policy, older campaign saves keep missing choices pending.
Only deterministic fixed grants are reconstructed; optional skills, Expertise and
languages are not invented. Version 8's feature/feat ledger is validated against
its original scope before adding training records. Formats 1–7 continue their
existing reconstruction. Pending choices survive another save/load unchanged.
The Review Training button remains tracked in #189, pending layout confirmation.

The campaign completion API is implemented for that flow. `preview_training`
returns an owned candidate with all supported pending choices filled. It
reconstructs creation and replays existing advancement choices, retaining the
appearance and inventory. Previously selected entries cannot be replaced, and
characters with completed training cannot use this operation to change it.
`complete_training` applies only a valid candidate. Incomplete/invalid choices,
unknown members and both operations during combat reject without changing the
campaign. Discarding a preview requires no rollback because it never edits the
live party. Confirmation preserves the original wounds, death state, effects,
spent resources, gear, XP, clock, RNG and rest eligibility; reserve members can
also complete pending selections. This API adds no save-format change.

Module 0.6.8 combat checkpoints and PC6 recipes retain their original effects,
resources, RNG and recipes. Older supported combat migrations also remain available.
Combat restoration does not manufacture training choices. Wounds, spent resources,
existing feats, equipment and turn state are unchanged by this increment.

## Verification

`opengold_training_tests` verifies the Rogue/Criminal package, all ordinary skill
bonuses, source overlap, Expertise eligibility, proficiency-table boundaries,
tool/skill Advantage, language permissions, incomplete selections and invalid
choices. Campaign tests check exact round trips and transactional rejection;
combat tests retain source records and reject forged ones. Frozen 0.6.8 files
verify pending choices, existing resources and exact unchanged combat state.
Completion checks use those migrated characters at levels one and four, including
an equipped Fighter with spent Second Wind and an active effect, a Wizard with
spent spell slots, a reserve Rogue, and an unconscious character. They verify
preview isolation, rejected-operation atomicity, preserved prior selections,
advancement replay and campaign/next-combat persistence.
Existing advancement, feature-grant, character, party, save and combat regressions
remain required. The UI children carry their own rendered integration checks.

SRD6 adds [sourced Temporary HP](TEMPORARY-HP.md); completing pending training
preserves that pool and all prior vital continuation.

Rules 0.6.15 also persists Orc Adrenaline Rush uses and pending Temporary HP
replacement in combat format 12 and SRD7; see [Temporary HP](TEMPORARY-HP.md).
