# Rogue attacks — frozen delivery batch

- Authorization: active standing SRD goal. One owner; no agents, new tasks or
  new issues. Branch `codex/srd-rogue-attacks`.
- Outcome: playable Sneak Attack at Rogue levels 1–4 and Steady Aim at levels
  3–4, reached through ordinary creation and advancement in game and demo.
- Owners: #112 (Sneak Attack, including existing #220/#221); #114 (Steady Aim).
  #116 owns the necessary ordinary level-three/four advancement integration,
  but remains open for its broader class-completion acceptance.
- Exclusions: Thief (#115), Hide (#219), weapon mastery (#60), new feats,
  new spell effects, level five+, multiclassing, combat saving controls.
  None of these requirements is removed or claimed complete by this batch.
- Reuse: pure Sneak eligibility/dice helper, pending weapon-hit/Savage pipeline,
  existing Bonus Action row, level-up dialog, sourced grants and history.
  SRD decides all eligibility, damage, budgets, movement and progression;
  Core retains generic transactions/history and UI presents rules options.
- Requested model: `gpt-6-astra/high` for optional-hit/reaction/damage and
  migration interactions. Actual configuration unverified; no setting change.
  Escalate after two unsuccessful fixes of the same failure or a decision
  beyond this assignment. Failed fixes: zero at preflight.
- Timing: observed preflight start 2026-09-25 21:27:56 UTC. Checkpoint
  22:27:56, no later than 22:57:56. Preserve this start across approval waits.
- Source: [official SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
  pp.61–63; Rogue feature table, Sneak Attack, Steady Aim and level-four ASI.
  Rechecked 2026-09-25. See [existing Sneak foundation](SNEAK-ATTACK.md).

## Fixed acceptance

| Requirement | Required evidence |
| --- | --- |
| Sneak Attack | Optional once per combatant turn, 1d6 at Rogue1/2 and 2d6 at Rogue3/4; hit with Finesse/Ranged weapon and net Advantage, or capable ally within five feet and no net Disadvantage. Actual ally query excludes attacker; no extra ally sight/reach rule. |
| Damage | Matching weapon type; double extra dice on criticals; fixed Blowgun base stays fixed. Thrown Dagger qualifies, thrown Handaxe does not. Spells/unarmed excluded. Combine signed weapon and extra components before flooring/resistance. Savage changes only weapon dice. |
| Timing | Miss, decline and ineligible hit do not spend Sneak. Reset on every actor's turn, including reactions during another turn. Original Action/Reaction remains spent; interrupted movement resumes after all hit decisions. |
| Steady Aim | Level3+, Bonus Action only before movement that turn; next attack roll gains Advantage, consumed even on miss and canceled normally by Disadvantage. Speed becomes zero for the rest of the turn. Unused Advantage expires; no rest pool. |
| Ordinary progression | Real XP-driven Rogue1→2→3→4, existing fixed HP growth and level-four available feat/ASI choices, Cunning Action retained, correct feature provenance/dice at each level. Thief and broader Rogue completion remain pending. |
| Persistence | Actual prior-writer continuation, all released histories retained, strict source/level/pending-state validation, deterministic RNG at each decision stage, no invented legacy combat abilities. Campaign migration adds only justified fixed grants. |
| Transactions | Invalid choices leave state/RNG unchanged. Wounds, equipment, training, resources, party/recruited ownership and rest/campaign handoff remain correct. |
| UI | Approved controls only; standard visible buttons, keyboard/mouse, proper disabled state, game EN/ES and demo EN input/renders at 1120×800 and 1920×1080. |

## Proposed controls — pending approval

**ROGUE-1 (supersedes pending Q25):** Centered 700×380 Sneak Attack dialog after
an eligible hit. Show target and extra dice; use two full-width, stacked buttons:
Use Sneak Attack and Keep hit; save Sneak Attack. Use spends the turn's use and
rolls the extra dice; decline/Escape preserves it. Savage Attacker follows if
eligible and changes only weapon dice. Damage and other actions wait until
decisions finish; the attack's Action/Reaction stays spent. Keyboard navigation
and normal control styling; no combat-save controls.

**ROGUE-2:** Reuse the existing Cunning Action dropdown/Use Bonus Action row,
relabeling its caption Bonus Action. Add Steady Aim at Rogue3+, alongside Dash
and Disengage. Keep the existing position and keyboard access. Disable illegal
options and disable Use for an unavailable selection. Steady Aim spends the
Bonus Action, grants Advantage on the next attack roll this turn and sets Speed
to zero until turn end; unavailable after movement. Other options retain their
normal eligibility. No new row or automatic activation.

**ROGUE-3:** Extend the existing level-up confirmation to Rogue levels three
and four. Level three lists Steady Aim and 2d6 Sneak Attack; level four reuses
the existing available feat/ability controls and fixed HP preview. Confirm
commits atomically; Cancel discards edits; existing wounds/resources remain.
The note explicitly says Thief features, Hide and weapon mastery are still
unavailable. This does not grant partial Thief or close #116.

## Evidence / current phase

Preflight only; zero of two original features delivered. No gameplay edits yet.
The existing 0.6.35 actual-writer campaign and pending-Savage fixtures are
immutable and already cover pre-Sneak capabilities and exact RNG continuation;
do not regenerate them. Current 0.6.51 baseline at `eef99a3` passes
`ctest --test-dir build/mac-check --output-on-failure -R '^opengold_(damage|training)_tests$'`
(2/2, 0.70 seconds, 21:34 UTC). This is baseline evidence, not feature completion.

Next: record answers, implement the frozen rules/advancement/presentation paths,
rebuild affected targets, run focused feature checks and final native regression
plus affected Godot checks. Capture a newer actual writer only if a required
continuation case is not already represented. Record final revision/commands in
coverage before closing any issue; commit and push owned changes.
