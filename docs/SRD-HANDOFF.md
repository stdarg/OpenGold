# SRD handoff

Updated2026-09-26. Standing goal **ACTIVE and incomplete**: close all
`SRD_improvements`, preserving all twelve classes through4 and planned higher
levels/multiclassing. Do not pause at delivery boundaries or ask for a new resume.
A decision reply alone never resumes a genuinely paused goal. No agents/new tasks
or model/runtime setting changes are authorized.

## Latest delivery — ASI conformance

- Branch `codex/srd-asi-conformance`; test/build commit **c3a88b4**, production
  rules remain0.6.55 (`69771eb`). **#82 CLOSED** after verifying existing behavior;
  no gameplay code duplicated. **144 open SRD issues**, repository API snapshot
  `/tmp/srd-open-after-asi.json`. Immediate GitHub search listings can lag closure;
  direct issue state/repository issues API confirmed the count.
- [Coverage](SRD-COVERAGE.md#ability-score-improvement-conformance-82) is canonical;
  [packet](ASI-CONFORMANCE.md#delivery) records acceptance, exact checks and limits.
-252 legal ASI allocation/class/ownership cases, all ability caps and all12 early
  entitlement checks pass. MainEN/ES+demoEN current UI flow passes for six classes
  at both sizes; all18 saved outputs match native results. No runtime/save changes.
  Existing78-check production baseline remains applicable. Old embedded
  `--advancement-check` assumes one-page Wizard flow; deferred maintenance is
  recorded separately. New tests use and verify the current two-page flow.
- Preflight03:41:40 to verified commit03:52:42 UTC2026-09-26:11m02s; one original
  issue reconciled, zero new issues. No claim of class completion or token savings.
- At that delivery there were no pending approvals, live builds/tests or code WIP. Requested Astra/high,
  configured model/effort unverified; no model change. Apply routing next batch.
- Previous Light/TWF/Loading delivery#59/#81/#56: runtime69771eb; see coverage
  and `LIGHT-ATTACKS.md`. Actual old0.6.53/0.6.54 writer fixtures retained.
- Main remains375f58f; no PR or main integration is claimed. #84/#85/#89 and
  incomplete class/level paths remain open. The full twelve-class goal is active.

## Active batch — Weapon Mastery

[Fixed packet](WEAPON-MASTERY.md) owns acceptance, source routes and proposed
controls MASTERY-1–4. Preflight began03:56:03 UTC2026-09-26; checkpoint04:56:03
(latest05:26:03). Branch `codex/srd-weapon-mastery`. Requested Astra/high;
configured model/effort unverified. One owner; no agents/new issues/settings.
Target #60/#85, with shared mastery portions of #140/#147/#111/#103. All eight
properties and five current class routes; unrelated class progression stays in
its existing issues. MASTERY-1–4 explicitly approved: “1. Yes. 2. Yes. 3. Yes. 4. Yes.”

Acquisition WIP: SRD mastery grants/validation, creation/preset/review choices,
Fighter4 selection, conditional PC39 and rules0.6.56. Combat effects and rest
replacement transactions/UI remain unimplemented; pure replacement policy alone
is tested. No issue closed. Latest committed/pushed compatibility preparation
`344a6bb`; subsequent acquisition is recorded in the commit containing this handoff.

Focused grants/training/advancement/weapons pass. MainEN/ES and demoEN Training,
Fighter4 ASI and Review Training controls pass at both sizes, with five saved
outputs compared against native expectations; logs `/tmp/mastery-ui-*.log`.
All50 native targets rebuilt; final regression **51/51 PASS** (4.05s),
`/tmp/mastery-native-final.log`. Localization972 and diff checks pass. No live
build/test. First-run10 failures were obsolete profile assertions or incomplete
training fixtures; corrected without dropping historical coverage. Acquisition
checkpoint is recorded on the feature branch, not declared full mastery delivery.

Next: Long Rest replacement orchestration/UI and eight combat properties. Two
additional decisions are being presented as MASTERY-5/6 (numbered1/2 in chat):
simultaneous mastery/Champion resolution selector and Graze/Vulnerability policy.
Do not implement those undecided behaviors. Independent approved rest work can
continue. The goal remains ACTIVE. No original issue closed;144 remain.

## Preserved decisions and exclusions

#97 remains physical spellbooks/copying; #165 retains missing spells. #98/#101
and the full Wizard tracker remain open. Two Evocation spells do not satisfy all
Savant learning. Q19–21 remain pending; Q23 is superseded by approved STYLE-1.
Unlimited ammunition is AMMO-UNLIMITED; do not reopen it. Camp/inn saves only.
Consult `SRD-DECISIONS.md` before questions; reuse approvals within recorded scope.

## Persistent execution agreements

Read `SRD-WORKFLOW.md`, `SRD-MODEL-ROUTING.md`, `SRD-REPO-MAP.md` and relevant
requirements. One owner/batch; no agents/tasks/settings changes/silent scope growth.
Rules stay in static SRD library; Core orchestrates; UI presents rule-owned choices.
No build-input edits during builds. Rebuild before testing; Godot runs serially.
Focused checks, then final integrated evidence; preserve all released-save history.
Commit/push only owned verified work. Coverage is canonical; handoff stays compact.

Questions restart at1. Play `/usr/bin/afplay /System/Library/Sounds/Glass.aiff`,
verify exit0, and write each full question visibly; widget alone is insufficient.
Do not ask repeatedly for routine choices already within accepted scope.
