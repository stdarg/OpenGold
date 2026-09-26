# Light attacks and Two-Weapon Fighting — frozen next batch

## Batch card

- Standing goal ACTIVE. One owner, no new tasks/agents/issues. Preflight began
  2026-09-26 00:45:02 UTC; checkpoint01:45:02, maximum02:15:02. Preserve this
  start across approval waits. All LIGHT-1/2/3 approvals received2026-09-26. Implementation continues.
- Original requirements: #59 Light extra attacks, #81 Two-Weapon Fighting,
  #56 Loading action boundaries. Bounded source integration under#85/#140/#147.
  Player outcome: equip and choose two one-handed weapons, earn a legal Light
  Bonus Action attack with a different physical weapon, and receive the feat's
  damage benefit through actual class entitlements. Loading remains per action,
  Bonus Action or Reaction, including separate Action Surge actions.
- Requested Astra/high, actual configuration unverified; no model change. Rules
  interactions, weapon identity, hand state and historical saves require this
  tier. Escalate after two failed fixes of the same failure. No delegation.
- Reuse physical carried/ground equipment, grip, legal command targeting, the
  Bonus Action row, Savage/Sneak decisions and the style routes in bf9ec73.
- Exclude Nick/other mastery#60, level5+/Extra Attack, multiclassing, other class
  features and a new artwork pipeline. Unlimited ammunition remains approved;
  do not reopen expenditure/recovery or change existing ammunition/hand behavior.
- Static SRD library owns attack eligibility, identities, hand legality, action
  budgets and modifiers. Core owns inventory/transactions/history only. UI
  presents rules-provided hand and attack choices, without SRD calculations.

## Fixed acceptance

1. Light qualification comes from an actual Attack action on the actor's turn,
   not a reaction/spell or the extra attack itself; a hit is not required.
   The extra attack occurs later on that turn, costs the one Bonus Action and
   uses a different physical Light weapon. Identical weapon types/stack units
   remain distinct actual items; the same weapon cannot qualify itself twice.
2. Preserve negative damage modifiers without the feat. The feat adds the normal
   modifier exactly once, including critical/Savage interaction. Do not grant an
   extra bonus to ordinary attacks, Sneak dice or spells. Check Strength/Finesse,
   melee/ranged/thrown, proficiency, ranges and typed defenses independently.
3. Actual Fighter1–4, Paladin/Ranger2–4 feat choices, Fighter replacement and
   independent level-four feats; prerequisite/source/nonrepeatability validation.
   Other starting classes may use Light without possessing this feat.
4. Actual equipped/carried hand state and named weapon selection, shield/grip
   conflicts, drop/pickup/throw/recovery, Opportunity Attacks and no duplicated
   items. Reuse existing transaction costs; any additional hand-manipulation
   workflow needed beyond the proposed controls is a dependency to report before
   implementing. No silently invented free equip/stow actions.
5. Loading remains tied to each particular action, Bonus Action or Reaction,
   never a once-per-turn limit. Action Surge and a legal Light Bonus Action are
   distinct opportunities. No ammunition tracking or recovery is added.
6. Actual PC/recruited campaign combat, ordinary camp/inn save/reload and rests;
   pending-hit continuation and historical profiles remain supported. Capture
   genuine0.6.53 writer evidence before changing any affected writer. No combat
   save controls, fabricated old fixtures or reduction in supported history.
7. Main EN/ES and demo EN,1120×800/1920×1080: controls, keyboard/mouse targeting,
   disabled states, Cancel/Escape atomicity and native comparison of saved state.
   Focused checks, final integrated regression, coverage, commit/push then only
   proven original-issue closures. Record actual phase times from here onward.

## Preflight evidence

Current Core `CampaignParty::equip` replaces the existing weapon; SRD profile
validation rejects a second weapon. This must change for the real two-weapon
path, using rule-owned validation rather than adding SRD branches to Core.
Physical item identities and Thrown/pickup mechanics already exist. Both combat
surfaces have a Bonus Action dropdown, but it currently contains Rogue choices.
Party inventory's existing Grip/Review Training/Spellbook row is full at1120px;
a centered hand-choice dialog avoids crowding it.

Official [SRD5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf)
pp88–90 and177 checked2026-09-26. Light depends on a different weapon; its example
of different hands is not an additional restriction. The Attack action's weapon
interaction and Thrown's draw permission must retain their actual boundaries.

## Approved controls

