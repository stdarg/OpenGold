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
- The commit containing this handoff adds Long Rest replacement transactions,
  sourced history replay and approved700×670 controls. SRD owns replacement
  outcomes; Core owns per-member one-use windows and generic history. Campaign18
  is conditional;1–17 remain accepted. Rules identity remains branch0.6.56.
- Main still375f58f. No PR/main/release integration claimed. Latest completed
  original issue was #82 (ASI); see [coverage](SRD-COVERAGE.md), not old narratives.
- **Combat properties remain unimplemented.** Do not mistake acquired masteries
  or pure replacement policy for runtime combat support. No whole-class closure.
- Pending-choice serialization works. A player-accessible Save game control inside
  the exclusive rest modal still needs MASTERY-7 approval; not implemented yet.

## Verification / next action

Rest focused tests and final freshly rebuilt **51/51 native checks PASS** (4.03s),
`/tmp/mastery-rest-native-final.log`. All50 native targets rebuilt. Existing full
rest UI acceptance passes (`/tmp/mastery-rest-neighbor.log`). New shared controls
pass mainEN/ES + demoEN at1120×800/1920×1080; screenshots `/tmp/mastery-rest-*`.
Corrected harness applies locale/size after startup; small Spanish and demo layouts
were visually checked. UI focus-loss bug fixed by retaining visible checkboxes.
976 localized messages/diff checks pass. No live build/test. Native prior writer
fixture and provenance live in tests/fixtures. Coverage links the detailed packet.

**MASTERY-1–4 approved. MASTERY-5/6/7 pending:**

1. Resolve next dropdown for simultaneous mastery/Champion movement.
2. Graze damage cap versus Vulnerability interpretation.
3. Save game button at bottom left of the mastery rest window, opening existing
   camp/inn saving and returning to the same pending choice without applying it.

Full proposals are in the packet/decision register; do not implement undecided
behaviors. Continue independent approved combat work while answers are pending.
No question approval has arrived after the acquisition turn's two questions.

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
