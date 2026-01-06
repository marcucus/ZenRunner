# ZenRunner Installation Script for Windows
# PowerShell script to automate the installation of ZenRunner on Windows

param(
    [switch]$Help,
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release",
    [switch]$SkipDepsCheck,
    [switch]$Clean,
    [switch]$NoInstall,
    [string]$QtPath = ""
)

# Color codes for output
$script:InfoColor = "Cyan"
$script:SuccessColor = "Green"
$script:WarningColor = "Yellow"
$script:ErrorColor = "Red"

################################################################################
# Helper Functions
################################################################################

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor $script:InfoColor
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor $script:SuccessColor
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor $script:WarningColor
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor $script:ErrorColor
}

function Write-Header {
    param([string]$Message)
    Write-Host ""
    Write-Host "========================================" -ForegroundColor $script:InfoColor
    Write-Host $Message -ForegroundColor $script:InfoColor
    Write-Host "========================================" -ForegroundColor $script:InfoColor
    Write-Host ""
}

function Test-CommandExists {
    param([string]$Command)
    $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

function Show-Usage {
    Write-Host @"
ZenRunner Installation Script for Windows

Usage: .\install.ps1 [OPTIONS]

Options:
    -Help               Show this help message
    -BuildType TYPE     Build type: 'Release' (default) or 'Debug'
    -SkipDepsCheck      Skip dependency checking
    -Clean              Clean build directory before building
    -NoInstall          Build only, do not install
    -QtPath PATH        Path to Qt installation (e.g., C:\Qt\6.5.0\msvc2019_64)
    
Examples:
    .\install.ps1                          # Standard installation
    .\install.ps1 -Clean                   # Clean build and install
    .\install.ps1 -BuildType Debug         # Debug build
    .\install.ps1 -QtPath "C:\Qt\6.5.0\msvc2019_64"

"@
}

function Find-QtInstallation {
    Write-Info "Searching for Qt installation..."
    
    # Check common Qt installation paths
    $commonPaths = @(
        "C:\Qt\6.5.0\msvc2019_64",
        "C:\Qt\6.5.0\msvc2022_64",
        "C:\Qt\6.6.0\msvc2019_64",
        "C:\Qt\6.6.0\msvc2022_64",
        "C:\Qt\6.7.0\msvc2019_64",
        "C:\Qt\6.7.0\msvc2022_64"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            Write-Success "Found Qt at: $path"
            return $path
        }
    }
    
    # Check environment variable
    if ($env:CMAKE_PREFIX_PATH) {
        if (Test-Path $env:CMAKE_PREFIX_PATH) {
            Write-Success "Using Qt from CMAKE_PREFIX_PATH: $env:CMAKE_PREFIX_PATH"
            return $env:CMAKE_PREFIX_PATH
        }
    }
    
    return $null
}

function Test-Dependencies {
    Write-Header "Checking Dependencies"
    
    $missingDeps = @()
    
    # Check CMake
    if (Test-CommandExists "cmake") {
        try {
            $cmakeOutput = cmake --version 2>&1 | Select-Object -First 1
            $cmakeVersion = $cmakeOutput -replace '.*version\s+', '' -replace '\s+.*', ''
            if ($cmakeVersion) {
                Write-Success "CMake $cmakeVersion found"
            } else {
                Write-Success "CMake found"
            }
        } catch {
            Write-Success "CMake found"
        }
    } else {
        $missingDeps += "cmake"
    }
    
    # Check for C++ compiler (Visual Studio or MinGW)
    $vswhereExists = Test-Path "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $mingwExists = Test-CommandExists "g++"
    
    if ($vswhereExists) {
        $vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
        if ($vsPath) {
            Write-Success "Visual Studio found at: $vsPath"
        }
    } elseif ($mingwExists) {
        $gccVersion = (g++ --version | Select-Object -First 1)
        Write-Success "MinGW found: $gccVersion"
    } else {
        $missingDeps += "Visual Studio or MinGW"
    }
    
    # Check Git
    if (Test-CommandExists "git") {
        Write-Success "Git found"
    } else {
        $missingDeps += "git"
    }
    
    if ($missingDeps.Count -gt 0) {
        Write-Error "Missing dependencies: $($missingDeps -join ', ')"
        Write-Info "Please install missing dependencies first."
        Show-DependencyInstructions
        exit 1
    }
    
    Write-Success "All dependencies satisfied"
}

