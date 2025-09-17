#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple test to verify basic functionality
int main() {
    printf("Testing basic memory operations...\n");
    
    char *test = malloc(100);
    if (test) {
        strcpy(test, "Hello World");
        printf("Allocated and set: %s\n", test);
        free(test);
        printf("Memory freed successfully\n");
    }
    
    return 0;
}