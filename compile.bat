@echo off
setlocal ENABLEDELAYEDEXPANSION

REM === Argument Checking ===
if "%~1"=="" (
    echo.
    echo [!] Usage: %~nx0 [release^|debug^|test] [true^|false]
    echo.
    exit /b 1
)
if "%~2"=="" (
    echo.
    echo [!] Usage: %~nx0 [release^|debug^|test] [true^|false]
    echo.
    exit /b 1
)

set "BUILD_TYPE=%~1"
set "RUN_AFTER=%~2"

if /I not "%BUILD_TYPE%"=="release" if /I not "%BUILD_TYPE%"=="debug" if /I not "%BUILD_TYPE%"=="test" (
    echo.
    echo [ERROR] Invalid build type: "%BUILD_TYPE%"
    echo         Must be "release", "debug", or "test"
    echo.
    exit /b 1
)
if /I not "%RUN_AFTER%"=="true" if /I not "%RUN_AFTER%"=="false" (
    echo.
    echo [ERROR] Invalid run option: "%RUN_AFTER%"
    echo         Must be either "true" or "false"
    echo.
    exit /b 1
)

REM === MSYS2 / Raylib Settings ===
set "MSYS2_DIR=C:\msys64"
set "RAYLIB_INC=%MSYS2_DIR%\mingw64\include"
set "RAYLIB_LIB=%MSYS2_DIR%\mingw64\lib"

REM === Build Flags ===
if /I "%BUILD_TYPE%"=="debug" (
    set "OUT_DIR=build\debug"
    set "CFLAGS=-std=c23 -Wno-inline -g -O0 -Wall -Wextra -DDEBUG=1 -I"%RAYLIB_INC%"" 
    set "LDFLAGS=-g -L"%RAYLIB_LIB%" -lraylib -lopengl32 -lgdi32 -lwinmm -lkernel32"
    echo [*] Build Mode: DEBUG
) else if /I "%BUILD_TYPE%"=="release" (
    set "OUT_DIR=build\release"
    set "CFLAGS=-std=c23 -Wno-inline -O2 -I"%RAYLIB_INC%"" 
    set "LDFLAGS=-L"%RAYLIB_LIB%" -lraylib -lopengl32 -lgdi32 -lwinmm -lkernel32"
    echo [*] Build Mode: RELEASE
) else (
    set "OUT_DIR=build\test"
    set "CFLAGS=-std=c23 -Wno-inline -g -O0 -Wall -Wextra -DINTESTING=1 -I"%RAYLIB_INC%"" 
    set "LDFLAGS=-g -L"%RAYLIB_LIB%" -lraylib -lopengl32 -lgdi32 -lwinmm -lkernel32"
    echo [*] Build Mode: TEST
)

if not exist "%OUT_DIR%" (
    echo [*] Creating output directory: %OUT_DIR%
    mkdir "%OUT_DIR%"
)

REM === Copy data directory ===
if exist "%OUT_DIR%\data" (
    echo [*] Deleting old data folder...
    rd /s /q "%OUT_DIR%\data"
)
if exist "project\data" (
    echo [*] Copying data files...
    xcopy /E /I /Y "project\data" "%OUT_DIR%\data" >nul
)

REM === Gather .c Sources ===
set "SOURCES="
for /R ".\project\src" %%f in (*.c) do (
    set "SOURCES=!SOURCES! "%%f""
)

set "EXE=%OUT_DIR%\program.exe"

REM === Compile Icon Resource ===
echo.
echo ============================================
echo            Compiling Icon Resource         
echo ============================================
windres "project\src\icon.rc" -O coff -o "%OUT_DIR%\icon.res"
if errorlevel 1 (
    echo.
    echo [X] Failed to compile icon resource.
    echo.
    exit /b 1
)

REM === Compile & Link ===
echo.
echo ============================================
echo          Compiling And Linking Source        
echo ============================================
gcc %CFLAGS% !SOURCES! %LDFLAGS% "%OUT_DIR%\icon.res" -o "!EXE!"
if errorlevel 1 (
    echo.
    echo [X] Compilation failed.
    echo.
    exit /b 1
)

echo.
echo [v/] Build successful: "!EXE!"
echo.

REM === Run If Requested Or If Test Mode ===
if /I "%BUILD_TYPE%"=="test" (
    echo ============================================
    echo             Running Test Build             
    echo ============================================
    "!EXE!"
    echo.
    if !ERRORLEVEL! EQU 0 (
        echo [v/] Test PASSED with exit code 0
    ) else (
        echo [X] Test FAILED with exit code !ERRORLEVEL!
    )
    exit /b !ERRORLEVEL!
) else if /I "%RUN_AFTER%"=="true" (
    echo ============================================
    echo               Executing Program            
    echo ============================================
    if /I "%BUILD_TYPE%"=="debug" (
        start "GDB Debugger" cmd /c gdb -tui -ex "set pagination off" -ex run -ex quit --args "!EXE!"
    ) else (
        start "" "!EXE!"
    )
)


endlocal
exit /b 0
            