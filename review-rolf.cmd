@echo off
setlocal
if not exist "%~dp0godot\bin\opengold_godot.dll" (
    echo Build the C++ Godot extension first: build-rolf.cmd
    exit /b 1
)
godot --headless --editor --path "%~dp0godot" --import --quit
if not "%errorlevel%"=="0" exit /b %errorlevel%
godot --path "%~dp0godot" --resolution 1280x900 res://scenes/rolf_tour.tscn %*
exit /b %errorlevel%
