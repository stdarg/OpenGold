# Campaign recovery and advancement subset

Issue [#6](https://github.com/stdarg/OpenGold/issues/6) now has a bounded native
implementation and a Godot acceptance route. It is not full SRD advancement or
complete original campaign service coverage.

## Supported behavior

- The authored party Bandit preview grants **300 XP per living active member**
  on its first victory. Its reward key is `preview:bandit:v1`; reopening the scene,
  restarting it or using a different seed cannot award it again. The original
  four-orc Slums adapter has a separate key, `por:ECL2:20:search1:orcs:v1`, and the
  same explicitly authored 300 XP conversion when a campaign party is attached.
  These amounts are conversion policies, not decoded original XP or general
  monster-XP calculations. Dead and reserve members receive no award.
- Fighter, Cleric and Wizard advance at **300 XP to level 2**. Maximum HP grows
  by the fixed-average class Hit Die plus Constitution (minimum 1), with a further
  +1 for Dwarven Toughness. Conscious members retain their existing HP deficit;
  advancement does not wake unconscious members or revive dead ones. Cleric and
  Wizard gain a third level-one slot while retaining already-spent slots.
  Second Wind already uses the actor's level in its healing calculation.
- XP beyond level 2 is retained, but further advancement is stopped until
  subclass, feat and higher-level spell support exists. Action Surge, Tactical
  Mind, Channel Divinity, Scholar, spell selection and other unimplemented class
  features are not granted. Other classes retain their level-one exploration
  profiles and reject advancement explicitly. The character sheet displays live
  level, XP, HP, Hit Dice and resource descriptions; subsequent combat uses the
  updated rules recipe.
- A group long rest requires every active member to have at least 1 HP, and at
  least 16 hours since their last completed long rest. It takes eight hours and
  restores HP and the supported spell/Second Wind resources. Empty parties,
  unconscious/dead members and premature repeated rests are denied without
  changing time or resources. Removed/rejoined members retain their rest timer.
  Short rests, Hit Die spending, exhaustion, species-specific rest durations and
  resuming interrupted rests remain unsupported.

## Original campaign mappings

**Camp [C]** runs ECL entry 2 before any recovery. `6DD3=255` denies rest.
An interruption-free profile permits the group rest. New Phlan's guaranteed
city-watch interruption (`6DD2=1`, `6DD3=100/101`) advances five minutes, runs
entry 3, and grants no recovery. Choose **GO** to leave peacefully; combat with
the watch remains unsupported and rolls the event back. Other nonzero
probabilistic interruption profiles fail explicitly.

The original inn's **PROGRAM 9** request follows its own pre-camp subroutine and
payment dialogue. A safe, eligible request completes a long rest. The script
collects one platinum piece from the chosen payer; the host does not invent a
currency exchange. Failed services restore the event checkpoint, including any
payment. Original training requests (PROGRAM 0) and victory services (PROGRAM 8)
remain unsupported. Automatic SRD XP advancement does not imply payment of an
original training fee or completion of a training-hall script.

The original temple **COMBAT** request with `6DE2=1` offers the supported
**Cure Wounds** service using the existing dialogue list. Select a wounded living
active member, or **Cancel**. The original Cure Light Wounds price is retained at
**100 gp**, collected in active-slot order from gold purses only. Its healing is
explicitly converted to SRD Cure Wounds, **2d8 + 3**, with an authored Wisdom +3
temple caster. Dice are deterministic and campaign-owned; the rules module
calculates healing, caps HP and clears death-save state without refilling slots
or Second Wind. Resurrection and other temple spells remain unsupported.

Unaffordable, dead, full-health, reserve and stale-ticket requests do not charge
or change HP, resources or RNG. Success and cancellation clear `6DE2` and resume
the waiting original script. A later unsupported instruction rolls back the
entire event, including healing, payment and RNG.

Campaign minutes are authoritative for recovery. ECL minute/hour/day fields are
synchronized from a noon, day-one starting point, using a bounded 30-day-month,
12-month calendar. Movement and combat do not yet advance this clock. General
campaign scheduling, quest rewards and non-shop treasure conversion remain open.

## Persistence and verification

`PartyState` native checkpoints retain XP, claimed reward IDs, HP/resources,
purses, recovery timers, clock and RNG for rollback. [Campaign file save/load](SAVES.md) now persists this supported state at the party/idle-town boundaries, with fresh-process restart verification.
Combat checkpoint format remains version 2, with level-bearing PC2 recipes.
The combat module identity is **0.3.0**, so incompatible earlier saves reject.

From PowerShell:

```powershell
.\build-rolf.cmd
godot --headless --path godot res://scenes/character_creation.tscn -- --party-check
```

The deterministic route uses the actual creation/shop/combat callbacks, verifies
the level-two sheet, then exercises the original city-watch interruption, temple
payment and inn rest, rejects a repeated rest and fights again without duplicate
XP. The service fixture supplies one wounded survivor, 200 gp and one platinum;
these are test-only setup changes. Add `--capture` and omit `--headless` for local
captures. Native party tests separately cover threshold boundaries, Dwarven HP,
caster resources, overflow, reward reentry, checkpoint continuation, rest denial,
temple eligibility, payment failure and event rollback.

Rules references: [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
character advancement, class tables, Dwarven Toughness, Cure Wounds and Long Rest.
Original interface/price evidence: [Lee's PC 1.3 research](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869),
sections 8.5, 12.3 and 12.4.2. Original dialogue, scripts and assets are loaded
from the user's installation and are not distributed.
