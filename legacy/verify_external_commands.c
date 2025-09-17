#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("=== External Command System Verification ===\n\n");
    
    printf("1. Testing PATH environment variable:\n");
    char *path = getenv("PATH");
    if (path) {
        printf("   PATH = %s\n", path);
        printf("   ✓ PATH is available\n");
    } else {
        printf("   ✗ PATH not found\n");
        return 1;
    }
    
    printf("\n2. Testing access to common commands:\n");
    
    // Test ls command
    if (access("/bin/ls", X_OK) == 0) {
        printf("   ✓ /bin/ls is executable\n");
    } else if (access("/usr/bin/ls", X_OK) == 0) {
        printf("   ✓ /usr/bin/ls is executable\n");
    } else {
        printf("   ✗ ls command not found\n");
    }
    
    // Test echo command
    if (access("/bin/echo", X_OK) == 0) {
        printf("   ✓ /bin/echo is executable\n");
    } else if (access("/usr/bin/echo", X_OK) == 0) {
        printf("   ✓ /usr/bin/echo is executable\n");
    } else {
        printf("   ✗ echo command not found\n");
    }
    
    printf("\n3. Testing fork/exec system calls:\n");
    pid_t pid = fork();
    if (pid == -1) {
        printf("   ✗ fork() failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process - execute echo
        char *args[] = {"echo", "✓ fork/exec working correctly", NULL};
        execvp("echo", args);
        printf("   ✗ execvp failed\n");
        exit(1);
    } else {
        // Parent process - wait for child
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // The echo output already printed the success message
        } else {
            printf("   ✗ Child process failed\n");
        }
    }
    
    printf("\n=== Verification Complete ===\n");
    printf("External command system is ready for use!\n");
    
    return 0;
}