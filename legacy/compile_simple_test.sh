#!/bin/bash

echo "Compiling simple parser test..."

# Compile without main.c to avoid main function conflicts
gcc -Wall -Wextra -std=c99 -g -I. -o test_parser_simple \
    test_parser_simple.c \
    src/parser.c \
    src/error.c \
    src/environment.c \
    src/io.c \
    src/builtin.c \
    src/external.c

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Running simple parser tests..."
    ./test_parser_simple
else
    echo "Compilation failed!"
    exit 1
fi