#!/bin/bash
################################################################################
# ZenRunner DMG Verification Script
# 
# This script verifies that a ZenRunner.app bundle is properly configured
# and can be launched without issues.
################################################################################

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

ERRORS=0
WARNINGS=0

# Helper functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[⚠]${NC} $1"
    WARNINGS=$((WARNINGS + 1))
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
    ERRORS=$((ERRORS + 1))
}

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

# Check if bundle path is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <path/to/ZenRunner.app>"
    echo ""
    echo "Examples:"
    echo "  $0 build/ZenRunner.app"
    echo "  $0 /Applications/ZenRunner.app"
    exit 1
fi

BUNDLE_PATH="$1"
APP_NAME="ZenRunner"
EXECUTABLE="$BUNDLE_PATH/Contents/MacOS/$APP_NAME"

print_header "ZenRunner DMG Verification"
print_info "Bundle path: $BUNDLE_PATH"
echo ""

# Check 1: Bundle exists
print_header "1. Bundle Structure"
if [ ! -d "$BUNDLE_PATH" ]; then
    print_error "Bundle not found: $BUNDLE_PATH"
    exit 1
fi
print_success "Bundle exists"

# Check required directories
for dir in Contents Contents/MacOS Contents/Frameworks Contents/Resources; do
    if [ -d "$BUNDLE_PATH/$dir" ]; then
        print_success "$dir/ exists"
    else
        print_error "$dir/ is missing"
    fi
done

# Check Info.plist
if [ -f "$BUNDLE_PATH/Contents/Info.plist" ]; then
    print_success "Info.plist exists"
    
    # Validate Info.plist
    if plutil -lint "$BUNDLE_PATH/Contents/Info.plist" > /dev/null 2>&1; then
        print_success "Info.plist is valid"
    else
        print_error "Info.plist is invalid"
    fi
else
    print_error "Info.plist is missing"
fi

# Check 2: Executable
print_header "2. Executable"
if [ ! -f "$EXECUTABLE" ]; then
    print_error "Executable not found: $EXECUTABLE"
else
    print_success "Executable exists"
    
    # Check executable permissions
    if [ -x "$EXECUTABLE" ]; then
        print_success "Executable has execute permissions"
    else
        print_error "Executable is not executable (chmod +x needed)"
    fi
    
    # Check file type
    file_type=$(file "$EXECUTABLE")
    if echo "$file_type" | grep -q "Mach-O.*executable"; then
        print_success "Executable is a valid Mach-O binary"
    else
        print_error "Executable is not a valid Mach-O binary"
    fi
fi

# Check 3: Qt Frameworks
print_header "3. Qt Frameworks"
FRAMEWORKS_DIR="$BUNDLE_PATH/Contents/Frameworks"
REQUIRED_FRAMEWORKS=("QtCore" "QtGui" "QtQuick" "QtWidgets" "QtQml")

for framework in "${REQUIRED_FRAMEWORKS[@]}"; do
    if [ -d "$FRAMEWORKS_DIR/${framework}.framework" ]; then
        print_success "${framework}.framework is present"
        
        # Check framework binary
        framework_binary="$FRAMEWORKS_DIR/${framework}.framework/Versions/A/$framework"
        if [ -f "$framework_binary" ]; then
            # Check install name
            install_name=$(otool -D "$framework_binary" | tail -1)
            if echo "$install_name" | grep -q "@rpath"; then
                print_success "${framework}.framework has correct install name"
            else
                print_warning "${framework}.framework install name: $install_name"
            fi
        fi
    else
        print_error "${framework}.framework is missing"
    fi
done

# Check 4: Rpaths
print_header "4. Rpaths"
if [ -f "$EXECUTABLE" ]; then
    rpaths=$(otool -l "$EXECUTABLE" | grep -A2 LC_RPATH | grep path | awk '{print $2}')
    
    if [ -z "$rpaths" ]; then
        print_error "No rpaths found"
    else
        print_success "Rpaths found:"
        while IFS= read -r rpath; do
            if [ -n "$rpath" ]; then
                echo "    $rpath"
                
                if echo "$rpath" | grep -q "@executable_path/../Frameworks"; then
                    print_success "  Correct rpath for bundled frameworks"
                elif echo "$rpath" | grep -qE "(homebrew|local)"; then
                    print_error "  Absolute Homebrew/local path detected (will not work on other machines)"
                fi
            fi
        done <<< "$rpaths"
    fi
