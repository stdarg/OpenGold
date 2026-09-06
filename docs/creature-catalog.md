# Pool of Radiance creature statistics

`opengold::por::CreatureCatalog` loads every monster/NPC character template from
the user's original DOS game directory. It owns its decoded data and has no
Godot dependency. The inspected installation contains **172 records**.

The API is in
[`creature_catalog.h`](../src/OpenGold.Core/include/opengold/creature_catalog.h).
The binary record types and independent, bounds-checked decoders are in
[`pool_radiance.h`](../src/OpenGold.Formats/include/opengold/pool_radiance.h).
Link the CMake target `opengold_core` (which publicly links `opengold_formats`).

## Use from C++

```cpp
#include <opengold/creature_catalog.h>
#include <iostream>

void inspect(const std::filesystem::path& game_directory)
{
    using namespace opengold::por;
    const auto catalog = CreatureCatalog::load(game_directory);

    for (const auto& [id, creature] : catalog.all()) {
        std::cout << id.key() << ' ' << creature.stored.name
                  << " HP=" << unsigned(creature.stored.max_hit_points)
                  << " AC=" << creature.stored.base_armor_class
                  << " THAC0=" << creature.stored.base_thac0 << '\n';
    }

    // IDs are scoped to an archive bank. This is MON2CHA.DAX record 31.
    if (const auto found = catalog.find({2, 31})) {
        const auto& troll = found->get();
        const auto& primary = troll.stored.base_attacks[0];
        // primary.attacks_per_round() == 2; primary.damage == 1d4+4.
        for (const auto& effect : troll.effects) {
            if (effect.definition.regeneration) {
                const auto& regeneration = *effect.definition.regeneration;
                std::cout << regeneration.hp_per_round << " HP/round\n";
            }
        }
    }

    // Case-insensitive exact lookup; retains all three stock troll records.
    const auto trolls = catalog.find_by_name("troll");
}
```

`load()` throws `CatalogError` with file/record context on missing required
assets, bad compression, invalid record lengths, missing item templates, or
orphan item/effect records. Loading is transactional: a failed load cannot
modify a previously loaded catalog. Lookup results borrow from the catalog;
keep it alive and do not move/replace it while using those references. Creatures
can also be copied as ordinary C++ values. Read-only lookups can be shared
across threads if the catalog's lifetime is externally guaranteed.

## Creating runtime monsters and NPCs

[`creature_factory.h`](../src/OpenGold.Core/include/opengold/creature_factory.h)
adds `CreatureFactory` and `CreatureInstance` to the same `opengold_core` target.
The factory loads the complete catalog once and creates instances by bank and
record ID. Monster and NPC records use the same API; duplicate names do not
silently select an arbitrary variant.

```cpp
#include <opengold/creature_factory.h>

void encounter(const std::filesystem::path& game_directory)
{
    using namespace opengold::por;
    const auto factory = CreatureFactory::load(game_directory);
    auto troll = factory.create({2, 31});
    auto another_troll = factory.create({2, 31});

    // All loaded stats, equipment, spell fields and effects remain accessible.
    const auto& stats = troll.definition();
    troll.take_damage(7); // Does not change another_troll or the catalog.
    troll.heal(3);        // Clamped to this instance's maximum HP.

    // Example allowance selected by the encounter rules: two primary attacks,
    // one secondary attack, and six movement units. Movement units and any
    // move/attack restrictions are defined by the caller, not inferred here.
    if (troll.begin_turn(1, {{2, 1}, 6})) {
        const auto choices = troll.available_actions();
        if (troll.try_attack(0)) {
            // Caller resolves this base attack against a validated target,
            // using stats.stored.base_attacks[0] and the combat rules.
        }
        troll.end_turn();
    }
}
```

An existing catalog can be transferred into `CreatureFactory` by value. The
factory and its instances share immutable catalog ownership with
`std::shared_ptr<const CreatureCatalog>`; instances remain valid after the
factory is destroyed. HP and turn state belong to each instance. Copying an
instance creates an independent state snapshot; the encounter layer owns token
identity. Equipment and effect definitions remain immutable source data.

By default, instances start at the template's stored maximum HP, not its raw
current HP. `create(id, positive_max_hp)` supports HP rolled or scaled externally.
Unknown IDs and nonpositive initial HP throw `CatalogError`; a zero-HP template
requires an explicit positive override. Negative damage/healing throws
`std::invalid_argument` without mutation. Large values clamp without overflow.
Zero HP cancels the turn and prevents actions; it does not classify a creature
as permanently dead. The caller decides when healing or revival is permitted.

