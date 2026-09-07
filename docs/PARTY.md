# Shared campaign party preview

Issue [#2](https://github.com/stdarg/OpenGold/issues/2) connects the existing
character creator, town and combat scenes through reusable C++20 classes.
Run from PowerShell:

```powershell
.\build-rolf.cmd
.\review-character.cmd
```

## Preview flow

1. Finish a character, then select **Add to party** on the character sheet.
   The preview grants each new PC 250 gp. **View party** opens the roster.
2. Select **Create character** to make additional PCs. Six PC positions and two
   separate NPC positions are available. Adding retains the finished character
   by value; subsequently editing the creator's draft does not edit that member.
3. Select a roster member to see identity, portrait, abilities, live HP, gold,
   combat AC, resources and inventory. Selecting an active member also selects
   that character for exploration/shop interactions.
4. **Remove member** places a member in reserve. **Rejoin party** restores the
   same member without resetting their possessions or live state.
5. **Recruit preview guard** supplies an authored level-one fighter companion.
   Remove/recruit preserves that same guard. This is preview content, not an
   identification or automatic conversion of an original NPC.
6. **Explore New Phlan** runs Rolf's tour and the existing original town host.
   Visit the arms shop at (13,8), accept its offer, buy equipment and leave.
   **Return to party** becomes available when the event finishes.
7. Select purchased gear and **Equip selected**. Unequip an existing weapon or
   armor before equipping its replacement. Invalid combinations leave equipment
   unchanged. **Party combat** opens the existing tactical UI against a Bandit.
8. Finish the fight and return. HP, death state and spent resources persist.
   Reopening exploration resumes the town session at its previous position.

Closing the application discards this session. Campaign saving, rewards, rests
and advancement remain issues #1 and #6; cross-area travel remains #3. The
preview opens combat explicitly; it does not add general town combat encounters.

## Supported combat profiles

The rules module evaluates the created scores and equipment; these characters
do not select the old Vanguard/Adept/Healer fixture statistics. This first shared
party increment supports **level-one Fighter, Cleric and Wizard combat subsets**.
Other classes can be created and retained in the roster, but must be put in
reserve before combat. An unsupported active profile fails explicitly.

- Fighter: ordinary attacks and two Second Wind uses.
- Cleric: ordinary attacks and Cure Wounds, with two level-one spell slots.
- Wizard: ordinary attacks, Fire Bolt and single-target Magic Missile, with two
  level-one spell slots.
- Unarmed attacks, one equipped melee weapon, one armor and one shield are
  modeled. Ability modifiers, proficiency, AC, maximum HP and movement come from
  SRD calculations. Dwarven starting HP and Goliath speed are included.
- Other class/species/background features, skills, origin feats, lineage and
  spell selection, components, weapon mastery, thrown weapons, versatile attacks,
  size-specific movement and expanded spells remain unimplemented. The displayed
  combat subset is not a complete SRD character implementation.

Original merchandise maps by item type, not its display name:

| Original type | Rules key | Supported use |
| --- | --- | --- |
| 8 | dagger | All supported classes; finesse melee |
| 23 | mace | Fighter/Cleric |
| 33 | quarterstaff | All supported classes; one-handed melee |
| 36 | longsword | Fighter; one-handed melee |
| 50 | leather | Fighter/Cleric; AC 11 + Dexterity |
| 55 | chain_mail | Fighter; AC 16, speed penalty below Strength 13 |
| 59 | shield | Fighter/Cleric; +2 AC |

Enchanted, cursed, effect-bearing and other types remain purchasable inventory
but reject equipping until explicitly converted. The decoded original item and
template are retained as provenance, alongside the stable inventory item ID.
Original records and raw AD&D statistics are never rewritten into SRD values.

## Reusable native boundaries

`CampaignParty` owns an injected rules module and a value-owned `PartyState`:
roster, stable member IDs, active slots and selected slot. `PartyMember` owns a
`Character`, its one authoritative `Inventory`, equipment IDs, original item
provenance, purse, NPC source/morale and live `VitalState`.
All public member views are const; reacquire them after mutations.

The creator, `RolfTourSession` and `CombatDemo` share the same `CampaignParty`
owner. The native core has no Godot dependency. Godot owns presentation nodes;
temporary instantiated scenes have RAII owners until attached to the scene tree.

`RulesModule::character_profile` is an optional capability. It validates a
character sheet and equipped content keys and returns a module-owned profile
recipe and display statistics. `Participant` carries that recipe and optional
live vitals into combat. The SRD session owns turn state and exposes an opaque,
versioned resource continuation for each actor in its snapshot.

`CombatDemo` synchronizes accepted combat changes into the party. Roster, shop,
equipment and script writes are locked during combat. When combat finishes,
editing resumes; no fresh HP or spell resources are granted on the next fight.
The module identity is now 0.2.0 and combat checkpoint format is version 2,
including character recipes. Old module saves fail the identity check explicitly.
Campaign save/load is disabled rather than saving an incomplete party/ECL state.

## ECL adapter

The town adapter now connects LOAD CHARACTER/store, WHO, FIND ITEM,
PARTYSTRENGTH, movement CHECKPARTY and explicitly profiled ADD NPC requests to
the shared party. WHO presents actual members. Empty slots read as empty.
Purchases charge the selected member and synchronize the VM on shop exit.
LOAD CHARACTER's temporary script cursor can scan every slot without changing
the player's selection; WHO and the roster explicitly change that selection.
Event rollback restores the whole party as well as VM state.

The PC 1.3 command contracts are described in sections 7.14 and 12.3 of
[Lee's technical research](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869).
Movement CHECKPARTY returns minimum, maximum, mean and zero; aliased output
addresses receive the last write. ADD NPC uses a monster ID and morale.
These contracts are reference-derived and synthetic-tested, not new DOS traces.

The SRD compatibility mapping is explicit: script movement is speed in five-foot
squares; incapacitated members contribute zero movement. Party strength uses
the documented per-member formula with descending AC represented by `20 - AC`
and THAC0 by `20 - melee attack bonus`, retaining byte wrap. This is a conversion
policy, not a claim of unchanged encounter balance. Dead members contribute zero.

Thief-skill/effect CHECKPARTY variants still fault. `PhlanResources::npc_profiles`
accepts explicit native character conversions for this bank; unregistered NPCs
and the special hostile NPC 24 fault. General bank resolution, morale behavior
and original NPC conversion coverage remain campaign work. The preview guard
and synthetic ADD NPC test establish ownership/recruitment without inventing
conversions for original content.

## Verification

`opengold_party_tests` covers roster capacity, independent ownership, NPC live
state, purchasing/equipping and rejected operations, script-selected characters,
WHO tickets, party queries, ADD NPC, FIND ITEM branching, event rollback, combat
handoffs, no resource refill and deterministic character checkpoint continuation.

The Godot acceptance route uses the actual installed original town shop and
native control callbacks, then equips and fights with that same party:

```powershell
godot --headless --path godot res://scenes/character_creation.tscn -- --party-check
```

For local screenshots, omit `--headless` and append `--capture`. Inspect at
1280x900 and 1120x800. Existing character, town and combat checks remain available.
