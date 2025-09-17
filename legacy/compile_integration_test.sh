#!/bin/bash

# Simple integration test compiler
echo "Compiling integration tests..."

# Compile integration test
gcc -Wall -Wextra -std=c99 -pedantic -g \
    test/test_integration.c \
    obj/parser.o obj/builtin.o obj/external.o obj/environment.o obj/io.o obj/error.o \
    -o test_integration

if [ $? -eq 0 ]; then
    echo "Integration test compiled successfully."
    echo "Run with: ./test_integration"
else
    echo "Compilation failed."
    exit 1
fi