function Show-DependencyInstructions {
    Write-Host ""
    Write-Info "Installation instructions:"
    Write-Host ""
    Write-Host "  1. CMake: Download from https://cmake.org/download/"
    Write-Host "  2. Visual Studio: Download from https://visualstudio.microsoft.com/"
    Write-Host "     - Select 'Desktop development with C++' workload"
    Write-Host "  3. Qt 6: Download from https://www.qt.io/download-open-source-software"
    Write-Host "     - Select Qt 6.5 or later with Qt Quick"
    Write-Host "  4. Git: Download from https://git-scm.com/download/win"
    Write-Host ""
}

function Build-Application {
    Write-Header "Building ZenRunner"
    
    # Clean build directory if requested
    if ($Clean -and (Test-Path "build")) {
        Write-Info "Cleaning previous build..."
        Remove-Item -Path "build" -Recurse -Force
    }
    
    # Create build directory
    if (-not (Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" | Out-Null
    }
    
    Set-Location "build"
    
    try {
        # Configure with CMake
        Write-Info "Configuring with CMake..."
        
        $cmakeArgs = @(
            "..",
            "-DCMAKE_BUILD_TYPE=$BuildType"
        )
        
        if ($script:QtInstallPath) {
            $cmakeArgs += "-DCMAKE_PREFIX_PATH=$script:QtInstallPath"
        }
        
        & cmake $cmakeArgs
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }
        
        # Build
        Write-Info "Compiling (this may take a few minutes)..."
        & cmake --build . --config $BuildType
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }
        
        Set-Location ".."
        
        # Check if executable was created
        $exePaths = @(
            "build\bin\$BuildType\ZenRunner.exe",
            "build\bin\ZenRunner.exe",
            "build\$BuildType\ZenRunner.exe"
        )
        
        $exeFound = $false
        foreach ($exePath in $exePaths) {
            if (Test-Path $exePath) {
                $script:ExecutablePath = $exePath
                $exeFound = $true
                break
            }
        }
        
        if (-not $exeFound) {
            throw "Build failed - executable not found"
        }
        
        Write-Success "Build completed successfully"
        Write-Info "Executable location: $script:ExecutablePath"
        
    } catch {
        Set-Location ".."
        Write-Error $_
        exit 1
    }
}

function Install-Application {
    Write-Header "Installing ZenRunner"
    
    # Create installation directory in user profile
    $installDir = "$env:LOCALAPPDATA\Programs\ZenRunner"
    
    if (-not (Test-Path $installDir)) {
        New-Item -ItemType Directory -Path $installDir -Force | Out-Null
    }
    
    # Copy executable
    Write-Info "Copying files to: $installDir"
    Copy-Item $script:ExecutablePath -Destination $installDir -Force
    
    # Copy Qt DLLs if needed
    if ($script:QtInstallPath) {
        $qtBinPath = Join-Path $script:QtInstallPath "bin"
        if (Test-Path $qtBinPath) {
            Write-Info "Copying Qt runtime libraries..."
            
            $qtDlls = @(
                "Qt6Core.dll",
                "Qt6Gui.dll",
                "Qt6Quick.dll",
                "Qt6Widgets.dll",
                "Qt6Qml.dll",
                "Qt6Network.dll"
            )
            
            foreach ($dll in $qtDlls) {
                $dllPath = Join-Path $qtBinPath $dll
                if (Test-Path $dllPath) {
                    Copy-Item $dllPath -Destination $installDir -Force -ErrorAction SilentlyContinue
                }
            }
            
            # Copy Qt plugins
            $pluginsPath = Join-Path $script:QtInstallPath "plugins"
            $installPluginsPath = Join-Path $installDir "plugins"
            
            if (Test-Path $pluginsPath) {
                Write-Info "Copying Qt plugins..."
                Copy-Item $pluginsPath -Destination $installPluginsPath -Recurse -Force -ErrorAction SilentlyContinue
            }
        }
    }
    
    Write-Success "Installed to: $installDir\ZenRunner.exe"
    
    # Add to PATH
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    if ($userPath -notlike "*$installDir*") {
        Write-Info "Adding installation directory to user PATH..."
        [Environment]::SetEnvironmentVariable(
            "Path",
            "$userPath;$installDir",
            "User"
        )
        Write-Warning "Please restart your terminal for PATH changes to take effect"
    }
}

