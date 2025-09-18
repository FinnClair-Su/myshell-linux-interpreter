#include "../src/shell.h"
#include <assert.h>

/**
 * 测试错误处理系统的基本功能
 */
void test_basic_error_handling(void) {
    printf("Testing basic error handling...\n");
    
    /* 初始化错误系统 */
    init_error_system();
    
    /* 测试错误代码设置和获取 */
    handle_error(ERROR_COMMAND_NOT_FOUND, "test_context");
    assert(get_last_error() == ERROR_COMMAND_NOT_FOUND);
    
    /* 测试错误计数 */
    int initial_count = get_error_count();
    handle_error(ERROR_FILE_NOT_FOUND, "another_test");
    assert(get_error_count() == initial_count + 1);
    
    /* 测试错误清除 */
    clear_last_error();
    assert(get_last_error() == ERROR_NONE);
    
    /* 清理 */
    cleanup_error_system();
    
    printf("Basic error handling tests passed.\n");
}

/**
 * 测试安全内存分配函数
 */
void test_safe_memory_functions(void) {
    printf("Testing safe memory functions...\n");
    
    init_error_system();
    
    /* 测试安全malloc */
    void *ptr = safe_malloc(100, "test_allocation");
    assert(ptr != NULL);
    free(ptr);
    
    /* 测试安全strdup */
    char *str = safe_strdup("Hello, World!", "test_string");
    assert(str != NULL);
    assert(strcmp(str, "Hello, World!") == 0);
    free(str);
    
    /* 测试无效参数 */
    char *null_str = safe_strdup(NULL, "null_test");
    assert(null_str == NULL);
    assert(get_last_error() == ERROR_INVALID_ARGUMENT);
    
    cleanup_error_system();
    
    printf("Safe memory function tests passed.\n");
}

/**
 * 测试错误消息获取
 */
void test_error_messages(void) {
    printf("Testing error messages...\n");
    
    /* 测试所有错误代码都有对应的消息 */
    for (int i = ERROR_NONE; i <= ERROR_RESOURCE_LIMIT; i++) {
        char *msg = get_error_message(i);
        assert(msg != NULL);
        assert(strlen(msg) > 0);
    }
    
    /* 测试未知错误代码 */
    char *unknown_msg = get_error_message(999);
    assert(strcmp(unknown_msg, "Unknown error") == 0);
    
    printf("Error message tests passed.\n");
}

/**
 * 测试日志级别
 */
void test_log_levels(void) {
    printf("Testing log levels...\n");
    
    init_error_system();
    
    /* 测试所有日志级别 */
    log_debug("Debug message");
    log_info("Info message");
    log_warning("Warning message");
    log_error("Error message");
    
    /* 测试日志级别字符串 */
    assert(strcmp(get_log_level_string(LOG_LEVEL_DEBUG), "DEBUG") == 0);
    assert(strcmp(get_log_level_string(LOG_LEVEL_INFO), "INFO") == 0);
    assert(strcmp(get_log_level_string(LOG_LEVEL_WARNING), "WARNING") == 0);
    assert(strcmp(get_log_level_string(LOG_LEVEL_ERROR), "ERROR") == 0);
    assert(strcmp(get_log_level_string(LOG_LEVEL_FATAL), "FATAL") == 0);
    
    cleanup_error_system();
    
    printf("Log level tests passed.\n");
}

/**
 * 运行所有错误处理测试
 */
void run_error_tests(void) {
    printf("Running Error Handling System Tests...\n\n");
    
    test_basic_error_handling();
    test_safe_memory_functions();
    test_error_messages();
    test_log_levels();
    
    printf("\nAll error handling tests passed successfully!\n");
}
