# Sneak Attack

Parent [#112](https://github.com/stdarg/OpenGold/issues/112) covers Rogue levels
one through four. [#220](https://github.com/stdarg/OpenGold/issues/220) implements
the first playable levels (one/two); [#221](https://github.com/stdarg/OpenGold/issues/221)
retains level-three/four integration after ordinary advancement in #116. All
remain open. The existing grants infrastructure is usable even though its wider
training tracker #29 remains open; the action-preservation prerequisite #24 is closed.

Authority: [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
pp. 16 and 61–62. Sneak Attack is optional extra damage once per turn on an
eligible weapon hit. Levels one/two add 1d6; levels three/four add 2d6. Critical
hits double those dice, and their damage type matches the weapon. The source
progression table continues to 10d6 at Rogue level nineteen/twenty.

## Prepared foundation

The internal `sneak_attack.h` helper expresses the source eligibility predicate
and Rogue-class-level damage dice. It has no mutable state, owned resources or
random draws. It is **not wired into live combat** and grants no feature access.
It does not introduce a new rules/profile version or change existing behavior.

Eligibility requires a Finesse or Ranged weapon and net Advantage, or a capable
ally within five feet of the target and no net Disadvantage. Opposing Advantage
and Disadvantage cancel normally. Weapon category is distinct from delivery:
a thrown Dagger qualifies, a thrown Handaxe does not gain eligibility from its
range. Blowgun qualifies despite having fixed base damage. A future caller must
supply the actual weapon attack, exclude the attacker from the ally search,
check allies' Incapacitated state, and gate hit/grant/once-per-turn availability.
It must not add a sight or weapon-reach condition to the ally clause.

`damage_tests.cpp` uses an independent named list across the complete weapon
catalog, all three net roll modes, ally/no-ally, weapon/nonweapon attacks and the
source progression table. Passing these predicate tests does not establish the
future spatial queries or state-machine correctness.

Real rules 0.6.35 fixtures captured at `3846c21` before production Sneak changes:

- `campaign-v11-sneak-before.ogs`: normally created Soldier Rogues at levels one
  and two, complete training, two missing HP, wealth and ordinary XP advancement.
- `combat-v15-sneak-before.save`: the level-two Rogue has spent Bonus Action Dash
  and a weapon attack; damage awaits the existing Savage Attacker decision.
- `combat-v15-sneak-second.save` and `combat-v15-sneak-resolved.save`: the real
  writer's second roll and accepted result, including RNG and action budgets.

The generator `opengold_training_tests --freeze-sneak` requires module 0.6.35.
Do not regenerate these with a later writer. `sneak_baseline.h` verifies exact
campaign/combat continuation, allowing only module identity/checksum changes.
When live campaign grants are added, amend the independent migration oracle to
allow only the justified fixed grant; legacy combat recipes must retain their
original capabilities and random continuation.

## Remaining implementation and approval

Q25 is pending. Proposed control: a centered dialog after an eligible hit showing
target and extra dice, with Use Sneak Attack and Keep hit; save Sneak Attack.
Using it spends this turn's use and rolls extra dice. Any Savage Attacker choice
follows and affects only weapon dice. Other actions wait; Action/Reaction stays
spent. Keyboard access and standard button styling are required. No combat-saving
controls are added. AGENTS.md requires numbered confirmation of control behavior.

After approval, reuse the pending weapon-hit pipeline with explicit decision
stages. Keep Sneak dice separate from both Savage weapon results. Include all
same-type attack damage before resistance rounding; retain signed weapon
components until the total damage floor, so a negative ability modifier is not
lost by flooring the weapon component too early. Critical hits double every
attack damage die once and do not double flat modifiers. Blowgun's fixed amount
remains fixed. Declines/misses/ineligible hits do not spend Sneak Attack.

Store source-derived entitlement, per-turn expenditure and each pending stage
with strict validation. Reset the use for every combatant's turn, permitting a
second use on an eligible reaction during another turn. Preserve suspended
movement until all hit decisions finish. Old profile versions must not acquire
Sneak Attack mid-encounter. Current grants/choices, malformed states, each decision
stage, actual opportunity attacks, ordinary advancement, campaign handoff, rest,
UI and bilingual renders remain required before closing #220. #112 additionally
requires the level-three/four work in #221.

Foundation verification: `opengold_damage_tests` and `opengold_training_tests`
pass, including real pre-change save continuation. No live UI/rules behavior
changed, so previous broad regression results are not claimed as proof of the
unimplemented feature.
