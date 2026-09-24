# Bard starting instrument proficiencies

Tracked in [#214](https://github.com/stdarg/OpenGold/issues/214), a child of
[Bard starting choices #125](https://github.com/stdarg/OpenGold/issues/125).
Rules 0.6.31 implements the starting proficiency choices; PC20 validates their grants.

SRD 5.2.1 Core Bard Traits (p. 31) grants three Musical Instrument
proficiencies. The instrument variants (p. 94) are bagpipes, drum, dulcimer,
flute, horn, lute, lyre, pan flute, shawm and viol. The proficiency entitlement
is separate from choosing/owning starting equipment. Multiclass entry grants
one instrument instead and remains outside this starting-character increment.
[Official source](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

## Delivered behavior

- The ten instrument IDs are added to the rules-owned tool catalog. Bards require three
  distinct selections in a Bard Training checkbox group, reusing the approved
  controls, keyboard access, Back preservation and class-change pruning.
- Choices emit level-one `tool:` grants from `class:bard:instruments`, show their sources
  on the sheet and presets receive selections through the existing generator.
- Checks apply proficiency once to instrument checks and the existing tool/skill
  Advantage when both are proficient. This grants proficiency, not equipment.
- PC20 selects the new entitlement policy and validates source/version boundaries.
  Old selections remain missing, preserving recorded skills and languages.
- Verification covers all 120 distinct triples, rejected choices, normal creation,
  sourced sheet display, presets and historical completion. Runtime/render and
  regression checks are recorded below.

## Prior-writer baseline

Before production changes, `freeze_bard_instruments()` in training_tests.cpp
used the actual rules 0.6.30 writer from `80a7d1a`. Four level-one Bards cover
all current backgrounds, with Performance/Persuasion/Perception, explicit
Elvish/Dwarvish, two missing HP and recorded resource state. Their combat
profiles are PC19. These fixtures must not be synthesized by downgrading a
future writer's version strings.

`bard_instrument_prior_writer()` compares the campaign body and complete combat
checkpoint with only module identity migration permitted. It separately asserts
recorded skill ordering, wounds, resource state and absent instrument choices.
The test also asserts pending Training and completes the missing selections
without changing historical HP or resources.

Full Bard spellcasting, Inspiration, starting equipment, instrument Utilize
actions and the Review Training UI remain separate issues. Baseline checks alone
do not establish instrument proficiency support.

## Verification

All 41 native/tool checks and all 16 Godot runtime checks, with seven native
prerequisites, pass. Main/demo extensions build; 788 English/Spanish messages
validate. The graphical main Training test verifies keyboard focus, limits,
Back preservation, the completed Bard sheet's instrument sources, and existing
class/Expertise flows. Instrument controls were rendered and inspected in both
languages at 1120×800 and 1920×1080. The demo lacks locale catalogs; localization
verification uses the main game. Current combat round trips and forged-source
rejections also pass in the final training test run. No production changes
followed the full regression run.
