#!/bin/bash

# MyShell Integration Tests Runner
# This script compiles and runs the integration tests for the shell

echo "=== MyShell Integration Tests Runner ==="
echo

# Check if we're in the right directory
if [ ! -f "src/shell.h" ]; then
    echo "Error: Please run this script from the project root directory"
    exit 1
fi

# Create obj directory if it doesn't exist
mkdir -p obj

# Compile the shell source files
echo "Compiling shell source files..."
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/main.c -o obj/main.o
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/parser.c -o obj/parser.o
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/builtin.c -o obj/builtin.o
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/external.c -o obj/external.o
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/environment.c -o obj/environment.o
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/io.c -o obj/io.o
gcc -Wall -Wextra -std=c99 -pedantic -g -c src/error.c -o obj/error.o

if [ $? -ne 0 ]; then
    echo "Error: Failed to compile shell source files"
    exit 1
fi

echo "Shell source files compiled successfully."

# Compile the integration test
echo "Compiling integration tests..."
gcc -Wall -Wextra -std=c99 -pedantic -g \
    test/test_integration.c \
    obj/parser.o obj/builtin.o obj/external.o obj/environment.o obj/io.o obj/error.o \
    -o test_integration

if [ $? -ne 0 ]; then
    echo "Error: Failed to compile integration tests"
    exit 1
fi

echo "Integration tests compiled successfully."
echo

# Run the integration tests
echo "Running integration tests..."
echo "========================================"
./test_integration

# Capture the exit status
test_result=$?

echo "========================================"

# Clean up the test executable
rm -f test_integration

# Report results
if [ $test_result -eq 0 ]; then
    echo "✓ All integration tests passed!"
    exit 0
else
    echo "✗ Some integration tests failed!"
    exit 1
fi