# SRD handoff

Updated2026-09-26. Standing goal **ACTIVE and incomplete**: close all
`SRD_improvements`, preserving all twelve classes through4 and planned higher
levels/multiclassing. Do not pause at delivery boundaries or ask for a new resume.
A decision reply alone never resumes a genuinely paused goal. No agents/new tasks
or model/runtime setting changes are authorized. **144 open SRD issues**, snapshot
`/tmp/srd-open-after-asi.json`; no issue closed in the current batch.

## Active batch — Weapon Mastery

Branch `codex/srd-weapon-mastery`. [Fixed packet](WEAPON-MASTERY.md) owns acceptance.
Target #60/#85, with mastery portions of #140/#147/#111/#103. Five current class
routes, all eight properties, retained saves and actual controls. Unrelated class
progression remains in existing issues. Requested Astra/high for rules interactions
and persistence; actual configured model/effort unverified. One owner; no agents.
Preflight03:56:03 UTC;60-minute checkpoint recorded04:56:24. Do not reset the clock.

- Acquisition committed/pushed **5b978df**; prior-writer preparation **344a6bb**.
  Creation/preset/review choices, Fighter4 selection and conditional PC39 work.
- Long Rest replacement **f31fded** committed/pushed: transactions,
  sourced history replay and approved700×670 controls. SRD owns replacement
  outcomes; Core owns per-member one-use windows and generic history. Campaign18
  is conditional;1–17 remain accepted. Rules identity is now branch0.6.58.
- Main still375f58f. No PR/main/release integration claimed. Latest completed
  original issue was #82 (ASI); see [coverage](SRD-COVERAGE.md), not old narratives.
- Sap/Vex committed/pushed86ac760. Nick committed/pushed0bba07e: approved selector/targeting, shared Light budget, combat23 and
  retained actual prior-writer continuation. Three of eight properties delivered;
  this remains partial combat work, not whole-class or issue completion.
- Pending-choice serialization works. A player-accessible Save game control inside
  the exclusive rest modal still needs MASTERY-7 approval; not implemented yet.

## Verification / next action

Nick and freshly rebuilt **51/51 native checks PASS** (8.78s),
`/tmp/mastery-nick-native-final.log`. MainEN/ES + demoEN Nick and neighboring Light
UI/native checks pass at1120×800/1920×1080;988 messages/diff checks pass. Pending
Sneak/Savage, Vex consumption and Champion movement retain exact continuation.
See [packet](WEAPON-MASTERY.md#nick-combat-checkpoint) for commands/evidence.
Observed verification06:06:09 UTC,2h10m06s since original batch start; no live
build/test. Coverage owns completion, and no issue has been closed.

Compatibility preparation aa6e3f8 captures actual0.6.57 Longbow/Maul critical
phases. Latest checkpoint adds Slow's effect lifecycle/stacking and shared
movement/persistence consumers; **51/51 freshly rebuilt native checks PASS**,
12.43s, `/tmp/mastery-slow-native.log`;989 messages/diff checks pass. See
[packet](WEAPON-MASTERY.md#slow-effect-lifecycle-checkpoint-combat-integration-incomplete)
for scope and evidence. No live process. Verified06:19:44 UTC,2h23m41s since
original preflight. Main/demo binaries remain at Nick0.6.57; rebuild before any
future UI tests. No UI control changed in the Slow lifecycle checkpoint.

Slow's weapon-hit choice and Topple remain unimplemented; playable count stays
3/8. Next: optional mastery combat integration, with simultaneous Champion
ordering requiring MASTERY-5's answer. Cleave/Graze/Push also remain. Independent
Slow lifecycle work is now verified; do not replace remaining full player paths
with isolated helpers or treat them as delivered. Preserve historical fixtures.
MASTERY-5/6/7 were re-presented visibly as questions1–3 with verified Glass alert;
no answers received. Do not repeat widgets or sound without a new question.
Previous turn was progress (aa6e3f8 compatibility); this one implements and verifies
Slow lifecycle. No new issues, scope, agents/tasks or settings.
**MASTERY-1–4 approved. MASTERY-5/6/7 pending:**

1. Resolve next dropdown for simultaneous mastery/Champion movement.
2. Graze damage cap versus Vulnerability interpretation.
3. Save game button at bottom left of the mastery rest window, opening existing
   camp/inn saving and returning to the same pending choice without applying it.

Full proposals are in the packet/decision register; do not implement undecided
behaviors. Continue independent approved combat work while answers are pending.
The four-item reply is the already-recorded MASTERY-1–4 approval; no later
three-item decision reply is present.

## Preserved decisions and execution

#97 remains physical spellbooks/copying; #165 retains missing spells. #98/#101
and the full Wizard tracker remain open. Q19–21 remain pending; Q23 superseded by
STYLE-1. Unlimited ammunition is AMMO-UNLIMITED; do not reopen it. Camp/inn saves
only. Read SRD-WORKFLOW, SRD-MODEL-ROUTING, SRD-REPO-MAP and SRD-DECISIONS.

No silent scope growth, new issues, agents/tasks/settings or compatibility cuts.
Finish source edits before building; never edit live build inputs. Rebuild before
testing; Godot runs serially. Preserve prior-writer evidence. Commit/push only
owned verified work; close only full original acceptance. Coverage is canonical.
Questions restart at1 per set. Play `/usr/bin/afplay /System/Library/Sounds/Glass.aiff`,
verify exit0, and write full questions visibly; widget alone is insufficient.
