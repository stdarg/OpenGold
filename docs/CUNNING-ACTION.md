# Rogue level two and Cunning Action

Scope: [#218](https://github.com/stdarg/OpenGold/issues/218). SRD 5.2.1,
printed pp. 61–62, gives Rogues Cunning Action at level two: on their turn they
can Dash, Disengage or Hide as a Bonus Action. This increment implements Dash
and Disengage. Hide and its prerequisite hidden-state rules remain
[#219](https://github.com/stdarg/OpenGold/issues/219); parent #113 stays open.
The Rogue attacks batch extends ordinary advancement through level four. Full
Rogue completion remains #116; see [Rogue attacks](ROGUE-ATTACKS.md).

Source: [official SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

## Player path and rules

Create a Rogue through the existing Training step, or load an existing Rogue.
At 300 XP use the ordinary level-up control. Fixed-average d8 growth adds
5 + Constitution modifier, plus Dwarven Toughness where applicable; wounds,
training, historical Constitution modifiers and resource expenditure survive.
The sourced grant is `feature:cunning_action`, `class:rogue`, acquired level 2.
Rules 0.6.52 extends advancement through level four and retains Cunning Action.

Q24 approved a labeled Cunning Action dropdown and Use Bonus Action button
in a row below the existing combat buttons. The original choices are Dash and Disengage;
standard control styling, labels and keyboard focus are retained. Unavailable
use is disabled, including off-turn, spent Bonus Actions and pending decisions.
The existing A/Space action cycle also exposes the two commands. No combat save
controls are added; player saves remain at camp or an inn.

The rules module owns eligibility and spending. Both commands preserve the
ordinary Action. Bonus Dash can combine with Action Dash, adds current Speed
and uses the existing reduction-aware Dash count. Bonus Disengage suppresses
opportunity attacks until the turn ends; the next turn clears its protection.
Other Bonus Actions compete for the same budget. There is no rest-use pool.

## Persistence and verification

Rules 0.6.35 / PC24 validates level-two Rogue profiles and grants. PC1–PC23 keep
their previous level restrictions. Older campaign identities reject advanced
Rogues; current campaign reconstruction replays their ordinary advancement.
Campaign format 11 is unchanged. Cunning encounters use existing combat format
15 to preserve explicit Dash counts along with Action/Bonus Action budgets,
movement, Disengage and pending reactions. Earlier combat profiles retain their
previous capabilities and exact continuation.

`campaign-v11-cunning-before.ogs` and `combat-v13-cunning-before.save` were
captured by the actual 0.6.34 writer at `a2aed46` before production changes.
They contain four Orc Rogues with all supported backgrounds, complete training,
two missing HP, and an active actor who already spent Action Dash and Adrenaline
Rush. Migration compares every serialized field except module identity/checksum.
Do not regenerate these fixtures with the newer writer.

Native checks are in `tests/cunning_checks.h`, executed by
`opengold_training_tests`; normal creation, XP-gated campaign advancement,
source validation, both Dash orders, shared Bonus Action use, Disengage expiry,
reaction/Temporary HP decisions, rest and old/current continuation are covered.
`tests/cunning_action_view_tests.gd` exercises the actual combat row in English
and Spanish at 1120×800 and 1920×1080, including keyboard interaction.

Verification completed: all 41 native/tool tests (full regression plus focused
reruns), all 17 Godot runtime tests and their eight native prerequisites, main
and demo builds, and localization validation (816 messages). English/Spanish
render captures at both sizes were inspected. Archery's existing test was updated
to distinguish newly supported Rogue level two from unsupported later levels.

ROGUE-2 approves the main-game caption Bonus Action and adds Steady Aim from
level three, using the same dropdown/button. The legacy demo lacks this row;
its proposed placement remains ROGUE-DEMO-1, pending separate approval.
