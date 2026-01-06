#!/bin/bash
################################################################################
# ZenRunner macOS DMG Creation Script
# 
# This script creates a .dmg installer for ZenRunner on macOS
################################################################################

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
APP_NAME="ZenRunner"
VERSION="1.0.0"
BUILD_DIR="build"
BUNDLE_NAME="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-${VERSION}.dmg"
VOLUME_NAME="${APP_NAME} ${VERSION}"

# Helper functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
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

# Check if running on macOS
check_platform() {
    if [[ "$OSTYPE" != "darwin"* ]]; then
        print_error "This script must be run on macOS"
        exit 1
    fi
}

# Check if build exists
check_build() {
    print_header "Checking Build"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found: $BUILD_DIR"
        print_info "Please build the application first using ./install.sh"
        exit 1
    fi
    
    # Check for executable
    local exe_paths=(
        "$BUILD_DIR/bin/$APP_NAME"
        "$BUILD_DIR/$APP_NAME"
    )
    
    for exe_path in "${exe_paths[@]}"; do
        if [ -f "$exe_path" ]; then
            EXECUTABLE_PATH="$exe_path"
            print_success "Found executable: $EXECUTABLE_PATH"
            return 0
        fi
    done
    
    print_error "Executable not found in build directory"
    exit 1
}

# Create .app bundle
create_app_bundle() {
    print_header "Creating .app Bundle"
    
    local bundle_dir="$BUILD_DIR/$BUNDLE_NAME"
    
    # Remove existing bundle if present
    if [ -d "$bundle_dir" ]; then
        print_info "Removing existing bundle..."
        rm -rf "$bundle_dir"
    fi
    
    # Create bundle structure
    print_info "Creating bundle structure..."
    mkdir -p "$bundle_dir/Contents/MacOS"
    mkdir -p "$bundle_dir/Contents/Resources"
    mkdir -p "$bundle_dir/Contents/Frameworks"
    
    # Copy executable
    print_info "Copying executable..."
    cp "$EXECUTABLE_PATH" "$bundle_dir/Contents/MacOS/$APP_NAME"
    chmod +x "$bundle_dir/Contents/MacOS/$APP_NAME"
    
    # Copy Info.plist
    print_info "Copying Info.plist..."
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cp "$script_dir/Info.plist" "$bundle_dir/Contents/Info.plist"
    
    # Copy icon if exists
    if [ -f "$script_dir/ZenRunner.icns" ]; then
        print_info "Copying icon..."
        cp "$script_dir/ZenRunner.icns" "$bundle_dir/Contents/Resources/"
    else
        print_info "No icon file found (optional)"
    fi
    
    # Copy Qt frameworks and plugins
    print_info "Copying Qt dependencies..."
    
    # Use macdeployqt if available
    if command -v macdeployqt &> /dev/null; then
        print_info "Running macdeployqt..."
        macdeployqt "$bundle_dir" -verbose=1
    elif [ -d "/opt/homebrew/opt/qt@6/bin" ]; then
        print_info "Running macdeployqt from Homebrew..."
        /opt/homebrew/opt/qt@6/bin/macdeployqt "$bundle_dir" -verbose=1
    elif [ -d "/usr/local/opt/qt@6/bin" ]; then
        print_info "Running macdeployqt from Homebrew (Intel)..."
        /usr/local/opt/qt@6/bin/macdeployqt "$bundle_dir" -verbose=1
    else
        print_error "macdeployqt not found"
        print_info "Please install Qt 6 or add it to PATH"
        exit 1
    fi
    
    print_success "App bundle created: $bundle_dir"
}

