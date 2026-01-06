#!/bin/bash

################################################################################
# ZenRunner Uninstallation Script
# 
# This script removes ZenRunner from your system, including the executable,
# configuration files, and launcher scripts.
################################################################################

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

################################################################################
# Helper Functions
################################################################################

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

# Ask for confirmation
confirm() {
    local prompt="$1"
    local response
    
    while true; do
        read -p "$prompt [y/N]: " response
        case "$response" in
            [Yy]* ) return 0;;
            [Nn]* | "" ) return 1;;
            * ) echo "Please answer yes or no.";;
        esac
    done
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    else
        OS="unknown"
    fi
}

# Remove system-wide installation
remove_system_installation() {
    print_header "Removing System-Wide Installation"
    
    local removed=false
    
    if [ -f "/usr/local/bin/ZenRunner" ]; then
        print_info "Removing /usr/local/bin/ZenRunner"
        sudo rm -f /usr/local/bin/ZenRunner
        removed=true
    fi
    
    if [ -d "/usr/local/share/zenrunner" ]; then
        print_info "Removing /usr/local/share/zenrunner"
        sudo rm -rf /usr/local/share/zenrunner
        removed=true
    fi
    
    if [ "$removed" = true ]; then
        print_success "System-wide installation removed"
    else
        print_info "No system-wide installation found"
    fi
}

# Remove user installation
remove_user_installation() {
    print_header "Removing User Installation"
    
    local removed=false
    
    if [ -f "$HOME/.local/bin/ZenRunner" ]; then
        print_info "Removing ~/.local/bin/ZenRunner"
        rm -f "$HOME/.local/bin/ZenRunner"
        removed=true
    fi
    
    if [ -d "$HOME/.local/share/zenrunner" ]; then
        print_info "Removing ~/.local/share/zenrunner"
        rm -rf "$HOME/.local/share/zenrunner"
        removed=true
    fi
    
    if [ "$removed" = true ]; then
        print_success "User installation removed"
    else
        print_info "No user installation found"
    fi
}

# Remove configuration files
remove_config_files() {
    print_header "Removing Configuration Files"
    
    local config_dirs=()
    
    # Linux config locations
    if [ "$OS" = "linux" ]; then
        config_dirs+=(
            "$HOME/.config/ZenRunner"
            "$HOME/.local/share/ZenRunner"
        )
    fi
    
    # macOS config locations
    if [ "$OS" = "macos" ]; then
        config_dirs+=(
            "$HOME/Library/Application Support/ZenRunner"
            "$HOME/Library/Preferences/com.zenrunner.ZenRunner.plist"
            "$HOME/Library/Caches/ZenRunner"
        )
    fi
    
    local removed=false
    for dir in "${config_dirs[@]}"; do
        if [ -e "$dir" ]; then
            print_info "Removing $dir"
            rm -rf "$dir"
            removed=true
        fi
    done
    
    if [ "$removed" = true ]; then
        print_success "Configuration files removed"
    else
        print_info "No configuration files found"
    fi
}

# Remove desktop integration
remove_desktop_integration() {
    print_header "Removing Desktop Integration"
    
    local removed=false
    
    # Remove desktop entry (Linux)
    if [ -f "$HOME/.local/share/applications/zenrunner.desktop" ]; then
        print_info "Removing desktop entry"
        rm -f "$HOME/.local/share/applications/zenrunner.desktop"
        
        # Update desktop database if possible
        if command -v update-desktop-database >/dev/null 2>&1; then
            update-desktop-database "$HOME/.local/share/applications/" 2>/dev/null || true
        fi
        
        removed=true
    fi
    
    if [ "$removed" = true ]; then
        print_success "Desktop integration removed"
    else
        print_info "No desktop integration found"
    fi
}

# Remove build directory
remove_build_directory() {
    print_header "Removing Build Directory"
    
    if [ -d "build" ]; then
        print_info "Removing build directory"
        rm -rf build
        print_success "Build directory removed"
    else
        print_info "No build directory found"
    fi
}

# Remove launcher scripts
remove_launcher_scripts() {
    print_header "Checking Launcher Scripts"
    
    if [ -f "zenrunner.sh" ]; then
        print_info "Launcher script zenrunner.sh will remain for future use"
        print_info "(You can manually delete it if desired)"
    fi
}

################################################################################
# Main Uninstallation Flow
################################################################################

main() {
    # Print banner
    clear
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║                                            ║"
    echo "║         ZenRunner Uninstallation           ║"
    echo "║                                            ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""
    
    # Detect OS
    detect_os
    print_info "Detected OS: $OS"
    echo ""
    
    # Ask for confirmation
    print_warning "This will remove ZenRunner from your system."
    echo ""
    
    if ! confirm "Do you want to continue?"; then
        print_info "Uninstallation cancelled"
        exit 0
    fi
    
    echo ""
    
    # Remove installations
    remove_system_installation
    remove_user_installation
    
    # Ask about configuration files
    echo ""
    print_warning "Configuration files contain your workspaces and settings."
    if confirm "Do you want to remove configuration files?"; then
        remove_config_files
    else
        print_info "Configuration files kept"
    fi
    
    # Remove desktop integration
    remove_desktop_integration
    
    # Ask about build directory
    echo ""
    if confirm "Do you want to remove the build directory?"; then
        remove_build_directory
    else
        print_info "Build directory kept"
    fi
    
    # Inform about launcher scripts
    remove_launcher_scripts
    
    # Final message
    print_header "Uninstallation Complete"
    echo ""
    print_success "ZenRunner has been uninstalled from your system."
    echo ""
    print_info "Thank you for using ZenRunner!"
    
    # Inform about manual cleanup if needed
    echo ""
    print_info "If you want to completely remove the source code:"
    echo "  cd .. && rm -rf ZenRunner/"
    echo ""
}

# Run main function
main "$@"
