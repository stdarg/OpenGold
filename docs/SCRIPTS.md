# ECL decoding and execution design

Status: initial native execution implementation added September 5, 2026. The
implementation described below loads ECL and executes a bounded subset with
resumable text/menu requests. The remaining sections describe the intended full
architecture; their host services, scheduler, snapshots, and campaign integration
are not all implemented. The map demo still does not execute cell events.

## Run the implementation

Build with `build.cmd`, then run the self-contained ECL demo (no game assets needed):

```powershell
.\build\opengold_scripts.exe --demo
```

The demo executes genuine ECL instruction encodings: writes 7 to a variable, adds
5, prints 12, asks you to choose CONTINUE or LEAVE, and prints the selected
one-based value before completing. It demonstrates bytecode execution and host
resume, not an original game event.

For an installed game, set `OPENGOLD_GAME_DIR` to the folder with ECL archives:

```powershell
.\build\opengold_scripts.exe --list "$env:OPENGOLD_GAME_DIR"
.\build\opengold_scripts.exe --inspect "$env:OPENGOLD_GAME_DIR" ECL7.DAX 17
.\build\opengold_scripts.exe --run "$env:OPENGOLD_GAME_DIR" ECL7.DAX 17 4 0x49FB=0
```

The final example starts reference startup slot 4 with one explicitly initialized
variable. It is a research invocation, not a complete startup environment: it
will stop when it encounters another missing binding or unsupported operation.
Bindings do not install the original engine's field side effects. Never interpret
successful execution with invented bindings as verification of game behavior.

Numbers accept decimal or `0x`-prefixed hexadecimal. Archive arguments are
case-insensitive; entry slots are 0..4. `--inspect` follows static control flow,
marks unsupported execution, and exits nonzero on decoding diagnostics. `--run`
returns nonzero on faults or input ending during a menu. The console acknowledges
text immediately and represents PRINTCLEAR with a visible `[clear text]` marker.

## Implemented API and execution profile

- [`ecl.h`](../src/OpenGold.Formats/include/opengold/ecl.h): immutable
  `EclProgram`, `EclInstruction`, tagged `EclOperand`, packed text decoder, and a
  shared PoR opcode grammar/support table.
- [`ecl_machine.h`](../src/OpenGold.Core/include/opengold/ecl_machine.h):
  `EclCatalog`, shared program ownership, `EclMachine`, and typed text/menu requests.
- [`scripts.cpp`](../tools/scripts.cpp): command-line catalog, disassembler, and
  interactive runner. Link `opengold_core` to use the runtime in another native app.

```cpp
#include <opengold/ecl_machine.h>

void prepare_script(const std::filesystem::path& game_directory)
{
    using namespace opengold::por;
    const auto catalog = EclCatalog::load(game_directory);
    const auto program = catalog.find({"ECL7.DAX", 17});
    if (!program) return;
    EclMachine machine(program);
    machine.bind_variable(0x49FB, 0); // Explicit research state; not a game-memory adapter.
    if (!machine.start(4)) return;
    const auto result = machine.run(1000);
    // running: budget yielded; call run() on a later update.
    // waiting: render result.request, then resume its ID with an optional choice.
    // completed/faulted: finish or inspect result.diagnostic and machine.trace().
    // Keep machine alive between calls; a production session owns it.
}
```

| Implemented instructions | Behavior in the initial research profile |
| --- | --- |
| EXIT, GOTO, GOSUB, RETURN | Completion, direct jumps, and bounded subroutine stack. Return without a call faults. |
| COMPARE, IF = / <> / < / > / <= / >= | Unsigned numeric or two inline-text comparisons; failed conditions skip one entire instruction. COMPARE is required before an IF. |
| SAVE, ADD, SUBTRACT, MULTIPLY, AND, OR | Numeric logical-variable operations with explicit unsigned 16-bit wrapping. SUBTRACT computes operand 2 minus operand 1. String SAVE is unsupported. |
| PRINT, PRINTCLEAR | Suspend with a typed text request. Inline packed text and numeric display are supported. |
| VERTICAL MENU, HORIZONTAL MENU | Suspend with choices and orientation; resume accepts a zero-based UI index and stores index+1. Empty menus fault. Vertical-menu timing/cancellation semantics are not implemented. |
| ON GOTO, ON GOSUB | One-based indexed dispatch; zero and out-of-range selectors fall through. |

