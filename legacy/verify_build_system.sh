#!/bin/bash

# Build System Verification Script
# Tests compilation and build process on Linux systems

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results
TESTS_PASSED=0
TESTS_FAILED=0
TEST_LOG="build_verification.log"

# Initialize log file
echo "Build System Verification - $(date)" > "$TEST_LOG"
echo "========================================" >> "$TEST_LOG"

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}[PASS]${NC} $message"
        echo "[PASS] $message" >> "$TEST_LOG"
        ((TESTS_PASSED++))
    elif [ "$status" = "FAIL" ]; then
        echo -e "${RED}[FAIL]${NC} $message"
        echo "[FAIL] $message" >> "$TEST_LOG"
        ((TESTS_FAILED++))
    elif [ "$status" = "INFO" ]; then
        echo -e "${BLUE}[INFO]${NC} $message"
        echo "[INFO] $message" >> "$TEST_LOG"
    elif [ "$status" = "WARN" ]; then
        echo -e "${YELLOW}[WARN]${NC} $message"
        echo "[WARN] $message" >> "$TEST_LOG"
    fi
}

# Function to run a test
run_test() {
    local test_name=$1
    local test_command=$2
    
    print_status "INFO" "Running test: $test_name"
    
    if eval "$test_command" >> "$TEST_LOG" 2>&1; then
        print_status "PASS" "$test_name"
        return 0
    else
        print_status "FAIL" "$test_name"
        return 1
    fi
}

# Function to check system information
check_system_info() {
    print_status "INFO" "System Information Check"
    echo "System: $(uname -a)" >> "$TEST_LOG"
    echo "Compiler: $(gcc --version | head -n1)" >> "$TEST_LOG"
    echo "Make: $(make --version | head -n1)" >> "$TEST_LOG"
    
    # Check if we're on Linux
    if [[ "$(uname)" == "Linux" ]]; then
        print_status "PASS" "Running on Linux system"
    else
        print_status "WARN" "Not running on Linux ($(uname))"
    fi
    
    # Check GCC availability
    if command -v gcc >/dev/null 2>&1; then
        print_status "PASS" "GCC compiler available"
    else
        print_status "FAIL" "GCC compiler not found"
        return 1
    fi
    
    # Check Make availability
    if command -v make >/dev/null 2>&1; then
        print_status "PASS" "Make build tool available"
    else
        print_status "FAIL" "Make build tool not found"
        return 1
    fi
}

# Function to test basic compilation
test_basic_compilation() {
    print_status "INFO" "Testing basic compilation"
    
    # Clean first
    run_test "Clean build directory" "make clean"
    
    # Test basic build
    run_test "Basic compilation" "make all"
    
    # Check if executable was created
    if [ -f "myshell" ]; then
        print_status "PASS" "Executable myshell created"
    else
        print_status "FAIL" "Executable myshell not found"
        return 1
    fi
    
    # Check if executable is runnable
    if [ -x "myshell" ]; then
        print_status "PASS" "Executable has execute permissions"
    else
        print_status "FAIL" "Executable lacks execute permissions"
        return 1
    fi
}

# Function to test different compiler flags
test_compiler_flags() {
    print_status "INFO" "Testing different compiler configurations"
    
    # Test debug build
    run_test "Debug build" "make debug"
    
    # Test release build  
    run_test "Release build" "make release"
    
    # Test with strict warnings
    run_test "Strict warnings build" "CFLAGS='-Wall -Wextra -Werror -std=c99 -pedantic' make clean all"
}

# Function to test different compilers (if available)
test_different_compilers() {
    print_status "INFO" "Testing different compilers"
    
    # Test with GCC (default)
    run_test "GCC compilation" "CC=gcc make clean all"
    
    # Test with Clang if available
    if command -v clang >/dev/null 2>&1; then
        run_test "Clang compilation" "CC=clang make clean all"
    else
        print_status "WARN" "Clang compiler not available for testing"
    fi
}

# Function to test dependencies and linking
test_dependencies() {
    print_status "INFO" "Testing dependencies and linking"
    
    # Check for required system libraries
    local libs=("libc" "libm")
    for lib in "${libs[@]}"; do
        if ldconfig -p | grep -q "$lib"; then
            print_status "PASS" "Library $lib available"
        else
            print_status "WARN" "Library $lib not found in ldconfig"
        fi
    done
    
    # Test linking
    run_test "Linking test" "make clean all"
    
    # Check for undefined symbols
    if command -v nm >/dev/null 2>&1; then
        if nm myshell | grep -q " U "; then
            print_status "INFO" "External symbols found (normal for system calls)"
            nm myshell | grep " U " >> "$TEST_LOG"
        else
            print_status "PASS" "No undefined symbols"
        fi
    fi
}

# Function to test build system features
test_build_features() {
    print_status "INFO" "Testing build system features"
    
    # Test clean target
    run_test "Clean target" "make clean"
    
    # Test help target
    run_test "Help target" "make help"
    
    # Test object file creation
    run_test "Object file creation" "make clean && make all && ls obj/*.o"
    
    # Test incremental build
    run_test "Incremental build" "make all"  # Should be fast since nothing changed
}

# Function to test cross-platform compatibility
test_cross_platform() {
    print_status "INFO" "Testing cross-platform compatibility"
    
    # Test with different C standards
    run_test "C99 standard" "CFLAGS='-std=c99' make clean all"
    run_test "C11 standard" "CFLAGS='-std=c11' make clean all"
    
    # Test with different optimization levels
    run_test "No optimization" "CFLAGS='-O0' make clean all"
    run_test "Basic optimization" "CFLAGS='-O1' make clean all"
    run_test "Full optimization" "CFLAGS='-O2' make clean all"
}

# Function to verify executable functionality
test_executable_basic() {
    print_status "INFO" "Testing basic executable functionality"
    
    # Make sure we have a fresh build
    make clean all >> "$TEST_LOG" 2>&1
    
    # Test that executable can start and exit
    if echo "exit" | timeout 5 ./myshell >> "$TEST_LOG" 2>&1; then
        print_status "PASS" "Executable starts and exits cleanly"
    else
        print_status "FAIL" "Executable failed to start or exit properly"
    fi
    
    # Test help command if available
    if echo "help" | timeout 5 ./myshell >> "$TEST_LOG" 2>&1; then
        print_status "PASS" "Help command works"
    else
        print_status "WARN" "Help command not available or failed"
    fi
}

# Main execution
main() {
    echo -e "${BLUE}=== Build System Verification ===${NC}"
    echo "Starting comprehensive build verification..."
    echo
    
    # Run all test suites
    check_system_info
    echo
    
    test_basic_compilation
    echo
    
    test_compiler_flags
    echo
    
    test_different_compilers
    echo
    
    test_dependencies
    echo
    
    test_build_features
    echo
    
    test_cross_platform
    echo
    
    test_executable_basic
    echo
    
    # Final summary
    echo -e "${BLUE}=== Test Summary ===${NC}"
    echo "Tests passed: $TESTS_PASSED"
    echo "Tests failed: $TESTS_FAILED"
    echo "Log file: $TEST_LOG"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        print_status "PASS" "All build verification tests completed successfully!"
        echo "Build system is ready for deployment."
        exit 0
    else
        print_status "FAIL" "Some tests failed. Check $TEST_LOG for details."
        exit 1
    fi
}

# Run main function
main "$@"