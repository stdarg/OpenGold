# SRD handoff

Updated 2026-09-24. **SRD implementation goal paused after #228**, at the user's
request. Full scope remains all twelve classes through level 4, then level 20
and multiclassing. Workflow maintenance is authorized; it does not resume coding.

## Entry points — read only what the active work needs

- [Workflow](SRD-WORKFLOW.md): adopted batch execution, checks and effort tracking.
- [Repository map](SRD-REPO-MAP.md): code ownership, test targets, local environment.
- [Decision register](SRD-DECISIONS.md): approved scope and pending questions.
- [Batch grouping](SRD-BATCHING-REVIEW.md): all 168 open issues at the review snapshot;
  use it when selecting a batch, not on every turn. Refresh affected GitHub issues
  before implementation/closure; cached counts are not live status.
- [Coverage](SRD-COVERAGE.md): canonical completion record and evidence links.

## Current batch card — workflow maintenance

Status: verified; commit/push delivery next. User authorized implementing efficiency suggestions.
Branch `main`; baseline `57b67a7`. Outcome: repository-enforced batch workflow,
compact navigation/decision records, reduced duplicate status and effort tracking.
No gameplay, compatibility, model, task or agent changes are authorized here.

| Acceptance | Owner / evidence |
| --- | --- |
| One batch, targeted reads, WIP limit, no speculative foundations | AGENTS.md and SRD-WORKFLOW.md |
| Find code/test entry points without broad rediscovery | SRD-REPO-MAP.md; paths and targets checked |
| Preserve approvals and unanswered questions | SRD-DECISIONS.md; existing handoff/conversation |
| Canonical completion and short current-state record | SRD-COVERAGE.md plus this card |
| Remove conflicting mandatory ticket-splitting policy | Implementation plan and spell inventory link to adopted workflow |
| Measure effort and report checkpoint | Workflow phase table; next implementation batch uses observed timing |

Verification: documentation links, map paths/targets, contradictory-instruction
search, `git diff --check`; no game build for a documentation-only change.
Compatibility policy: unchanged. Live process handles: none before delivery.

Effort: investigation began before explicit timing was recorded (unknown).
Documentation implementation timestamp: 2026-09-24 21:52:55 UTC. Verification completed 21:56:28 UTC (3m33s observed interval). Build: not
applicable. Token delta: unavailable. [Coverage](SRD-COVERAGE.md) records the
validated outcome; commit/push result belongs in the delivery response.

## Next implementation batch — not started

Recommended candidate A: #29/#189 Review Training, using the existing native
preview/commit and Training controls. **Blocked on pending Q11 layout** and
resumption. Retrieve its exact original wording before dependent coding; do not
infer approval or repeat the question automatically. User has not authorized
resuming the goal. Other pending decisions: Q19–21, Q23, Q25; see register.

On resumption, replace this card with the selected package: outcome, original
issues, fixed acceptance rows, source/level matrix, dependencies, approved/pending
decisions, affected paths, persistence policy, commands/tested revision, timestamps,
two-hour review checkpoint and live handles. One batch plus at most one necessary
prerequisite; do not rotate through unrelated partial features.

Last gameplay delivery: #228 in `8429158`; [evidence](SORCERER-CANTRIPS.md).
Rules 0.6.40 / PC28, campaign 11, combat 13–15, FX1–3. Compatibility unchanged.
Prepared but not playable: Sneak Attack [foundation](SNEAK-ATTACK.md) at `ff297ef`
(Q25), Great Weapon Fighting [foundation](GREAT-WEAPON-FIGHTING.md) at `8da4565`
(Q23), concentration [foundation](CONCENTRATION.md) (Q19–21 integration).
