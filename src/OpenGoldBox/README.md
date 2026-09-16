# OpenGoldBox game source

This is the editable game application, copied from the character demo. It starts
at character creation and retains the connected party, town, combat, inventory,
save, and advancement flow. The reference demos remain under `demos/`.
This starting point does not claim a complete campaign.

The C++20 code builds as a Godot GDExtension. `godot/` contains the game's own
scenes and theme. Godot exports the Windows executable; no demo project or
installed Godot editor is needed when running the exported game folder.

## Build

Use the existing Visual Studio/CMake/Godot prerequisites. Install a Windows x64
debug export template matching the Godot editor. The default template location
is `build/export-templates/windows_debug_x86_64.exe`; alternatively pass
`-DOPENGOLDBOX_DEBUG_TEMPLATE=C:/path/to/windows_debug_x86_64.exe` to the build helper.
Templates come from the official Godot release's export-template archive and
are local build dependencies, not repository content.

From the repository root in PowerShell:

```powershell
.\build-opengoldbox.cmd
.\win-package\opengoldbox.exe
```

The game requests a **1920 x 1080** window (16:9, excluding window borders).
The minimum resizable game area remains 1120 x 800. To launch fullscreen at the
monitor's dimensions, run `./win-package/opengoldbox.exe --fullscreen`.

By default startup opens character generation directly. To show the two splash
screens first, run `./win-package/opengoldbox.exe --splash`. Any key advances
from the OpenGoldBox image to the Pool of Radiance image, then to character
generation. The background appears immediately and the first lettering fades
in over 0.6 seconds. Advancing clears it immediately and fades the second
lettering in over 0.6 seconds. Keys remain responsive during either fade;
Escape skips directly to character generation from either image.
Both splash screens use the same static background texture; only the transparent
lettering overlay changes. The background is identical to the
pixel throughout the transition. The lettering restores the reference
image's beveled metallic gold title and slender ivory serif supporting text.
The shared generator output is 1672 x 941 and fits the display proportionally;
other aspect ratios use black letterboxing. The two screen previews in `art/`
are 1920 x 1080 captures from Godot. The build packages the shared background and both transparent lettering layers.
See `art/OpenGoldBoxScreens.provenance.md` for sources and prompts.

Close the main window with its X button, or press Ctrl+X from any screen or
dialog, to exit through the same normal shutdown path. The scene tree and native
resources are released as Godot shuts down. Ctrl+X does not advance splash screens.

The CMake target is `OpenGoldBox`. Its output is
`win-package/opengoldbox.exe`. Every build creates and provisions `win-package/`
at the repository root with the PCK (scenes, theme, and portraits), GDExtension
DLL, rules data and notices, and Visual C++ runtime DLLs. Keep the whole package
folder together. Native code uses `RelWithDebInfo` and the redistributable runtime;
the Godot debug template retains development checks. Visual Studio is not needed
to run the package.
To build the executable target directly after configuring:

```powershell
cmake --build build/game --target OpenGoldBox
```

The game reads original assets from the path saved in `settings.cfg` beside the
executable, with `OPENGOLD_GAME_DIR` as a runtime override. Missing settings prompt
for the language, then the game folder. `--reset-game-path` and `--reset-lang` force
selection again. See [configuration](../../docs/CONFIGURATION.md) for portable
settings, environment overrides and checksum warnings. The game preserves the
existing per-user save directory. Rules and
authored portraits are copied from root `data/` and `art/` at build time.
No original game assets are copied or exported. Diagnostic captures use
`user://checks` so exported builds do not depend on the repository layout.

The game copy delays resize handling until the town view's dynamic controls
exist, avoiding the `SaveGame` startup crash observed in the reference demo.

Validation: the game build and all 12 native test suites pass. The exported
`opengoldbox.exe` passes headless character and party integration checks,
including tour, shops, combat, recovery, and XP awards, from the output folder.
These automated checks do not establish full campaign completion.

Startup input checks (run once without `--splash` and once with it):

```powershell
godot_console --headless --path src/OpenGoldBox/godot --script ../../../tests/startup_tests.gd
godot_console --headless --path src/OpenGoldBox/godot --script ../../../tests/startup_tests.gd -- --splash
```

These cover key progression, Escape from either splash, and ignored mouse,
key-release, and held-key repeat events. Both screens were also visually checked
at the 1920 x 1080 target size; the rendered check verifies identical background
pixels across the text transition.

Shutdown checks use `tests/shutdown_tests.gd` with `--shutdown-close` or
`--shutdown-key`; optional `--splash`, `--shutdown-second`, `--shutdown-dialog`,
and `--shutdown-paused` cover other active views. Tests verify one shutdown
request and scene teardown. The packaged executable also passed a native
Windows `WM_CLOSE` check with exit code 0.

Combat terrain, figures, and battlefield overlays render at 3x magnification.
Use the mouse wheel to scroll vertically, Shift+wheel to scroll horizontally,
or hold the middle mouse button and drag to pan. Both scrollbars are always
visible and support keyboard focus; arrow keys pan one square when the view or
a scrollbar has focus. The camera centers on the active combatant
at the start of a turn and after they move; manual panning persists otherwise.
Menus and the combat log retain their normal size.

Combat viewport checks:

```powershell
godot_console --headless --path src/OpenGoldBox/godot --script ../../../tests/combat_view_tests.gd
```
## Language selection

English and Spanish are available. Launch `win-package\opengoldbox.exe --reset-lang`
from CMD to select **English** or **Español**. The selection is saved for later
launches. Combine with `--splash` to show the translated splash lettering after
selection. See [translation setup and coverage](../../docs/LOCALIZATION.md).
