#!/bin/bash

echo "Compiling parser test..."
gcc -Wall -Wextra -std=c99 -g -I. -o test_parser \
    test/test_parser.c \
    src/parser.c \
    src/error.c \
    src/environment.c \
    src/io.c \
    src/builtin.c \
    src/external.c

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Running parser tests..."
    ./test_parser
else
    echo "Compilation failed!"
    exit 1
fi