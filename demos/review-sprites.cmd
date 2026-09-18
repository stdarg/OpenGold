@echo off
setlocal
call "%~dp0build-rolf.cmd"
if not "%errorlevel%"=="0" exit /b %errorlevel%
godot --headless --editor --path "%~dp0godot" --import --quit
if not "%errorlevel%"=="0" exit /b %errorlevel%
godot --path "%~dp0godot" --resolution 1920x1080 res://scenes/combat_sprite_demo.tscn %*
exit /b %errorlevel%
