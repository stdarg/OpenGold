# Shared campaign party preview

Issue [#2](https://github.com/stdarg/OpenGold/issues/2) connects the existing
character creator, town and combat scenes through reusable C++20 classes.
Run from PowerShell:

```powershell
.\build-rolf.cmd
.\review-character.cmd
```

## Preview flow

Losing shared-party combat opens a defeat window with **Reload a Saved Game**
and **Exit to OS**. Return to party is blocked; defeat does not revive anyone.
Reload uses the existing named-save dialog. Cancelling or rejecting a damaged
save retains the defeated campaign; a confirmed valid load replaces it and
removes the old combat scene. Standalone combat demos retain their restart controls.

Run `.\review-character.cmd --defeat-check` for the automated loss/reload/exit
check and a capture at `user-data/party-defeat.png`. It uses an isolated save
profile. This completes defeat handling for shared-party combat; town-to-Slums
travel and original tactical geometry are still pending.

1. Finish a character, then select **Add to party** on the character sheet.
   The preview grants each new PC 250 gp. **View party** opens the roster.
   Alternatively, **Character Pool** opens 48 level-one characters, four for each
   SRD class, with varied alignments, genders, strong ability scores, and selected
   portrait heads/bodies. The ready/action sprite palettes use colors sampled
   from their portraits and matched to the original 16-color sprite palette.
   Select a candidate to preview the sheet and art, then **Add to party**.
   The same candidate cannot be added twice; use Rejoin for a reserved member.
2. Select **Create character** to make additional PCs. Six PC positions and two
   separate NPC positions are available. Adding retains the finished character
   by value; subsequently editing the creator's draft does not edit that member.
3. Select a roster member by name to see the same character sheet as the creator,
   including gender between race and class, portrait, scores, saving throws,
   live HP, gold, combat AC, resources and inventory. **Modifiers** opens the
   detailed modifier modal for that member. Selecting an active member also selects
   that character for exploration/shop interactions.
4. **Remove member** places a member in reserve. **Rejoin party** restores the
   same member without resetting their possessions or live state.
5. **Recruit preview guard** supplies an authored level-one fighter companion.
   Remove/recruit preserves that same guard. This is preview content, not an
   identification or automatic conversion of an original NPC.
6. **Explore New Phlan** runs Rolf's tour and the existing original town host.
   Visit the arms shop at (13,8), accept its offer, buy equipment and leave.
   **Return to party** becomes available when the event finishes. The street or
   encounter view occupies only its required width at the left. Immediately to
   its right, scrollable member rows show name, class, AC and current/maximum HP.
   Clicking a row opens the character sheet without leaving town. During an
   active dialogue it is read-only; outside events it also selects that member.
   Down turns 180 degrees in place; Up steps forward and Left/Right turn 90 degrees.
7. In town, **Inventory** offers **Equip** and **Unequip** for the selected item,
   with the training penalty visible when selecting and equipping it. The party
   screen also offers **Equip selected**. Unequip an existing weapon or
   armor before equipping its replacement. Invalid combinations leave equipment
   unchanged. **Party combat** opens the existing tactical UI against a Bandit.
8. Finish the fight and return. HP, death state and spent resources persist.
   Reopening exploration resumes the town session at its previous position.

Closing the application discards this session. Campaign file saving remains #1;
the bounded [recovery and advancement subset](RECOVERY.md) covers part of #6.
Cross-area travel remains #3. The
preview opens combat explicitly; it does not add general town combat encounters.

## Supported combat profiles

The rules module evaluates the created scores and equipment; these characters
do not select the old Vanguard/Adept/Healer fixture statistics. This first shared
party increment supports **level 1-2 Fighter, Cleric and Wizard combat subsets**.
All twelve classes have exploration equipment profiles, including armor training,
unarmored AC and HP. Other classes must still be put in
reserve before combat. An unsupported active profile fails explicitly.

- Fighter: ordinary attacks and two Second Wind uses.
- Cleric: ordinary attacks and Cure Wounds, with two level-one spell slots
  (three at character level 2).
- Wizard: ordinary attacks, Fire Bolt and single-target Magic Missile, with two
  level-one spell slots (three at character level 2).
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
| 8 | dagger | Simple weapon; all classes trained; finesse melee |
| 23 | mace | Simple weapon; all classes trained |
| 33 | quarterstaff | Simple weapon; all classes trained; one-handed melee |
| 36 | longsword | Barbarian/Fighter/Paladin/Ranger trained; one-handed melee |
| 50 | leather | All except Monk/Sorcerer/Wizard trained; AC 11 + Dexterity |
| 55 | chain_mail | Fighter/Paladin trained; AC 16, speed penalty below Strength 13 |
| 59 | shield | Barbarian/Cleric/Druid/Fighter/Paladin/Ranger trained; +2 AC |

Untrained use is allowed under [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf):
- Weapons omit the +2 level-one proficiency bonus on attacks.
- Shields provide no AC bonus without training.
- Armor retains its AC but imposes disadvantage on Strength/Dexterity D20 Tests
  and prevents spellcasting. Combat applies this to initiative and physical
  attacks; the saving-throw dialog identifies disadvantage on Strength/Dexterity
  saves. General ability checks and additional saving-throw combat effects are
  not yet implemented.

Training uses the base level-one class traits; optional Cleric/Druid orders that
add training are not selected by this creator. Equipping or removing gear rebuilds
these effects from the current loadout. Duplicate weapon/armor/shield slots remain
invalid, and heavy armor's low-Strength speed penalty is shown separately.

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

`CombatDemo` synchronizes accepted combat changes into the party. Campaign
victories award 300 XP once under a stable encounter key; reaching 300 XP
advances the supported character to level 2 and updates the rules profile's HP
and caster slots. The authored preview award is one-time, including across scene
recreation; original rewards require explicit mappings. Roster, shop,
equipment and script writes are locked during combat. When combat finishes,
editing resumes; no fresh HP or spell resources are granted on the next fight.
The module identity is now 0.3.0 and combat checkpoint format is version 2,
including character recipes. Old module saves fail the identity check explicitly.
[Campaign file saves](SAVES.md) now retain the complete supported party and idle New Phlan state. Native `PartyState` checkpoints remain the in-session rollback mechanism. Pending dialogue/services and combat are not campaign save boundaries.

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
and original NPC conversion coverage remain campaign work. An eligible,
uninterrupted long rest advances campaign time by eight hours. The original
city-watch interruption grants no recovery. Temple Cure Wounds costs 100 gp for
a wounded living active member; resurrection is unsupported. See
[service limits and checks](RECOVERY.md). The preview guard
and synthetic ADD NPC test establish ownership/recruitment without inventing
conversions for original content.

## Verification

`opengold_party_tests` covers roster capacity, independent ownership, NPC live
state, purchasing/equipping and rejected operations, script-selected characters,
WHO tickets, party queries, ADD NPC, FIND ITEM branching, event rollback, combat
handoffs, no resource refill and deterministic character checkpoint continuation.
The [recovery route](RECOVERY.md) adds XP thresholds, scene reentry, long rest,
original temple/inn services, interruption and payment/rollback checks.

The Godot acceptance route uses the actual installed original town shop and
native control callbacks, then equips and fights with that same party:

```powershell
godot --headless --path godot res://scenes/character_creation.tscn -- --party-check
```

For local screenshots, omit `--headless` and append `--capture`. Inspect at
1280x900 and 1120x800. Existing character, town and combat checks remain available.