This profile makes width, comparison order, and indexing behavior explicit and
tests them with synthetic programs. It is not an execution-verified PoR memory
or arithmetic compatibility layer. Numeric tags 0/2 supply immediate values;
tags 1/3 read explicitly bound logical variables. In branch roles their encoded
addresses are used directly. Address tags 1/3 identify writable destinations.
Tag `0x81` string references decode but cannot yet execute.

`bind_variable()` initializes individual 16-bit logical slots while idle or
completed; it does not expose a raw 64 KiB memory buffer. Program bytes cannot be
bound as variables. Unbound reads and writes fault. Bindings survive completed
invocations; comparison/stack state resets on `start()`. Program data is shared
and immutable; each machine's variables, requests, stack, and trace are independent.

The machine executes at most the supplied budget per `run()`, with a hard limit
of one million instructions per invocation and 256 call frames. It retains the
last 64 instruction addresses. Malformed instructions, overlapping encountered
instruction spans, invalid targets, and unsupported operations fault with source,
virtual PC, record offset, and stack depth. Faulted machines cannot silently
restart; construct a new machine with deliberate initial state.

Waiting results expose the same request ID until resumed. Callers must dispatch
each ID once. Incorrect IDs, duplicate results, out-of-range choices, and choices
submitted to text requests are rejected without changing state. Mutations before
a later fault remain committed. There is no persistent save/replay API yet.

Division and random numbers remain unsupported because their complete engine
side effects/conventions are not implemented. So do table access, mapped party
state, native services, resource changes, pictures, combat, recruitment, and other
gameplay opcodes. They fault explicitly rather than claiming to have occurred.
The current text/menu request API is the first part of the proposed host contract;
there is no Godot session binding or automatic map-event scheduler yet.

### Implementation validation

`build.cmd` runs `opengold_ecl_tests` alongside the existing native suites. Tests
cover packed text, operand truncation, invalid entries, conditional skips,
arithmetic wrapping, branch/stack errors, menu resume validation, execution limits,
independent variables, archive loading, and program lifetime after catalog destruction.
Setting `OPENGOLD_GAME_DIR` adds an installed archive load check.

In the inspected installation all **29 ECL records** load and their entry tables
validate. Full static traversal currently reports diagnostics in six programs:
`ECL2.DAX:9`, `ECL2.DAX:15`, `ECL4.DAX:21`, `ECL5.DAX:7`, `ECL6.DAX:28`, and
`ECL8.DAX:16`. Reports include out-of-body targets and unsupported operand tags;
these need investigation rather than suppressing diagnostics or declaring the
original assets corrupt. Loading success is not full decoding or gameplay coverage.

## Full-engine design and remaining work

Build a native, resumable ECL interpreter for the Pool of Radiance DOS release.
Keep binary decoding, script execution, game rules, and presentation separate.
Execute original ECL through explicit OpenGold operations; original machine-code
routines and DOS addresses must never become native calls or pointers.

The first playable milestone is one verified location event that displays text,
accepts a choice, changes a persistent variable, and behaves correctly on revisit.
Full campaign compatibility follows incrementally, with unsupported behavior
reported rather than silently approximated.

## Existing foundations

