# Reference demos

These are the existing OpenGoldBox demos and research views, preserved for
reference while the game is developed. They are not the game's entry point.
Change these files only when maintaining or explicitly working on a demo.

- `godot/`: demo project, scenes, scripts, shaders, and theme.
- `src/OpenGold.Godot/`: C++ GDExtension presentation used by the demos.
- `build-rolf.cmd`: builds the demo extension and runs the native tests.
- `review-*.cmd`: launch character/party, combat, Rolf/town, art, map, and sound demos.

From PowerShell at the repository root:

```powershell
.\demos\build-rolf.cmd
.\demos\review-character.cmd
```

The other launchers work the same way. Map inspection also needs the native
exporter built by `.\build.cmd`. Open `demos/godot/project.godot` for editor use.

The demos reuse the engine and rules in the root `src/`, authored assets in
`art/` and `data/`, and tests in `tests/`. Native inspection utilities remain in
`tools/`. Build output remains in `build/godot/`; the demo DLL and imported
portrait copies go to `demos/godot/bin/`. Captures and isolated test profiles
remain under the root `user-data/`. Original game files are still supplied by
the user through `OPENGOLD_GAME_DIR` or the demo project's existing setting.

No game project is created by this reorganization. See the root [README](../README.md)
and [feature documentation](../docs/CHARACTER-CREATION.md) for demo limitations.

## Relocation validation

The demo build and all 12 native tests pass. Headless character, training combat,
map inspector, and sound-board checks pass from this directory.
The standalone Rolf `--tour-check` currently crashes during startup while
looking up `SaveGame`; its control initialization is unchanged by the relocation.
That demo needs a separate startup fix before it can be used reliably.