else
    print_error "Cannot check rpaths - executable not found"
fi

# Check 5: Dependencies
print_header "5. Dependencies"
if [ -f "$EXECUTABLE" ]; then
    deps=$(otool -L "$EXECUTABLE" | tail -n +2)
    
    has_absolute_paths=0
    while read -r line; do
        lib=$(echo "$line" | awk '{print $1}')
        
        if echo "$lib" | grep -q "@rpath"; then
            # This is good - using rpath
            :
        elif echo "$lib" | grep -q "@executable_path"; then
            # This is good - using relative path
            :
        elif echo "$lib" | grep -q "/System/Library" || echo "$lib" | grep -q "/usr/lib"; then
            # System libraries are ok
            :
        elif echo "$lib" | grep -qE "(homebrew|local)"; then
            print_error "Absolute path dependency: $lib"
            has_absolute_paths=1
        fi
    done <<< "$deps"
    
    if [ $has_absolute_paths -eq 0 ]; then
        print_success "All dependencies use relative paths or system libraries"
    fi
else
    print_error "Cannot check dependencies - executable not found"
fi

# Check 6: Code Signature
print_header "6. Code Signature"
if codesign --verify --verbose "$BUNDLE_PATH" 2>&1 | grep -q "valid on disk"; then
    print_success "Bundle has valid code signature"
    
    # Check signature details
    sig_info=$(codesign -dv --verbose=4 "$BUNDLE_PATH" 2>&1)
    
    if echo "$sig_info" | grep -q "Authority=Apple Development\|Authority=Developer ID"; then
        print_success "Signed with Apple Developer certificate"
    else
        print_info "Signed with ad-hoc signature (ok for local use)"
    fi
else
    print_warning "Bundle signature is invalid or missing"
    print_info "This may cause issues on macOS 10.15+"
fi

# Check 7: Plugins
print_header "7. Qt Plugins"
PLUGINS_DIR="$BUNDLE_PATH/Contents/PlugIns"

if [ -d "$PLUGINS_DIR" ]; then
    plugin_count=$(find "$PLUGINS_DIR" -name "*.dylib" | wc -l)
    if [ "$plugin_count" -gt 0 ]; then
        print_success "Found $plugin_count Qt plugin(s)"
        
        # Check a few important plugins
        for plugin_dir in platforms styles imageformats; do
            if [ -d "$PLUGINS_DIR/$plugin_dir" ]; then
                print_success "  $plugin_dir/ plugin directory exists"
            fi
        done
    else
        print_warning "No plugins found"
    fi
else
    print_warning "PlugIns directory not found"
fi

# Check 8: Resources
print_header "8. Resources"
RESOURCES_DIR="$BUNDLE_PATH/Contents/Resources"

if [ -d "$RESOURCES_DIR" ]; then
    # Check for icon
    if [ -f "$RESOURCES_DIR/ZenRunner.icns" ]; then
        print_success "Application icon is present"
    else
        print_info "Application icon not found (optional)"
    fi
    
    # Check for qt.conf
    if [ -f "$RESOURCES_DIR/qt.conf" ]; then
        print_success "qt.conf is present"
    else
        print_info "qt.conf not found (may be in Resources/)"
    fi
else
    print_warning "Resources directory not found"
fi

# Summary
print_header "Verification Summary"
if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    print_success "All checks passed! The bundle appears to be properly configured."
    exit 0
elif [ $ERRORS -eq 0 ]; then
    print_warning "Bundle has $WARNINGS warning(s) but should work."
    exit 0
else
    print_error "Bundle has $ERRORS error(s) and $WARNINGS warning(s)."
    echo ""
    echo "The application may not launch correctly. Please review the errors above."
    echo "See packaging/macos/DIAGNOSTIC_DMG.md for troubleshooting help."
    exit 1
fi
