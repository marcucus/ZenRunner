#!/bin/bash

# ZenRunner Launcher Script
# This script launches ZenRunner with proper environment configuration

# Detect the script's directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Try to find ZenRunner executable
if [ -f "$SCRIPT_DIR/build/bin/ZenRunner" ]; then
    ZENRUNNER_BIN="$SCRIPT_DIR/build/bin/ZenRunner"
elif [ -f "$SCRIPT_DIR/build/bin/Release/ZenRunner" ]; then
    ZENRUNNER_BIN="$SCRIPT_DIR/build/bin/Release/ZenRunner"
elif command -v ZenRunner >/dev/null 2>&1; then
    ZENRUNNER_BIN="ZenRunner"
elif [ -f "$HOME/.local/bin/ZenRunner" ]; then
    ZENRUNNER_BIN="$HOME/.local/bin/ZenRunner"
elif [ -f "/usr/local/bin/ZenRunner" ]; then
    ZENRUNNER_BIN="/usr/local/bin/ZenRunner"
else
    echo "ERROR: ZenRunner executable not found!"
    echo "Please run ./install.sh first to build and install ZenRunner."
    exit 1
fi

# Set up Qt environment for macOS if needed
if [[ "$OSTYPE" == "darwin"* ]]; then
    if [ -d "/opt/homebrew/opt/qt@6" ]; then
        export DYLD_LIBRARY_PATH="/opt/homebrew/opt/qt@6/lib:$DYLD_LIBRARY_PATH"
    elif [ -d "/usr/local/opt/qt@6" ]; then
        export DYLD_LIBRARY_PATH="/usr/local/opt/qt@6/lib:$DYLD_LIBRARY_PATH"
    fi
fi

# Launch ZenRunner
echo "Starting ZenRunner..."
exec "$ZENRUNNER_BIN" "$@"
