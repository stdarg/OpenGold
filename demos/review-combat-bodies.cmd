@echo off
setlocal
call "%~dp0build-rolf.cmd"
if not "%errorlevel%"=="0" exit /b %errorlevel%
godot --headless --editor --path "%~dp0godot" --import --quit
if not "%errorlevel%"=="0" exit /b %errorlevel%
godot --path "%~dp0godot" --resolution 1280x720 res://scenes/combat_body_review.tscn %*
exit /b %errorlevel%
