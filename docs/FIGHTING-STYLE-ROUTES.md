# Fighting Style routes — proposed controls, frozen acceptance

## Batch card

- Standing SRD goal; one owner, no agents/new tasks/issues. Implementation
  awaits the specific new controls/optional-rule choices below.
- Player outcome: Archery, Defense and Great Weapon Fighting through actual
  Fighter, Paladin and Ranger entitlements at every supported level through4.
  Original feat owners#78/#79/#80. Existing #85/#140/#147 own class integration;
  they stay open for other styles, mastery and cantrip alternatives.
- Include normal Paladin/Ranger advancement1→4 with fixed HP and level4
  available feat/ASI choices, because actual level2 entitlements are required.
  Do not claim full Paladin/Ranger support. Show their unsupported features
  explicitly in the existing note. No forged high-level test-only route.
- Fighter starting selection and level4 choices include the new feat. Its
  class-granted starting style can optionally change when gaining levels2–4;
  an independently acquired level4 feat is a different entitlement.
- Exclude Two-Weapon Fighting/Light/Nick, mastery, Blessed/Druidic Warrior,
  spellcasting, other Paladin/Ranger features/subclasses, level5+, multiclassing.
  Their original acceptance stays required. No new spell/item/action systems.
- Requested `gpt-6-astra/high` for sourced replacement, progression/history
  migration and optional damage interaction; actual configuration unverified.
  No switch. Escalate after two failed fixes of the same issue; no new agents.
- Preflight observed2026-09-26 00:08:10 UTC; checkpoint01:08:10, maximum01:38:10.
  Preserve original start across approval wait. No implementation yet.
- Reuse feature grants, Training/Review Training, advancement history, grip,
  shared damage roller, pending Savage hit and current-source public tests.
- Static SRD library owns choices/eligibility/damage/progression. Core handles
  generic advancement transactions/history; Godot presents rules-provided options.

## Acceptance

1. All three feats require the actual Fighting Style feature and cannot repeat.
   Fighter receives it at1, Paladin/Ranger at2. Source/acquisition provenance
   remains distinct from level4 feat selection; invalid/duplicate choices reject
   atomically without wounds, resources, equipment or history changes.
2. Archery applies only to Ranged weapons (+2), including Dart and excluding
   thrown Melee weapons/unarmed fallback. Defense grants+1 only while wearing
   Light/Medium/Heavy armor, never merely holding a shield.
3. Great Weapon Fighting affects eligible Melee weapons held in two hands with
   Two-Handed/Versatile. Each rolled damage die1/2 becomes3 without another RNG
   draw. Flat modifiers unchanged, critical dice each treated once; Savage's
   two rolls use the same policy. No spell/unarmed/thrown/Ranged-weapon benefit.
   Grip, Heavy penalties, defenses, reactions and saved pending rolls stay correct.
4. Ordinary creation/XP advancement/combat/rest/reload through levels1–4 for
   all three source classes, PC and recruited routes. Fighter replacements
   preserve valid prior choices and cannot duplicate the separately acquired feat.
5. Existing saves/history remain supported. Use actual prior-writer GWF fixtures
   and existing current module0.6.52 evidence; capture another writer only for a
   missing acceptance case. Version new grants/choices where validation requires.
6. Main EN/ES and demo EN controls/input/render at1120×800 and1920×1080;
   exact native comparison of saved choices. Relevant focused checks, final
   native regression for shared rules/history, affected Godot checks/builds.
   Coverage, tested revision, commit/push then only proven full issue closures.

## Proposed decisions (not yet approved)

STYLE-1 (replaces unresolved Q23): automatically apply beneficial Great Weapon
Fighting die replacement on every eligible attack. No new per-hit dialog. Savage
shows the modified totals; it still changes only weapon dice. This chooses the
optional benefit on the player's behalf and needs explicit confirmation.

STYLE-2: Add Great Weapon Fighting to existing Fighter Training/Review Training
and eligible level4 feat dropdowns. In main/demo's existing level-up window,
reuse the unused spell-choice area for a labeled Fighting Style dropdown
(x24,y325,w652,h38; label x24,y292). For Fighters gaining levels2–4, default Keep
current; list eligible replacements. For Paladin/Ranger level2, require one of
Archery, Defense or Great Weapon Fighting. Preserve selections while reviewing;
Confirm applies atomically, Cancel discards, keyboard navigation retained.
At level4 the existing feat/ability controls remain independently usable.
The note explicitly identifies unsupported class features and cantrip alternatives;
these choices do not imply full class completion. Presets/legacy choices follow
existing pre-generated/pending policies; do not invent historical selections.

## Source

Official [SRD5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
pp47,54,59,87–88, checked2026-09-26. Fighter alone has the stated style-replacement
permission. Paladin/Ranger gain the feature at2 and have separate cantrip
alternatives; those alternatives are preserved under their class issues.

## Remaining work

Obtain STYLE-1/2 decisions before dependent behavior or UI. Implement and verify
this exact batch; report any blocking dependency instead of silently adding it.
