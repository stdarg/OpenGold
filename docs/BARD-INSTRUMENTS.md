# Bard starting instrument proficiencies

Tracked in [#214](https://github.com/stdarg/OpenGold/issues/214), a child of
[Bard starting choices #125](https://github.com/stdarg/OpenGold/issues/125).
Implementation is pending; this document records the source and migration baseline.

SRD 5.2.1 Core Bard Traits (p. 31) grants three Musical Instrument
proficiencies. The instrument variants (p. 94) are bagpipes, drum, dulcimer,
flute, horn, lute, lyre, pan flute, shawm and viol. The proficiency entitlement
is separate from choosing/owning starting equipment. Multiclass entry grants
one instrument instead and remains outside this starting-character increment.
[Official source](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

## Implementation checklist

- Add the ten instrument IDs to the rules-owned tool catalog and require three
  distinct selections in a Bard Training checkbox group. Reuse the approved
  controls, keyboard access, Back preservation and class-change pruning.
- Emit level-one `tool:` grants from `class:bard:instruments`, show their sources
  on the sheet and generate preset selections through the existing generator.
- Apply proficiency once to instrument checks and the existing tool/skill
  Advantage when both are proficient. Do not grant an inventory item.
- Version the new entitlement policy and validate source/version boundaries.
  Old selections remain missing, preserving recorded skills and languages.
- Verify every offered instrument, rejected choices, ordinary creation and
  presets; inspect English/Spanish Training at 1120×800 and 1920×1080.
- Run prior-writer continuation and native regression checks; update the
  coverage ledger, commit and push before closing #214.

## Prior-writer baseline

Before production changes, `freeze_bard_instruments()` in training_tests.cpp
uses the actual rules 0.6.30 writer from `80a7d1a`. Four level-one Bards cover
all current backgrounds, with Performance/Persuasion/Perception, explicit
Elvish/Dwarvish, two missing HP and recorded resource state. Their combat
profiles are PC19. These fixtures must not be synthesized by downgrading a
future writer's version strings.

`bard_instrument_prior_writer()` compares the campaign body and complete combat
checkpoint with only module identity migration permitted. It separately asserts
recorded skill ordering, wounds, resource state and absent instrument choices.
After implementing the entitlement, extend this test to assert pending Training
and complete the missing selections without changing historical state.

Full Bard spellcasting, Inspiration, starting equipment, instrument Utilize
actions and the Review Training UI remain separate issues. Baseline checks alone
do not establish instrument proficiency support.
