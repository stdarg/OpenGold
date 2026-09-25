# SRD handoff

Updated 2026-09-25 after #58 delivery. The full goal remains incomplete: close all
`SRD_improvements` issues, all twelve classes through level four, then level
twenty/multiclassing. One batch is not goal completion.

## Latest completed batch — #58 Thrown weapon inventory

- Runtime `5d7813d`, module 0.6.47, branch `codex/srd-thrown-inventory`.
  [Coverage](SRD-COVERAGE.md#thrown-weapon-inventory) is the completion record;
  [packet](THROWN-WEAPONS.md#verified-delivery--runtime-5d7813d) holds acceptance,
  compatibility, commands, limitations and measured timing. Q43/Q44 implemented.
- 49 native/tool tests and 26 Godot runtime checks pass (42 with prerequisites);
  main EN/ES and demo EN renders/input pass at both sizes. 896 messages validate.
  Actual prior-writer fixture bytes unchanged. No live build/test process remains.
- Authorized continuation 17:11:00 UTC; tested runtime committed 17:41:31.
  Original preparation/wait/checkpoint intervals remain in the packet. Recorded
  Astra/high retained; no model switch, agents, new issues or expanded scope.
- Runtime and coverage documentation (`bd5ed6a`) are pushed to the current
  branch and `main`. #58 is verified CLOSED with evidence. No other issue closed
  and no issues added. This handoff update changes no tested runtime inputs.

## Latest decision — #57 unlimited ammunition

The user explicitly declined ammunition tracking: owning the ranged weapon
assumes ammunition. AMMO-1/AMMO-2 are withdrawn, not pending. Preserve unlimited
ranged behavior; no expenditure, stock requirement or recovery controls.
[Coverage](SRD-COVERAGE.md#approved-exception-unlimited-ranged-ammunition) and
[policy/evidence](AMMUNITION.md) record the exception. Existing saved ammo records
remain readable in runtime `79558fd` (0.6.48); all 50 native/tool tests pass and
three focused checks were rerun successfully after the decision. No UI changes.
The 17:58:09 batch began before the decision; the 18:58:09 checkpoint was not reset.

#57 is to be resolved as not planned, not as implemented tracking. After its
verified closure, continue the standing SRD goal with the next frozen batch.
No live build or input blocker remains from ammunition. The goal tool's last
BLOCKED result concerned AMMO-1/AMMO-2, which this answer now resolves; do not
interpret it as a user pause or require another resume instruction. No tool
operation can set that tracker active directly. Keep the full goal incomplete.

When continuing authorized goal work, use [batch grouping](SRD-BATCHING-REVIEW.md),
[workflow](SRD-WORKFLOW.md), [model routing](SRD-MODEL-ROUTING.md) and
[repository map](SRD-REPO-MAP.md). Freeze one outcome and acceptance before coding.
Do not reopen completed feature checks without a relevant change.
Q19–21/Q23/Q25 remain outside this batch; see [decisions](SRD-DECISIONS.md).
Package C's broader Light/draw-stow, mastery, remaining Fighting Styles
and later advancement remain separate requirements. No automatic new issues.

No model/runtime change, agents/new tasks or compatibility reduction without
explicit authorization. Keep one owner and one delivery batch. New visible
question sets start at 1; use topic-specific ledger IDs, preserving historical Q
references. Questions must be visible in the conversation and preceded by
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff`; a widget alone is insufficient.
A decision reply alone does not resume a paused goal. Saving remains camping/inn
only, never player combat saving.
