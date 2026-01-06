#!/usr/bin/env pwsh
# Build Windows Installer Script for ZenRunner
# This script builds the NSIS installer for Windows

param(
    [string]$BuildDir = "build",
    [string]$QtPath = "",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Show-Usage {
    Write-Host @"
Build Windows Installer for ZenRunner

Usage: .\build-installer.ps1 [OPTIONS]

Options:
    -BuildDir DIR    Build directory (default: build)
    -QtPath PATH     Path to Qt installation
    -Help            Show this help message

Examples:
    .\build-installer.ps1
    .\build-installer.ps1 -QtPath "C:\Qt\6.5.0\msvc2019_64"
"@
}

function Test-NsisInstalled {
    $nsisPath = $null
    
    # Check common installation paths
    $commonPaths = @(
        "${env:ProgramFiles}\NSIS\makensis.exe",
        "${env:ProgramFiles(x86)}\NSIS\makensis.exe",
        "C:\Program Files\NSIS\makensis.exe",
        "C:\Program Files (x86)\NSIS\makensis.exe"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    # Check PATH
    $makensis = Get-Command makensis -ErrorAction SilentlyContinue
    if ($makensis) {
        return $makensis.Source
    }
    
    return $null
}

function Copy-QtDependencies {
    param(
        [string]$QtBinPath,
        [string]$TargetDir
    )
    
    Write-Info "Copying Qt dependencies..."
    
    $qtDlls = @(
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Quick.dll",
        "Qt6Qml.dll",
        "Qt6Widgets.dll",
        "Qt6Network.dll",
        "Qt6QmlModels.dll",
        "Qt6OpenGL.dll"
    )
    
    foreach ($dll in $qtDlls) {
        $sourcePath = Join-Path $QtBinPath $dll
        if (Test-Path $sourcePath) {
            Copy-Item $sourcePath -Destination $TargetDir -Force
            Write-Info "  Copied: $dll"
        }
    }
    
    # Copy Qt plugins
    $qtRoot = Split-Path $QtBinPath -Parent
    $pluginsPath = Join-Path $qtRoot "plugins"
    
    if (Test-Path $pluginsPath) {
        Write-Info "Copying Qt plugins..."
        
        $pluginDirs = @("platforms", "imageformats", "styles", "qmltooling")
        
        foreach ($pluginDir in $pluginDirs) {
            $sourcePath = Join-Path $pluginsPath $pluginDir
            $targetPath = Join-Path $TargetDir $pluginDir
            
            if (Test-Path $sourcePath) {
                if (-not (Test-Path $targetPath)) {
                    New-Item -ItemType Directory -Path $targetPath -Force | Out-Null
                }
                Copy-Item "$sourcePath\*" -Destination $targetPath -Recurse -Force
                Write-Info "  Copied plugin: $pluginDir"
            }
        }
    }
}

function Build-Installer {
    Write-Info "Building ZenRunner Windows Installer..."
    
    # Check if NSIS is installed
    $nsisPath = Test-NsisInstalled
    if (-not $nsisPath) {
        Write-Error "NSIS not found. Please install NSIS from https://nsis.sourceforge.io/"
        Write-Info "After installation, make sure makensis.exe is in your PATH"
        exit 1
    }
    
    Write-Success "Found NSIS at: $nsisPath"
    
    # Get script directory
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $rootDir = Split-Path (Split-Path $scriptDir -Parent) -Parent
    
    # Check if build directory exists
    $buildPath = Join-Path $rootDir $BuildDir
    if (-not (Test-Path $buildPath)) {
        Write-Error "Build directory not found: $buildPath"
        Write-Info "Please build the application first using install.ps1"
        exit 1
    }
    
    # Find executable
    $exePaths = @(
        "$buildPath\bin\Release\ZenRunner.exe",
        "$buildPath\bin\ZenRunner.exe",
        "$buildPath\Release\ZenRunner.exe"
    )
    
    $exeFound = $false
    foreach ($exePath in $exePaths) {
        if (Test-Path $exePath) {
            Write-Success "Found executable: $exePath"
            $exeFound = $true
            $executablePath = $exePath
            break
        }
    }
    
    if (-not $exeFound) {
        Write-Error "ZenRunner.exe not found in build directory"
        Write-Info "Please build the application first"
        exit 1
    }
    
    # Copy Qt dependencies if Qt path provided
    if ($QtPath -and (Test-Path $QtPath)) {
        $qtBinPath = Join-Path $QtPath "bin"
        $releaseDir = Split-Path $executablePath -Parent
        
        Copy-QtDependencies -QtBinPath $qtBinPath -TargetDir $releaseDir
    } else {
        Write-Info "Qt path not provided, assuming dependencies are already in place"
    }
    
    # Check if LICENSE.txt exists, create if not
    $licensePath = Join-Path $rootDir "LICENSE.txt"
    if (-not (Test-Path $licensePath)) {
        Write-Info "Creating placeholder LICENSE.txt"
        "This software is provided as-is." | Out-File -FilePath $licensePath -Encoding UTF8
    }
    
    # Build installer
    $nsisScript = Join-Path $scriptDir "installer.nsi"
    
    Write-Info "Running NSIS compiler..."
    & $nsisPath $nsisScript
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "NSIS compilation failed"
        exit 1
    }
    
    # Check if installer was created
    $installerPath = "$buildPath\ZenRunner-Setup-1.0.0.exe"
    if (Test-Path $installerPath) {
        Write-Success "Installer created successfully!"
        Write-Info "Location: $installerPath"
        
        # Get file size
        $fileSize = (Get-Item $installerPath).Length
        $fileSizeMB = [math]::Round($fileSize / 1MB, 2)
        Write-Info "Size: $fileSizeMB MB"
    } else {
        Write-Error "Installer not found at expected location"
        exit 1
    }
}

# Main execution
if ($Help) {
    Show-Usage
    exit 0
}

try {
    Build-Installer
    Write-Success "Build completed successfully!"
} catch {
    Write-Error "Build failed: $_"
    exit 1
}
