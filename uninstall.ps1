# ZenRunner Uninstallation Script for Windows
# PowerShell script to remove ZenRunner from your system

param(
    [switch]$Help,
    [switch]$KeepConfig
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

function Confirm-Action {
    param(
        [string]$Prompt
    )
    
    $response = Read-Host "$Prompt [y/N]"
    return $response -match '^[Yy]'
}

function Show-Usage {
    Write-Host @"
ZenRunner Uninstallation Script for Windows

Usage: .\uninstall.ps1 [OPTIONS]

Options:
    -Help           Show this help message
    -KeepConfig     Keep configuration files (workspaces and settings)
    
Examples:
    .\uninstall.ps1                 # Remove everything, prompt for config
    .\uninstall.ps1 -KeepConfig     # Keep configuration files

"@
}

function Remove-Installation {
    Write-Header "Removing Installation"
    
    $installDir = "$env:LOCALAPPDATA\Programs\ZenRunner"
    $removed = $false
    
    if (Test-Path $installDir) {
        Write-Info "Removing installation directory: $installDir"
        try {
            Remove-Item -Path $installDir -Recurse -Force -ErrorAction Stop
            $removed = $true
        } catch {
            Write-Error "Failed to remove installation directory: $_"
        }
    }
    
    # Check for executable in PATH locations
    $pathLocations = @(
        "$env:ProgramFiles\ZenRunner",
        "$env:ProgramFiles(x86)\ZenRunner"
    )
    
    foreach ($location in $pathLocations) {
        if (Test-Path $location) {
            Write-Info "Removing: $location"
            try {
                Remove-Item -Path $location -Recurse -Force -ErrorAction Stop
                $removed = $true
            } catch {
                Write-Error "Failed to remove $location : $_"
            }
        }
    }
    
    if ($removed) {
        Write-Success "Installation removed"
    } else {
        Write-Info "No installation found"
    }
}

function Remove-FromPath {
    Write-Header "Removing from PATH"
    
    $installDir = "$env:LOCALAPPDATA\Programs\ZenRunner"
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    
    if ($userPath -like "*$installDir*") {
        Write-Info "Removing ZenRunner from user PATH..."
        $newPath = $userPath -replace [regex]::Escape(";$installDir"), ""
        $newPath = $newPath -replace [regex]::Escape("$installDir;"), ""
        $newPath = $newPath -replace [regex]::Escape($installDir), ""
        
        [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
        Write-Success "Removed from PATH"
        Write-Warning "Please restart your terminal for PATH changes to take effect"
    } else {
        Write-Info "ZenRunner not found in user PATH"
    }
}

function Remove-ConfigFiles {
    Write-Header "Removing Configuration Files"
    
    $configLocations = @(
        "$env:APPDATA\ZenRunner",
        "$env:LOCALAPPDATA\ZenRunner"
    )
    
    $removed = $false
    foreach ($location in $configLocations) {
        if (Test-Path $location) {
            Write-Info "Removing: $location"
            try {
                Remove-Item -Path $location -Recurse -Force -ErrorAction Stop
                $removed = $true
            } catch {
                Write-Error "Failed to remove $location : $_"
            }
        }
    }
    
    if ($removed) {
        Write-Success "Configuration files removed"
    } else {
        Write-Info "No configuration files found"
    }
}

function Remove-Shortcuts {
    Write-Header "Removing Shortcuts"
    
    $shortcutLocations = @(
        "$env:USERPROFILE\Desktop\ZenRunner.lnk",
        "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\ZenRunner.lnk"
    )
    
    $removed = $false
    foreach ($shortcut in $shortcutLocations) {
        if (Test-Path $shortcut) {
            Write-Info "Removing shortcut: $shortcut"
            Remove-Item -Path $shortcut -Force -ErrorAction SilentlyContinue
            $removed = $true
        }
    }
    
    if ($removed) {
        Write-Success "Shortcuts removed"
    } else {
        Write-Info "No shortcuts found"
    }
}

function Remove-BuildDirectory {
    Write-Header "Removing Build Directory"
    
    if (Test-Path "build") {
        Write-Info "Removing build directory"
        Remove-Item -Path "build" -Recurse -Force
        Write-Success "Build directory removed"
    } else {
        Write-Info "No build directory found"
    }
}

function Check-LauncherScripts {
    Write-Header "Checking Launcher Scripts"
    
    if (Test-Path "zenrunner.bat") {
        Write-Info "Launcher script zenrunner.bat will remain for future use"
        Write-Info "(You can manually delete it if desired)"
    }
}

################################################################################
# Main Uninstallation Flow
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
    Write-Host "║         ZenRunner Uninstallation           ║" -ForegroundColor Cyan
    Write-Host "║               (Windows)                    ║" -ForegroundColor Cyan
    Write-Host "║                                            ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""
    
    # Ask for confirmation
    Write-Warning "This will remove ZenRunner from your system."
    Write-Host ""
    
    if (-not (Confirm-Action "Do you want to continue?")) {
        Write-Info "Uninstallation cancelled"
        exit 0
    }
    
    Write-Host ""
    
    # Remove installation
    Remove-Installation
    
    # Remove from PATH
    Remove-FromPath
    
    # Remove shortcuts
    Remove-Shortcuts
    
    # Ask about configuration files
    if (-not $KeepConfig) {
        Write-Host ""
        Write-Warning "Configuration files contain your workspaces and settings."
        if (Confirm-Action "Do you want to remove configuration files?") {
            Remove-ConfigFiles
        } else {
            Write-Info "Configuration files kept"
        }
    } else {
        Write-Info "Keeping configuration files (-KeepConfig flag)"
    }
    
    # Ask about build directory
    Write-Host ""
    if (Confirm-Action "Do you want to remove the build directory?") {
        Remove-BuildDirectory
    } else {
        Write-Info "Build directory kept"
    }
    
    # Inform about launcher scripts
    Check-LauncherScripts
    
    # Final message
    Write-Header "Uninstallation Complete"
    Write-Host ""
    Write-Success "ZenRunner has been uninstalled from your system."
    Write-Host ""
    Write-Info "Thank you for using ZenRunner!"
    
    # Inform about manual cleanup if needed
    Write-Host ""
    Write-Info "If you want to completely remove the source code:"
    Write-Host "  cd .."
    Write-Host "  Remove-Item -Path ZenRunner -Recurse -Force"
    Write-Host ""
}

# Run main function
Main
