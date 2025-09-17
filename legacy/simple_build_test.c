/*
 * Simple Build Test
 * Quick verification that the build system works
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Simple Build Verification Test\n");
    printf("==============================\n\n");
    
    int result = 0;
    
    // Test 1: Clean build
    printf("Test 1: Cleaning build directory...\n");
    if (system("make clean") == 0) {
        printf("  [PASS] Clean successful\n");
    } else {
        printf("  [FAIL] Clean failed\n");
        result = 1;
    }
    
    // Test 2: Build project
    printf("\nTest 2: Building project...\n");
    if (system("make all") == 0) {
        printf("  [PASS] Build successful\n");
    } else {
        printf("  [FAIL] Build failed\n");
        result = 1;
    }
    
    // Test 3: Check executable
    printf("\nTest 3: Checking executable...\n");
    if (access("myshell", F_OK) == 0) {
        printf("  [PASS] Executable exists\n");
        
        if (access("myshell", X_OK) == 0) {
            printf("  [PASS] Executable has execute permissions\n");
        } else {
            printf("  [FAIL] Executable lacks execute permissions\n");
            result = 1;
        }
    } else {
        printf("  [FAIL] Executable not found\n");
        result = 1;
    }
    
    // Test 4: Test shell startup
    printf("\nTest 4: Testing shell startup...\n");
    if (system("echo 'exit' | timeout 5 ./myshell > /dev/null 2>&1") == 0) {
        printf("  [PASS] Shell starts and exits cleanly\n");
    } else {
        printf("  [FAIL] Shell startup/exit failed\n");
        result = 1;
    }
    
    printf("\n");
    if (result == 0) {
        printf("All basic build tests PASSED!\n");
        printf("Build system verification: SUCCESS\n");
    } else {
        printf("Some build tests FAILED!\n");
        printf("Build system verification: FAILURE\n");
    }
    
    return result;
}