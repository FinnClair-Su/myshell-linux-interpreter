#!/bin/bash

# Comprehensive Build Verification Runner
# Runs all build verification tests

set -e

echo "=== Comprehensive Build Verification ==="
echo "Date: $(date)"
echo "System: $(uname -a)"
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test program
run_test_program() {
    local test_name="$1"
    local source_file="$2"
    local executable="$3"
    
    echo -e "${BLUE}Running $test_name...${NC}"
    
    # Compile the test
    if gcc -o "$executable" "$source_file" 2>/dev/null; then
        echo "  Compilation: [PASS]"
        
        # Run the test
        if ./"$executable"; then
            echo -e "  Execution: ${GREEN}[PASS]${NC}"
            ((PASSED_TESTS++))
        else
            echo -e "  Execution: ${RED}[FAIL]${NC}"
            ((FAILED_TESTS++))
        fi
    else
        echo -e "  Compilation: ${RED}[FAIL]${NC}"
        echo -e "  Execution: ${RED}[SKIP]${NC}"
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
    echo
}

# Function to test basic build
test_basic_build() {
    echo -e "${BLUE}Testing Basic Build Process...${NC}"
    
    # Clean first
    if make clean > /dev/null 2>&1; then
        echo "  Clean: [PASS]"
    else
        echo -e "  Clean: ${RED}[FAIL]${NC}"
        ((FAILED_TESTS++))
    fi
    
    # Build
    if make all > /dev/null 2>&1; then
        echo -e "  Build: ${GREEN}[PASS]${NC}"
        ((PASSED_TESTS++))
        
        # Check executable
        if [ -f "myshell" ] && [ -x "myshell" ]; then
            echo -e "  Executable: ${GREEN}[PASS]${NC}"
        else
            echo -e "  Executable: ${RED}[FAIL]${NC}"
            ((FAILED_TESTS++))
        fi
    else
        echo -e "  Build: ${RED}[FAIL]${NC}"
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
    echo
}

# Function to test shell functionality
test_shell_functionality() {
    echo -e "${BLUE}Testing Shell Functionality...${NC}"
    
    # Ensure we have a build
    make clean all > /dev/null 2>&1
    
    # Test basic shell startup and exit
    if echo "exit" | timeout 10 ./myshell > /dev/null 2>&1; then
        echo -e "  Shell startup/exit: ${GREEN}[PASS]${NC}"
        ((PASSED_TESTS++))
    else
        echo -e "  Shell startup/exit: ${RED}[FAIL]${NC}"
        ((FAILED_TESTS++))
    fi
    
    # Test basic command
    if echo -e "pwd\nexit" | timeout 10 ./myshell > /dev/null 2>&1; then
        echo -e "  Basic command: ${GREEN}[PASS]${NC}"
        ((PASSED_TESTS++))
    else
        echo -e "  Basic command: ${RED}[FAIL]${NC}"
        ((FAILED_TESTS++))
    fi
    
    TOTAL_TESTS=$((TOTAL_TESTS + 2))
    echo
}

# Main execution
main() {
    echo "Starting comprehensive build verification..."
    echo
    
    # Test basic build process
    test_basic_build
    
    # Run individual test programs
    run_test_program "Makefile Features Test" "test_makefile_features.c" "test_makefile"
    run_test_program "System Compatibility Test" "test_system_compatibility.c" "test_compatibility"
    run_test_program "Build Verification Test" "test_build_verification.c" "test_build"
    
    # Test shell functionality
    test_shell_functionality
    
    # Final summary
    echo -e "${BLUE}=== Final Summary ===${NC}"
    echo "Total test suites: $TOTAL_TESTS"
    echo "Passed: $PASSED_TESTS"
    echo "Failed: $FAILED_TESTS"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "${GREEN}All build verification tests PASSED!${NC}"
        echo "Build system is ready for deployment."
        exit 0
    else
        echo -e "${RED}Some tests FAILED.${NC}"
        echo "Please review the build system configuration."
        exit 1
    fi
}

# Cleanup function
cleanup() {
    echo "Cleaning up test files..."
    rm -f test_makefile test_compatibility test_build
}

# Set trap for cleanup
trap cleanup EXIT

# Run main function
main "$@"