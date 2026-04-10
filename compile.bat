@echo off
if "%~1"=="" (
    echo Usage: .\compile.bat [filename.cpp]
    echo Example: .\compile.bat Main.cpp
    exit /b 1
)

set "output=%~n1"
echo Compiling %~1...
g++ %~1 -o %output% -I raylib/src -L raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm

if %errorlevel% equ 0 (
    echo Compilation successful! Output: %output%.exe
    echo Running %output%.exe...
    echo.
    %output%.exe
) else (
    echo Compilation failed!
    exit /b %errorlevel%
)