| Existing component | Reuse and limits |
| --- | --- |
| `decode_dax_archive` in `OpenGold.Formats` | Validates and decompresses DAX containers. Reuse for `ECL*.DAX`. |
| [por_ecl_decoder.gd](../godot/scripts/por_ecl_decoder.gd) | Inspection decoder for typed operands, packed text, five entry jumps, and reachable control flow. Port its tested format knowledge; do not treat static analysis as execution. |
| [por_ecl_tables.gd](../godot/scripts/por_ecl_tables.gd) | Bounded table analysis for asset research. It deliberately stops at uncertain writes/calls and is not a VM. |
| [ECL tests](../tests/ecl_art_tests.gd) | Synthetic format fixtures and installed-data inspection checks. Useful for parity, not independent proof of runtime semantics. |
| `MapCatalog` / `GeoMap` | Map identity, directional geometry, and raw cell event data. No event dispatcher or movement rules yet. |
| `CreatureFactory` | Creates monster/NPC instances with HP and explicit turn budgets. Does not resolve combat, spell effects, or script resource-bank selection. |

Use `opengold::por` for the first implementation. Put decoding in
`OpenGold.Formats` and execution in `OpenGold.Core`, which remains Godot-free.
Keep title-specific address and resource mappings behind a PoR adapter; it can
move into the planned `OpenGold.Game.PoolOfRadiance` target when that target exists.

## Components and ownership

| Proposed type | Responsibility |
| --- | --- |
| `ScriptId` | Canonical archive filename plus record ID; never assume record IDs identify scripts across banks. |
| `EclCatalog` | Load immutable program records from an installation and retain source hashes and diagnostics. |
| `EclProgram` | Own original record bytes, entry table, typed instruction data, and address-to-offset translation. |
| `OpcodeSpec` | Pool-specific operand grammar, control-flow behavior, and execution support status for each opcode. |
| `EclMachine` | Instruction position, return stack, comparisons, script memory, pending request, and bounded execution. |
| `PorScriptMemory` | Checked reads/writes of virtual addresses; explicit bindings to party, map, and engine fields. |
| `PorScriptHost` | Apply typed gameplay requests through world, encounter, resource, and party services. |
| `ScriptScheduler` | Deliver triggers, advance the active machine, and resume it after a matching result. |

Programs are immutable and genuinely shared by catalogs, active machines, and
debugger views through `std::shared_ptr<const EclProgram>`. A campaign session owns
its scheduler, world state, and host by value or `std::unique_ptr`. Stacks,
operands, queues, and memory use standard RAII containers. No owned raw pointers
or file handles survive loading. References to host services are borrowed and
must not outlive the session.

Use one active exploration script invocation per session initially. UI callbacks
enqueue results on the game thread; they never recursively call the interpreter.
Nested ECL subroutine calls use the VM stack. A second machine or nested script
invocation requires evidence that the original behavior needs it.

```mermaid
flowchart LR
    DAX[Original ECL archives] --> Catalog[EclCatalog / EclProgram]
    GEO[Map and party state] --> Scheduler[ScriptScheduler]
    Catalog --> VM[EclMachine]
    Scheduler --> VM
    VM -->|Typed request| Host[PoR script host]
    Host --> World[World and combat services]
    Host -->|Presentation request| UI[Godot]
    UI -->|Choice or acknowledgement| Scheduler
    World -->|Operation result| Scheduler
```

## Loading and decoding

1. Discover ECL archives case-insensitively, retaining canonical names and bank
   identity. Reject ambiguous filenames, malformed DAX containers, and duplicate
   record IDs. Bound file sizes and allocations as in the existing catalogs.
2. Retain the complete decompressed record. The current PoR decoder removes a
   two-byte prefix and uses virtual code origin `0x9900`. Verify the prefix's
   length convention against installed records before enforcing exact equality.
3. Decode the five initial jump instructions as an entry table. Keep numeric
   entry slots in the format layer; give them gameplay names only in the adapter.
4. Decode reachable instructions and retain undecoded bytes as data. Preserve
   opcode, operand encoding tags, raw virtual addresses, byte spans, and source
   record offsets for diagnostics. Never search arbitrary bytes for opcodes.
5. Validate operand bounds and branch destinations. A target cannot point into
   the middle of a known instruction or operand. A dynamically resolved target
   must be checked and decoded before execution, not assumed to have appeared in
   the static graph. Reject conflicting code/data interpretations in the initial
   implementation and report any original script that requires an exception.