LIGHT-2/3 approved by “2. Yes. 3. Yes”; LIGHT-1 subsequently approved by
“1. Yes”2026-09-26. All exact controls below are authorized.

**LIGHT-1.** In main/demo, when Equip selected would add or replace a second
one-handed weapon, open a centered660×340 dialog. Show the selected weapon and
two labeled choices, Main hand and Other hand, each naming the current item or
Empty. Show replacement/conflict text before confirmation; disable illegal
choices. Equip confirms atomically; Cancel/Escape changes nothing. Retain
keyboard navigation. List held weapons by hand in inventory/sheet. Existing
available sprite poses may depict only the primary weapon; labels identify both.

**LIGHT-2.** Add a labeled Weapon dropdown in a new44px row immediately above
the existing Bonus Action row in main/demo. It selects the weapon for ordinary
attacks and reactions without spending an action. Label x24,w180; selector
x214,w450,h36. Move lower rows down44px and reserve that height from the board.
Show the row only when weapon selection is meaningful. Extend the Bonus Action
choices with named legal Light attacks (melee/ranged/thrown). Use Bonus Action
enters existing target highlighting; click/keyboard confirms and spends the
Bonus Action only on submission. Escape cancels freely; illegal choices disable.
Full weapon names remain available in dropdowns/tooltips. No combat saving.

**LIGHT-3.** Add Two-Weapon Fighting to the existing approved Fighter Training/
Review Training and all three classes' advancement selectors. Automatically
apply its beneficial damage modifier to eligible Light extra attacks; no new
per-hit decision. Preserve existing source, replacement and separate-feat rules.

## Independent compatibility work while controls await approval

