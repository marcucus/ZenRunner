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
    
    # Check for executable or .app bundle
    local exe_paths=(
        "$BUILD_DIR/bin/$BUNDLE_NAME/Contents/MacOS/$APP_NAME"
        "$BUILD_DIR/bin/$APP_NAME"
        "$BUILD_DIR/$APP_NAME"
    )
    
    # Check if .app bundle already exists
    if [ -d "$BUILD_DIR/bin/$BUNDLE_NAME" ]; then
        APP_BUNDLE_PATH="$BUILD_DIR/bin/$BUNDLE_NAME"
        print_success "Found .app bundle: $APP_BUNDLE_PATH"
        return 0
    fi
    
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
    
    # If the app bundle already exists in build/bin, use it
    if [ -n "$APP_BUNDLE_PATH" ] && [ -d "$APP_BUNDLE_PATH" ]; then
        print_info "Using existing .app bundle from build..."
        # Copy the existing bundle to the expected location
        if [ "$APP_BUNDLE_PATH" != "$bundle_dir" ]; then
            rm -rf "$bundle_dir"
            cp -R "$APP_BUNDLE_PATH" "$bundle_dir"
            print_success "Copied .app bundle to: $bundle_dir"
        else
            print_success "Using .app bundle at: $bundle_dir"
        fi
        
        # Deploy Qt frameworks and plugins using macdeployqt
        print_info "Deploying Qt dependencies to existing bundle..."
        
        # Find macdeployqt
        local macdeployqt_path=""
        local qt_path=""
        
        if command -v macdeployqt &> /dev/null; then
            macdeployqt_path="macdeployqt"
            qt_path="$(dirname "$(dirname "$(which macdeployqt)")")"
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
        if ! "$macdeployqt_path" "$bundle_dir" \
            -qmldir="$PROJECT_ROOT/src/ui" \
            -always-overwrite \
            -verbose=1; then
            print_error "macdeployqt failed on first pass"
            exit 1
        fi
        
        # Run macdeployqt again to fix any missing dependencies in plugins/frameworks
        # This second pass handles transitive dependencies that the first pass might miss,
        # particularly dependencies of Qt plugins (like platform-specific dylibs)
        print_info "Running second pass to fix plugin dependencies..."
        if ! "$macdeployqt_path" "$bundle_dir" \
            -always-overwrite \
            -verbose=0; then
            print_warning "macdeployqt second pass failed (non-critical)"
        fi
        
        # Fix rpaths and sign the bundle
        copy_missing_dependencies "$bundle_dir"
        fix_bundle_rpaths "$bundle_dir"
        sign_bundle "$bundle_dir"
        
        return 0
    fi
    
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
        qt_path="$(dirname "$(dirname "$(which macdeployqt)")")"
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
    if ! "$macdeployqt_path" "$bundle_dir" \
        -qmldir="$PROJECT_ROOT/src/ui" \
        -always-overwrite \
        -verbose=1; then
        print_error "macdeployqt failed on first pass"
        exit 1
    fi
    
    # Run macdeployqt again to fix any missing dependencies in plugins/frameworks
    # This second pass handles transitive dependencies that the first pass might miss,
    # particularly dependencies of Qt plugins (like platform-specific dylibs)
    print_info "Running second pass to fix plugin dependencies..."
    if ! "$macdeployqt_path" "$bundle_dir" \
        -always-overwrite \
        -verbose=0; then
        print_warning "macdeployqt second pass failed (non-critical)"
    fi
    
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
        find "$bundle_dir/Contents/PlugIns" -name "*.dylib" -type f -print0 | while IFS= read -r -d '' plugin; do
            # Remove Homebrew rpaths from plugins
            for rpath in $(otool -l "$plugin" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | grep -E '(homebrew|local)' || true); do
                install_name_tool -delete_rpath "$rpath" "$plugin" 2>/dev/null || true
            done
        done
    fi
    
    # Ad-hoc sign the bundle for local testing (required on modern macOS)
    print_info "Ad-hoc signing bundle (required for execution)..."
    
    # First, sign all frameworks (sign the actual executable inside)
    print_info "Signing frameworks..."
    if [ -d "$bundle_dir/Contents/Frameworks" ]; then
        for framework_dir in "$bundle_dir/Contents/Frameworks"/*.framework; do
            if [ -d "$framework_dir" ]; then
                # Sign the framework itself (codesign handles framework bundles)
                codesign --force --sign - "$framework_dir" 2>/dev/null || true
            fi
        done
        
        # Also sign any standalone dylibs in Frameworks
        find "$bundle_dir/Contents/Frameworks" -maxdepth 1 -name "*.dylib" -type f -print0 | while IFS= read -r -d '' lib; do
            codesign --force --sign - "$lib" 2>/dev/null || true
        done
    fi
    
    # Sign plugins
    print_info "Signing plugins..."
    if [ -d "$bundle_dir/Contents/PlugIns" ]; then
        find "$bundle_dir/Contents/PlugIns" -name "*.dylib" -type f -print0 | while IFS= read -r -d '' plugin; do
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

# Copy missing dependencies that macdeployqt might have missed
copy_missing_dependencies() {
    local bundle_dir="$1"
    
    print_header "Checking Missing Dependencies"
    
    local frameworks_dir="$bundle_dir/Contents/Frameworks"
    
    # List of known missing dependencies and their Homebrew paths
    local missing_libs=()
    
    # Check for missing brotli libraries
    if [ -f "$frameworks_dir/libbrotlidec.1.dylib" ] && [ ! -f "$frameworks_dir/libbrotlicommon.1.dylib" ]; then
        print_info "Missing libbrotlicommon.1.dylib - will copy from Homebrew"
        missing_libs+=("libbrotlicommon.1.dylib")
    fi
    
    # Determine Homebrew prefix
    local brew_prefix=""
    if [ -d "/opt/homebrew" ]; then
        brew_prefix="/opt/homebrew"
    elif [ -d "/usr/local" ]; then
        brew_prefix="/usr/local"
    else
        print_warning "Homebrew not found - skipping missing dependencies check"
        return 0
    fi
    
    # Copy missing libraries
    if [ ${#missing_libs[@]} -gt 0 ]; then
        for lib in "${missing_libs[@]}"; do
            local lib_path=""
            
            # Try to find the library in Homebrew
            lib_path=$(find "$brew_prefix/Cellar" -name "$lib" 2>/dev/null | head -1)
            
            if [ -n "$lib_path" ] && [ -f "$lib_path" ]; then
                print_info "Copying $lib from Homebrew..."
                cp "$lib_path" "$frameworks_dir/"
                
                # Fix the install name
                local lib_name=$(basename "$lib" .dylib)
                install_name_tool -id "@executable_path/../Frameworks/$lib" "$frameworks_dir/$lib" 2>/dev/null || true
                
                print_success "Copied $lib"
            else
                print_warning "Could not find $lib in Homebrew"
            fi
        done
    else
        print_success "No missing dependencies detected"
    fi
}

# Fix bundle rpaths to ensure proper framework loading
fix_bundle_rpaths() {
    local bundle_dir="$1"
    
    print_header "Fixing Bundle Rpaths"
    
    local exe="$bundle_dir/Contents/MacOS/$APP_NAME"
    
    if [ ! -f "$exe" ]; then
        print_error "Executable not found: $exe"
        return 1
    fi
    
    # Remove all Homebrew rpaths
    print_info "Removing absolute Homebrew rpaths..."
    for rpath in $(otool -l "$exe" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | grep -E '(homebrew|local)' || true); do
        print_info "  Removing rpath: $rpath"
        install_name_tool -delete_rpath "$rpath" "$exe" 2>/dev/null || true
    done
    
    # Add proper rpath for bundled frameworks if not already present
    if ! otool -l "$exe" | grep -A2 LC_RPATH | grep -q "@executable_path/../Frameworks"; then
        print_info "Adding rpath: @executable_path/../Frameworks"
        install_name_tool -add_rpath "@executable_path/../Frameworks" "$exe" || true
    fi
    
    # Fix framework install names and rpaths
    print_info "Fixing framework rpaths..."
    local frameworks_dir="$bundle_dir/Contents/Frameworks"
    
    if [ -d "$frameworks_dir" ]; then
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
    fi
    
    # Fix plugin rpaths
    print_info "Fixing plugin rpaths..."
    if [ -d "$bundle_dir/Contents/PlugIns" ]; then
        find "$bundle_dir/Contents/PlugIns" -name "*.dylib" -type f -print0 | while IFS= read -r -d '' plugin; do
            # Remove Homebrew rpaths from plugins
            for rpath in $(otool -l "$plugin" | grep -A2 LC_RPATH | grep path | awk '{print $2}' | grep -E '(homebrew|local)' || true); do
                install_name_tool -delete_rpath "$rpath" "$plugin" 2>/dev/null || true
            done
        done
    fi
    
    print_success "Rpaths fixed"
}

# Code signing (required after rpath changes)
sign_bundle() {
    local bundle_dir="$1"
    
    if [ -z "$bundle_dir" ]; then
        bundle_dir="$BUILD_DIR/$BUNDLE_NAME"
    fi
    
    print_header "Code Signing"
    
    print_info "Checking for code signing certificate..."
    
    # Check if certificate is available
    if security find-identity -v -p codesigning | grep -q "Developer ID Application"; then
        print_info "Found Developer ID certificate"
        read -p "Do you want to sign with Developer ID? (y/N): " -n 1 -r
        echo
        
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            local entitlements="$SCRIPT_DIR/entitlements.plist"
            
            print_info "Signing application with Developer ID and entitlements..."
            
            # Sign all frameworks first
            if [ -d "$bundle_dir/Contents/Frameworks" ]; then
                for framework_dir in "$bundle_dir/Contents/Frameworks"/*.framework; do
                    if [ -d "$framework_dir" ]; then
                        codesign --force --sign "Developer ID Application" \
                            --options runtime "$framework_dir" 2>/dev/null || true
                    fi
                done
                
                # Also sign standalone dylibs
                find "$bundle_dir/Contents/Frameworks" -maxdepth 1 -name "*.dylib" -type f -print0 | while IFS= read -r -d '' lib; do
                    codesign --force --sign "Developer ID Application" \
                        --options runtime "$lib" 2>/dev/null || true
                done
            fi
            
            # Sign plugins
            if [ -d "$bundle_dir/Contents/PlugIns" ]; then
                find "$bundle_dir/Contents/PlugIns" -name "*.dylib" -type f -print0 | while IFS= read -r -d '' plugin; do
                    codesign --force --sign "Developer ID Application" \
                        --options runtime "$plugin" 2>/dev/null || true
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
            
            print_success "Application signed with Developer ID"
            return 0
        fi
    fi
    
    # Ad-hoc signing (required on macOS 10.15+ after rpath changes)
    print_info "Performing ad-hoc code signing (required for execution)..."
    
    # Sign all frameworks
    if [ -d "$bundle_dir/Contents/Frameworks" ]; then
        print_info "Signing frameworks..."
        for framework_dir in "$bundle_dir/Contents/Frameworks"/*.framework; do
            if [ -d "$framework_dir" ]; then
                codesign --force --sign - "$framework_dir" 2>/dev/null || true
            fi
        done
        
        # Also sign standalone dylibs
        find "$bundle_dir/Contents/Frameworks" -maxdepth 1 -name "*.dylib" -type f 2>/dev/null | while read -r lib; do
            codesign --force --sign - "$lib" 2>/dev/null || true
        done
    fi
    
    # Sign plugins
    if [ -d "$bundle_dir/Contents/PlugIns" ]; then
        print_info "Signing plugins..."
        find "$bundle_dir/Contents/PlugIns" -name "*.dylib" -type f 2>/dev/null | while read -r plugin; do
            codesign --force --sign - "$plugin" 2>/dev/null || true
        done
    fi
    
    # Sign QML modules
    if [ -d "$bundle_dir/Contents/Resources/qml" ]; then
        print_info "Signing QML plugins..."
        find "$bundle_dir/Contents/Resources/qml" -name "*.dylib" -type f 2>/dev/null | while read -r qml_plugin; do
            codesign --force --sign - "$qml_plugin" 2>/dev/null || true
        done
    fi
    
    # Sign the main executable
    print_info "Signing main executable..."
    codesign --force --sign - "$bundle_dir/Contents/MacOS/$APP_NAME"
    
    # Sign the entire bundle
    print_info "Signing bundle..."
    codesign --force --sign - "$bundle_dir"
    
    # Verify signature
    if codesign --verify --verbose "$bundle_dir" 2>/dev/null; then
        print_success "Bundle signed successfully (ad-hoc)"
    else
        print_warning "Signature verification returned warnings (this is normal for ad-hoc signing)"
    fi
}

# Main execution
main() {
    print_header "ZenRunner macOS DMG Builder"
    
    check_platform
    check_build
    create_app_bundle
    
    # Verify the bundle before signing/packaging
    print_header "Verifying Bundle"
    local bundle_dir="$BUILD_DIR/$BUNDLE_NAME"
    if [ -f "$SCRIPT_DIR/verify-bundle.sh" ]; then
        print_info "Running bundle verification..."
        if "$SCRIPT_DIR/verify-bundle.sh" "$bundle_dir"; then
            print_success "Bundle verification passed"
        else
            print_warning "Bundle verification found issues - see output above"
            read -p "Continue anyway? (y/N): " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                print_error "Aborted by user"
                exit 1
            fi
        fi
    else
        print_info "Verification script not found, skipping verification"
    fi
    
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
    print_info "Verification:"
    echo "  - Run: $SCRIPT_DIR/verify-bundle.sh $BUILD_DIR/$BUNDLE_NAME"
    echo ""
    print_info "For distribution:"
    echo "  - Code signing recommended (requires Apple Developer account)"
    echo "  - Notarization required for Gatekeeper (macOS 10.15+)"
    echo "  - See: https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution"
    echo ""
}

# Run main function
main "$@"
