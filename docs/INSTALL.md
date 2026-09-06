# Development installation

## Windows prerequisites

Install these tools and make sure each command is available in a new terminal:

- Visual Studio 2022 Build Tools with **Desktop development with C++**, or an
  equivalent C++20 compiler
- CMake 3.24 or newer
- Ninja, if using the committed `default` preset
- Godot 4.5+ for the presentation project and native tour
- Git and Python 3.8+ when building the optional Godot C++ extension
- VS Code extensions recommended by `.vscode/extensions.json`

The Visual Studio Developer PowerShell is the simplest environment for the
Microsoft compiler. A MinGW or LLVM toolchain can also be used by selecting a
matching CMake generator.

## Native build

From the repository root:

```powershell
cmake --preset default
cmake --build --preset default
ctest --test-dir build --output-on-failure
```

The build compiles `src/OpenGold.Formats` and `src/OpenGold.Core` as C++20.
Tests do not require Godot.

## Godot project

Open `godot/project.godot` in Godot. The existing main scene remains the art
comparison tool. Build and launch the C++ Rolf tour from Windows CMD with
`build-rolf.cmd` and `review-rolf.cmd`. The optional GDExtension fetches pinned
official Godot C++ bindings; the core remains independently testable.
See [Rolf's tour](ROLF.md) for build options, controls and data configuration.

Do not commit original SSI game files or derived assets. Keep local copies in
ignored directories such as `testdata/local/` or `user-data/`.
