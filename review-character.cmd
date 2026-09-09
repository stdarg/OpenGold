@echo off
setlocal
if "%~1"=="--save-restart" set "APPDATA=%~dp0user-data\save-check-profile"
if "%~1"=="--defeat-check" set "APPDATA=%~dp0user-data\defeat-check-profile"
if not exist "%~dp0godot\bin\opengold_godot.dll" (
    echo Build the C++ Godot extension first: build-rolf.cmd
    exit /b 1
)
godot --headless --editor --path "%~dp0godot" --import --quit
if not "%errorlevel%"=="0" exit /b %errorlevel%
if "%~1"=="--save-restart" goto save_restart
if "%~1"=="--defeat-check" goto defeat_check
godot --path "%~dp0godot" --resolution 1280x900 res://scenes/character_creation.tscn %*
exit /b %errorlevel%

:defeat_check
rem Exercise loss and recovery using isolated saves, then capture the defeat UI.
godot --path "%~dp0godot" --resolution 1280x900 res://scenes/character_creation.tscn -- --defeat-check --capture
exit /b %errorlevel%

:save_restart
rem Isolate acceptance saves from the player's normal per-user saves.
set "APPDATA=%~dp0user-data\save-check-profile"
godot --headless --path "%~dp0godot" res://scenes/character_creation.tscn -- --party-check --save-check-write
if not "%errorlevel%"=="0" exit /b %errorlevel%
rem The writer has exited; this is a new process with new native/Godot owners.
godot --headless --path "%~dp0godot" res://scenes/character_creation.tscn -- --save-check-read
exit /b %errorlevel%
