# Rolf's tour in C++ and Godot

This native scene runs the original Rolf sequence through his farewell: eight
dialogue pauses and 35 scripted redraws with the currently configured game files.
It extends the welcome with the temple, docks, training hall, city hall, park and
gate stops. Dialogue and route tables come from the original ECL at runtime.
The first-person view now assembles Phlan's original wall, door, shop, temple,
and waterfront pieces directly from your installed game files.

## Run from Windows CMD

From the repository root:

```cmd
build-rolf.cmd
review-rolf.cmd
```

The first command builds the C++ GDExtension and runs all five native test suites.
The second imports the extension and opens the Godot scene. Once built, use only
`review-rolf.cmd` to run it again. Close the scene before rebuilding its DLL.

Game data uses `OPENGOLD_GAME_DIR`, falling back to `opengold/game_directory` in
`godot/project.godot`. To override it in CMD:

```cmd
set "OPENGOLD_GAME_DIR=C:\Games\POOLRAD"
review-rolf.cmd
```

Use **Continue** or **Enter** at each pause. Movement remains locked until the
farewell finishes. Then use the arrow keys or the turn/step buttons to inspect
the map. **Replay tour** resets the isolated session. **Map: full/visited** changes
map visibility; visited means cells actually occupied, not a line-of-sight rule.

## Build boundary

The scene is `godot/scenes/rolf_tour.tscn`. Its `RolfTourView` node is implemented
in `src/OpenGold.Godot` using C++; no GDScript drives this scene. The native
`opengold::por::RolfTourSession` reuses `EclMachine`, `MapCatalog` and the existing
sprite decoder. Core code has no Godot dependency. Scene nodes own their children;
the wrapper uses Godot `Ref` values for textures/audio and value-owned session
state. All original resources remain local.

The optional CMake target is enabled by `OPENGOLD_BUILD_GODOT=ON`. Prerequisites
are a C++20 toolchain, CMake, Ninja, Git, Python 3.8+ for binding generation, and
Godot 4.5 or later with the standard single-precision build. The CMD helper uses
the same installed Visual Studio Build Tools location as `build.cmd`. With a
configured developer environment, the equivalent commands are:

```cmd
cmake -S . -B build/godot -G Ninja -DCMAKE_BUILD_TYPE=Debug -DOPENGOLD_BUILD_GODOT=ON
cmake --build build/godot
ctest --test-dir build/godot --output-on-failure
```

The first configure fetches official MIT-licensed
[godot-cpp](https://github.com/godotengine/godot-cpp) at commit
`e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77` (`godot-4.5-stable`). Supply
`-DPython3_EXECUTABLE=C:/path/to/python.exe` if Python is not on PATH. A local
checkout can be supplied with `-DFETCHCONTENT_SOURCE_DIR_GODOT_CPP=...`.
Libraries and the generated extension descriptor go in ignored `godot/bin/`.
The default native-only build requires neither Python nor Godot bindings.

Verified here on Windows x64 with Godot 4.7.2 and MSVC. Linux/macOS builds have
not been verified. See the official [CMake integration documentation](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/build_system/cmake.html).

## Evidence and deliberate limits

This is an isolated event host, not a complete campaign scheduler. It starts
`ECL3.DAX:0` explicitly at `0xB071`, with minimal mapped state and midday as its
time fixture. It does not establish the campaign's entry conditions or revisit
policy. The loader checks the expected entry instructions before running.

| Source/service | Implemented behavior |
| --- | --- |
| `GEO3.DAX:0` | Original 16 x 16 New Phlan geometry for both views. |
| `WALLDEF3.DAX:0` and `8X8D` resources | All fifteen Phlan appearances, ten stored perspectives each; see [wall-art mapping](phlan-wall-art.md). |
| `ECL3:0`, `0xB0AD` | SETUP MONSTER operands 12, 2, 9 select the provisional `SPRIT3.DAX:12` resource profile. |
| APPROACH / SPRITE OFF / PICTURE 255 | Select stored far, medium and near images; clear the encounter image. |
| `0xB0B9`, helper `0xAF1F` | Original introduction and a single Continue choice. Later text executes from the original program. |
| `0xC04B` / `0xC04C` / `0xC04D` | Authoritative VM party X / Y / facing, with 0=N, 1=E, 2=S, 3=W. |
| CALL `0x2C90` | Validate pose, derive current wall/event cells and publish a shared view snapshot. |
| CALL `0xBA03`, selector 8 | Emit a footstep cue; Godot plays newly generated OpenGold audio. |
| DELAY | Nonblocking 0.22-second presentation pause; original timing is not verified. |
| `0x4AC5` | Original script writes 1; replay explicitly resets the research session. |

The view selects artwork from directional GEO appearance IDs. Door interaction
bits do not select a generic door overlay: wooden doors and their knobs are
already in the original images. Opposite sides of an edge retain their own art.
Native code assembles tiles, composites far-to-near on an 88 x 88 canvas, and
updates a cached Godot texture when the party moves or turns. Godot fits the
complete frame using nearest-neighbor sampling and 6:5 pixel-height correction;
wide windows have side margins so nearby door tops and thresholds stay visible.

The resource profile is explicit for Phlan, with a guard on the decoded
`LOAD PIECES 127,127,127` instruction at `0x9B11`. It does not establish general
127 semantics or select art for other maps. City Hall, training hall, and gateway
captures were checked against the original-game screenshots and tour positions.
The flat daytime sky/ground colors, indoor ceiling color and sprite anchoring
remain presentation approximations; exact dynamic backgrounds, every distant
occlusion case, and original audio are not yet verified. See
[map findings](MAPS.md), [NPC art evidence](npc-art-identification.md), and the
[published PC 1.3 ECL reference](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869).

After the tour, inspection movement blocks walls/doors on either side of an
edge and blocks map boundaries. It does not open doors, dispatch area events,
leave New Phlan, persist campaign state, or implement combat.

## Verification

`opengold_tour_tests` uses generated data to check mapped pose, nonblocking delay,
sprite changes, text, input locks, stale/duplicate replies, replay, collision and
unsupported services. With `OPENGOLD_GAME_DIR` set, it also runs the entire local
original tour and checks its final script flag, Phlan art resources and landmark
IDs. Synthetic wall fixtures check tile bounds, normal EGA colors, transparency,
all four facings, opposite-edge appearance selection, near-wall occlusion, frame
placement and map boundaries. No original text/assets are
embedded in the test fixtures.

After building, check the actual Godot scene and input path:

```cmd
review-rolf.cmd --headless -- --tour-check
```

To capture its eight rendered pauses for local inspection:

```cmd
review-rolf.cmd -- --tour-check --capture
```

Screenshots go to ignored `user-data/rolf-tour-1.png` through `rolf-tour-8.png`.
The check exits automatically and verifies Enter with another button focused,
held-key rejection, movement locking and turning after the tour.
