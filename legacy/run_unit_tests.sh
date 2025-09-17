#!/bin/bash

# Unit Test Runner for Linux Shell Interpreter
# This script compiles and runs all unit tests

echo "=== Linux Shell Interpreter Unit Test Runner ==="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name=$1
    local test_file=$2
    local source_files=$3
    
    echo -e "${YELLOW}Running $test_name tests...${NC}"
    
    # Compile the test
    gcc -Wall -Wextra -std=c99 -g -I. -o "test_$test_name" \
        "$test_file" $source_files
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ Failed to compile $test_name tests${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
    
    # Run the test
    "./test_$test_name"
    local test_result=$?
    
    if [ $test_result -eq 0 ]; then
        echo -e "${GREEN}✓ $test_name tests passed${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ $test_name tests failed${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    # Clean up
    rm -f "test_$test_name"
    
    echo
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    return $test_result
}

# Build the main project first
echo "Building main project..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Failed to build main project${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Main project built successfully${NC}"
echo

# Define source files (excluding main.c to avoid main function conflicts)
SOURCE_FILES="obj/parser.o obj/error.o obj/environment.o obj/io.o obj/builtin.o obj/external.o"

# Run all unit tests
echo "Starting unit tests..."
echo

# Test 1: Command Parser Tests
run_test "parser" "test/test_parser.c" "$SOURCE_FILES"

# Test 2: Internal Command Tests  
run_test "builtin" "test/test_builtin.c" "$SOURCE_FILES"

# Test 3: Environment Variable Tests
run_test "environment" "test/test_environment.c" "$SOURCE_FILES"

# Print final results
echo "=== Test Summary ==="
echo "Total test suites: $TOTAL_TESTS"
echo -e "Passed: ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed: ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo
    echo -e "${GREEN}🎉 All unit tests passed!${NC}"
    exit 0
else
    echo
    echo -e "${RED}❌ Some unit tests failed!${NC}"
    exit 1
fi