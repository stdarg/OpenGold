# OpenGold Technical Design Document

Date: 2026-08-29
Status: Draft v0.1
Project: OpenGold

## 1. Purpose

This document captures the current technical design for OpenGold based on the technology decisions discussed in the "Pool of Radiance Re-Master" project chat. It complements the current PRD in `docs/PRD.md` by translating product direction into an implementable architecture.

OpenGold is intended to be a modern, open-source reimplementation of the SSI Gold Box engine that:

- uses user-supplied original game files at runtime
- preserves original game rules, progression, and content behavior as closely as practical
- replaces the original interface with a modern desktop UI
- starts with Pool of Radiance, but is architected to support additional Gold Box titles later

## 2. Source Decisions From The Chat

This design is based on these decisions and strong preferences from the referenced chat:

- use `OpenGold` as the project identity rather than a Pool of Radiance-branded product
- build the presentation layer with `Godot 4.x .NET`
- use `C#` for both engine logic and Godot-facing code
- keep the core engine independent from the renderer and UI
- do not emulate the original executable; read original data files directly
- require players to supply their own legally obtained game files
- decode and render original assets at runtime rather than distributing them
- treat original scripts and data as inputs to a new engine, not code to be copied
- support enhanced graphics through shader/filter services, with `xBR` as the leading option
- prefer permissive or file-level-copyleft dependencies and avoid incorporating GPL code into the codebase without an explicit licensing decision

One earlier exploratory answer suggested `C# + MonoGame`. Later discussion and the existing repository direction clearly settled on `Godot` as the chosen presentation stack, so this document treats `Godot + C#` as the active design.

## 3. Technical Goals

- deterministic, testable gameplay behavior
- strong separation between parsing, rules, campaign logic, and presentation
- minimal legal exposure from asset handling and reverse-engineering workflow
- high confidence in behavioral compatibility through automated tests
- reusable engine boundaries so Pool of Radiance is the first supported game, not a one-off implementation

## 4. Chosen Stack

### Runtime and Language

- `Godot 4.x .NET` for desktop presentation, input, scenes, UI, shaders, and packaging
- `C#` as the primary implementation language across the project
- `.NET 8+` for the engine, tools, and tests

### Why This Stack

`C#` fits the project because most of the work is domain-heavy software engineering rather than high-end real-time rendering:

- binary format parsing
- rules processing
- combat state
- scripting/event execution
- save/load behavior
- compatibility quirks
- automated testing

`Godot` is used as the shell around that engine because it gives the project:

- fast UI iteration
- scene composition
- built-in 2D rendering
- shader support
- input abstraction
- desktop export targets

## 5. High-Level Architecture

OpenGold should be implemented as a small set of clearly separated modules:

```text
OpenGold.sln

src/
  OpenGold.Core/
  OpenGold.Formats/
  OpenGold.Game.PoolOfRadiance/
  OpenGold.Godot/
  OpenGold.Tools/                 optional

tests/
  OpenGold.Tests/
```

### `OpenGold.Formats`

Responsible for reading and decoding original game data:

- DAX containers
- maps
- portraits
- combat sprites
- wall and scene art
- items
- monsters
- characters
- save structures used for analysis or import
- ECL script data

This layer converts raw files into typed structures. It should not contain gameplay rules or UI logic.

### `OpenGold.Core`

Responsible for engine behavior implemented by OpenGold:

- world state
- party state
- combat loop
- movement and targeting
- AD&D rule execution as used by the game
- status conditions
- inventory/equipment logic
- event dispatch
- compatibility behaviors

This layer must have no dependency on Godot. It should be runnable under tests with plain .NET.

### `OpenGold.Game.PoolOfRadiance`

Responsible for Pool of Radiance-specific configuration and adapters:

- campaign bootstrap
- title-specific content wiring
- title-specific compatibility workarounds
- mappings between decoded data and engine concepts

This allows future support for additional Gold Box games without contaminating the generic engine.