Use one opcode specification for the native decoder, disassembler, and VM.
Separate statuses such as `grammar_known`, `execution_supported`, and
`runtime_verified`. An inspection catalog may retain programs with diagnostics;
execution may proceed only through instructions whose grammar and semantics are
supported. Do not label an entire program compatible merely because it parses.

The current inspection decoder recognizes tags `0`, `1`, `2`, `3`, `0x80`, and
`0x81`, including inline packed six-bit text and address-bearing operands. Its
coarse `memory` label is insufficient for execution: preserve distinctions among
tags and resolve operands according to each opcode's role. In particular, a jump
address must not be accidentally dereferenced as a variable. Specify byte/word
width, signedness, endianness, string bounds, and writable destination rules.

Menus and indexed branches have variable operand lists. Validate their count
encoding independently of execution. Dynamic lengths not yet understood remain
unsupported rather than desynchronizing the instruction stream. Likewise,
conditional skips must advance over one complete decoded instruction, including
variable operands, rather than a fixed number of bytes.

There are known reference discrepancies: our Pool decoder/tests use one operand
for `ECL CLOCK` (`0x34`), while the inspected GBE command registration uses two.
Keep a per-opcode evidence ledger with record offsets, reference revision, tested
grammar, and unresolved semantics. Do not copy another game's table wholesale.

## Virtual memory and game state

ECL addresses are integers in a virtual address space. A proposed 64 KiB byte
array provides backing storage, with checked region descriptors controlling
access. Regions distinguish program bytes, script variables, and mapped engine
state. Unknown addresses produce a diagnostic; do not silently return zero.

Mapped reads and writes go through the PoR adapter. Maintain one authoritative
value for party position, selected character, time, quest state, and similar
fields. Do not maintain an unsynchronized copy in both the VM and world model.
Raw backing bytes may preserve unexplained data for inspection but must not imply
that its gameplay effects have been implemented.

Specify and test arithmetic width, overflow, division by zero, comparison flags,
random bounds, table stride, and string termination before enabling the relevant
operation. Implement explicit integer operations rather than relying on C++
signed overflow. Inject a deterministic RNG; save its algorithm/version and state.

The default code region is read-only. If installed scripts require writes to
tables embedded in a program, add a per-session writable overlay and invalidate
affected decode caches. Do not mutate shared `EclProgram` data. Self-modifying
instructions remain unsupported until their behavior is demonstrated.

## Execution and suspension

The following is an API sketch, not a compilable declaration or committed ABI:

```cpp
StartResult start(EntrySlot entry, TriggerContext context);
RunResult run(std::size_t instruction_budget);
ResumeResult resume(RequestId request, HostResult result);
MachineSnapshot snapshot() const;
```

`RunResult` is a tagged value containing one of:

| Result | Scheduler behavior |
| --- | --- |
| `Completed` | Invocation ended; process the next permitted trigger. |
| `AwaitingHost` | Dispatch a typed request once and wait for its result. |
| `BudgetExhausted` | Preserve state and continue on a later update. |
| `Fault` | Stop this invocation and show source context and the execution trace. |

Machine states are idle, running, waiting, completed, and faulted. Each update has
an instruction budget; stack depth, string sizes, request sizes, and trigger queue
length also have bounds. A repeated budget yield keeps the UI responsive and is
not itself a fault. A configurable total invocation limit detects runaway loops
and records a diagnostic rather than hanging forever.

For a host operation, first resolve and validate operands, then store its request
ID and continuation, and suspend. The continuation includes the next instruction
and any validated result destination. Calling `run()` while waiting must not
redispatch or reapply the operation. `resume()` validates the request ID and result
type; duplicate, stale, or mismatched replies leave state unchanged.

