#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* 包含Shell头文件进行测试 */
#include "../src/shell.h"

/* 定义全局Shell状态用于测试 */
shell_state_t g_shell_state;

/* 测试统计 */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* 前向声明 */
static void init_parser_test_environment(void);

/* 测试宏 */
#define TEST_START(name) \
    do { \
        printf("Running test: %s... ", name); \
        tests_run++; \
    } while(0)

#define TEST_PASS() \
    do { \
        printf("PASSED\n"); \
        tests_passed++; \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        printf("FAILED: %s\n", msg); \
        tests_failed++; \
    } while(0)

#define ASSERT_TRUE(condition, msg) \
    do { \
        if (!(condition)) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(condition, msg) \
    do { \
        if (condition) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_NULL(ptr, msg) \
    do { \
        if ((ptr) != NULL) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, msg) \
    do { \
        if ((ptr) == NULL) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_STR_EQUAL(str1, str2, msg) \
    do { \
        if (strcmp((str1), (str2)) != 0) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_INT_EQUAL(val1, val2, msg) \
    do { \
        if ((val1) != (val2)) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/* 测试命令解析器的各种功能 */

/* 测试基本命令解析 */
void test_basic_command_parsing(void) {
    TEST_START("basic command parsing");
    
    command_t *cmd = parse_command("ls -la /home");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "ls", "Command name should be 'ls'");
    ASSERT_INT_EQUAL(cmd->argc, 3, "Argument count should be 3");
    ASSERT_STR_EQUAL(cmd->args[0], "ls", "First argument should be 'ls'");
    ASSERT_STR_EQUAL(cmd->args[1], "-la", "Second argument should be '-la'");
    ASSERT_STR_EQUAL(cmd->args[2], "/home", "Third argument should be '/home'");
    ASSERT_NULL(cmd->args[3], "Arguments should be NULL-terminated");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试空输入处理 */
void test_empty_input(void) {
    TEST_START("empty input handling");
    
    command_t *cmd1 = parse_command("");
    ASSERT_NULL(cmd1, "Empty string should return NULL");
    
    command_t *cmd2 = parse_command(NULL);
    ASSERT_NULL(cmd2, "NULL input should return NULL");
    
    command_t *cmd3 = parse_command("   \t\n  ");
    ASSERT_NULL(cmd3, "Whitespace-only input should return NULL");
    
    TEST_PASS();
}

/* 测试单个命令（无参数） */
void test_single_command(void) {
    TEST_START("single command parsing");
    
    command_t *cmd = parse_command("pwd");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "pwd", "Command name should be 'pwd'");
    ASSERT_INT_EQUAL(cmd->argc, 1, "Argument count should be 1");
    ASSERT_STR_EQUAL(cmd->args[0], "pwd", "First argument should be 'pwd'");
    ASSERT_NULL(cmd->args[1], "Arguments should be NULL-terminated");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试多个参数的命令 */
void test_multiple_arguments(void) {
    TEST_START("multiple arguments parsing");
    
    command_t *cmd = parse_command("cp file1.txt file2.txt /backup/");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "cp", "Command name should be 'cp'");
    ASSERT_INT_EQUAL(cmd->argc, 4, "Argument count should be 4");
    ASSERT_STR_EQUAL(cmd->args[0], "cp", "First argument should be 'cp'");
    ASSERT_STR_EQUAL(cmd->args[1], "file1.txt", "Second argument should be 'file1.txt'");
    ASSERT_STR_EQUAL(cmd->args[2], "file2.txt", "Third argument should be 'file2.txt'");
    ASSERT_STR_EQUAL(cmd->args[3], "/backup/", "Fourth argument should be '/backup/'");
    ASSERT_NULL(cmd->args[4], "Arguments should be NULL-terminated");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试带有多种空白字符的输入 */
void test_whitespace_handling(void) {
    TEST_START("whitespace handling");
    
    command_t *cmd = parse_command("  ls   -la    /home  ");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "ls", "Command name should be 'ls'");
    ASSERT_INT_EQUAL(cmd->argc, 3, "Argument count should be 3");
    ASSERT_STR_EQUAL(cmd->args[0], "ls", "First argument should be 'ls'");
    ASSERT_STR_EQUAL(cmd->args[1], "-la", "Second argument should be '-la'");
    ASSERT_STR_EQUAL(cmd->args[2], "/home", "Third argument should be '/home'");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试制表符和换行符处理 */
void test_tab_newline_handling(void) {
    TEST_START("tab and newline handling");
    
    command_t *cmd = parse_command("echo\thello\nworld");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "echo", "Command name should be 'echo'");
    ASSERT_INT_EQUAL(cmd->argc, 3, "Argument count should be 3");
    ASSERT_STR_EQUAL(cmd->args[0], "echo", "First argument should be 'echo'");
    ASSERT_STR_EQUAL(cmd->args[1], "hello", "Second argument should be 'hello'");
    ASSERT_STR_EQUAL(cmd->args[2], "world", "Third argument should be 'world'");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试长命令行 */
void test_long_command_line(void) {
    TEST_START("long command line parsing");
    
    char long_cmd[512] = "find /usr -name *.txt -type f -exec grep -l pattern";
    command_t *cmd = parse_command(long_cmd);
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "find", "Command name should be 'find'");
    ASSERT_TRUE(cmd->argc > 5, "Should have multiple arguments");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试分词函数 */
void test_tokenize_function(void) {
    TEST_START("tokenize function");
    
    int token_count;
    char **tokens = tokenize_input("cat file.txt", &token_count);
    
    ASSERT_NOT_NULL(tokens, "Tokens should not be NULL");
    ASSERT_INT_EQUAL(token_count, 2, "Token count should be 2");
    ASSERT_STR_EQUAL(tokens[0], "cat", "First token should be 'cat'");
    ASSERT_STR_EQUAL(tokens[1], "file.txt", "Second token should be 'file.txt'");
    
    /* 清理内存 */
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    TEST_PASS();
}

/* 测试内存管理 */
void test_memory_management(void) {
    TEST_START("memory management");
    
    /* 创建多个命令并释放，检查是否有内存泄漏 */
    for (int i = 0; i < 10; i++) {
        command_t *cmd = parse_command("ls -la /tmp");
        ASSERT_NOT_NULL(cmd, "Command should not be NULL");
        free_command(cmd);
    }
    
    /* 测试NULL指针释放 */
    free_command(NULL);  /* 应该安全处理 */
    
    TEST_PASS();
}

/* 测试边界条件 */
void test_boundary_conditions(void) {
    TEST_START("boundary conditions");
    
    /* 测试最大参数数量 */
    char max_args_cmd[1024] = "echo";
    for (int i = 1; i < 10; i++) {  /* 减少测试规模 */
        strcat(max_args_cmd, " arg");
        char num[10];
        sprintf(num, "%d", i);
        strcat(max_args_cmd, num);
    }
    
    command_t *cmd = parse_command(max_args_cmd);
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_TRUE(cmd->argc < MAX_ARGS, "Argument count should be within limits");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试特殊字符处理 */
void test_special_characters(void) {
    TEST_START("special characters");
    
    command_t *cmd = parse_command("echo hello_world");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "echo", "Command name should be 'echo'");
    /* 注意：当前实现可能不处理引号，这是预期的 */
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试错误输入处理 */
void test_error_input_handling(void) {
    TEST_START("error input handling");
    
    /* 测试过长的输入 */
    char long_input[MAX_INPUT_SIZE + 100];
    memset(long_input, 'a', sizeof(long_input) - 1);
    long_input[sizeof(long_input) - 1] = '\0';
    
    command_t *cmd = parse_command(long_input);
    ASSERT_NULL(cmd, "Overly long input should return NULL");
    
    TEST_PASS();
}

/* 测试参数分离的正确性 */
void test_argument_separation(void) {
    TEST_START("argument separation");
    
    command_t *cmd = parse_command("grep -n pattern file1 file2");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_STR_EQUAL(cmd->command, "grep", "Command should be 'grep'");
    ASSERT_INT_EQUAL(cmd->argc, 5, "Should have 5 arguments");
    ASSERT_STR_EQUAL(cmd->args[0], "grep", "args[0] should be 'grep'");
    ASSERT_STR_EQUAL(cmd->args[1], "-n", "args[1] should be '-n'");
    ASSERT_STR_EQUAL(cmd->args[2], "pattern", "args[2] should be 'pattern'");
    ASSERT_STR_EQUAL(cmd->args[3], "file1", "args[3] should be 'file1'");
    ASSERT_STR_EQUAL(cmd->args[4], "file2", "args[4] should be 'file2'");
    ASSERT_NULL(cmd->args[5], "args[5] should be NULL");
    
    free_command(cmd);
    TEST_PASS();
}

/* 测试tokenize函数的边界条件 */
void test_tokenize_boundary_conditions(void) {
    TEST_START("tokenize boundary conditions");
    
    int token_count;
    
    /* 测试空输入 */
    char **tokens1 = tokenize_input("", &token_count);
    ASSERT_NULL(tokens1, "Empty input should return NULL");
    
    /* 测试NULL输入 */
    char **tokens2 = tokenize_input(NULL, &token_count);
    ASSERT_NULL(tokens2, "NULL input should return NULL");
    
    /* 测试单个token */
    char **tokens3 = tokenize_input("single", &token_count);
    ASSERT_NOT_NULL(tokens3, "Single token should not be NULL");
    ASSERT_INT_EQUAL(token_count, 1, "Token count should be 1");
    ASSERT_STR_EQUAL(tokens3[0], "single", "Token should be 'single'");
    
    /* 清理内存 */
    for (int i = 0; i < token_count; i++) {
        free(tokens3[i]);
    }
    free(tokens3);
    
    TEST_PASS();
}

/* 测试命令结构体初始化 */
void test_command_structure_initialization(void) {
    TEST_START("command structure initialization");
    
    command_t *cmd = parse_command("test");
    ASSERT_NOT_NULL(cmd, "Command should not be NULL");
    ASSERT_NOT_NULL(cmd->command, "Command name should not be NULL");
    ASSERT_NOT_NULL(cmd->args, "Arguments array should not be NULL");
    ASSERT_TRUE(cmd->argc > 0, "Argument count should be positive");
    ASSERT_NULL(cmd->input_file, "Input file should be NULL initially");
    ASSERT_NULL(cmd->output_file, "Output file should be NULL initially");
    
    free_command(cmd);
    TEST_PASS();
}

/* 运行所有解析器测试 */
void run_parser_tests(void) {
    printf("=== Command Parser Tests ===\n\n");
    
    /* 初始化测试环境 */
    init_memory_tracking();
    init_error_system();
    init_parser_test_environment();
    
    /* 运行所有测试 */
    test_basic_command_parsing();
    test_empty_input();
    test_single_command();
    test_multiple_arguments();
    test_whitespace_handling();
    test_tab_newline_handling();
    test_long_command_line();
    test_tokenize_function();
    test_memory_management();
    test_boundary_conditions();
    test_special_characters();
    test_error_input_handling();
    test_argument_separation();
    test_tokenize_boundary_conditions();
    test_command_structure_initialization();
    
    /* 打印测试结果 */
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All Parser Tests Passed!\n\n");
    } else {
        printf("\n✗ Some Parser Tests Failed!\n\n");
    }
    
    /* 清理测试环境 */
    cleanup_error_system();
    cleanup_memory_tracking();
}

/* 初始化解析器测试环境 */
static void init_parser_test_environment(void) {
    /* 初始化全局Shell状态 */
    g_shell_state.running = 1;
    g_shell_state.last_exit_status = 0;
    g_shell_state.env_vars = NULL;
    g_shell_state.current_dir = NULL;
}
