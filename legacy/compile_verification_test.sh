#!/bin/bash

# Compile and run build verification test
echo "Compiling build verification test..."
gcc -o test_build_verification test_build_verification.c

if [ $? -eq 0 ]; then
    echo "Running build verification..."
    ./test_build_verification
else
    echo "Failed to compile build verification test"
    exit 1
fi