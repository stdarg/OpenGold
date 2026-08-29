Please read @Docs/PRD.md before doing anything.

Project documentation lives in `Docs/`:
- `PRD.md` — product requirements & design (start here)
- `TECH.md` — engineering & technical requirements, and the simulation/rendering architecture
- `CHAR.md` — character design (population model, attributes, skills, classes, needs/mood)
- `SPECIALIST.md` — specialists (vocational production/knowledge professions: alchemist, enchanter, artificer, scholar, master farmer)
- `MAP.md` — map data structure (1D arrays for terrain, walkability, and ground items)
- `MOVEMENT.md` — grid movement & occupancy (creature size/footprint, can't-end-in-occupied, pass-through non-enemies (transit only, can't stop); enemies block, occupancy-aware pathfinding)
- `PATHFINDING.md` — pathfinding implementation (octile A*, adjacent-to goals, occupancy-aware search, claim-then-step consumption, reusable workspace)
- `TERRAIN.md` — terrain, map features & cover (terrain move-cost/difficult terrain, trees/rocks/bushes/walls as fill-% Things, SRD cover tiers, line of sight)
- `OBJECT.md` — object/entity model (base object, mobile objects; characters inherit from it)
- `WORK.md` — work & jobs system (designations, streamlined priorities, utility-AI job selection)
- `BUILDING.md` — buildings & rooms (BuildingDef, construction flow, functions, light room quality)
- `INVENTORY.md` — resources & inventory (physical items vs abstract counters, stacks, stockpiles, carrying, research)
- `RESEARCH.md` — research & tech (tech DAG, direct-progress at the Library, branches, RimWorld-adapted starter techs)
- `HAZARDS.md` — survival hazards (disease immunity race, weather, seasonal cycle)
- `COMBAT.md` — combat resolution (RTwP over rounds, action economy, hit/damage, downing, injuries)
- `EQUIPMENT.md` — equipment & crafting (slots, gear→combat, quality tiers by crafter skill, recipes, durability)
- `EXPEDITION.md` — the away loop (party assembly, travel, tactical handoff, loot return & caravans, retreat/capture)
- `MONSTER.md` — monsters & enemies (MonsterDef, families/roles, CR scaling, AI & morale, loot, bosses; SRD-seeded)
- `THREAT.md` — Lich King threat meter (stages, escalation triggers, reprieves, endgame)
- `FACTION.md` — factions & politics (reputation standings, faction strength, political influence, threat interplay)
- `RECRUITMENT.md` — recruitment & starting economy (starting roster/budget, hiring costs, spoils shares)
- `STORYTELLER.md` — event director (budget-driven pacing, data-driven event catalog, recruitment via Arrival)
- `SCORE.md` — scoring (category weights, difficulty/save-mode multipliers, speed bonus)
- `CLASS.md` — classes (ClassDef data model, shared ability library, the four launch classes)
- `SPELL.md` — spells & magic schools (SpellDef, school/tradition access, optional components, starter spells)
- `DEITY.md` — deities & nature-powers (DeityDef, the authored pantheon, Faith hooks, divine disfavor, divine favor & bonuses)
- `ALIGNMENT.md` — alignment system (good↔evil + law↔chaos axes, bands, drift from deeds, faction alignment; feeds favor/conversion/rescue)
- `PROCGEN.md` — procedural generation (seed tree, generate–validate loop, world/dungeon/quest/loot/character/villain generators)
- `SAVE.md` — save/load (signed JSON snapshot, per-object Save(), polymorphic load, seeds, versioning)
- `MODS.md` — mods (mod package format, data/asset/code override tiers, saves & compatibility, score-neutral)
- `TUTORIAL.md` — tutorial & onboarding (hybrid scripted opening + objective checklist; advisor deferred)
- `OPEN5E.md` — base game rules source (Open5E), adapted for this game
- `OPEN5E_DEVIATIONS.md` — canonical registry of decided deviations from Open5E (and the conform-here guardrails)
- `INSTALL.md` — machine setup (dependencies: .NET SDK, Godot .NET edition, tooling)
- `RUNNING.md` — running the tests and tools (data pipeline, bake, config editor, balance harness)
- `RUNNING_GAME.md` — running the game and its scenes (CLI + editor, headless/deterministic runs, controls)

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it
work") require constant clarification.

## 5. Verify Your Work Through Tests

**Do not assume your code works, verify it through tests.**

Always run all tests after completing task. Then, commit your work and push to
origin.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer
rewrites due to overcomplication, and clarifying questions come before
implementation rather than after mistakes.
