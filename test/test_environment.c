#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/* 包含Shell头文件进行测试 */
#include "../src/shell.h"

/* 定义全局Shell状态用于测试 */
shell_state_t g_shell_state;

/* 测试统计 */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

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

/* 测试基本环境变量操作 */
void test_env_var_basic(void) {
    TEST_START("basic environment variable operations");
    
    /* 初始化环境 */
    init_environment();
    
    /* 测试设置和获取环境变量 */
    ASSERT_INT_EQUAL(set_env_var("TEST_VAR", "test_value"), 0, "Setting environment variable should succeed");
    char *value = get_env_var("TEST_VAR");
    ASSERT_NOT_NULL(value, "Environment variable should exist");
    ASSERT_STR_EQUAL(value, "test_value", "Environment variable value should match");
    
    /* 测试更新环境变量 */
    ASSERT_INT_EQUAL(set_env_var("TEST_VAR", "new_value"), 0, "Updating environment variable should succeed");
    value = get_env_var("TEST_VAR");
    ASSERT_NOT_NULL(value, "Updated environment variable should exist");
    ASSERT_STR_EQUAL(value, "new_value", "Updated environment variable value should match");
    
    /* 测试环境变量存在性检查 */
    ASSERT_TRUE(env_var_exists("TEST_VAR"), "TEST_VAR should exist");
    ASSERT_FALSE(env_var_exists("NONEXISTENT_VAR"), "NONEXISTENT_VAR should not exist");
    
    TEST_PASS();
}

/* 测试变量扩展功能 */
void test_variable_expansion(void) {
    TEST_START("variable expansion");
    
    /* 设置测试变量 */
    set_env_var("HOME", "/home/user");
    set_env_var("USER", "testuser");
    
    /* 测试简单变量扩展 */
    char *result = expand_variables("$HOME/documents");
    ASSERT_NOT_NULL(result, "Variable expansion should not return NULL");
    ASSERT_STR_EQUAL(result, "/home/user/documents", "Simple variable expansion should work");
    free(result);
    
    /* 测试多个变量扩展 */
    result = expand_variables("$USER lives in $HOME");
    ASSERT_NOT_NULL(result, "Multiple variable expansion should not return NULL");
    ASSERT_STR_EQUAL(result, "testuser lives in /home/user", "Multiple variable expansion should work");
    free(result);
    
    /* 测试不存在的变量 */
    result = expand_variables("$NONEXISTENT");
    ASSERT_NOT_NULL(result, "Nonexistent variable expansion should not return NULL");
    /* 不存在的变量可能返回空字符串或原字符串，这取决于实现 */
    free(result);
    
    TEST_PASS();
}

/* 测试PATH目录解析 */
void test_path_dirs(void) {
    TEST_START("PATH directory parsing");
    
    /* 设置测试PATH */
    set_env_var("PATH", "/bin:/usr/bin:/usr/local/bin");
    
    char **dirs = get_path_dirs();
    ASSERT_NOT_NULL(dirs, "PATH directories should not be NULL");
    ASSERT_STR_EQUAL(dirs[0], "/bin", "First PATH directory should be /bin");
    ASSERT_STR_EQUAL(dirs[1], "/usr/bin", "Second PATH directory should be /usr/bin");
    ASSERT_STR_EQUAL(dirs[2], "/usr/local/bin", "Third PATH directory should be /usr/local/bin");
    ASSERT_NULL(dirs[3], "PATH directories should be NULL-terminated");
    
    free_path_dirs(dirs);
    
    TEST_PASS();
}

/* 测试环境变量初始化 */
void test_environment_initialization(void) {
    TEST_START("environment initialization");
    
    /* 重新初始化环境 */
    cleanup_environment();
    init_environment();
    
    /* 检查基本环境变量是否被设置 */
    char *home = get_env_var("HOME");
    ASSERT_NOT_NULL(home, "HOME environment variable should be set");
    
    char *path = get_env_var("PATH");
    ASSERT_NOT_NULL(path, "PATH environment variable should be set");
    
    TEST_PASS();
}

/* 测试环境变量设置边界条件 */
void test_env_var_boundary_conditions(void) {
    TEST_START("environment variable boundary conditions");
    
    /* 测试NULL参数 */
    ASSERT_TRUE(set_env_var(NULL, "value") != 0, "Setting NULL name should fail");
    ASSERT_TRUE(set_env_var("NAME", NULL) != 0, "Setting NULL value should fail");
    
    /* 测试空字符串 */
    ASSERT_INT_EQUAL(set_env_var("EMPTY_VAR", ""), 0, "Setting empty value should succeed");
    char *empty_value = get_env_var("EMPTY_VAR");
    ASSERT_NOT_NULL(empty_value, "Empty environment variable should exist");
    ASSERT_STR_EQUAL(empty_value, "", "Empty environment variable should have empty value");
    
    /* 测试获取不存在的变量 */
    char *nonexistent = get_env_var("DEFINITELY_NONEXISTENT_VAR_12345");
    ASSERT_NULL(nonexistent, "Nonexistent variable should return NULL");
    
    TEST_PASS();
}

