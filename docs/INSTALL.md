# Development installation

## Windows prerequisites

Install these tools and make sure each command is available in a new terminal:

- Visual Studio 2022 Build Tools with **Desktop development with C++**, or an
  equivalent C++20 compiler
- CMake 3.24 or newer
- Ninja, if using the committed `default` preset
- Godot 4.x for the presentation project
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

Open `godot/project.godot` in Godot 4.x. The project currently provides the
presentation shell and main scene. The CMake-built native libraries are kept
independent until the GDExtension binding target is introduced.

Do not commit original SSI game files or derived assets. Keep local copies in
ignored directories such as `testdata/local/` or `user-data/`.
