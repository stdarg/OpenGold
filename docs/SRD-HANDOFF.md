# SRD handoff

Updated 2026-09-25 after verified Wizard spell-choice delivery. The standing goal
remains ACTIVE and incomplete: close all `SRD_improvements` issues, support all
twelve classes through level four, then the planned higher levels/multiclassing.
Do not pause or request another resume at this batch boundary.

## Latest delivery — Wizard spell choices (#37; implementation owner #97)

- Runtime `db864c8`, SRD module 0.6.51; branch
  `codex/srd-wizard-spell-choices`. WIZCHOICE-1/2/3 approved and implemented.
- [Coverage](SRD-COVERAGE.md#wizard-spell-learning-and-preparation-controls) is
  the completion record; [packet](WIZARD-SPELL-CHOICES.md) holds acceptance,
  compatibility, commands, measured phases and exclusions.
- All 51 native/tool and 26 Godot checks pass on the final integrated tree.
  Main EN/ES and demo EN creator, advancement, Spellbook and two-Wizard Long
  Rest controls/renders pass at both sizes. Exact UI-save comparisons pass;
  922 translations validate. Actual 0.6.50 fixtures unchanged; campaign1–15
  and prior combat recipes remain supported. Conditional campaign16/PC34.
- Original start 20:33:54 UTC; verified runtime commit 21:24:10: 50m16s,
  including approval wait/builds, before the original 21:33:54 checkpoint.
  One original requirement delivered; no new issues, added spells or scope.
- Requested Astra/high retained for rules/persistence complexity. Actual
  configuration unverified; no model switch, agents or new tasks.
- Runtime and evidence are pushed to the feature branch and main. #37 is
  verified CLOSED at 21:26:31 UTC (52m37s from original start); #97 is verified
  OPEN with its preserved copying/book-lifecycle acceptance. No delivery step
  or live build/test remains. The full standing goal is still active.

## Active batch — Rogue attacks (#112/#114)

[Frozen packet](ROGUE-ATTACKS.md) covers Sneak Attack through level four and
Steady Aim at levels three/four, including ordinary Rogue3/4 advancement.
Branch `codex/srd-rogue-attacks`; one owner, no agents/new tasks/issues.
#220/#221 share Sneak evidence; #116 remains open for broader class completion.
Thief, Hide and weapon mastery stay excluded and required in their own issues.

Started 2026-09-25 21:27:56 UTC; checkpoint 22:27:56, maximum 22:57:56.
Requested Astra/high for hit-state/persistence interactions; actual settings
unverified, no switch. Existing actual-writer fixtures remain intact.
ROGUE-1/2/3 APPROVED by a new “1. Approved. 2. Approved. 3. Approved.” reply.
Rules/main controls and demo Sneak controls are implemented: module 0.6.52,
conditional PC35/combat21, signed weapon components, separate Sneak/Savage
choices, Steady Aim and ordinary Rogue3/4 advancement. All 78 integrated checks
pass (51 native/tool, 27 Godot; 37.53s). Main EN/ES and demo EN combat and
advancement UI/renders pass at both sizes, including exact native saved-state
comparison. Localization validates all 934 messages. Actual old fixtures intact.

The demo lacks the assumed Cunning Action row. ROGUE-DEMO-1 proposes a precise
row below its battlefield; see packet. This placement needs a new visible
numbered answer; do not implement it under the main-game placement approval.
#114 remains open until that playable demo path and its checks are complete.

The complete training suite now passes with additional malformed pending-hit
and recruited Rogue handoff checks through all four levels. No live build/test
remains. The78-check run predates only these added tests, not production edits.
Verified runtime `7f3ab30` and evidence `0d7f803` are pushed on
`codex/srd-rogue-attacks`; #112/#220/#221 CLOSED at22:26 UTC. #114 and #116
remain OPEN. Main has not been advanced during this partial batch delivery.
Delivery checkpoint at22:26 UTC, about58 minutes after the original start:
one of two original feature requirements delivered; three corresponding issues
closed; zero issues added. Remaining work is the demo row and its verification.
ROGUE-DEMO-1 is the next visible question1, with Glass audio; goal remains ACTIVE.

## Preserved exclusions from Wizard delivery

#97 stays open for physical spellbooks, copying costs/time and replacement/loss.
#165 owns all missing spells; this batch added no spell effects. #98 Ritual
Adept and #101 Evoker remain open. In particular, Evocation Savant cannot be
claimed complete merely from the two currently implemented Evocation spells;
ordinary learning already consumes those choices. Inspect the next source route
and existing [batch grouping](SRD-BATCHING-REVIEW.md) before freezing one batch.
Do not silently expand the catalog, copying workflow or subclass scope.

Q19–21/Q23 remain outside this delivery; see
[decisions](SRD-DECISIONS.md). Unlimited ranged ammunition remains the approved
AMMO-UNLIMITED exception; do not reopen ammunition tracking. Prior completed
work and evidence are in coverage, not repeated in this handoff.

## Persistent execution agreements

Read [workflow](SRD-WORKFLOW.md), [model routing](SRD-MODEL-ROUTING.md) and
[repository map](SRD-REPO-MAP.md); reuse recorded approvals. Freeze outcome,
acceptance, exclusions and a 60-minute checkpoint before coding. Rules stay in
static `opengold_rules_srd5`; Core owns generic transactions/history and the UI
presents rules options. No unapproved compatibility reduction, parallel agents,
new tasks, runtime/model changes, speculative tooling or automatic new issues.

Finish a build's inputs before starting it; rebuild affected targets before
checks. Run Godot instances serially because they share user data. Commit/push
owned work after appropriate checks. Record tested revision and limitations.

Question sets restart at 1. Play
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff` before asking, verify command
completion, and put every question visibly in the conversation; a widget alone
is insufficient. A decision reply does not resume a paused goal. This goal is
currently active. Player saves remain camp/inn only, never combat controls.