/* 测试环境变量覆盖 */
void test_env_var_override(void) {
    TEST_START("environment variable override");
    
    /* 设置初始值 */
    ASSERT_INT_EQUAL(set_env_var("OVERRIDE_TEST", "initial"), 0, "Setting initial value should succeed");
    char *initial = get_env_var("OVERRIDE_TEST");
    ASSERT_STR_EQUAL(initial, "initial", "Initial value should match");
    
    /* 覆盖值 */
    ASSERT_INT_EQUAL(set_env_var("OVERRIDE_TEST", "overridden"), 0, "Overriding value should succeed");
    char *overridden = get_env_var("OVERRIDE_TEST");
    ASSERT_STR_EQUAL(overridden, "overridden", "Overridden value should match");
    
    TEST_PASS();
}

/* 测试PATH搜索功能 */
void test_path_search(void) {
    TEST_START("PATH search functionality");
    
    /* 设置测试PATH，包含当前目录 */
    set_env_var("PATH", "/bin:/usr/bin:.");
    
    char **dirs = get_path_dirs();
    ASSERT_NOT_NULL(dirs, "PATH directories should not be NULL");
    
    /* 验证目录数量 */
    int count = 0;
    while (dirs[count] != NULL) {
        count++;
    }
    ASSERT_TRUE(count >= 3, "Should have at least 3 PATH directories");
    
    free_path_dirs(dirs);
    
    TEST_PASS();
}

/* 测试环境变量存在性检查 */
void test_env_var_existence(void) {
    TEST_START("environment variable existence check");
    
    /* 设置测试变量 */
    set_env_var("EXISTENCE_TEST", "exists");
    
    /* 测试存在的变量 */
    ASSERT_TRUE(env_var_exists("EXISTENCE_TEST"), "Set variable should exist");
    
    /* 测试不存在的变量 */
    ASSERT_FALSE(env_var_exists("DOES_NOT_EXIST_12345"), "Unset variable should not exist");
    
    /* 测试NULL参数 */
    ASSERT_FALSE(env_var_exists(NULL), "NULL variable name should not exist");
    
    TEST_PASS();
}

/* 测试环境变量删除 */
void test_env_var_unset(void) {
    TEST_START("environment variable unset");
    
    /* 设置测试变量 */
    set_env_var("UNSET_TEST", "to_be_removed");
    ASSERT_TRUE(env_var_exists("UNSET_TEST"), "Variable should exist before unset");
    
    /* 删除变量 */
    ASSERT_INT_EQUAL(unset_env_var("UNSET_TEST"), 0, "Unsetting variable should succeed");
    ASSERT_FALSE(env_var_exists("UNSET_TEST"), "Variable should not exist after unset");
    
    /* 尝试删除不存在的变量 */
    ASSERT_TRUE(unset_env_var("NONEXISTENT_VAR") != 0, "Unsetting nonexistent variable should fail");
    
    TEST_PASS();
}

/* 测试变量扩展边界条件 */
void test_variable_expansion_boundary(void) {
    TEST_START("variable expansion boundary conditions");
    
    /* 测试NULL输入 */
    char *result = expand_variables(NULL);
    ASSERT_NULL(result, "Expanding NULL should return NULL");
    
    /* 测试空字符串 */
    result = expand_variables("");
    ASSERT_NOT_NULL(result, "Expanding empty string should not return NULL");
    ASSERT_STR_EQUAL(result, "", "Expanding empty string should return empty string");
    free(result);
    
    /* 测试没有变量的字符串 */
    result = expand_variables("no variables here");
    ASSERT_NOT_NULL(result, "Expanding string without variables should not return NULL");
    ASSERT_STR_EQUAL(result, "no variables here", "String without variables should remain unchanged");
    free(result);
    
    TEST_PASS();
}

/* 运行所有环境变量测试 */
void run_environment_tests(void) {
    printf("=== Environment Variable Tests ===\n\n");
    
    /* 初始化测试环境 */
    init_memory_tracking();
    init_error_system();
    
    /* 运行所有测试 */
    test_environment_initialization();
    test_env_var_basic();
    test_env_var_boundary_conditions();
    test_env_var_override();
    test_env_var_existence();
    test_env_var_unset();
    test_variable_expansion();
    test_variable_expansion_boundary();
    test_path_dirs();
    test_path_search();
    
    /* 打印测试结果 */
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All Environment Tests Passed!\n\n");
    } else {
        printf("\n✗ Some Environment Tests Failed!\n\n");
    }
    
    /* 清理测试环境 */
    cleanup_environment();
    cleanup_error_system();
    cleanup_memory_tracking();
}

/* 主函数 */
int main(void) {
    printf("Starting Environment Variable Unit Tests...\n\n");
    
    /* 初始化全局Shell状态 */
    g_shell_state.running = 1;
    g_shell_state.last_exit_status = 0;
    g_shell_state.env_vars = NULL;
    g_shell_state.current_dir = NULL;
    
    run_environment_tests();
    
    return (tests_failed == 0) ? 0 : 1;
}