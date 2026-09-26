# Weapon mastery delivery packet

## Frozen batch

- Active user goal; one owner, no agents or new issues.
- Deliver all eight mastery properties through actual chosen weapon kinds, with
  creation, advancement, Long Rest replacement, combat and retained saves.
- Original issues: #60 and mastery acceptance of #85/#140/#147/#111/#103.
  Target closure #60/#85; other parent issues close only if their remaining
  original acceptance is independently complete. Do not equate mastery with
  whole-class completion or add missing unrelated class features.
- Current source routes: Fighter/Rogue/Paladin/Ranger levels1–4, Barbarian level1.
  Fighter gains its fourth kind at4. Barbarian's third kind at4 remains part of
  its existing unavailable progression (#104–109), not a new progression batch.
  Higher levels and multiclass remain required in the original goal.
- No starting-equipment packages, ammunition tracking, other conditions/classes,
  unrelated fixes, new UI framework or reduced historical compatibility.
- Requested Astra/high for combat trigger interactions and persistence; actual
  configured model/effort unverified. No model/runtime setting changes. Escalate
  unresolved architecture/rule interpretation and two failed fixes; attempts0.
- Preflight began03:56:03 UTC2026-09-26. Checkpoint04:56:03, latest05:26:03.
  Earlier context-reset boundary is retained; no timing reset.

## Source and acceptance

Official [SRD5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
printed pp28–29,47–48,54,58–59,62 and90, checked2026-09-26.

| Source | Chosen kinds in this batch | Eligible weapons | Long Rest replacement |
| --- | --- | --- | --- |
| Barbarian1 |2|Simple/Martial Melee|At most one kind|
| Fighter1–3 /4 |3 /4|Simple/Martial|At most one kind|
| Paladin1–4 |2|Proficient kinds|Either/both kinds|
| Ranger1–4 |2|Proficient kinds|Either/both kinds|
| Rogue1–4 |2|Proficient kinds|Either/both kinds|

1. Validate counts, duplicate/unknown/unproficient kinds, source and acquisition
   levels in SRD. All38 ordinary catalog weapons are eligible where class rules
   allow; the compatibility Wand is not a weapon choice. Presets pre-generate
   choices; prior saves leave missing selections pending. Existing training,
   wounds, resources, equipment and advancement survive completion/replacement.
2. Cleave: after a melee hit, one additional same-weapon melee attack against a
   different creature within5 feet of the first and the attacker's reach, once
   per turn. Retain negative ability damage; omit positive modifier.
3. Graze: optional miss damage equal to the attack ability modifier, weapon damage
   type; no added weapon/Sneak/style dice or other damage increases. Apply relevant
   typed defenses and zero-damage behavior.
4. Nick: the Light extra attack can use its qualifying Attack action instead of
   the Bonus Action. Share the one extra-attack-per-turn limit with Light; retain
   different physical weapon identity and Two-Weapon Fighting behavior.
5. Push: optional straight-away forced movement up to10 feet, Large or smaller;
   legal reachable destinations, collision and no Opportunity Attack. Declining
   does not refund the triggering attack. Preserve actual occupied cells.
6. Sap: next attack roll has Disadvantage before the source's next turn starts.
   Slow: optional damage-triggered10-foot Speed penalty until that same boundary;
   multiple Slow sources cannot increase its penalty; compose with other effects.
7. Topple: optional Constitution save, DC8+attack ability modifier+proficiency;
   failure Prone. Vex: on damage, source's next attack against this target has
   Advantage before the end of the source's next turn. Consume/expire effects at
   exact boundaries; use shared Advantage/Disadvantage cancellation.
8. Cover misses/hits/zero damage/critical, melee/ranged/thrown/reaction, Action
   Surge, Savage Attacker, Sneak Attack, Champion movement, sizes/obstacles,
   incapacitation/death, PC/NPC ownership, rejected-command atomicity and all
   pending-decision continuation. No mastery from catalog metadata alone.
9. SRD owns grants, outcomes, expiry, targeting and codecs. Core orchestrates
   transactions through generic rules interfaces; UI only presents queries.
   Preserve all released save formats; capture actual0.6.55 writer before edits.
10. Verify normal creation, Review Training, fourth Fighter choice, camp/inn rest
    replacement/save/reload and combat. MainEN/ES and demoEN at1120×800/1920×1080,
    keyboard/mouse, focus, disabled states and Cancel/Escape. Focused native tests,
    then final integrated regression on freshly built targets. Commit/push and
    update coverage before closing only fully proven original issues.

## Approved controls

**MASTERY-1.** Reuse scrollable Training checkbox groups for Weapon Mastery in
creation and Review Training, listing weapon plus mastery name and selection
count. Require class-appropriate counts; preserve Back, lock prior selections in
Review Training, pre-generate presets, keep missing old-save choices pending.
For Fighter4 add a labeled fourth-weapon dropdown below Fighting Style in the
existing Level Up dialog (label y374, selector y406, width652, height38); Confirm
requires it. Cancel/Escape changes nothing; sheet lists selected weapon kinds.

**MASTERY-2.** After a completed Long Rest, use a centered700×670 Weapon Mastery
window for each eligible member, following existing spell choices. Reuse a
scrollable labeled checkbox list, current selections and count/replacement limit.
Keep current/Apply buttons; Escape keeps current; Apply requires a legal complete
set. Choices finish before exploration resumes; camp/inn saving preserves a
pending choice. No combat saving. Interrupted/ineligible rests do not grant a
replacement window; each completed rest grants only one.

**MASTERY-3.** Reuse a centered combat decision dialog for optional Graze, Slow,
Topple, Cleave and Push: show the property/effect and Use/Skip buttons. Cleave's
Use enters existing highlighted creature targeting; Push's Use highlights legal
landing cells up to10 feet directly away. Click/arrows confirm; Escape or Skip
(or the temporarily relabeled End Turn button) declines. Other actions wait;
original Action/Reaction stays spent. Sap/Vex apply automatically as specified.
The dialog is shown only when an optional effect can change the result. All
controls retain keyboard access and standard button styling. No combat saves.

**MASTERY-4.** Add Nick attack beside End Turn, in the otherwise unused ordinary
turn position occupied by Decline during reactions (x208,w174,h36). Show for an
actor with Nick; disable until legal. It opens a centered640×300 selector naming
eligible weapon and melee/ranged/thrown attack, followed by Target/Cancel.
Target uses existing highlighted targeting and keyboard cycle; Escape cancels
before submission. It spends no Bonus Action and shares Light's one-extra-attack
limit. Existing Light Bonus Action choices remain available where legal.

## Evidence and phase record

Preflight confirms mastery currently exists only as weapon catalog metadata;
no runtime mastery grants/effects are implemented. The existing Training,
advancement, rest-spell and combat weapon controls provide reuse routes above.
No production source or UI change is authorized by these proposals alone.

Compatibility preparation verified04:05:58 UTC. Production remains69771eb /
rules0.6.55. Five genuine fixtures and exact replay checks are recorded in
[fixture provenance](../tests/fixtures/README.md#weapon-mastery-baseline--actual-0655-writer).
`opengold_training_tests --mastery-baseline` and the full rebuilt training target
pass; logs `/tmp/mastery-baseline-build.log` and `/tmp/mastery-baseline-tests.log`.
Only tests/documents changed, so no game rebuild or repeated integrated run.
One compile correction (unique ownership) and one fixture correction (two-NPC
party limit); neither failure repeated. No original issue closed by preparation.
All four controls subsequently approved by “1. Yes. 2. Yes. 3. Yes. 4. Yes.”
Implementation now proceeds within those boundaries.

### Acquisition implementation checkpoint (WIP)

By04:39 UTC the acquisition paths and pure replacement policy were implemented.
Focused grant/training/advancement/weapon checks pass. MainEN/ES and demoEN
Training, Review Training and Fighter4 ASI checks pass at both supported sizes;
five UI saves match native expectations. Artifacts/logs use `/tmp/mastery-*`.
All50 native targets rebuilt; first regression41/51 passed. Ten older tests need
updated complete-training inputs or conditional profile assertions; compact old
rest-format fixtures retain explicitly pending training. Corrections in progress.
Combat effects and actual Long Rest replacement remain outstanding. This is not
batch delivery, and no original issue has been closed. Timing remains03:56:03
start,04:56:03 checkpoint; no reset. Token/cost measurement unavailable.

Acquisition verification recorded04:44:04 UTC (observed clock; final log records
4.05s test duration): all51 native checks pass after rebuilding50 targets.
`ctest --test-dir build/mac-check --output-on-failure -E '^opengold_godot_' -j6`
produced `/tmp/mastery-native-final.log`; targeted correction builds are in
`/tmp/mastery-regression-*-build.log`. Archery legacy rejection tests deliberately
omit mastery grants so the original Archery boundary remains the cause of failure.
Review found no SRD calculation in Core/UI. Localization972 and diff checks pass.
This acquisition checkpoint is committed on the feature branch; full combat/rest
acceptance and issue closures remain pending.

### Additional combat decisions (pending)

**MASTERY-5.** When mastery and Champion movement trigger simultaneously on a
player-controlled turn, add a labeled Resolve next dropdown above the approved
Use/Skip buttons in the same centered decision dialog. Each entry names an
eligible pending effect. Use/Skip handles only the selected entry, retaining the
others; existing movement/targeting controls resolve the chosen effect. Retain
separate critical-hit movement entitlements from a Cleave critical. Keyboard
access, no action refund, other actions waiting and no combat saves are unchanged.
On enemy turns the game chooses a fixed mastery-before-movement order. Required
UI confirmation under AGENTS.md; simultaneous effects are explicitly within
acceptance row8, so this does not add unrelated trigger systems.

**MASTERY-6.** Proposed Graze interpretation: its ability modifier is the upper
limit on damage. Resistance/Immunity can reduce it; Vulnerability cannot increase
it. For modifier+3, ordinary/vulnerable damage3, resistant1, immune0. This resolves
the wording that Graze damage increases only with its ability modifier against
the general Vulnerability doubling rule. Await user interpretation; do not treat
this proposal as approval.

Sources: SRD5.2.1 p90 (Graze) and official
[Simultaneous Effects](https://www.dndbeyond.com/sources/dnd/br-2024/rules-glossary#SimultaneousEffects).

### 60-minute checkpoint —04:56:24 UTC

Elapsed60m21s from03:56:03. Acquisition committed/pushed as `5b978df`; no original
issue closed. Rest replacement transaction, history replay, conditional campaign18
and approved dialog implemented in WIP. Native five-class PC/NPC tests pass,
including exact prior acquisition-writer replay and later training/advancement.
Combat effects remain unimplemented; MASTERY-5/6 await answers. No scope added.
Live dialog verification exposed checkbox focus loss during list refresh; one
test-timing adjustment did not fix it. The identified cause is hiding/re-showing
all checkboxes during refresh; fix only hides obsolete options. Rebuild and
keyboard verification next. Native API change necessitates final integrated
rebuild; current rest build logs `/tmp/mastery-rest-*-build.log`. No measured
token/cost savings claimed; no new agents/tasks or changed model settings.

### Long Rest implementation evidence (WIP)

Generic RulesModule replacement options/outcomes now drive per-member completed
rest windows. SRD owns eligible kinds, capacity, source preservation and limits.
Core records level/rest-session edits, replays them between advancement records,
and blocks exploration until choices finish. Campaign18 is conditional on a
pending training window or replacement history; older1–17 formats remain accepted.
Review Training adds missing unrelated choices without reverting replacements.
Keeping current consumes the same one-use window and is recorded in history.

Native `mastery_rest_checks.h` passes for all five class routes and PC/NPC owners:
Short/interrupted/ineligible/dead/reserve exclusions, stale/duplicate/invalid
commands, atomic previews, preservation, repeated rests, level1-to4 replay and
Review Training after replacement. The actual prior acquisition writer fixture
round-trips byte-for-byte; see fixture provenance. Final native regression awaits
the50-target rebuild, `/tmp/mastery-rest-native-build.log`.

The shared700×670 dialog and `mastery_rest_view_tests.gd` pass mainEN/ES and demoEN
at1120×800 and1920×1080. Five eligible members follow Wizard spell choices;
keyboard selection/Apply/Escape, sequential windows and pending-codec reload pass.
Screenshots are `/tmp/mastery-rest-{main,demo}-{en,es}-{1120,1920}/`; only main has
Spanish runs. Initial runs exposed a checkbox focus bug (fixed by retaining
visible controls) and test-startup overrides of locale/size (fixed in the script).
Corrected minimum-size Spanish screenshot was inspected; dimensions verified.
Existing full rest-control acceptance passes (`/tmp/mastery-rest-neighbor.log`).
976 localized messages and diff validation pass. These tests verify pending save
serialization, not yet an accessible Save button inside the modal.

**MASTERY-7 (pending control approval).** The approved exclusive modal covers the
existing Save game control. To make unresolved choices saveable through the
player UI, place a standard Save game button at its bottom left (x24,y614,w234,
h40), beside Keep current/Apply. Reuse the existing camp/inn save dialog; return
to the same pending mastery choice after saving or Cancel, with keyboard access.
Saving must not apply or discard the choice; loading retains its entitlement.
No combat saving. This is needed to complete acceptance row10 and MASTERY-2's
pending-save behavior; await approval before adding this control.

Rest verification recorded05:04:27 UTC. Final rebuilt native suite **51/51 PASS**,
4.03s, `/tmp/mastery-rest-native-final.log`. The only initial native regression
failure was the Temporary HP test attempting combat while a new rest choice was
pending; it now verifies rejection, explicitly keeps training, then completes its
original pool-preservation assertions. No production behavior weakened. Corrected
UI runs and existing rest controls pass as described above. Scope/architecture
review confirms rule outcomes stay in the static SRD implementation and Core/UI
only orchestrate/present them. Checkpoint commits do not close #60/#85; combat and
pending Save control approval remain outstanding.
