#!/bin/bash

echo "Compiling memory tracking test..."

# Create obj directory
mkdir -p obj

# Compile all source files
gcc -Wall -Wextra -std=c99 -pedantic -g \
    src/main.c \
    src/parser.c \
    src/builtin.c \
    src/external.c \
    src/environment.c \
    src/io.c \
    src/error.c \
    -o myshell

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Testing memory tracking..."
    
    # Test basic functionality
    echo "pwd" | ./myshell
    echo "memstat" | ./myshell
    
else
    echo "Compilation failed!"
    exit 1
fi