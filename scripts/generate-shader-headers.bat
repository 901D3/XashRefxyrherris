@echo off
setlocal EnableDelayedExpansion

:: XashRefXyrherris
:: └──scripts
::     .bat

set "ORIGINAL_PATH=%~dp0"
cd /d %~dp0

cd ..\src\backend\gl\shader\

set "ROOT=%CD%"
set "OUTDIR=%ROOT%\out"

for /R %%F in (*.frag *.vert *.glsl) do (

    .\..\..\..\..\tools\glsl2header\glsl2header.exe -i "%%F" -include "utils.glsl"

    set "REL=%%F"
    set "REL=!REL:%ROOT%\=!"

    mkdir "%OUTDIR%\!REL!\.." 2>nul

    move /y "%%F.h" "%OUTDIR%\!REL!.h"
)

cd /d %ORIGINAL_PATH%