The host validates a mutation before committing it and returns a result in the
original script's value convention. Apply that result exactly once before
continuing. Unsupported opcodes and calls fault with script ID, virtual address,
record offset, raw operands, stack, and trigger context. Previous successful
instructions remain committed: the VM is not an automatic whole-event rollback
system. Debugger replay uses an isolated snapshot to avoid repeating rewards.

## Host requests and Godot integration

Requests and results are typed value objects, preferably `std::variant`, with no
Godot node references inside the VM.

| Request family | Host behavior |
| --- | --- |
| Text, pictures, sprite setup, clear display | Update presentation in script order; acknowledgement semantics are opcode-specific. |
| Choice, number, string, character selection | Pause for input; validate range, encoding, cancellation behavior, and result destination. |
| Resource loading and script replacement | Resolve bank context, stage assets, then commit the requested change. |
| Monster setup and combat | Build encounter definitions, run combat, and return the validated result code. |
| NPC recruitment, inventory, treasure, damage | Delegate to party/world rules, retaining script-visible results. |
| Delay and time queries | Use the game clock, separating game-time advancement from presentation delay. |
| Built-in `CALL` / program operations | Dispatch through an explicit, versioned table of implemented engine services. Unknown service IDs fault. |

`SAVE` and `SAVE TABLE` instruction names must not be confused with saving a game:
their actual variable/table write semantics belong in the opcode specification.

Use a future GDExtension session wrapper to expose advance/resume and immutable
presentation snapshots to Godot. Keep the command-line interpreter harness for
tests and research. The map demo's one-shot JSON subprocess is suitable for
inspection, but should not become the per-instruction gameplay transport.

## Map triggers and transitions

An event number is not an ECL record ID or instruction address. The active area's
script can dispatch through branches or tables and can inspect coordinates,
facing, flags, and party state. Several cells may share an event number while
behaving differently.

The reference entry table suggests normal movement/update, search, pre-camp,
camp-interruption, and startup roles. Preserve that as evidence, not proof of the
exact scheduling conditions. Resolve entry roles and movement timing with a
trace of the original PoR release before connecting general gameplay dispatch.

The proposed flow is: validate a move, commit the appropriate position/time
changes, construct trigger context, invoke the verified entry slot, then process
the resulting requests. The precise order, blocked-move behavior, repeat checks,
and whether an entry runs every exploration update remain adapter policies to
verify. A completed script does not automatically consume or clear a map event.

`TriggerContext` carries map ID, old/new position, facing, trigger kind, raw cell
byte, low-seven-bit event ID, and relevant state revision. Bit 7 has evidence for
an indoor flag; never use it as event-enabled or event-completed state. Event zero
does not justify skipping all area logic. See [MAPS.md](MAPS.md).

While an invocation waits for a menu or combat, pause exploration commands rather
than accumulating clicks as future movement. Queue only explicitly permitted
engine triggers. Validate queued contexts against world/area revisions before
dispatch; discard stale old-area events after a transition.

For `LOAD FILES`, `LOAD PIECES`, and `NEW ECL`, keep map, script, graphics, and
monster-bank context distinct. Do not infer that all record IDs or banks match.
Stage and validate replacement resources before committing a transition. Define
which variables and call frames survive each opcode through evidence; do not
assume a script replacement behaves like a returning subroutine call. Run startup
only where the validated transition rules require it.

## Creature integration

`LOAD MONSTER` supplies separate creature, count, and combat-icon operands in the
existing inspection model. Resolve the creature's archive bank through current
resource context, then call `CreatureFactory::create` for each instance. Store art
selection separately from `CreatureId`, since script-selected artwork can differ.
Stage the whole group before adding it so missing definitions do not create a
partial encounter. Bound counts and diagnose unresolved resource references.

HP generation, surprise, hostility, combat formation, and turn allowances belong
to encounter/combat rules. The factory's stored-max-HP default is not proof of
original encounter HP generation. `SETUP MONSTER`, combat monster loading, and
`ADD NPC` have different purposes; do not assume one automatically performs the
others. Recruitment must resolve which existing or new instance is transferred
to party ownership and preserve its live state according to validated behavior.

