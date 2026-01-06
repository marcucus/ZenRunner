#!/bin/bash

################################################################################
# ZenRunner Installation Script
# 
# This script automates the installation of ZenRunner on Linux and macOS
# systems. It will check dependencies, build the application, and optionally
# install it system-wide or to the user directory.
################################################################################

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Installation mode: "user" or "system"
INSTALL_MODE="user"
BUILD_TYPE="Release"
SKIP_DEPS_CHECK=false
CLEAN_BUILD=false

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

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
        if [ -f /etc/os-release ]; then
            . /etc/os-release
            DISTRO=$ID
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        DISTRO="macos"
    else
        OS="unknown"
        DISTRO="unknown"
    fi
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required dependencies
check_dependencies() {
    print_header "Checking Dependencies"
    
    local missing_deps=()
    
    # Check CMake
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    else
        CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
        print_success "CMake $CMAKE_VERSION found"
    fi
    
    # Check C++ compiler
    if ! command_exists g++ && ! command_exists clang++; then
        missing_deps+=("g++ or clang++")
    else
        if command_exists g++; then
            GCC_VERSION=$(g++ --version | head -n1)
            print_success "GCC found: $GCC_VERSION"
        elif command_exists clang++; then
            CLANG_VERSION=$(clang++ --version | head -n1)
            print_success "Clang found: $CLANG_VERSION"
        fi
    fi
    
    # Check Qt6
    if ! command_exists qmake6 && ! command_exists qmake && [ ! -d "/opt/homebrew/opt/qt@6" ]; then
        missing_deps+=("qt6")
    else
        print_success "Qt6 found"
    fi
    
    # Check Git
    if ! command_exists git; then
        missing_deps+=("git")
    else
        print_success "Git found"
    fi
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_info "Please install missing dependencies first."
        show_dependency_install_instructions
        exit 1
    fi
    
    print_success "All dependencies satisfied"
}

# Show dependency installation instructions
show_dependency_install_instructions() {
    echo ""
    print_info "Installation instructions for your system:"
    echo ""
    
    case "$DISTRO" in
        ubuntu|debian)
            echo "  sudo apt update"
            echo "  sudo apt install -y build-essential cmake qt6-base-dev qt6-declarative-dev qt6-tools-dev git"
            ;;
        fedora|rhel|centos)
            echo "  sudo dnf install -y cmake gcc-c++ qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qttools-devel git"
            ;;
        arch)
            echo "  sudo pacman -S --needed base-devel cmake qt6-base qt6-declarative qt6-tools git"
            ;;
        macos)
            echo "  # Install Homebrew if not already installed:"
            echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            echo ""
            echo "  # Install dependencies:"
            echo "  brew install cmake qt@6 git"
            ;;
        *)
            echo "  Please refer to INSTALL.md for detailed instructions for your distribution."
            ;;
    esac
    echo ""
}

# Configure Qt paths for macOS
configure_qt_macos() {
    if [ "$OS" = "macos" ]; then
        if [ -d "/opt/homebrew/opt/qt@6" ]; then
            export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
            export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"
            print_info "Qt6 path configured for macOS (Homebrew)"
        elif [ -d "/usr/local/opt/qt@6" ]; then
            export PATH="/usr/local/opt/qt@6/bin:$PATH"
            export CMAKE_PREFIX_PATH="/usr/local/opt/qt@6"
            print_info "Qt6 path configured for macOS (Intel)"
        fi
    fi
}

# Build the application
build_application() {
    print_header "Building ZenRunner"
    
    # Clean build directory if requested
    if [ "$CLEAN_BUILD" = true ] && [ -d "build" ]; then
        print_info "Cleaning previous build..."
        rm -rf build
    fi
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with CMake
    print_info "Configuring with CMake..."
    if [ -n "$CMAKE_PREFIX_PATH" ]; then
        cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
    else
        cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    fi
    
    # Build
    print_info "Compiling (this may take a few minutes)..."
    # Detect number of CPU cores with reasonable fallback
    local num_cores
    if command -v nproc >/dev/null 2>&1; then
        num_cores=$(nproc)
    elif command -v sysctl >/dev/null 2>&1; then
        num_cores=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
    else
        # Fallback to 4 cores (reasonable for modern systems)
        num_cores=4
        print_info "Could not detect CPU cores, using $num_cores parallel jobs"
    fi
    cmake --build . --config $BUILD_TYPE -j$num_cores
    
    cd ..
    
    if [ -f "build/bin/ZenRunner" ] || [ -f "build/bin/Release/ZenRunner" ]; then
        print_success "Build completed successfully"
    else
        print_error "Build failed - executable not found"
        exit 1
    fi
}

