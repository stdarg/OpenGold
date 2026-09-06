@echo off
setlocal
if not exist "%~dp0build\opengold_maps.exe" (
    echo Build the native map loader first by running build.cmd.
    exit /b 1
)
godot --path "%~dp0godot" res://scenes/map_inspector.tscn