### `OpenGold.Godot`

Responsible for presentation and interaction:

- scene composition
- rendering
- input mapping
- UI layout
- audio hooks
- animation hooks
- filter selection
- user-facing installation/import flow

This project should consume the engine rather than define it.

## 6. Data Flow

The intended runtime flow is:

```text
User selects Pool of Radiance installation
        ↓
OpenGold validates required files
        ↓
OpenGold.Formats decodes DAX/ECL/maps/items/graphics
        ↓
OpenGold.Game.PoolOfRadiance maps title data into engine inputs
        ↓
OpenGold.Core simulates gameplay
        ↓
OpenGold.Godot renders state and collects player input
```

The original SSI executable never runs. OpenGold interprets game data through its own code.

## 7. Reverse Engineering Approach

The project should follow a clean-room-compatible discipline even if one developer performs multiple roles:

- prefer published format documentation and black-box observation first
- inspect binary behavior only when needed to answer a specific compatibility question
- document findings as specifications, not code translations
- implement from specification, not from copied routines

Recommended documentation outputs:

- `docs/dax-format.md`
- `docs/graphics-format.md`
- `docs/map-format.md`
- `docs/item-format.md`
- `docs/monster-format.md`
- `docs/character-format.md`
- `docs/ecl-format.md`
- `docs/combat-behavior.md`

The key rule is that repository artifacts should describe file formats and observed behavior, not reproduce original source or decompiled logic.

## 8. Content and Asset Strategy

OpenGold must not ship copyrighted Pool of Radiance assets or data. The application should:

- prompt the user to locate a valid game installation
- verify required files are present
- decode assets locally at runtime
- optionally cache derived textures on the user's machine

It must not:

- include extracted original sprites in the repository
- ship original maps, text, portraits, or music
- convert copyrighted assets into distributable project content

This keeps the distribution limited to OpenGold-authored code, UI, shaders, and documentation.

## 9. Graphics and Rendering Design

The rendering strategy should separate original artwork from modern UI.

### Original Art Layer

This layer renders decoded game assets such as:

- combat sprites
- portraits
- map tiles
- wall textures
- scene illustrations

### Modern UI Layer

This layer replaces the original interface with native presentation:

- modern fonts
- tooltips
- combat log
- party panel
- action bars
- character sheets
- inventory screens
- journal and map views

The UI should be drawn natively in Godot rather than by scaling the original text interface.

### Filter Pipeline

The chat strongly favored `xBR`-style enhancement as the first upscale path. The preferred architecture is:

```text
Decoded original image
        ↓
Graphics filter service
        ├── Original
        ├── Nearest
        ├── Scale2x
        ├── xBR
        └── optional CRT/post effects
        ↓
Godot texture output
```

Important constraints:

- do not embed GPL `xBRZ` source directly unless the repository license strategy explicitly changes to allow that
- prefer permissively licensed shader implementations or an original implementation
- treat AI-assisted upscaling as optional and post-1.0
- never ship AI-derived versions of original assets as bundled content

### Texture Cache

Derived textures may be cached locally to avoid repeated processing:

```text
user-data/
  cache/
    por/
      xbr4/
      nearest/
```

This cache is a local optimization, not a distributed asset pack.

## 10. Scripting and Event Execution

The ECL system is a critical part of the design.

OpenGold should:

- decode ECL resources into typed instruction data
- execute those instructions in an OpenGold-owned runtime
- let original scripts drive progression and event behavior where practical

Target flow:

```text
ECL*.DAX
   ↓
EclDecoder
   ↓
Instruction stream / IR
   ↓
OpenGold ECL runtime
   ↓
Core state changes, encounters, dialogs, flags, transitions
```

This is preferable to rewriting campaign progression as hand-authored C# scene logic because it keeps the engine reusable and preserves original behavior more faithfully.

## 11. Rules and Compatibility

OpenGold should preserve the original decision-making model wherever practical:

- THAC0 and descending AC
- spell memorization behavior
- movement and initiative timing
- spell interruption
- class and race restrictions
- monster statistics
- encounter composition
- treasure and reward behavior
- quest and progression flags

The guiding rule is:

> modernize how the player interacts with the game, not what the game decides

Where exact behavior is unclear, the project should document compatibility assumptions rather than silently inventing new rules.

## 12. Testing Strategy

Testing is a first-class architectural concern.

### Automated Tests

The project should maintain a compatibility-heavy test suite covering:

- parser correctness
- rule calculations
- combat edge cases
- spell behavior
- event/runtime execution
- state serialization

Parameter-heavy rule verification is one reason `C#` was preferred. `NUnit` or `xUnit` both fit; defaulting to one consistent framework is recommended early.

### Behavioral Oracle Testing

The original game should be used as an observation oracle:

- construct repeatable scenarios
- record outcomes from the original game
- encode expectations as tests

High-value cases include:

- spells
- surprise and initiative
- ranged combat
- condition timing
- resting
- fleeing
- event triggers

### Differential Format Testing

For uncertain binary structures:

- create small controlled differences in original saves or game state
- diff the resulting data
- map offsets to meaning
- encode findings in parser tests

## 13. UI Design Constraints

The technical design assumes a modern mouse-first desktop UI with keyboard support.

Primary Godot-driven screens include:

- exploration
- combat
- character sheet
- inventory
- spellbook
- journal
- onboarding/import

The UI should support both a faithful and enhanced presentation model:

- `Classic`: original mechanics and conservative visual treatment
- `Enhanced`: optional quality-of-life overlays such as movement range, spell radius, combat detail, and quick-save affordances

## 14. Platform Targets

The chosen stack is optimized for desktop releases:

- Windows first
- Linux next
- macOS where practical

Web export is not part of the primary design. That aligns with both the game’s desktop-oriented workflow and the current limitations of Godot C# export support discussed in the source chat.

## 15. Dependency Policy

Dependency selection should follow the licensing posture described in the source chat:

- preferred: `MIT`, `BSD-2`, `BSD-3`, `Apache-2.0`, `MPL-2.0`
- review carefully: `LGPL`
- avoid incorporation by default: `GPL`, `AGPL`

Reference implementations may still be studied, but source incorporation must respect license boundaries.

Examples from the source chat:

- `Gold Box Explorer` is a good implementation reference because it is MIT licensed and focused on data extraction
- `Dungeon Craft` is useful as a behavioral reference, but its GPL license makes direct code reuse undesirable under the current project direction

## 16. Prototype Plan

The recommended first technical milestone is a rendering-first proof of concept:

1. validate a user-supplied Pool of Radiance installation
2. decode one portrait, one combat sprite, one wall asset, and one map asset
3. display them in Godot
4. apply filter switching between original and enhanced modes
5. mock a modern combat screen around real assets

This proves:

- file ingestion
- legal asset boundary discipline
- presentation viability
- graphics enhancement pipeline

before deeper investment in rules fidelity and full campaign support.

## 17. Risks

- exact event and rules compatibility will be harder than asset decoding
- ECL semantics may contain title-specific behavior that resists generic modeling
- license mistakes in scaler or reference-code selection could create avoidable problems
- too much early abstraction for future Gold Box titles could delay a playable Pool of Radiance milestone

## 18. Current Recommendation

The current technical recommendation is:

- `Godot 4.x .NET` for presentation
- `C#` across engine and UI code
- `.NET 8+` for the solution baseline
- a layered architecture centered on `Formats`, `Core`, `Game.PoolOfRadiance`, and `Godot`
- user-supplied original assets loaded at runtime
- ECL-driven campaign execution
- shader-based graphics enhancement with `xBR`-style filtering
- test-driven compatibility work from the start

This gives OpenGold the best balance of faithfulness, maintainability, legal caution, and room to grow into a reusable Gold Box engine.