## Persistence and debugging

A session snapshot includes asset identities/hashes, compatibility-profile
version, active program, PC, return stack, comparison state, mutable VM regions,
resource context, pending request/continuation, trigger context/queue, RNG state,
and persistent world/party/encounter state. Store IDs and values, never native
pointers. Restore only against matching assets and supported snapshot versions.

Permit saves initially at defined boundaries: idle or waiting on a supported
serializable request. An interrupted combat is not resumable until combat state
has its own snapshot contract. Pending UI requests are reconstructed on load;
completed mutations are not reissued. This provides OpenGold saves, not automatic
compatibility with original DOS save files.

Extend the map review tool with a read-only script inspector: candidate programs
that reference a map, decoded entry points, relevant branch/table evidence, and
dialogue previews. Clearly distinguish proven dispatch from static candidates.
Then add an explicit isolated-session runner with step, continue, breakpoints,
memory watches, and a bounded trace. Clicking a cell must not silently execute
its script against the user's campaign state.

## Delivery sequence and acceptance criteria

| Stage | Deliverable | Acceptance |
| --- | --- | --- |
| 1. Format and evidence | Native ECL catalog, opcode grammar, disassembler, support report | Synthetic fixtures cover all supported encodings; installed archive inventory reports every record and diagnostic. Compare with existing inspector without assuming it is infallible. |
| 2. Pure execution | Variables, arithmetic, comparisons, branching, subroutines, table access, deterministic RNG | Hand-authored programs produce expected state and traces; bounds, divide errors, stack errors, conditional skips, and budget yields behave predictably. |
| 3. Resumable host | Text, menus, typed requests/results, save boundaries | Fake host proves pause/resume, result validation, no duplicate side effects, and deterministic save/reload. Godot shows the same request sequence. |
| 4. One real event | Verified map-to-program dispatch and a persistent choice | Match original-game entry, text, choices, flag changes, revisit behavior, and save/reload for one documented location. Label unimplemented reachable paths explicitly. |
| 5. Area changes and encounters | Resource banks, transitions, factory integration, combat result plumbing | Correct destination/startup order, no stale triggers, distinct creature/art IDs, independent creature HP, and correct result-dependent script continuation. |
| 6. Campaign coverage | Remaining opcodes, services, and compatibility profiles | Publish exercised paths and unresolved behaviors; completing sample events is not a claim of whole-game compatibility. |

Use generated fixtures without proprietary assets in the normal native suite.
Optional installed-data tests should report coverage by script and opcode, not
only a total pass count. Record original-game observations separately from
reference-derived expectations. For each verified event, retain script/map IDs,
coordinates, initial state, actions, outputs, state differences, and game version.

Before stage 4, resolve entry scheduling, mapped-memory addresses, arithmetic and
random conventions needed by that event, resource selection, and input result
codes. Unknown built-in services, one-time event behavior, save persistence, and
script replacement lifetimes are explicit research tasks, not default guesses.

## Evidence and related design

- [Asset source audit](asset-source-audit.md) and
  [NPC art identification](npc-art-identification.md): current static-analysis
  capabilities and limits.
- [Maps](MAPS.md) and [creature API](creature-catalog.md): integration contracts.
- [Gold Box Explorer ECL program decoder](https://github.com/bsimser/Gold-Box-Explorer/blob/eac30abaa6ee66aea6f5d65ebe6d676b10015a8f/src/Common/Plugins/DaxEcl/Program.cs)
  and [command descriptions](https://github.com/bsimser/Gold-Box-Explorer/blob/eac30abaa6ee66aea6f5d65ebe6d676b10015a8f/src/Common/Plugins/DaxEcl/Commands.cs):
  pinned format references, not a complete PoR execution specification.

Later Gold Box games need separate compatibility profiles. Keep generic VM
mechanisms reusable, but require evidence before sharing their opcode meanings,
memory layouts, or gameplay behavior with Pool of Radiance.