Instances start without an active turn. `begin_turn(number, budget)` accepts
strictly increasing turn numbers and refuses to replace an active turn or replay
a completed/skipped turn. Combat rules provide the two base-attack counts and
movement allowance after resolving fractional attack rates and other modifiers.
Allowances for absent base-attack slots throw `std::invalid_argument` without
starting a turn. `available_actions()` lists remaining base attacks, movement,
and the option to end the turn. `try_attack(slot)` and `try_move(distance)` consume
only valid remaining allowances; failed requests do not spend anything.

`set_incapacitated(true)` cancels the turn. Healing or clearing incapacitation
does not restore its budget. Starting a turn at zero HP or while incapacitated
records that turn as skipped. A later turn can begin after recovery.

This is action availability and consumption infrastructure, not a full combat
resolver. It does not infer equipped weapon attacks, cast spells, activate items,
execute special attacks or passive effects, apply damage resistance, validate
targets or paths, or enforce movement/attack tradeoffs. Spell and special-effect
data remain accessible through `definition()` for those future rules. The caller
must validate an action before consuming its allowance and use the incapacitation
API when resolved conditions prevent acting. Raw template status bytes are not
automatically interpreted as live conditions.

## Available data

| Area | API | Interpretation |
| --- | --- | --- |
| Identity | `id`, `stored.name`, race/class/type/NPC flag | Original source identity and name; no name-based deduplication or inferred PC/NPC filtering. |
| HP | `max_hit_points`, `rolled_hit_points`, `drained_hit_points` | Stored template fields. |
| AC / THAC0 | `base_armor_class`, `base_thac0` | Descending AC and THAC0 decoded as `60 - byte`. |
| Attacks | `base_attacks[0..1]` | Both stored base/unarmed attack slots, exact half-attack rate, dice count/sides and signed damage modifier. |
| Abilities | `stored.abilities`, `creature.abilities` | All six scores and exceptional strength; separate STR, DEX and CON reference combat adjustments. |
| Equipment | `equipment` | Full inventory, readiness, curse/reveal flags, magic/save bonus, weight, quantity, value, name components and raw effect bytes. |
| Item templates | `equipment[i].base` | Weapon damage against small/medium and large targets, built-in damage modifiers, rate, range, armor protection, slot, hands, restrictions and ammo flags. |
| Equipment contributions | `equipment[i].bonuses` | Weapon magic to-hit/damage, armor/armor-substitute base AC, shield/protection AC adjustment and save bonus. |
| Special effects | `effects` | Pool-specific label/kind/subject plus duration, data byte, table flag and raw disk pointer. |
| Known effect mechanics | `effects[i].definition` | Where documented: regeneration/revival, resistance percentage, half-damage multiplier, target saving throw adjustment, levels drained. |
| Other statistics | `stored` | Five saves, save bonus, movement, levels, undead-turning level, thief skills, XP/XP award/XP per HP, wealth, age, status and encumbrance. |
| Spells | `spell_book`, `memorized_spells`, class spell slots | All 55 spell flags/counters, 21 memorized entries, and six slot counts; ambiguous byte 44 remains separate. |
| Stored runtime fields | `stored.current` | Current/rear AC, THAC0, attack/damage bytes, HP and movement, explicitly separate from base statistics. |
| Uninterpreted data | `raw` arrays, `interpretation_notes` | Every character, item, effect and template byte is retained; unknowns are not discarded. |

## What the numbers mean

This object retrieves **templates and separate modifier contributions**. It
does not simulate an encounter, roll HP, activate items, execute ECL scripts,
or calculate a final attack roll/AC after stacking all spells and equipment.
Script-only people without a MON character record have no stat block to load.
Party saves and live DOS memory are not read by this catalog.

For example, the stock troll stores STR 19 but also stores `2 x (1d4+4)` and
`1 x (2d6)` attacks. Its reference STR damage adjustment is +7; adding that to
both attacks would not be justified. `strength_bonus_allowed_hint` exposes
GBE's interpretation of byte 170, with the raw byte retained. GBC leaves that
byte unidentified, so the hint is not an execution-verified rule.

The ability helper supplies the AD&D reference STR table (3..25, including all
18/xx boundaries), DEX missile/AC adjustments (3..19), and CON HP-per-hit-die
adjustments (3..18). Values outside those ranges return `nullopt`. CON rates
are available separately for all eight classes; the scalar is unset for
multiclass characters because HP must be calculated per class and averaged.
These are reference modifiers, not proof that the DOS executable applies every
table rule to every MON template. INT, WIS and CHA are retained as scores; no
5e-style universal modifier is invented for them. Stored spell availability,
slots and saving throws are exposed directly.