# Create DMG
create_dmg() {
    print_header "Creating DMG"
    
    local bundle_dir="$BUILD_DIR/$BUNDLE_NAME"
    local dmg_path="$BUILD_DIR/$DMG_NAME"
    local temp_dmg="$BUILD_DIR/temp.dmg"
    
    # Remove existing DMG if present
    if [ -f "$dmg_path" ]; then
        print_info "Removing existing DMG..."
        rm -f "$dmg_path"
    fi
    
    # Remove temp DMG if present
    [ -f "$temp_dmg" ] && rm -f "$temp_dmg"
    
    # Create temporary DMG directory
    local dmg_staging="$BUILD_DIR/dmg_staging"
    if [ -d "$dmg_staging" ]; then
        rm -rf "$dmg_staging"
    fi
    mkdir -p "$dmg_staging"
    
    # Copy app bundle to staging
    print_info "Preparing DMG contents..."
    cp -R "$bundle_dir" "$dmg_staging/"
    
    # Create Applications symlink
    ln -s /Applications "$dmg_staging/Applications"
    
    # Create DMG
    print_info "Creating disk image..."
    hdiutil create -volname "$VOLUME_NAME" \
        -srcfolder "$dmg_staging" \
        -ov -format UDRW \
        "$temp_dmg"
    
    # Mount DMG
    print_info "Mounting disk image..."
    local device=$(hdiutil attach -readwrite -noverify -noautoopen "$temp_dmg" | \
        egrep '^/dev/' | sed 1q | awk '{print $1}')
    
    sleep 2
    
    # Set DMG window properties (optional, requires AppleScript)
    if command -v osascript &> /dev/null; then
        print_info "Configuring DMG window..."
        osascript <<EOF
tell application "Finder"
    tell disk "$VOLUME_NAME"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {100, 100, 600, 400}
        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 128
        set position of item "$BUNDLE_NAME" of container window to {150, 150}
        set position of item "Applications" of container window to {350, 150}
        update without registering applications
        delay 2
    end tell
end tell
EOF
    fi
    
    # Unmount DMG
    print_info "Unmounting disk image..."
    sync
    hdiutil detach "$device" || true
    
    # Convert to compressed DMG
    print_info "Compressing disk image..."
    hdiutil convert "$temp_dmg" \
        -format UDZO \
        -imagekey zlib-level=9 \
        -o "$dmg_path"
    
    # Clean up
    rm -f "$temp_dmg"
    rm -rf "$dmg_staging"
    
    # Get DMG size
    local dmg_size=$(du -h "$dmg_path" | cut -f1)
    
    print_success "DMG created successfully!"
    print_info "Location: $dmg_path"
    print_info "Size: $dmg_size"
}

# Code signing (optional, requires developer certificate)
sign_bundle() {
    print_header "Code Signing (Optional)"
    
    print_info "Checking for code signing certificate..."
    
    # Check if certificate is available
    if security find-identity -v -p codesigning | grep -q "Developer ID Application"; then
        print_info "Found Developer ID certificate"
        read -p "Do you want to sign the application? (y/N): " -n 1 -r
        echo
        
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            local bundle_dir="$BUILD_DIR/$BUNDLE_NAME"
            
            print_info "Signing application..."
            codesign --force --deep --sign "Developer ID Application" "$bundle_dir"
            
            print_info "Verifying signature..."
            codesign --verify --verbose "$bundle_dir"
            
            print_success "Application signed successfully"
        else
            print_info "Skipping code signing"
        fi
    else
        print_info "No Developer ID certificate found"
        print_info "Skipping code signing (optional for distribution)"
        print_info "To sign: https://developer.apple.com/account/"
    fi
}

# Main execution
main() {
    print_header "ZenRunner macOS DMG Builder"
    
    check_platform
    check_build
    create_app_bundle
    sign_bundle
    create_dmg
    
    print_header "Build Complete!"
    echo ""
    print_success "ZenRunner DMG has been created successfully!"
    echo ""
    print_info "Next steps:"
    echo "  1. Test the DMG: open $BUILD_DIR/$DMG_NAME"
    echo "  2. Mount and drag ZenRunner to Applications"
    echo "  3. Launch from Applications folder"
    echo ""
    print_info "For distribution:"
    echo "  - Code signing recommended (requires Apple Developer account)"
    echo "  - Notarization required for Gatekeeper (macOS 10.15+)"
    echo "  - See: https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution"
    echo ""
}

# Run main function
main "$@"