function New-LauncherScript {
    Write-Header "Creating Launcher Script"
    
    $launcherContent = @'
@echo off
REM ZenRunner Launcher Script
REM This script launches ZenRunner with proper environment configuration

setlocal

REM Detect script directory
set "SCRIPT_DIR=%~dp0"

REM Try to find ZenRunner executable
if exist "%SCRIPT_DIR%build\bin\Release\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%SCRIPT_DIR%build\bin\Release\ZenRunner.exe"
) else if exist "%SCRIPT_DIR%build\bin\Debug\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%SCRIPT_DIR%build\bin\Debug\ZenRunner.exe"
) else if exist "%SCRIPT_DIR%build\bin\ZenRunner.exe" (
    set "ZENRUNNER_BIN=%SCRIPT_DIR%build\bin\ZenRunner.exe"
) else (
    where ZenRunner.exe >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set "ZENRUNNER_BIN=ZenRunner.exe"
    ) else if exist "%LOCALAPPDATA%\Programs\ZenRunner\ZenRunner.exe" (
        set "ZENRUNNER_BIN=%LOCALAPPDATA%\Programs\ZenRunner\ZenRunner.exe"
    ) else (
        echo ERROR: ZenRunner executable not found!
        echo Please run install.ps1 first to build and install ZenRunner.
        exit /b 1
    )
)

REM Launch ZenRunner
echo Starting ZenRunner...
start "" "%ZENRUNNER_BIN%" %*

endlocal
'@
    
    $launcherContent | Out-File -FilePath "zenrunner.bat" -Encoding ASCII
    Write-Success "Created launcher script: zenrunner.bat"
}

################################################################################
# Main Installation Flow
################################################################################

function Main {
    # Show help if requested
    if ($Help) {
        Show-Usage
        exit 0
    }
    
    # Print banner
    Clear-Host
    Write-Host ""
    Write-Host "╔════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║                                            ║" -ForegroundColor Cyan
    Write-Host "║          ZenRunner Installation            ║" -ForegroundColor Cyan
    Write-Host "║               (Windows)                    ║" -ForegroundColor Cyan
    Write-Host "║                                            ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""
    
    Write-Info "Build type: $BuildType"
    Write-Host ""
    
    # Check dependencies
    if (-not $SkipDepsCheck) {
        Test-Dependencies
    } else {
        Write-Warning "Skipping dependency check"
    }
    
    # Find Qt installation
    if ($QtPath) {
        if (Test-Path $QtPath) {
            $script:QtInstallPath = $QtPath
            Write-Success "Using Qt from: $QtPath"
        } else {
            Write-Error "Specified Qt path does not exist: $QtPath"
            exit 1
        }
    } else {
        $script:QtInstallPath = Find-QtInstallation
        if (-not $script:QtInstallPath) {
            Write-Warning "Qt installation not found automatically"
            Write-Info "You may need to specify Qt path with -QtPath parameter"
            Write-Info "Or set CMAKE_PREFIX_PATH environment variable"
        }
    }
    
    # Build application
    Build-Application
    
    # Install application
    if (-not $NoInstall) {
        Install-Application
    } else {
        Write-Info "Skipping installation (-NoInstall flag)"
    }
    
    # Create launcher script
    New-LauncherScript
    
    # Success message
    Write-Header "Installation Complete!"
    Write-Host ""
    Write-Host "ZenRunner has been successfully installed!" -ForegroundColor Green
    Write-Host ""
    Write-Host "To launch ZenRunner:" -ForegroundColor Cyan
    Write-Host ""
    
    if (-not $NoInstall) {
        Write-Host "  ZenRunner.exe" -ForegroundColor White
        Write-Host ""
        Write-Host "Or use the launcher script:" -ForegroundColor Cyan
        Write-Host ""
    }
    
    Write-Host "  .\zenrunner.bat" -ForegroundColor White
    Write-Host ""
    Write-Host "For more information, see:" -ForegroundColor Cyan
    Write-Host "  - INSTALL.md for detailed installation guide"
    Write-Host "  - README.md for usage instructions"
    Write-Host "  - docs\WORKSPACE_GUIDE.md for workspace management"
    Write-Host ""
    Write-Success "Happy coding! 🚀"
    Write-Host ""
}

# Run main function
Main
