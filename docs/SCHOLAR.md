# Wizard Scholar — #100 frozen batch

- Authorization: standing SRD goal, explicitly continued by the user. One owner,
  existing #100 only, branch `codex/srd-scholar`; no new issues/agents/tasks.
- Player outcome: Wizard level 2 grants Expertise in one proficient skill from
  Arcana, History, Investigation, Medicine, Nature or Religion. Levels 3–4 retain
  that selection; level 1 and other classes receive no Scholar entitlement.
- Acceptance: all six eligible skills with real proficiency provenance; exactly
  one choice; doubled proficiency once, correct modifiers/source display,
  invalid/untrained/replaced/forged choice rejection, ordinary advancement,
  actual Medicine use, pending old-save choices, existing selection locks,
  current/previous campaign and combat continuation. Preserve wounds, equipment,
  spells, Arcane Recovery and all other expenditure. Main/demo player paths and
  game EN/ES/demo EN visual checks at both supported sizes.
- Exclusions: Ritual Adept, Evoker, additional spells, other class Expertise,
  new skill/source catalogs, levels above four and multiclassing. No unrelated
  training redesign or reduced compatibility history.
- Reuse: existing sourced Expertise arithmetic and skill display, transactional
  advancement, Review Training controls, save replay and static SRD library.
  Keep entitlements and eligibility in SRD; Core owns generic history/transactions.
- Source: [SRD 5.2.1 p.78](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
  Scholar, verified at PDF lines 6615–6618. All six candidate skills are already
  in the existing Wizard class skill catalog; no new proficiency route is needed.
- Model: requested `gpt-6-astra` / `high`, retained for advancement-history and
  legacy pending-choice migrations. Actual configuration not independently
  verified; no switch. Escalate out-of-tier decisions or two unsuccessful fixes
  of the same failure. This does not authorize delegation.
- Verification: capture real 0.6.49 writer evidence before changing persistence;
  focused choice/bonus/advancement tests, final native regression and affected
  game/demo checks. No stale binaries or editing active build inputs.
- Timing: frozen 2026-09-25 19:57:47 UTC; checkpoint 20:57:47, maximum 21:27:47.
  Record actual phase endpoints; no invented durations or token savings.

## Pending SCHOLAR-1 — visible question 1

Propose reusing the disabled ability-points area of the Wizard level-two Level Up
window for a labeled Scholar Expertise dropdown. It lists eligible proficient
skills; Confirm requires a valid selection and grants it with the new level.
Existing level-two through level-four Wizards keep missing choices pending and
complete them in the existing Review Training checkbox groups. Previously chosen
training stays locked. Cancel discards edits; keyboard access and wounds/resources
are preserved. The existing windows retain their dimensions. This is a proposal,
not approval; do not implement dependent UI before the user answers.

Next independent work: capture actual previous-writer state (including spent
Arcane Recovery), then rules-owned Scholar grant/choice validation and generic
advancement/pending-training persistence. Freeze persistence details from the
existing replay paths; do not store level-two choices as false level-one grants.
