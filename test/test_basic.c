#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* 包含Shell头文件进行测试 */
#include "../src/shell.h"

/* 定义全局Shell状态用于测试 */
shell_state_t g_shell_state;

/* 简单的测试框架 */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s... ", #name); \
        tests_run++; \
        if (name()) { \
            printf("PASSED\n"); \
            tests_passed++; \
        } else { \
            printf("FAILED\n"); \
        } \
    } while(0)

/* 测试命令解析功能 */
int test_command_parsing(void) {
    char input[] = "ls -la /home";
    int token_count;
    char **tokens = tokenize_input(input, &token_count);
    
    if (tokens == NULL || token_count != 3) {
        return 0;
    }
    
    int result = (strcmp(tokens[0], "ls") == 0 &&
                  strcmp(tokens[1], "-la") == 0 &&
                  strcmp(tokens[2], "/home") == 0);
    
    /* 清理内存 */
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    return result;
}

/* 测试内部命令识别 */
int test_builtin_recognition(void) {
    return (is_builtin("ls") == 1 &&
            is_builtin("pwd") == 1 &&
            is_builtin("cd") == 1 &&
            is_builtin("exit") == 1 &&
            is_builtin("nonexistent") == 0);
}

/* 测试环境变量设置和获取 */
int test_environment_variables(void) {
    /* 初始化全局状态 */
    g_shell_state.env_vars = NULL;
    
    /* 设置环境变量 */
    if (set_env_var("TEST_VAR", "test_value") != 0) {
        return 0;
    }
    
    /* 获取环境变量 */
    char *value = get_env_var("TEST_VAR");
    if (value == NULL || strcmp(value, "test_value") != 0) {
        return 0;
    }
    
    return 1;
}

/* 测试错误处理 */
int test_error_handling(void) {
    char *msg = get_error_message(ERROR_COMMAND_NOT_FOUND);
    return (msg != NULL && strlen(msg) > 0);
}

/* 主测试函数 */
int test_main(void) {
    printf("=== MyShell Basic Tests ===\n\n");
    
    /* 运行测试 */
    TEST(test_command_parsing);
    TEST(test_builtin_recognition);
    TEST(test_environment_variables);
    TEST(test_error_handling);
    
    /* 输出测试结果 */
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
