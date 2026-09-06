@echo off
setlocal

set "VSDEV_CMD=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"
set "CMAKE_EXE=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not exist "%VSDEV_CMD%" goto missing_vs
if not exist "%CMAKE_EXE%" goto missing_cmake

call "%VSDEV_CMD%" -arch=x64
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_EXE%" --preset default
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_EXE%" --build --preset default
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_EXE%" -E chdir build ctest --output-on-failure
exit /b %errorlevel%

:missing_vs
echo Visual Studio Build Tools environment was not found:
echo %VSDEV_CMD%
exit /b 1

:missing_cmake
echo CMake was not found:
echo %CMAKE_EXE%
exit /b 1
