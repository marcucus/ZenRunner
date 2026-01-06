#!/bin/bash

################################################################################
# ZenRunner Launcher Script
# 
# This script launches ZenRunner with proper environment configuration.
# It automatically detects where ZenRunner is installed and sets up the
# necessary environment variables.
################################################################################

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Detect the script's directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# Try to find ZenRunner executable in various locations
find_zenrunner() {
    local locations=(
        "$SCRIPT_DIR/build/bin/ZenRunner"
        "$SCRIPT_DIR/build/bin/Release/ZenRunner"
        "$HOME/.local/bin/ZenRunner"
        "/usr/local/bin/ZenRunner"
        "/usr/bin/ZenRunner"
    )
    
    for location in "${locations[@]}"; do
        if [ -f "$location" ] && [ -x "$location" ]; then
            echo "$location"
            return 0
        fi
    done
    
    # Try to find in PATH
    if command -v ZenRunner >/dev/null 2>&1; then
        echo "ZenRunner"
        return 0
    fi
    
    return 1
}

# Set up Qt environment for macOS if needed
setup_macos_environment() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # Check for Homebrew Qt installation
        if [ -d "/opt/homebrew/opt/qt@6" ]; then
            export DYLD_LIBRARY_PATH="/opt/homebrew/opt/qt@6/lib:$DYLD_LIBRARY_PATH"
            export QT_PLUGIN_PATH="/opt/homebrew/opt/qt@6/plugins"
        elif [ -d "/usr/local/opt/qt@6" ]; then
            export DYLD_LIBRARY_PATH="/usr/local/opt/qt@6/lib:$DYLD_LIBRARY_PATH"
            export QT_PLUGIN_PATH="/usr/local/opt/qt@6/plugins"
        fi
    fi
}

# Set up Qt environment for Linux if needed
setup_linux_environment() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Add common Qt library paths
        if [ -d "/usr/lib/x86_64-linux-gnu/qt6" ]; then
            export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu/qt6:$LD_LIBRARY_PATH"
        fi
    fi
}

# Main launcher logic
main() {
    # Print banner
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║                                            ║"
    echo "║              ZenRunner Launcher            ║"
    echo "║                                            ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""
    
    # Find ZenRunner executable
    print_info "Searching for ZenRunner executable..."
    ZENRUNNER_BIN=$(find_zenrunner)
    
    if [ -z "$ZENRUNNER_BIN" ]; then
        print_error "ZenRunner executable not found!"
        echo ""
        echo "Please install ZenRunner first by running:"
        echo "  ./install.sh"
        echo ""
        echo "Or build it manually:"
        echo "  mkdir build && cd build"
        echo "  cmake .. -DCMAKE_BUILD_TYPE=Release"
        echo "  cmake --build . --config Release"
        echo ""
        exit 1
    fi
    
    print_success "Found ZenRunner at: $ZENRUNNER_BIN"
    
    # Set up environment
    setup_macos_environment
    setup_linux_environment
    
    # Launch ZenRunner
    print_info "Starting ZenRunner..."
    echo ""
    
    # Execute ZenRunner with all passed arguments
    exec "$ZENRUNNER_BIN" "$@"
}

# Run main function
main "$@"
