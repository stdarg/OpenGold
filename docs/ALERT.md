# Alert feat delivery packet

## Frozen batch

- Standing SRD goal; original issue #74. One owner, no agents or new issues.
- Player outcome: Alert adds proficiency to Initiative and permits one optional
  swap with a willing eligible ally after rolling, before the first turn.
- Sources: automatic Criminal background grant for all current class creation
  routes; selection through existing level-four feat entitlements. Nonrepeatable;
  reject duplicate acquisition. Human's extra Origin-feat selector remains #71;
  this batch does not implement that separate species feature.
- Exclusions: starting equipment/wealth (#84 and background packages), other
  feats, new class progression, initiative tie UI, general AI improvements and
  combat saving controls. Existing tie adjudication stays unchanged.
- Reuse: feature-grant provenance, existing feat selectors, initiative sorting,
  shared modal/control styling and rules-owned pending decisions. Core performs
  orchestration; the statically linked SRD library owns bonuses and legality.
- Requested Astra/high for combat-start sequencing and save migrations; actual
  runtime selection unverified. No settings change or delegation. Escalate after
  two unsuccessful fixes of the same failure or a new out-of-scope dependency.
- Preflight observed2026-09-26 15:34:23 UTC. No implementation started. ALERT-1
  below needs control approval under AGENTS.md before UI implementation.

## Acceptance

1. The feat's proficiency bonus is added once, scales from the actual character,
   and combines with existing Initiative Advantage/Disadvantage. Incapacitation
   blocks swapping, not the Initiative proficiency bonus.
2. Roll Initiative once. Each eligible holder may swap current Initiative totals
   with one willing ally in the same combat, or keep the result. Both creatures
   must lack Incapacitated. Re-sort after a swap without rerolling or spending
   actions, movement, time, spell slots or resources.
3. Preserve other holders' pending decisions. Resolve all choices before starting
   any turn or applying turn-start recovery/effects. Reject stale/illegal commands
   atomically. Enemy AI keeps its Initiative by default.
4. Criminal creation/presets and current level-four feat selection use real
   entitlements, with source/provenance checks and no duplicate feat acquisition.
   Fixed background grants migrate without replacing chosen training or feats,
   healing wounds, replenishing resources or changing wealth/equipment.
5. Capture genuine0.6.60 campaign/combat fixtures before changing the writer.
   Existing combats retain their rolled order and do not reopen initial choices.
   Retain every supported save version; new pending decisions round-trip exactly.
6. Verify main/demo actual controls, keyboard and mouse, English/Spanish main and
   English demo at1120×800/1920×1080. Focused native conformance plus final affected
   native regression, existing creation/advancement and adjacent combat controls.
7. Record completion in SRD-COVERAGE, commit/push owned checked changes, then close
   #74 only after its acceptance is proven. No background/species issue closure
   is implied by delivery of its Alert dependency.

## Source and implementation entry points

[SRD5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
feat Alert; [official rules text](https://www.dndbeyond.com/sources/dnd/br-2024/feats#Alert).
Alert is an Origin feat with Initiative Proficiency and Initiative Swap; it is
not repeatable. Current `feature_grants.cpp` lacks Alert and `srd5.cpp` rolls
Initiative using Dexterity and existing Advantage/Disadvantage, then immediately
starts combat. Feature grants, character profile/migration, that initialization
boundary, generic pending-choice interfaces and shared combat views are the
bounded source routes. Capture old-writer evidence before editing them.

## Proposed control: ALERT-1 (pending)

In both game and demo, use a centered640×360 Alert dialog before the first combat
turn. Show the selected holder's Initiative total and a labeled Ally dropdown
containing eligible party allies and their current totals. Standard styled
“Swap initiative” and “Keep initiative” buttons resolve only that holder's choice.
Swap is disabled until an eligible ally is selected. Escape keeps that holder's
Initiative. Other combat actions wait.

When several party Alert holders remain, a labeled Resolve next dropdown selects
which holder to resolve; otherwise omit that selector. Swaps update displayed
totals, and every holder chooses once. Support normal keyboard focus/selection
and mouse input. Enemy AI keeps its rolls automatically. Existing feat-selection
controls gain Alert as an ordinary eligible option; no separate creation layout.

## Preflight findings for subsequent Fighter integration

#84 explicitly requires starting equipment choices, still absent from normal
creation's250gp policy. #89 must be reconciled with the remaining feat catalog;
do not close it merely because Weapon Mastery and Fighting Styles are complete.
Those issues remain open. Alert is a bounded original feat requirement and a
Criminal-background dependency; no new issue or silent prerequisite is added.
