# Arcane Recovery — #99 delivery packet

## Frozen batch

- Authorization: standing SRD goal. One owner/batch, #99 only, branch
  `codex/srd-arcane-recovery`. No new issues, agents or tasks.
- Current behavior: Wizards have no Arcane Recovery implementation. Ordinary
  Wizard creation/advancement already reaches levels 1–4; existing Short Rest
  recovery controls and camp/inn saves provide the integration path.
- Player outcome: at a completed Short Rest, an eligible Wizard may choose
  spent spell slots with combined levels at most half Wizard level rounded up.
  Spend one Arcane Recovery use only on a valid nonempty choice; restore the use
  on Long Rest. Preserve resources, wounds, training and equipment otherwise.
- Acceptance: attained Wizard levels 1–4, fixed source grant, capacities and
  missing-slot limits, Short Rest eligibility/ticket validation, once-per-Long-Rest
  use, no partial/repeated transaction exploits, rejected-command atomicity,
  current/old campaign and combat continuation, rest interruption benefits and
  save/load. Actual main/demo UI, keyboard, EN/ES at both supported sizes.
- Exclusions: Scholar, Ritual Adept, Evoker, additional spell catalogs,
  spellbook/preparation redesign, other classes' recovery features and later
  level/multiclass progression. No player combat saving.
- Reuse: Q29–31 rest dialog and transactional Short Rest spending; existing
  resource pools, SRD grant provenance and opaque vitals. Core owns transactions
  and clock; statically linked SRD library owns eligibility, selection outcomes
  and resource arithmetic; shared Godot rest dialog presents choices.
- Routing: requested `gpt-6-astra` / `high`, retained for rest-state/persistence
  interactions. Actual configuration not independently verified; no switch.
  Escalate an out-of-tier decision or two unsuccessful same-failure fixes.
- Verification: capture actual 0.6.48 writer evidence before runtime changes;
  focused feature/rest/save checks, final native regression, affected rest UI
  and rendered main/demo EN/ES. Never regenerate frozen old saves with new code.
- Timing: investigation began 2026-09-25 18:57:23 UTC; checkpoint 19:57:23,
  maximum 20:27:23. Retain these times across questions and context resets.

## Source

[SRD 5.2.1, printed page 78](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf):
level-one Wizard feature, recovery chosen when a Short Rest finishes; combined
slot levels at most half Wizard level rounded up, no slot of level six or higher;
one use per Long Rest. Thus supported levels 1–2 recover at most one level-one
slot; levels 3–4 may recover one/two level-one slots or one level-two slot.
Using a smaller legal combination still consumes the feature's use.

## Pending control decision — AR-1

Visible question 1 proposes a labeled Arcane Recovery dropdown and Recover slots
button in the existing Rest window, above Result, shortening the scrollable Info
area. Show legal combinations for the selected eligible Wizard after Short Rest;
commit slots/use immediately on Recover slots. Finish/Escape without use preserves
the use. Keyboard controls and existing camp/inn saving. Current window is 720×640;
Info occupies y274–482, Result y490–560, action buttons y580–620, so the proposed
row fits within that existing window by shortening Info. Shared main/demo code.
This proposal is not approval. Sound played before asking. No dependent UI edits.

## State

Investigation complete enough to freeze the batch and ask AR-1. No implementation
or issue closure yet. Next independent work: actual prior-writer fixtures and
rules/Core recovery integration; UI waits for AR-1. Preserve the unlimited ranged
ammunition decision; it is unrelated to this batch.
