# Thrown weapon inventory — batch #58

## Frozen player outcome

Complete actual removal, battlefield location and recovery of thrown weapons,
including drawing a carried Thrown weapon as part of its attack. Use all seven
SRD Thrown weapons (Dagger, Handaxe, Javelin, Light Hammer, Spear, Dart, Trident),
for all currently supported class/level routes; do not substitute fixed demo
actors for ordinary campaign inventory. Preserve ability, range, critical,
Savage Attacker and Champion behavior, with physical quantities and identity.

Source: [SRD 5.2.1 pp.90 and 177, Thrown and Attack action](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).
Thrown permits drawing as part of the attack and retains a Melee weapon's normal
ability modifier. The SRD does not prescribe a grid landing square for misses;
Q44 requests the project policy rather than presenting one as an SRD rule.

## Acceptance and boundaries

- Actual held and carried weapon units, including stacks, map to campaign item
  identity/provenance; each accepted throw removes exactly one available unit.
  No removal for canceled targeting or rejected commands. No unlimited copies.
- Drawing/hand eligibility and any necessary weapon stowing use SRD attack
  timing; shields retain their normal don/doff action requirements. All choices
  are visible before committing. No general inventory-management screen.
- Hit/miss, critical, damage defenses and optional Savage decisions preserve
  the original weapon/dice even after it leaves the hand. Champion's immediate
  movement follows critical resolution. Spent actions/reactions and RNG persist.
- Landed weapons use the existing Ground item/Pick up flow and its normal reach,
  line-of-sight, free object interaction/Utilize rules; safe collection follows
  approved Q37. Ownership transfer, quantities and unreachable items survive
  campaign handoff and camping/inn saves. No combat saving controls.
- Historical saves/recipes retain their supported continuation. Capture actual
  module 0.6.46 writer evidence before runtime changes; do not rewrite old fixtures.
- Main/demo keyboard/mouse controls; main EN/ES and demo EN at both supported
  sizes; rules remain in static SRD library, Core owns inventory transactions.
- Excluded: #57 ammunition, #59 Light bonus attacks, #60 mastery effects,
  #80/#81 style feats, later class advancement, unrelated item/equipment fixes.
  Any additional prerequisite outside this boundary requires explicit approval.

## Decisions awaiting the user

Q43 proposes a labeled Thrown weapon dropdown and Throw button in a new row
immediately below Ground item/Pick up in game/demo. List held/carried Thrown
weapons with quantities. Throw highlights legal targets; click/keyboard target
confirmation draws if needed and throws one unit. Where stowing a held weapon
is needed, show that change before confirmation and apply its proper cost/timing.
Cancel/Escape before target confirmation spends nothing; illegal actions disable.
Reuse standard styling, keyboard access, existing ground pickup and Q37 recovery.

Q44 proposes placing the thrown weapon on the target's battlefield square on a
hit or miss, without embedding/breakage/scatter. It remains a ground item rather
than automatically joining the target's inventory. Existing pickup and safe
collection rules decide when it can be retrieved. This is a game policy for an
SRD-unspecified detail, requiring explicit approval.

## Execution card

- Full goal active. Previous turn: PROGRESS, #88 implemented/closed and main clean
  at `74dfb32`, verified before this preflight. One owner; no agents/new tasks.
- Original issue #58, package B03/C. No other issue closure claimed or new issue
  created. No gameplay implementation before Q43/Q44 are answered.
- Requested/recorded Astra/high retained for physical inventory transactions,
  pending-hit state and legacy continuation. No configured-model change claimed.
  Escalate after two failed fixes of the same failure or decisions beyond tier.
- Observed preflight start 2026-09-25 16:08:35 UTC; checkpoint 17:08:35, latest
  17:38:35. Approval wait will be recorded separately; do not silently reset clock.
- Verification: all weapon/level/source rows through real inventory, focused
  combat/inventory/save tests, final native regression and affected Godot checks,
  both apps and required renders/localization. Exact targets after code preflight.
- Current state: requirements/control proposal prepared; implementation pending
  answers. No live build/test process. No compatibility reduction authorized.

## Independent compatibility preparation — 2026-09-25

Before either pending gameplay decision, added `opengold_thrown_weapon_tests`
and captured the actual 0.6.46 writer's campaign and combat continuations.
[Fixture provenance and hashes](../tests/fixtures/README.md#pre-thrown-inventory-writer-0646)
record the ordinary character, all seven physical stacks, equipped/ground items,
spent Second Wind, critical Savage decision, Champion phase and second throw.
Replay passes byte-exactly for loaded old combat, and campaign bodies/quantities
remain unchanged except any future module identity. No production code changed.

The initial capture used unsaved constructor state; the old format-16 implicit
movement flag then differed from the state produced by actual loading. Capture
now reloads the genuine prior save before continuation. This preserves the old
codec's actual behavior and does not normalize test output or change the writer.
The frozen fixtures must not be regenerated by a future implementation.

Commands passed:

```bash
cmake -S . -B build/mac-check
cmake --build build/mac-check --target opengold_thrown_weapon_tests -j6
build/mac-check/opengold_thrown_weapon_tests --capture-prior-writer
ctest --test-dir build/mac-check --output-on-failure -R '^opengold_thrown_weapon_tests$'
```

Configure was required for the added test target. Logs are
`/tmp/thrown-baseline-configure.log` and
`/tmp/thrown-baseline-final-{build,capture,tests}.log`.
No build/test remains live. Q43/Q44 remain unanswered; these tests establish
compatibility preparation, not completion of #58 or permission to implement its
pending controls/landing policy. The batch clock remains 16:08:35 UTC.

## Blocking-input checkpoint — 16:19:15 UTC

Goal status confirmed BLOCKED after Q43/Q44 remained unanswered across three
consecutive goal turns (proposal, completed independent baseline, revalidation).
The baseline turn was PROGRESS; this audit found no remaining independent work
inside the frozen batch before those decisions. No process is still running.
Preparation is committed/pushed in `15d5e09`, with focused checks passing. The
full SRD objective remains unchanged and incomplete. Await user decisions before
runtime/control implementation; do not rotate into unrelated batches.
