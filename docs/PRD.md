# OpenGold Product Requirements Document

Date: 2026-08-29
Status: Draft v0.1
Project: OpenGold

## 1. Summary

OpenGold is an open-source, modern reimplementation of SSI's Gold Box engine focused first on Pool of Radiance. It should let players use their own legally obtained original game files while experiencing the game through a cleaner interface, improved rendering, and modern quality-of-life features without changing the core rules, progression, or feel that made the original compelling.

## 2. Problem

Pool of Radiance remains historically important and still has strong design fundamentals, especially its tactical combat and party-based progression. Today, the original experience is hard for many players to approach because it depends on dated presentation, awkward UI conventions, low-resolution rendering, and aging distribution formats. There is no broadly available remake that preserves the original game's structure and mechanics while making it feel comfortable on a modern PC.

## 3. Vision

Create the version of Pool of Radiance that players wish existed in 2026:

- faithful to the original game's data, rules, tone, pacing, and encounter design
- modern in usability, readability, accessibility, and visual presentation
- legally clean by requiring users to provide their own original game data
- architected as a reusable open engine that can later support additional Gold Box titles

## 4. Product Principles

- Faithfulness first. Changes should preserve gameplay outcomes unless the team explicitly chooses a compatibility break.
- Modernize the shell, not the soul. Replace the UI and presentation layer while keeping core game behavior intact.
- Player-owned assets only. OpenGold must not ship copyrighted Pool of Radiance assets or game data.
- Inspectable and testable. Core rules and format readers should be deterministic and heavily testable.
- Modest by default. The first shipped experience should solve the original game's usability problems before expanding into broader engine ambitions.

## 5. Target Users

- players who own Pool of Radiance and want a more usable way to replay it
- retro RPG fans curious about Gold Box games but blocked by the original UI
- preservation-minded contributors who want a clean open-source codebase
- future developers who may extend the engine to other Gold Box titles

## 6. Goals

### Primary Goals

- Ship a playable Pool of Radiance experience that requires the user's original game files.
- Preserve the original campaign flow, combat model, character systems, and scripted progression.
- Deliver a modern UI that makes movement, combat, inventory, character review, spell use, shopping, resting, and save/load significantly easier to understand.
- Improve visual presentation through clean scaling, better layout, and optional pixel-art upscaling filters.
- Build an engine architecture that cleanly separates file parsing, game logic, and presentation.

### Secondary Goals

- Make compatibility gaps visible through tests, debug tooling, and documented known differences.
- Support Windows first, with portability to other desktop platforms where practical.
- Encourage community contribution through MPL-2.0 licensing, clear boundaries, and documentation.

## 7. Non-Goals

- shipping original SSI, TSR, or Wizards of the Coast copyrighted assets in the repository or binaries
- rewriting the campaign story, encounters, or rules into a new game
- creating a full 3D remake or action RPG reinterpretation
- supporting every Gold Box title in the first release
- introducing balance changes, new classes, or fan expansion content in the initial product
- requiring emulation of the original executable rather than reading original data files directly

## 8. User Experience Requirements

### Core Player Experience

The player installs OpenGold, points it at their own Pool of Radiance files, and starts playing with minimal setup friction. The game should feel recognizably like Pool of Radiance within minutes, but far easier to read and operate.

### UX Expectations

- clear onboarding for locating and validating original game files
- resolution-independent interface with readable fonts and modern layout
- intuitive exploration, combat, inventory, and spellcasting flows
- useful feedback for status effects, hit chances, damage, movement, and combat order where feasible
- quick save/load and predictable input behavior
- mouse-first interaction with keyboard shortcuts for power users
- optional original-look and enhanced-rendering display modes

### Accessibility and Quality-of-Life

- larger text and scalable UI
- clearer color contrast than the original presentation
- tooltips or equivalent contextual help for stats, commands, and combat options
- concise combat log and party status visibility
- reduced menu friction compared to the DOS interface

## 9. Functional Scope

### MVP / Vertical Slice

- import and validate Pool of Radiance game data from a user-supplied installation
- load core assets, maps, character data, items, monsters, and scripted content needed for a representative early-game slice
- party creation or loading compatible starting data
- town and dungeon exploration
- turn-based tactical combat with faithful movement, attacks, spells, and victory conditions
- inventory, equipment, rest, save, and load
- modern UI shell for exploration and combat

### Version 1.0 Scope

- full playable Pool of Radiance campaign
- end-to-end support for required game data formats and scripted events
- compatibility-focused implementation of core AD&D mechanics as used by the original game
- modernized interfaces for shops, temples, journals, loot, and party management
- optional rendering filters for original art assets
- packaging, installer guidance, and contributor documentation

### Post-1.0 Opportunities

- support for additional Gold Box titles
- modding and debugging tools
- optional assist features such as encounter replay, combat speed controls, or annotated logs
- higher-level content validation and conversion utilities

## 10. Technical Direction

OpenGold should be structured in three major layers:

- `OpenGold.Formats`: readers and decoders for original game data, assets, maps, scripts, and save structures
- `OpenGold.Core`: engine behavior including rules, combat, world state, event handling, and compatibility logic
- `OpenGold.Godot`: presentation, input, rendering, scenes, and modern UI

The core engine should remain independent from the presentation layer wherever possible so that gameplay behavior can be tested without the UI runtime.

## 11. Legal and Content Constraints

- The project must not include copyrighted Pool of Radiance assets, maps, text, music, portraits, or data files.
- Users must supply their own legally obtained original game installation.
- The repository should clearly distinguish open-source engine code from copyrighted game content.
- Branding and documentation should state that OpenGold is an independent reimplementation and is not affiliated with or endorsed by the original rights holders.

## 12. Success Metrics

- a new player can install the app and reach gameplay using their own files without external technical help
- the first hour of Pool of Radiance is fully playable in OpenGold with no blocking progression issues
- core combat flows are understandable without consulting the original manual
- compatibility test coverage exists for high-risk rules and data parsing behavior
- contributors can understand the architecture and add features without touching unrelated layers

## 13. Milestones

### Milestone 1: Foundation

- define repository structure and module boundaries
- document supported source files and legal boundaries
- implement initial file import and validation flow
- stand up basic rendering and UI shell

### Milestone 2: Vertical Slice

- load a limited playable area and encounters
- implement core combat loop, movement, targeting, and rewards
- support save/load and party state
- validate feel and fidelity with repeated playtesting

### Milestone 3: Full Campaign

- complete content coverage for the full Pool of Radiance campaign
- close major compatibility gaps in rules and scripting
- harden UX, installer flow, and performance

### Milestone 4: Community Release

- publish contribution guidelines and roadmap
- formalize testing and issue taxonomy
- prepare an openly distributable release that requires external game data

## 14. Key Risks

- exact behavioral compatibility may be much harder than asset and UI work
- legal confusion could damage the project if asset boundaries are not explicit
- event scripting and edge-case rule handling may take disproportionate time
- an overly ambitious engine-generalization effort could delay a playable Pool of Radiance release

## 15. Open Questions

- How strict should compatibility be when original behavior is opaque, inconsistent, or user-hostile?
- Which parts of the original UI should be preserved for nostalgia versus replaced entirely?
- What minimum operating systems and desktop targets should Version 1.0 support?
- How much modding or multi-game support should be designed up front versus deferred?
- Should enhanced rendering filters be part of the first public release or follow shortly after?
