# Temporary Hit Points

The native foundation in [#196](https://github.com/stdarg/OpenGold/issues/196)
implements the Temporary Hit Point pool for [F05b #34](https://github.com/stdarg/OpenGold/issues/34).
It does not yet grant a playable feature or expose a new control.
[#197](https://github.com/stdarg/OpenGold/issues/197) supplies Orc Adrenaline Rush,
the replacement decision and the reviewed HP presentation. Keep #34 open until
that ordinary game flow and its applicable campaign integration are complete.

Authority: [SRD 5.2.1 pp. 17–18](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

## Rules and caller contract

Temporary HP is a separate nonnegative amount with a bounded source identifier.
It may exceed maximum HP. A granting feature establishes entitlement, spends
its action/resources and obtains an explicit keep-current or use-new decision.
`grant_temporary_hit_points` commits that decision atomically; it never adds the
two amounts or automatically chooses the larger one. Keeping requires an
existing pool, and a new grant must be positive and have a valid source. The
shared operation grants no species, spell or class entitlement by itself.

Typed damage adjustments, resistance, vulnerability and immunity resolve first.
The resulting damage consumes Temporary HP before ordinary HP; any remainder
uses the existing unconsciousness and massive-damage transitions. Depletion
clears the exhausted source. Healing, Hit Dice, temple services, advancement and
training completion preserve the pool. An original script's absolute HP
assignment changes actual HP and bypasses the damage buffer.

Temporary HP does not heal, stabilize or wake a creature. Absorbing HP loss does
not mean that no damage was taken: at zero HP, positive resolved damage still
ends Stable and causes one death-save failure, or two for a Critical Hit. Damage
at zero HP reaching the HP maximum still causes death. Damage reduced to zero
by defenses has none of these consequences. Future concentration checks must
likewise use damage taken, not just the ordinary HP lost.

The default pool lasts until depleted or until its recipient completes a Long
Rest. Waiting eight hours, a Short Rest or another member's Long Rest does not
expire it. Existing campaign rest eligibility determines which active members
complete the rest; reserves and ineligible members retain their pools. Features
with a specific overriding lifetime must implement that lifetime when added.

## Persistence and architecture

Rules **0.6.14** writes combat format **11**. Each actor row appends the amount
and quoted source ID. **SRD6** stores the same data with existing resources,
Hit Dice, mortality clocks and effects in the rules-owned vital continuation.
Characters without a pool retain the existing compact SRD1–SRD5 formats.
Campaign format **10**, **PC8** and the content pack are unchanged.

Earlier supported combat modules through **0.6.13** migrate with an empty pool.
Old campaigns gain no invented grant, healing, elapsed time or RNG draw. The
previous fixed Dwarf resistance grant is validated and retained. Current saves
reject negative/overflowing amounts, missing or malformed sources and trailing
fields. Candidate state is validated before replacing a live character/session.

The source and amount are available through `RecoveryInfo` and `CombatantView`.
The shared life-cycle value type owns the pool; no ownership uses raw pointers,
and Core/Godot do not decode the SRD resource string. Rules remain C++20 and
independent of Godot. No new runtime, framework or presentation control is added
by #196.

## Verification and remaining delivery

`opengold_temporary_hp_tests` uses independent absorption and massive-damage
examples, including fully absorbed hits at zero HP, explicit smaller replacement,
healing and script-assignment distinctions, overflow and rejection. Actual
combat tests apply Dwarf Poison resistance before consuming the buffer and
continue identically after a checkpoint. Campaign tests cover PC/NPC/reserve
state, training, advancement, Hit Dice and individual Long Rest expiry.

Frozen files from the **0.6.13** writer at commit `eea7368` preserve wounds,
resources, Dwarf grants, mortality timers, RNG and a pending opportunity decision.
They were generated before changing the writer; see [fixture provenance](../tests/fixtures/README.md).

The user approved an Adrenaline Rush button beside Dash. For #197, ordinary HP
will be green at full health, yellow when wounded above 20%, and red at 20% or
below; Temporary HP remains separate and green. Tooltips identify applicable HP
increases/reductions and their actual sources. A centered keep-current/use-new
decision retains spent costs and survives save/load. Those controls, pending
offer state, feature use/recharge tracking and playable entitlement remain #197.

Delivery checks for #196: both Godot libraries built; all 17 selected native
suites, six Godot control/ownership checks and both game/demo party routes passed.
Catalog validation passes for all 574 entries; catalog changes only refresh
source locations. Scope review confirms value ownership, C++20 and unchanged
controls. The game integration route still reports the previously observed
ObjectDB cleanup warning (five instances on this run); its assertions and the
dedicated node-ownership check pass. No cleanup fix is claimed here.