Seven genuine0.6.53 campaign/combat captures and a verifier are now in
[`light_attack_baseline.h`](../tests/light_attack_baseline.h). Capture provenance
and SHA-256 hashes are recorded in [fixtures](../tests/fixtures/README.md#lighthand-state-baseline--actual-0653-writer).
The full training test passes after rebuilding; no runtime source was changed.
This proves prior-style/weapon/history continuation, not Light/TWF completion.
No original issue is closed by this preparation.

Preflight also confirms that Loading is present in the weapon catalog but the
current combat implementation permits only one weapon attack per consumed
Action/Reaction budget. Preserve separate Action Surge actions. The new Light
Bonus Action route must be verified before claiming the whole #56 boundary.

## Rules-owned equipment transition work

Before LIGHT-1/2/3 were approved, existing Equip/Unequip decisions moved
from Core into a `RulesModule::equipment_change` operation. It returns candidate
indices and equipment continuation as values. Core maps those indices to owned
inventory items and commits atomically, rejecting invalid/duplicate indices.
The SRD implementation retains current single-weapon replacement and grip
normalization; this step adds no hand dialog, dual-weapon control or TWF benefit.
It is a direct consumer of the new rules hook, required before two-weapon
integration can avoid putting SRD hand/replacement decisions into Core.

The alternate-rules test in `party_tests.cpp` deliberately retains two weapons,
reorders them and returns non-SRD equipment state. It proves that Core follows
module outcomes; malformed plans and rule rejections preserve inventory/vitals.
The rest-boundary test explicitly delegates equipment decisions to its SRD
fixture implementation. No runtime/save version change is needed for this
behavior-preserving move. Focused Party/Training/Rest/Versatile checks pass.
Runtime/test revision `bc4edae` passed all78 integrated checks after rebuilding
all native targets, main project and demo. The game equipment check also passed
all47 weapons through actual controls, PC/NPC, both poses, save/load, training
and campaign. Localization941 messages and `git diff --check` passed. This is
an architecture prerequisite; no Light/TWF/Loading issue is complete yet.

Verification commands (full logs retained under `/tmp/light-equipment-*`):

```bash
ctest --test-dir build/mac-check -E '^opengold_godot_prepare$' --fixture-exclude-setup godot_project --output-on-failure
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD OPENGOLD_LANG=en /Applications/Godot_mono.app/Contents/MacOS/Godot --headless --path src/OpenGoldBox/godot res://scenes/character_creation.tscn -- --equipment-art-check
python3 tools/localization.py --check
```

Current native/runtime behavior and save formats remain unchanged. The integrated
run followed a successful current-tree project preparation; fixture setup was
excluded only to avoid repeating that preparation. Verification was confirmed at
01:17:53 UTC. The remaining equipment-dialog approval is LIGHT-1; LIGHT-2/3 were subsequently
approved. The goal remains ACTIVE.

Phase observations: compatibility capture began00:53:12 and was committed at
01:00:04 (see d24c146 metadata for exact commit time). Equipment-transition
implementation began after01:01:39; focused checks passed before01:08:36, when
the integrated and demo rebuilds started. Original checkpoint remains01:45:02.

Implementation resumed01:20:09 UTC after LIGHT-1. Original checkpoint unchanged.

## Approved hand equipment implementation

LIGHT-1 is implemented in the current tree: shared game/demo dialog,
rules-provided legal hand choices and atomic item splitting. Main/other labels
appear in inventory and character sheets; artwork uses the first equipped weapon.
The selected stack unit retains its original item provenance. Cancel/Escape
make no changes. Two equipped weapons block a two-handed Versatile grip,
shields and somatic casting without a free hand. Ordinary replacement and old
save recipes retain their previous behavior; only dual-weapon profiles use PC37.
Module0.6.54 accepts all previously supported module identities, including0.6.53.

Focused Party/Training checks pass. The game EN/ES and demo EN hand dialog
checks pass at1120×800/1920×1080; captured game layouts were inspected. Their
three output saves match independently constructed native results exactly,
including original wounds, resources, PC/NPC state and item quantities. The
existing47-weapon game-control/artwork check passes. Localization952 passes.
Runtime/test commit `c539347` passed all78 integrated checks after rebuilding
all native targets and game/demo. This includes27 Godot checks. No original
issue is closed by the hand-equipment substep.

This implements the hand-equipment prerequisite only. LIGHT-2 combat weapon
selection/Light attack sequencing and LIGHT-3 feat acquisition/damage are still
to implement. No additional approval is needed. No new issue, agent or scope.

Phase observations: implementation resumed01:20:09; focused checks passed by
01:30:41; game/demo UI checks, native save comparisons and47-weapon check passed
by01:34:17 UTC. Broad native rebuild began around01:33; all78 integrated checks were confirmed
complete after01:39:37. Original01:45:02 checkpoint is unchanged. Fixes covered
two C++ version expressions, Godot node lookup, an inventory-view invalidation,
and two UI harness setup problems (type annotation, fixture content identity).
The legacy-version test now constructs its altered save outside the rejection
assertion and uses the actual module version. No runtime test was weakened. Elapsed phase observations overlap; no token/cost
measurement or claimed speedup.

Hand-equipment verification commands (main project was prepared after the final
runtime inputs; demo built separately; Godot runs were serial):

```bash
ctest --test-dir build/mac-check -E '^opengold_godot_prepare$' --fixture-exclude-setup godot_project --output-on-failure
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD build/mac-check/opengold_training_tests --hands-ui
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD OPENGOLD_LANG=en /Applications/Godot_mono.app/Contents/MacOS/Godot --path src/OpenGoldBox/godot --script /Users/edmond/src/OpenGold/tests/equipment_hand_view_tests.gd -- --hands-fixture=/Users/edmond/src/OpenGold/build/mac-check/hands-ui.ogs --hands-captures=/tmp/light-hands-game --hands-output=/tmp/light-hands-game
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD /Applications/Godot_mono.app/Contents/MacOS/Godot --path demos/godot --script /Users/edmond/src/OpenGold/tests/equipment_hand_view_tests.gd -- --hands-demo --hands-fixture=/Users/edmond/src/OpenGold/build/mac-check/hands-ui.ogs --hands-captures=/tmp/light-hands-demo --hands-output=/tmp/light-hands-demo
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD build/mac-check/opengold_training_tests --verify-hands-ui /tmp/light-hands-game-en.ogs
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD build/mac-check/opengold_training_tests --verify-hands-ui /tmp/light-hands-game-es.ogs
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD build/mac-check/opengold_training_tests --verify-hands-ui /tmp/light-hands-demo-en.ogs
```

Full logs `/tmp/light-hands-integrated.log`, `/tmp/light-hands-game-ui.log`,
`/tmp/light-hands-demo-ui.log`, `/tmp/light-hands-art-check.log`; captures in
`/tmp/light-hands-game` and `/tmp/light-hands-demo`. Runtime revision above is
the tested tree. No pending process or approval remains. Preserve genuine0.6.54
dual-hand writer evidence before later changing these new recipes/continuations.
Continue the same batch with focused checks; do the next broad regression on
the integrated Light/TWF implementation, rather than every small edit.
