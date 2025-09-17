#!/bin/bash

echo "Compiling parser test with existing object files..."

# Use existing object files, excluding main.o
gcc -Wall -Wextra -std=c99 -g -o test_parser_simple \
    test_parser_simple.c \
    obj/parser.o \
    obj/error.o \
    obj/environment.o \
    obj/io.o \
    obj/builtin.o \
    obj/external.o

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Running parser tests..."
    ./test_parser_simple
    echo "Test execution complete."
else
    echo "Compilation failed!"
    exit 1
fi