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

The game reads original assets from `OPENGOLD_GAME_DIR`, falling back to its
project setting. It preserves the existing per-user save directory. Rules and
authored portraits are copied from root `data/` and `art/` at build time.
No original game assets are copied or exported. Diagnostic captures use
`user://checks` so exported builds do not depend on the repository layout.

The game copy delays resize handling until the town view's dynamic controls
exist, avoiding the `SaveGame` startup crash observed in the reference demo.

Validation: the game build and all 12 native test suites pass. The exported
`opengoldbox.exe` passes headless character and party integration checks,
including tour, shops, combat, recovery, and XP awards, from the output folder.
These automated checks do not establish full campaign completion.