# Install the application
install_application() {
    print_header "Installing ZenRunner"
    
    if [ "$INSTALL_MODE" = "system" ]; then
        print_info "Installing system-wide (requires sudo)..."
        sudo cmake --install build/ --prefix /usr/local
        print_success "Installed to /usr/local/bin/ZenRunner"
    else
        print_info "Installing to user directory..."
        cmake --install build/ --prefix ~/.local
        
        # Add to PATH if not already there
        if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
            print_info "Adding ~/.local/bin to PATH"
            
            # Determine shell config file
            if [ -n "$BASH_VERSION" ]; then
                SHELL_CONFIG="$HOME/.bashrc"
            elif [ -n "$ZSH_VERSION" ]; then
                SHELL_CONFIG="$HOME/.zshrc"
            else
                SHELL_CONFIG="$HOME/.profile"
            fi
            
            echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$SHELL_CONFIG"
            print_warning "Please restart your shell or run: source $SHELL_CONFIG"
        fi
        
        print_success "Installed to ~/.local/bin/ZenRunner"
    fi
}

# Create launcher script
create_launcher() {
    print_header "Creating Launcher Script"
    
    cat > zenrunner.sh << 'EOF'
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
EOF
    
    chmod +x zenrunner.sh
    print_success "Created launcher script: zenrunner.sh"
}

# Show usage information
show_usage() {
    cat << EOF
ZenRunner Installation Script

Usage: $0 [OPTIONS]

Options:
    -h, --help              Show this help message
    -m, --mode MODE         Installation mode: 'user' (default) or 'system'
    -d, --debug             Build in Debug mode instead of Release
    -s, --skip-deps         Skip dependency checking
    -c, --clean             Clean build directory before building
    --no-install            Build only, do not install
    
Examples:
    $0                      # Install to user directory (~/.local)
    $0 -m system            # Install system-wide (requires sudo)
    $0 -c                   # Clean build and install
    $0 --no-install         # Build only without installing

EOF
}

################################################################################
# Main Installation Flow
################################################################################

main() {
    # Parse command line arguments
    NO_INSTALL=false
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -m|--mode)
                INSTALL_MODE="$2"
                shift 2
                ;;
            -d|--debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            -s|--skip-deps)
                SKIP_DEPS_CHECK=true
                shift
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            --no-install)
                NO_INSTALL=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    # Validate installation mode
    if [ "$INSTALL_MODE" != "user" ] && [ "$INSTALL_MODE" != "system" ]; then
        print_error "Invalid installation mode: $INSTALL_MODE"
        print_info "Use 'user' or 'system'"
        exit 1
    fi
    
    # Print banner
    clear
    echo ""
    echo "╔════════════════════════════════════════════╗"
    echo "║                                            ║"
    echo "║          ZenRunner Installation            ║"
    echo "║                                            ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""
    
    # Detect OS
    detect_os
    print_info "Detected OS: $OS ($DISTRO)"
    print_info "Build type: $BUILD_TYPE"
    print_info "Install mode: $INSTALL_MODE"
    echo ""
    
    # Check dependencies
    if [ "$SKIP_DEPS_CHECK" = false ]; then
        check_dependencies
    else
        print_warning "Skipping dependency check"
    fi
    
    # Configure Qt paths
    configure_qt_macos
    
    # Build application
    build_application
    
    # Install application
    if [ "$NO_INSTALL" = false ]; then
        install_application
    else
        print_info "Skipping installation (--no-install flag)"
    fi
    
    # Create launcher script
    create_launcher
    
    # Success message
    print_header "Installation Complete!"
    echo ""
    echo "ZenRunner has been successfully installed!"
    echo ""
    echo "To launch ZenRunner:"
    echo ""
    
    if [ "$NO_INSTALL" = false ]; then
        if [ "$INSTALL_MODE" = "system" ]; then
            echo "  ZenRunner"
        else
            echo "  ~/.local/bin/ZenRunner"
        fi
        echo ""
        echo "Or use the launcher script:"
        echo ""
    fi
    
    echo "  ./zenrunner.sh"
    echo ""
    echo "For more information, see:"
    echo "  - INSTALL.md for detailed installation guide"
    echo "  - README.md for usage instructions"
    echo "  - docs/WORKSPACE_GUIDE.md for workspace management"
    echo ""
    print_success "Happy coding! 🚀"
    echo ""
}

# Run main function
main "$@"
