# Light attacks and Two-Weapon Fighting — frozen next batch

## Batch card

- Standing goal ACTIVE. One owner, no new tasks/agents/issues. Preflight began
  2026-09-26 00:45:02 UTC; checkpoint01:45:02, maximum02:15:02. Preserve this
  start across approval waits. No implementation yet; LIGHT-1/2/3 pending.
- Original requirements: #59 Light extra attacks, #81 Two-Weapon Fighting,
  #56 Loading action boundaries. Bounded source integration under#85/#140/#147.
  Player outcome: equip and choose two one-handed weapons, earn a legal Light
  Bonus Action attack with a different physical weapon, and receive the feat's
  damage benefit through actual class entitlements. Loading remains per action,
  Bonus Action or Reaction, including separate Action Surge actions.
- Requested Astra/high, actual configuration unverified; no model change. Rules
  interactions, weapon identity, hand state and historical saves require this
  tier. Escalate after two failed fixes of the same failure. No delegation.
- Reuse physical carried/ground equipment, grip, legal command targeting, the
  Bonus Action row, Savage/Sneak decisions and the style routes in bf9ec73.
- Exclude Nick/other mastery#60, level5+/Extra Attack, multiclassing, other class
  features and a new artwork pipeline. Unlimited ammunition remains approved;
  do not reopen expenditure/recovery or change existing ammunition/hand behavior.
- Static SRD library owns attack eligibility, identities, hand legality, action
  budgets and modifiers. Core owns inventory/transactions/history only. UI
  presents rules-provided hand and attack choices, without SRD calculations.

## Fixed acceptance

1. Light qualification comes from an actual Attack action on the actor's turn,
   not a reaction/spell or the extra attack itself; a hit is not required.
   The extra attack occurs later on that turn, costs the one Bonus Action and
   uses a different physical Light weapon. Identical weapon types/stack units
   remain distinct actual items; the same weapon cannot qualify itself twice.
2. Preserve negative damage modifiers without the feat. The feat adds the normal
   modifier exactly once, including critical/Savage interaction. Do not grant an
   extra bonus to ordinary attacks, Sneak dice or spells. Check Strength/Finesse,
   melee/ranged/thrown, proficiency, ranges and typed defenses independently.
3. Actual Fighter1–4, Paladin/Ranger2–4 feat choices, Fighter replacement and
   independent level-four feats; prerequisite/source/nonrepeatability validation.
   Other starting classes may use Light without possessing this feat.
4. Actual equipped/carried hand state and named weapon selection, shield/grip
   conflicts, drop/pickup/throw/recovery, Opportunity Attacks and no duplicated
   items. Reuse existing transaction costs; any additional hand-manipulation
   workflow needed beyond the proposed controls is a dependency to report before
   implementing. No silently invented free equip/stow actions.
5. Loading remains tied to each particular action, Bonus Action or Reaction,
   never a once-per-turn limit. Action Surge and a legal Light Bonus Action are
   distinct opportunities. No ammunition tracking or recovery is added.
6. Actual PC/recruited campaign combat, ordinary camp/inn save/reload and rests;
   pending-hit continuation and historical profiles remain supported. Capture
   genuine0.6.53 writer evidence before changing any affected writer. No combat
   save controls, fabricated old fixtures or reduction in supported history.
7. Main EN/ES and demo EN,1120×800/1920×1080: controls, keyboard/mouse targeting,
   disabled states, Cancel/Escape atomicity and native comparison of saved state.
   Focused checks, final integrated regression, coverage, commit/push then only
   proven original-issue closures. Record actual phase times from here onward.

## Preflight evidence

Current Core `CampaignParty::equip` replaces the existing weapon; SRD profile
validation rejects a second weapon. This must change for the real two-weapon
path, using rule-owned validation rather than adding SRD branches to Core.
Physical item identities and Thrown/pickup mechanics already exist. Both combat
surfaces have a Bonus Action dropdown, but it currently contains Rogue choices.
Party inventory's existing Grip/Review Training/Spellbook row is full at1120px;
a centered hand-choice dialog avoids crowding it.

Official [SRD5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf)
pp88–90 and177 checked2026-09-26. Light depends on a different weapon; its example
of different hands is not an additional restriction. The Attack action's weapon
interaction and Thrown's draw permission must retain their actual boundaries.

## Proposed controls — approval required

**LIGHT-1.** In main/demo, when Equip selected would add or replace a second
one-handed weapon, open a centered660×340 dialog. Show the selected weapon and
two labeled choices, Main hand and Other hand, each naming the current item or
Empty. Show replacement/conflict text before confirmation; disable illegal
choices. Equip confirms atomically; Cancel/Escape changes nothing. Retain
keyboard navigation. List held weapons by hand in inventory/sheet. Existing
available sprite poses may depict only the primary weapon; labels identify both.

**LIGHT-2.** Add a labeled Weapon dropdown in a new44px row immediately above
the existing Bonus Action row in main/demo. It selects the weapon for ordinary
attacks and reactions without spending an action. Label x24,w180; selector
x214,w450,h36. Move lower rows down44px and reserve that height from the board.
Show the row only when weapon selection is meaningful. Extend the Bonus Action
choices with named legal Light attacks (melee/ranged/thrown). Use Bonus Action
enters existing target highlighting; click/keyboard confirms and spends the
Bonus Action only on submission. Escape cancels freely; illegal choices disable.
Full weapon names remain available in dropdowns/tooltips. No combat saving.

**LIGHT-3.** Add Two-Weapon Fighting to the existing approved Fighter Training/
Review Training and all three classes' advancement selectors. Automatically
apply its beneficial damage modifier to eligible Light extra attacks; no new
per-hit decision. Preserve existing source, replacement and separate-feat rules.
