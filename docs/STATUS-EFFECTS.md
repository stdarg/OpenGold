# Saving throws and status effects

The first increment implements ordinary ability saving throws and the **Blinded**
condition through the blindness option of **Blindness/Deafness**, using SRD 5.2.1.
The deafness option, concentration, other conditions and higher-level casts are
outside this increment. Existing supported Clerics and Wizards can select
Blindness when advancing to levels 3–4. It spends an action and a level-two slot.

The caster must see a target within 120 feet with a clear path. The target makes
a Constitution save against `8 + casting ability modifier + proficiency`.
Failure applies blindness for up to one minute. A repeat save at the end of each
target turn can end that application. Ordinary saves succeed when their total
meets the DC; natural 1/20 have no special success/failure rule. Existing death
saves retain their separate rules.

## One model for PCs, NPCs and monsters

All combat actors own `detail::EffectState` by value. Its vector owns individual
applications, and its increasing counter assigns IDs local to that creature.
Each application stores:

- its ID/application order and effect-definition kind;
- the originating encounter scope, caster ID and caster name;
- the save DC fixed at application time;
- remaining duration and time until the next recovery check, in milliseconds.

The target is the owner of the collection. Sources are provenance IDs, with no
owned or borrowed actor pointers. An effect survives the source actor leaving or
dying. The definition supplies the condition and recovery behavior; instances
carry parameters and timing. Adding another kind requires explicit rules and
validation. No arbitrary executable expressions are loaded from saves.

`status_effects.h/.cpp` owns dice-mode resolution, save arithmetic, the shared
class saving-throw proficiency table, effect lifecycle and bounded effect codec.
Character sheets and created combat profiles use the same proficiency table.
Curated monsters have explicit six-ability save bonuses in `combat.rules`.
Original creature bytes remain unchanged. Authored legacy/synthetic content
without a `saves` row retains zero bonuses.

Base statistics stay unchanged when effects are applied or removed. Queries
derive sight and attack modifiers from current effects. Blind attackers have
disadvantage; attacks against blinded targets have advantage. Opposing sources
cancel, and repeated sources never add extra d20s. Blinded creatures cannot
perform opportunity attacks or cast the supported spells requiring sight
(Blindness, Magic Missile and Healing Word). Fire Bolt, Scorching Ray and weapon
attacks remain available against known creatures with a clear path, subject to
disadvantage. Touch-range Cure Wounds remains usable.

Dodge imposes attack disadvantage only when its user can see the attacker.
Adjacent enemies impose ranged-attack disadvantage only when they can see the
attacker. Multiple blindness applications each retain their own duration and
recovery check, while their shared condition contributes only once. Removing
one application leaves remaining applications effective.

## Time, recovery and campaign handoff

Time advances through accepted gameplay, independently of display frames:

- A complete combat round is exactly 6,000 ms. Fixed initiative slots partition
  that interval using integer boundaries, including dead and unconscious slots.
  Differences between successive boundaries sum to exactly 6,000 for any actor
  count. A finishing combat action completes its current slot.
- Recovery checks occur at target turn ends. Imported campaign effects retain
  their duration and align their next check with the new target initiative.
  Combat checkpoint restore preserves the exact existing check schedule.
- A successful forward exploration step advances six seconds. Turning, looking,
  blocked steps, menus, drawing, saving and loading advance no time.
- Existing waits and rests advance their specified game time. Outside combat,
  recovery checks continue every six seconds until success or expiration.

The effect scheduler finds the next expiration/recovery boundary across all
subjects. Simultaneous events use entity ID then application order. Expiration
takes precedence over a recovery roll at the same instant. This makes one large
time advance equivalent to many smaller advances, including identical RNG use.
Dead creatures consume no recovery rolls; their effects still expire.

Combat exclusively updates participating actors. `CampaignParty` receives their
rules-owned `VitalState` and advances the campaign clock by the newly observed
combat time. It advances reserve members' effects separately. Reapplying the
same snapshot advances no time. Effect updates, clock updates and RNG changes
are prepared in an owned candidate before replacing live campaign state.

## Persistence and boundaries

- Rules module **0.6.6** writes **OGCOMBAT 7** checkpoints containing source scope,
  elapsed time and each actor's effect collection. The checkpoint byte limit is
  4 MiB; each creature supports at most 128 simultaneous applications. A full
  collection offers no further Blindness command.
- Rules-owned **SRD3** continuation embeds the structured **FX1** effect codec.
  SRD1/SRD2 resource continuations remain readable. Unaffected actors continue
  using those older encodings. Effect counters remain saved after expiration.
- **OPENGOLD-CAMPAIGN 6** adds sub-minute time, precise rest-completion offsets and
  the next encounter scope. Versions 1–5 migrate with zero sub-minute offsets
  and no effects. Supported preceding content packs migrate; unrelated identities
  still reject. The [combat migration](RULES.md#library-boundary) accepts module
  0.6.4/0.6.5 with matching content; other old combat identities require their original module.
- PCs and recruited NPCs carry effects through campaign handoff, reserve status,
  healing, advancement, saves and subsequent encounters. Current encounter-only
  monsters retain effects for their encounter and its checkpoints.
- Campaign saves remain restricted to existing idle boundaries. This increment
  does not introduce mid-combat campaign saving or persistent world monsters.

Godot displays rules-produced condition labels in the existing combat roster and
uses offered commands for targeting. The Blindness button uses the same styles,
keyboard focus and targeting interaction as existing spells. Save rolls and
recovery messages appear in the existing log. The character sheet's persistent
state summary also reports blindness carried out of combat.

## Verification and review

`opengold_status_effect_tests` covers all natural save results and six abilities,
modifier combinations, duration/recovery boundaries, overlapping applications,
time chunking, reordered subjects, death, source provenance, codec validation,
combat restrictions, spell costs and deterministic combat/campaign continuation.
Existing save tests cover older campaign migrations. The checkpoint fuzzer and
normal mutation smoke tests include active and overlapping effect seeds.

The graphical test selects Blindness through the actual button and target click,
checks the roster/log, saves and restores an active effect, and checks layout at
1120×800 and 1280×900. It preserves any preexisting training checkpoint files.

On macOS, open the reproducible fixture using the existing scene:

```bash
/Applications/Godot_mono.app/Contents/MacOS/Godot \
  --path src/OpenGoldBox/godot res://scenes/combat_demo.tscn -- --conditions
```

The fixture uses an authored caster, a bandit and seed 3. The initial Constitution
save fails, allowing immediate inspection. **Restart** reproduces it. This is a
mode of the existing combat scene, not another application or UI framework.

Reference: [official SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
Blindness/Deafness p. 113, Blinded p. 177, combining spell effects p. 106.
Existing content attribution in `data/rules/srd-5.2.1/NOTICE.md` applies.
