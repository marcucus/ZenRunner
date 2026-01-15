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
# Get project root directory (two levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
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
    
    # Find macdeployqt and Qt installation
    local macdeployqt_path=""
    local qt_path=""
    
    if command -v macdeployqt &> /dev/null; then
        macdeployqt_path="macdeployqt"
        qt_path="$(dirname $(dirname $(which macdeployqt)))"
    elif [ -d "/opt/homebrew/opt/qt@6/bin" ]; then
        macdeployqt_path="/opt/homebrew/opt/qt@6/bin/macdeployqt"
        qt_path="/opt/homebrew/opt/qt@6"
    elif [ -d "/usr/local/opt/qt@6/bin" ]; then
        macdeployqt_path="/usr/local/opt/qt@6/bin/macdeployqt"
        qt_path="/usr/local/opt/qt@6"
    else
        print_error "macdeployqt not found"
        print_info "Please install Qt 6 or add it to PATH"
        exit 1
    fi
    
    print_info "Running macdeployqt from: $qt_path"
    print_info "QML directory: $PROJECT_ROOT/src/ui"
    
    # Run macdeployqt with proper options
    "$macdeployqt_path" "$bundle_dir" \
        -qmldir="$PROJECT_ROOT/src/ui" \
        -always-overwrite \
        -verbose=1
    
    # Run macdeployqt again to fix any missing dependencies in plugins/frameworks
    print_info "Running second pass to fix plugin dependencies..."
    "$macdeployqt_path" "$bundle_dir" \
        -always-overwrite \
        -verbose=0
    
    print_success "App bundle created: $bundle_dir"
    
    # Fix rpaths to remove absolute Homebrew paths and add proper relative paths
    print_info "Fixing rpaths for executable..."
    local exe="$bundle_dir/Contents/MacOS/$APP_NAME"
    
    # Remove all Homebrew rpaths
    for rpath in $(otool -l "$exe" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | grep -E '(homebrew|local)' || true); do
        print_info "Removing rpath: $rpath"
        install_name_tool -delete_rpath "$rpath" "$exe" 2>/dev/null || true
    done
    
    # Add proper rpath for bundled frameworks if not already present
    if ! otool -l "$exe" | grep -A2 LC_RPATH | grep -q "@executable_path/../Frameworks"; then
        print_info "Adding rpath: @executable_path/../Frameworks"
        install_name_tool -add_rpath "@executable_path/../Frameworks" "$exe" || true
    fi
    
    # Verify Qt frameworks are present
    print_info "Verifying Qt frameworks..."
    local frameworks_dir="$bundle_dir/Contents/Frameworks"
    local required_frameworks=("QtCore" "QtGui" "QtQuick" "QtWidgets" "QtQml")
    local missing_frameworks=0
    
    for framework in "${required_frameworks[@]}"; do
        if [ ! -d "$frameworks_dir/${framework}.framework" ]; then
            print_error "Missing framework: ${framework}.framework"
            missing_frameworks=1
        else
            print_info "✓ Found ${framework}.framework"
        fi
    done
    
    if [ $missing_frameworks -eq 1 ]; then
        print_error "Some Qt frameworks are missing. Please check your Qt installation."
        exit 1
    fi
    
    # Fix framework rpaths and install names
    print_info "Fixing framework rpaths..."
    for framework_dir in "$frameworks_dir"/*.framework; do
        if [ -d "$framework_dir" ]; then
            local framework_name=$(basename "$framework_dir" .framework)
            local framework_exec="$framework_dir/Versions/A/$framework_name"
            
            if [ -f "$framework_exec" ]; then
                # Update the framework's ID
                install_name_tool -id "@rpath/${framework_name}.framework/Versions/A/$framework_name" "$framework_exec" 2>/dev/null || true
                
                # Remove any Homebrew rpaths from frameworks
                for rpath in $(otool -l "$framework_exec" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | grep -E '(homebrew|local)' || true); do
                    install_name_tool -delete_rpath "$rpath" "$framework_exec" 2>/dev/null || true
                done
            fi
        fi
    done
    
    # Fix plugin rpaths
    print_info "Fixing plugin rpaths..."
    if [ -d "$bundle_dir/Contents/PlugIns" ]; then
        find "$bundle_dir/Contents/PlugIns" -name "*.dylib" -type f | while read plugin; do
            # Remove Homebrew rpaths from plugins
            for rpath in $(otool -l "$plugin" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | grep -E '(homebrew|local)' || true); do
                install_name_tool -delete_rpath "$rpath" "$plugin" 2>/dev/null || true
            done
        done
    fi
    
    # Ad-hoc sign the bundle for local testing (required on modern macOS)
    print_info "Ad-hoc signing bundle (required for execution)..."
    
    # First, sign all libraries and frameworks
    print_info "Signing frameworks and plugins..."
    if [ -d "$bundle_dir/Contents/Frameworks" ]; then
        find "$bundle_dir/Contents/Frameworks" -name "*.dylib" -o -name "*.framework" | while read lib; do
            codesign --force --sign - "$lib" 2>/dev/null || true
        done
    fi
    
    if [ -d "$bundle_dir/Contents/PlugIns" ]; then
        find "$bundle_dir/Contents/PlugIns" -name "*.dylib" | while read plugin; do
            codesign --force --sign - "$plugin" 2>/dev/null || true
        done
    fi
    
    # Finally, sign the entire bundle with proper options
    codesign --force --deep --sign - --options runtime "$bundle_dir"
    if [ $? -eq 0 ]; then
        print_success "Bundle signed successfully"
        
        # Verify the signature
        print_info "Verifying signature..."
        codesign --verify --verbose "$bundle_dir"
        if [ $? -eq 0 ]; then
            print_success "Signature verified"
        else
            print_error "Signature verification failed"
        fi
    else
        print_error "Failed to sign bundle"
        exit 1
    fi
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
    
    # Set DMG window properties (optional, requires AppleScript and GUI access)
    if command -v osascript &> /dev/null; then
        print_info "Configuring DMG window (optional)..."
        # Try to configure window, but don't fail if it doesn't work
        osascript <<EOF 2>/dev/null || print_info "AppleScript configuration skipped (may require GUI access)"
tell application "Finder"
    try
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
    on error errMsg
        log "Window configuration failed: " & errMsg
    end try
end tell
EOF
    else
        print_info "osascript not available, skipping window configuration"
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
            local entitlements="$SCRIPT_DIR/entitlements.plist"
            
            print_info "Signing application with Developer ID and entitlements..."
            
            # Sign all frameworks and libraries first
            if [ -d "$bundle_dir/Contents/Frameworks" ]; then
                find "$bundle_dir/Contents/Frameworks" -name "*.dylib" -or -name "*.framework" | while read lib; do
                    codesign --force --sign "Developer ID Application" --options runtime "$lib" 2>/dev/null || true
                done
            fi
            
            if [ -d "$bundle_dir/Contents/PlugIns" ]; then
                find "$bundle_dir/Contents/PlugIns" -name "*.dylib" | while read plugin; do
                    codesign --force --sign "Developer ID Application" --options runtime "$plugin" 2>/dev/null || true
                done
            fi
            
            # Sign the main executable with entitlements
            codesign --force --sign "Developer ID Application" \
                --options runtime \
                --entitlements "$entitlements" \
                "$bundle_dir/Contents/MacOS/$APP_NAME"
            
            # Sign the entire bundle
            codesign --force --sign "Developer ID Application" \
                --options runtime \
                --entitlements "$entitlements" \
                "$bundle_dir"
            
            print_info "Verifying signature..."
            codesign --verify --verbose "$bundle_dir"
            
            if [ $? -eq 0 ]; then
                print_success "Application signed successfully"
                
                # Display signing info
                codesign -dv --verbose=4 "$bundle_dir" 2>&1 | grep -E "(Identifier|Authority|TeamIdentifier)" || true
            else
                print_error "Signature verification failed"
            fi
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
