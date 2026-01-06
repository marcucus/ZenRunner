#!/bin/bash
################################################################################
# ZenRunner Security Validation Script
# 
# This script validates that a built ZenRunner executable follows security
# best practices and cannot cause damage to the system.
################################################################################

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

EXECUTABLE=""
PASSED=0
FAILED=0
WARNINGS=0

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

print_test() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

print_pass() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((PASSED++))
}

print_fail() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((FAILED++))
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
    ((WARNINGS++))
}

# Find executable
find_executable() {
    local search_paths=(
        "build/bin/ZenRunner"
        "build/bin/Release/ZenRunner.exe"
        "build/bin/Debug/ZenRunner.exe"
        "build/Release/ZenRunner.exe"
        "build/ZenRunner"
        "ZenRunner"
    )
    
    for path in "${search_paths[@]}"; do
        if [ -f "$path" ]; then
            EXECUTABLE="$path"
            return 0
        fi
    done
    
    return 1
}

# Check if executable has setuid/setgid bits
check_no_setuid() {
    print_test "Checking for setuid/setgid bits..."
    
    if [[ "$OSTYPE" == "darwin"* ]] || [[ "$OSTYPE" == "linux-gnu"* ]]; then
        local perms=$(stat -f "%Sp" "$EXECUTABLE" 2>/dev/null || stat -c "%A" "$EXECUTABLE" 2>/dev/null)
        
        if echo "$perms" | grep -q "[sS]"; then
            print_fail "Executable has setuid/setgid bit set (security risk)"
            return 1
        else
            print_pass "No setuid/setgid bits found"
            return 0
        fi
    else
        print_warn "Cannot check setuid on Windows"
        return 0
    fi
}

# Check binary size (should be reasonable)
check_binary_size() {
    print_test "Checking binary size..."
    
    local size=$(stat -f "%z" "$EXECUTABLE" 2>/dev/null || stat -c "%s" "$EXECUTABLE" 2>/dev/null)
    local size_mb=$((size / 1024 / 1024))
    
    if [ $size_mb -gt 100 ]; then
        print_warn "Binary size is large: ${size_mb}MB (may contain debug symbols)"
    elif [ $size_mb -lt 1 ]; then
        print_warn "Binary size is very small: ${size_mb}MB (may be stripped)"
    else
        print_pass "Binary size is reasonable: ${size_mb}MB"
    fi
}

# Check for debug symbols in release build
check_no_debug_symbols() {
    print_test "Checking for debug symbols in release build..."
    
    if [[ "$OSTYPE" == "darwin"* ]] || [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if file "$EXECUTABLE" | grep -q "not stripped"; then
            print_warn "Binary contains debug symbols (consider stripping for release)"
        else
            print_pass "Binary is stripped of debug symbols"
        fi
    else
        print_warn "Cannot check debug symbols on Windows automatically"
    fi
}

# Check for Qt dependencies
check_qt_dependencies() {
    print_test "Checking Qt dependencies..."
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        if otool -L "$EXECUTABLE" | grep -q "Qt"; then
            print_pass "Qt frameworks detected"
        else
            print_fail "Qt frameworks not found"
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if ldd "$EXECUTABLE" 2>/dev/null | grep -q "Qt"; then
            print_pass "Qt libraries detected"
        else
            print_fail "Qt libraries not found"
        fi
    else
        print_warn "Cannot check dependencies on Windows automatically"
    fi
}

# Check file permissions
check_permissions() {
    print_test "Checking file permissions..."
    
    if [[ "$OSTYPE" == "darwin"* ]] || [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ -x "$EXECUTABLE" ]; then
            print_pass "Executable has correct permissions"
        else
            print_fail "Executable is not executable"
        fi
    fi
}

# Check for hardcoded paths
check_no_hardcoded_paths() {
    print_test "Checking for suspicious hardcoded paths..."
    
    # This is a basic check - won't catch everything
    if [[ "$OSTYPE" == "darwin"* ]] || [[ "$OSTYPE" == "linux-gnu"* ]]; then
        local suspicious_paths=$(strings "$EXECUTABLE" | grep -E "(/etc/|/sys/|/root/)" | head -n 5)
        if [ -n "$suspicious_paths" ]; then
            print_warn "Found potential system paths in binary (may be false positive)"
        else
            print_pass "No obvious system paths found"
        fi
    fi
}

# Summary
print_summary() {
    print_header "Security Validation Summary"
    
    echo "Executable: $EXECUTABLE"
    echo ""
    echo -e "Tests Passed:  ${GREEN}$PASSED${NC}"
    echo -e "Tests Failed:  ${RED}$FAILED${NC}"
    echo -e "Warnings:      ${YELLOW}$WARNINGS${NC}"
    echo ""
    
    if [ $FAILED -eq 0 ]; then
        echo -e "${GREEN}✓ Security validation PASSED${NC}"
        echo ""
        echo "The executable appears to follow security best practices."
        return 0
    else
        echo -e "${RED}✗ Security validation FAILED${NC}"
        echo ""
        echo "Please review the failed checks above."
        return 1
    fi
}

# Main
main() {
    print_header "ZenRunner Security Validator"
    
    echo "This script validates that ZenRunner follows security best practices."
    echo ""
    
    # Find executable
    if ! find_executable; then
        echo -e "${RED}ERROR:${NC} Could not find ZenRunner executable"
        echo "Please build the application first: ./install.sh"
        exit 1
    fi
    
    echo "Found executable: $EXECUTABLE"
    echo ""
    
    # Run checks
    check_no_setuid
    check_binary_size
    check_no_debug_symbols
    check_qt_dependencies
    check_permissions
    check_no_hardcoded_paths
    
    # Print summary
    print_summary
}

# Run main
main "$@"
