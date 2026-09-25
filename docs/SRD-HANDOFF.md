# SRD handoff

Updated 2026-09-25 after Champion delivery. The full goal is ACTIVE: close all
`SRD_improvements` issues, all twelve classes through level four, then level
twenty/multiclassing. A batch delivery is not goal completion.

## Latest completed batch — #88 Champion

- Runtime `6324a51`, module 0.6.46, on `codex/srd-champion`. #88 is verified
  CLOSED with delivery evidence. No other issue closed; no issues added.
- [Coverage](SRD-COVERAGE.md#fighter-champion-through-level-four) is the completion
  record; [Champion packet](CHAMPION.md) holds fixed scope, mechanics, compatibility,
  limitations, commands and measured timing. Q41/Q42 implemented.
- 48 native/tool tests and 25 Godot runtime checks (40 with prerequisites) pass;
  game EN/ES and demo EN renders/keyboard/mouse checks pass at both sizes.
  Localization validates 891 messages. Historical fixture bytes are unchanged.
  No live build/test handles or unresolved batch questions remain.
- Start 15:26 UTC; verified runtime committed 16:06:21, about 40 minutes and before
  the 16:26 checkpoint. Recorded/requested Astra/high retained for critical/reaction
  state and persistence; no model switch or agents. Token/cost deltas unavailable.
- Delivery sequence: push runtime and documentation to `codex/srd-champion`,
  fast-forward `main`, push `origin/main`. Verify Git state on the next turn.

## Next work

Select one bounded authorized outcome from [batch grouping](SRD-BATCHING-REVIEW.md),
using [workflow](SRD-WORKFLOW.md), [model routing](SRD-MODEL-ROUTING.md) and
[repository map](SRD-REPO-MAP.md). No next implementation batch is frozen yet.
Refresh only its issues; record acceptance/exclusions, model assignment and timing
before coding. Do not reopen completed Medicine/Tactical Mind or Champion checks.

Q19–21/Q23/Q25 remain unresolved outside this batch; see
[decisions](SRD-DECISIONS.md). Package C's broader Light/draw-stow and Paladin/
Ranger style paths remain deferred. Champion Athletics queries are implemented;
new Athletics actions and future Champion spell-access sources remain separate
requirements. Do not create extra issues or prerequisites without authorization.

No model/runtime change, agents/new tasks or compatibility reduction without
explicit authorization. Keep one implementation owner and one frozen batch.
Questions must be numbered, visible in the conversation, and preceded by
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff`; a widget alone is insufficient.
A decision reply alone does not resume a paused goal. Saving remains camping/inn
only, never player combat saving.
