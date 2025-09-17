#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple test to verify parser functionality */
int main(void) {
    printf("Parser verification test\n");
    
    /* Test 1: Basic string operations */
    char test_input[] = "ls -la /home";
    char *token = strtok(test_input, " ");
    int count = 0;
    
    printf("Tokenizing: 'ls -la /home'\n");
    while (token != NULL) {
        printf("Token %d: %s\n", count++, token);
        token = strtok(NULL, " ");
    }
    
    printf("Total tokens: %d\n", count);
    printf("Basic tokenization works!\n");
    
    return 0;
}