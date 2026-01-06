@echo off
REM ============================================================================
REM ZenRunner Launcher Script for Windows
REM 
REM This script launches ZenRunner with proper environment configuration.
REM It automatically detects where ZenRunner is installed.
REM ============================================================================

setlocal EnableDelayedExpansion

REM Print banner
echo.
echo ╔════════════════════════════════════════════╗
echo ║                                            ║
echo ║              ZenRunner Launcher            ║
echo ║                                            ║
echo ╚════════════════════════════════════════════╝
echo.

REM Detect script directory
set "SCRIPT_DIR=%~dp0"

REM Try to find ZenRunner executable
set "ZENRUNNER_BIN="

REM Check build directory (Release)
if exist "%SCRIPT_DIR%build\bin\Release\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%SCRIPT_DIR%build\bin\Release\ZenRunner.exe"
    echo [INFO] Found ZenRunner in build\bin\Release
    goto :found
)

REM Check build directory (Debug)
if exist "%SCRIPT_DIR%build\bin\Debug\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%SCRIPT_DIR%build\bin\Debug\ZenRunner.exe"
    echo [INFO] Found ZenRunner in build\bin\Debug
    goto :found
)

REM Check build directory (root)
if exist "%SCRIPT_DIR%build\bin\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%SCRIPT_DIR%build\bin\ZenRunner.exe"
    echo [INFO] Found ZenRunner in build\bin
    goto :found
)

REM Check installed location
if exist "%LOCALAPPDATA%\Programs\ZenRunner\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%LOCALAPPDATA%\Programs\ZenRunner\ZenRunner.exe"
    echo [INFO] Found ZenRunner in installed location
    goto :found
)

REM Check if in PATH
where ZenRunner.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set "ZENRUNNER_BIN=ZenRunner.exe"
    echo [INFO] Found ZenRunner in PATH
    goto :found
)

REM Not found
echo [ERROR] ZenRunner executable not found!
echo.
echo Please install ZenRunner first by running:
echo   .\install.ps1
echo.
echo Or build it manually:
echo   mkdir build
echo   cd build
echo   cmake .. -DCMAKE_BUILD_TYPE=Release
echo   cmake --build . --config Release
echo.
pause
exit /b 1

:found
echo [SUCCESS] Found ZenRunner: %ZENRUNNER_BIN%
echo.
echo [INFO] Starting ZenRunner...
echo.

REM Launch ZenRunner with all arguments
start "" "%ZENRUNNER_BIN%" %*

endlocal