Item bonuses are **per item**. Consult `stored.readied()` before applying them.
Weapon and ammunition enchantments remain distinct. A wand's charge/bonus
byte is not treated as a weapon's +N. Protection items expose their individual
AC/save contribution; competing armor, magic armor versus protection rings,
and other stacking rules need a combat rules layer. Item bytes 60..62 can
encode activation functions, spell IDs or other item behavior. They are
preserved in `effect_codes` and are **not run through the SPC effect table**.
Full magical item activation and ability replacement remain uninterpreted.

SPC definitions follow the **Pool of Radiance column** of GBC's effect research.
For example, `0x64` is the troll fire/acid vulnerability and `0x65` is its
3 HP/round regeneration with 3d6-round revival. Unknown IDs remain
`EffectKind::unknown` with the original bytes. A missing numeric parameter
means it is not established, not zero. Duration zero is documented as
permanent; nonzero duration units and effect-table flags are preserved rather
than guessed. These definitions describe effects; they do not execute them.

Several template "current" fields are uninitialized. In particular, raw zero
AC/THAC0 decodes to 60 and must not replace the base value. Current HP remains
an unsigned raw byte; no universal signed/dead interpretation is assumed.
CHA's stored item count can also be stale/zero: the associated ITM array is
retained in full and a discrepancy is reported.

## Files and validation

The loader reads `ITEMS`, `MON1CHA.DAX` through `MON8CHA.DAX`, corresponding
`MONnITM.DAX` archives, and `MONnSPC.DAX`. The stock game omits SPC banks 1 and
3; the other six are required. If banks 1/3 are supplied, they are also read
and validated. File matching is ASCII case-insensitive for copied DOS installs.

CHA records are 285 bytes, ITM arrays have 63-byte entries, SPC arrays have
9-byte entries. `ITEMS` has a two-byte header followed by 16-byte templates,
indexed by item type. Supplemental records join on **bank plus record ID**.
Stored DOS addresses are never dereferenced or used to follow linked lists.

The test suite uses generated fixtures, without original game assets. It
checks malformed/truncated DAX data, RLE boundaries, duplicate IDs, both attack
slots, signed bonuses, exceptional strength boundaries, DEX penalties, CON
class caps, item templates, carried/readied equipment, cursed shields,
protection items, charge bytes, unknown effects, missing assets and bank joins.
Setting `OPENGOLD_GAME_DIR` additionally checks all 172 installed records and
specific troll, fighter-equipment and Hassad values.

## Command-line inspection

From the repository root on Windows:

```powershell
.\build.cmd
.\build\opengold_creatures.exe "D:\path\to\POOLRAD" "TROLL"
```

Omit the name to list every record. `opengold_creatures` is also a normal
CMake target on other platforms. To enable installed-game tests:

```powershell
$env:OPENGOLD_GAME_DIR = "D:\path\to\POOLRAD"
.\build.cmd
```

## Evidence and sources

- [Gold Box Companion formats](https://gbc.zorbus.net/formats.zip): Pool of
  Radiance character offsets and the Pool-specific effects column. The
  [generated monster manual](https://gbc.zorbus.net/mm/01_por.html) provides
  independent spot checks of extracted game values.
- [Gold Box Explorer character annotations](https://github.com/bsimser/Gold-Box-Explorer/blob/eac30abaa6ee66aea6f5d65ebe6d676b10015a8f/src/Common/Plugins/Character/FruaCharacter.cs)
  and `FruaCharacterFile.cs`: DAX record layouts, item templates, compressed
  attack rates and the provisional strength flag. Its guessed effect names
  are not substituted for GBC's Pool-specific definitions.
- [Curse reimplementation, equipment/ability research](https://github.com/simeonpilgrim/coab/blob/master/engine/ovr025.cs):
  cross-check of reference STR/DEX adjustments and protection encoding.
  This is a different game and is not proof of Pool's execution behavior.
  No implementation source from this project is incorporated.
- SSI's [Pool of Radiance rule book](https://www.mocagh.org/ssi/pool-manual.pdf)
  and [journal](https://www.mocagh.org/ssi/pool-journal.pdf): original rules
  context, armor and spells. These do not completely specify the executable.

The implementation reads game records at runtime. No original monster database,
executable, artwork or proprietary game archive is included in the native tool.
