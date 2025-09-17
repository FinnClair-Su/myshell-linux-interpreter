#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

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

/* 测试pwd命令 */
int test_pwd_command(void) {
    /* 获取当前目录用于比较 */
    char expected_cwd[MAX_PATH_SIZE];
    if (getcwd(expected_cwd, sizeof(expected_cwd)) == NULL) {
        printf("Failed to get current directory for test\n");
        return 0;
    }
    
    /* 重定向stdout到缓冲区以捕获输出 */
    FILE *original_stdout = stdout;
    char output_buffer[MAX_PATH_SIZE + 10];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        printf("Failed to create temporary stdout\n");
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行pwd命令 */
    int result = builtin_pwd(NULL);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 检查返回值 */
    if (result != 0) {
        printf("pwd command returned non-zero exit code: %d\n", result);
        return 0;
    }
    
    /* 检查输出是否包含当前目录 */
    /* 移除输出末尾的换行符 */
    size_t len = strlen(output_buffer);
    if (len > 0 && output_buffer[len-1] == '\n') {
        output_buffer[len-1] = '\0';
    }
    
    /* 比较输出和期望的目录 */
    if (strcmp(output_buffer, expected_cwd) != 0) {
        printf("Expected: %s, Got: %s\n", expected_cwd, output_buffer);
        return 0;
    }
    
    return 1;
}

/* 测试pwd命令在不同目录下的工作 */
int test_pwd_different_directory(void) {
    /* 保存当前目录 */
    char original_cwd[MAX_PATH_SIZE];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        return 0;
    }
    
    /* 切换到根目录 */
    if (chdir("/") != 0) {
        printf("Failed to change to root directory\n");
        return 0;
    }
    
    /* 重定向stdout */
    FILE *original_stdout = stdout;
    char output_buffer[MAX_PATH_SIZE + 10];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        chdir(original_cwd);  /* 恢复目录 */
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行pwd命令 */
    int result = builtin_pwd(NULL);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 恢复原目录 */
    chdir(original_cwd);
    
    /* 检查结果 */
    if (result != 0) {
        return 0;
    }
    
    /* 移除换行符 */
    size_t len = strlen(output_buffer);
    if (len > 0 && output_buffer[len-1] == '\n') {
        output_buffer[len-1] = '\0';
    }
    
    /* 应该输出根目录 */
    return (strcmp(output_buffer, "/") == 0);
}

/* 测试cd命令基本功能 */
int test_cd_command(void) {
    /* 保存当前目录 */
    char original_cwd[MAX_PATH_SIZE];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        return 0;
    }
    
    /* 初始化环境变量系统 */
    init_environment();
    
    /* 切换到根目录 */
    char *args[] = {"/", NULL};
    int result = builtin_cd(args);
    
    if (result != 0) {
        chdir(original_cwd);  /* 恢复目录 */
        return 0;
    }
    
    /* 验证是否真的切换了 */
    char current_cwd[MAX_PATH_SIZE];
    if (getcwd(current_cwd, sizeof(current_cwd)) == NULL) {
        chdir(original_cwd);
        return 0;
    }
    
    /* 恢复原目录 */
    chdir(original_cwd);
    
    /* 检查是否切换到了根目录 */
    return (strcmp(current_cwd, "/") == 0);
}

/* 测试cd命令切换到HOME目录 */
int test_cd_home_directory(void) {
    /* 保存当前目录 */
    char original_cwd[MAX_PATH_SIZE];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        return 0;
    }
    
    /* 初始化环境变量系统 */
    init_environment();
    
    /* 设置HOME环境变量为根目录（肯定存在） */
    if (set_env_var("HOME", "/") != 0) {
        return 0;
    }
    
    /* 不带参数调用cd（应该切换到HOME） */
    int result = builtin_cd(NULL);
    
    if (result != 0) {
        chdir(original_cwd);
        return 0;
    }
    
    /* 验证是否切换到了/tmp */
    char current_cwd[MAX_PATH_SIZE];
    if (getcwd(current_cwd, sizeof(current_cwd)) == NULL) {
        chdir(original_cwd);
        return 0;
    }
    
    /* 恢复原目录 */
    chdir(original_cwd);
    
    /* 检查是否切换到了根目录 */
    return (strcmp(current_cwd, "/") == 0);
}

/* 测试cd命令处理无效目录 */
int test_cd_invalid_directory(void) {
    /* 保存当前目录 */
    char original_cwd[MAX_PATH_SIZE];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        return 0;
    }
    
    /* 初始化环境变量系统 */
    init_environment();
    
    /* 尝试切换到不存在的目录 */
    char *args[] = {"/nonexistent_directory_12345", NULL};
    int result = builtin_cd(args);
    
    /* 应该返回错误 */
    if (result == 0) {
        return 0;
    }
    
    /* 验证当前目录没有改变 */
    char current_cwd[MAX_PATH_SIZE];
    if (getcwd(current_cwd, sizeof(current_cwd)) == NULL) {
        return 0;
    }
    
    /* 当前目录应该保持不变 */
    return (strcmp(current_cwd, original_cwd) == 0);
}

/* 测试ls命令基本功能 */
int test_ls_command(void) {
    /* 重定向stdout到缓冲区以捕获输出 */
    FILE *original_stdout = stdout;
    char output_buffer[4096];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行ls命令（当前目录） */
    int result = builtin_ls(NULL);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 检查返回值 */
    if (result != 0) {
        return 0;
    }
    
    /* 检查输出是否包含一些预期的文件 */
    /* 应该至少包含一些内容 */
    return (strlen(output_buffer) > 0);
}

/* 测试ls命令指定目录 */
int test_ls_specific_directory(void) {
    /* 重定向stdout */
    FILE *original_stdout = stdout;
    char output_buffer[4096];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行ls命令（根目录） */
    char *args[] = {"/", NULL};
    int result = builtin_ls(args);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 检查返回值 */
    if (result != 0) {
        return 0;
    }
    
    /* 根目录应该包含一些标准目录 */
    return (strstr(output_buffer, "usr") != NULL || 
            strstr(output_buffer, "bin") != NULL ||
            strstr(output_buffer, "etc") != NULL);
}

/* 测试ls命令处理无效目录 */
int test_ls_invalid_directory(void) {
    /* 尝试列出不存在的目录 */
    char *args[] = {"/nonexistent_directory_12345", NULL};
    int result = builtin_ls(args);
    
    /* 应该返回错误 */
    return (result != 0);
}

/* 测试echo命令基本功能 */
int test_echo_command(void) {
    /* 重定向stdout */
    FILE *original_stdout = stdout;
    char output_buffer[1024];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行echo命令 */
    char *args[] = {"echo", "hello", "world", NULL};
    int result = builtin_echo(args);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 检查返回值 */
    if (result != 0) {
        return 0;
    }
    
    /* 检查输出 */
    return (strstr(output_buffer, "hello world") != NULL);
}

/* 测试echo命令无参数 */
int test_echo_no_args(void) {
    /* 重定向stdout */
    FILE *original_stdout = stdout;
    char output_buffer[1024];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行echo命令（无参数） */
    char *args[] = {"echo", NULL};
    int result = builtin_echo(args);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 应该成功执行并输出换行 */
    return (result == 0);
}

/* 测试date命令 */
int test_date_command(void) {
    /* 重定向stdout */
    FILE *original_stdout = stdout;
    char output_buffer[1024];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行date命令 */
    int result = builtin_date(NULL);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 检查返回值和输出 */
    return (result == 0 && strlen(output_buffer) > 0);
}

/* 测试touch命令创建文件 */
int test_touch_command(void) {
    /* 创建测试文件名 */
    char test_file[] = "test_touch_file.tmp";
    
    /* 确保文件不存在 */
    unlink(test_file);
    
    /* 执行touch命令 */
    char *args[] = {"touch", test_file, NULL};
    int result = builtin_touch(args);
    
    /* 检查返回值 */
    if (result != 0) {
        return 0;
    }
    
    /* 检查文件是否被创建 */
    struct stat st;
    int file_exists = (stat(test_file, &st) == 0);
    
    /* 清理测试文件 */
    unlink(test_file);
    
    return file_exists;
}

/* 测试touch命令无参数 */
int test_touch_no_args(void) {
    /* 执行touch命令（无参数） */
    int result = builtin_touch(NULL);
    
    /* 应该返回错误 */
    return (result != 0);
}

/* 测试rm命令删除文件 */
int test_rm_command(void) {
    /* 创建测试文件 */
    char test_file[] = "test_rm_file.tmp";
    FILE *fp = fopen(test_file, "w");
    if (fp == NULL) {
        return 0;
    }
    fprintf(fp, "test content");
    fclose(fp);
    
    /* 执行rm命令 */
    char *args[] = {"rm", test_file, NULL};
    int result = builtin_rm(args);
    
    /* 检查返回值 */
    if (result != 0) {
        unlink(test_file);  /* 清理 */
        return 0;
    }
    
    /* 检查文件是否被删除 */
    struct stat st;
    int file_exists = (stat(test_file, &st) == 0);
    
    /* 如果文件仍然存在，清理它 */
    if (file_exists) {
        unlink(test_file);
    }
    
    return !file_exists;
}

/* 测试rm命令删除不存在的文件 */
int test_rm_nonexistent_file(void) {
    /* 尝试删除不存在的文件 */
    char *args[] = {"rm", "nonexistent_file_12345.tmp", NULL};
    int result = builtin_rm(args);
    
    /* 应该返回错误 */
    return (result != 0);
}

/* 测试cat命令读取文件 */
int test_cat_command(void) {
    /* 创建测试文件 */
    char test_file[] = "test_cat_file.tmp";
    char test_content[] = "Hello, World!\nThis is a test file.";
    
    FILE *fp = fopen(test_file, "w");
    if (fp == NULL) {
        return 0;
    }
    fprintf(fp, "%s", test_content);
    fclose(fp);
    
    /* 重定向stdout */
    FILE *original_stdout = stdout;
    char output_buffer[1024];
    FILE *temp_stdout = fmemopen(output_buffer, sizeof(output_buffer), "w");
    if (temp_stdout == NULL) {
        unlink(test_file);
        return 0;
    }
    
    stdout = temp_stdout;
    
    /* 执行cat命令 */
    char *args[] = {"cat", test_file, NULL};
    int result = builtin_cat(args);
    
    /* 恢复stdout */
    fclose(temp_stdout);
    stdout = original_stdout;
    
    /* 清理测试文件 */
    unlink(test_file);
    
    /* 检查返回值和输出 */
    return (result == 0 && strstr(output_buffer, "Hello, World!") != NULL);
}

/* 测试cat命令读取不存在的文件 */
int test_cat_nonexistent_file(void) {
    /* 尝试读取不存在的文件 */
    char *args[] = {"cat", "nonexistent_file_12345.tmp", NULL};
    int result = builtin_cat(args);
    
    /* 应该返回错误 */
    return (result != 0);
}

/* 测试cp命令复制文件 */
int test_cp_command(void) {
    /* 创建源文件 */
    char src_file[] = "test_cp_src.tmp";
    char dst_file[] = "test_cp_dst.tmp";
    char test_content[] = "Test content for copy";
    
    FILE *fp = fopen(src_file, "w");
    if (fp == NULL) {
        return 0;
    }
    fprintf(fp, "%s", test_content);
    fclose(fp);
    
    /* 确保目标文件不存在 */
    unlink(dst_file);
    
    /* 执行cp命令 */
    char *args[] = {"cp", src_file, dst_file, NULL};
    int result = builtin_cp(args);
    
    /* 检查返回值 */
    if (result != 0) {
        unlink(src_file);
        return 0;
    }
    
    /* 检查目标文件是否存在 */
    struct stat st;
    int dst_exists = (stat(dst_file, &st) == 0);
    
    /* 验证文件内容 */
    int content_match = 0;
    if (dst_exists) {
        FILE *dst_fp = fopen(dst_file, "r");
        if (dst_fp != NULL) {
            char buffer[1024];
            if (fgets(buffer, sizeof(buffer), dst_fp) != NULL) {
                content_match = (strstr(buffer, test_content) != NULL);
            }
            fclose(dst_fp);
        }
    }
    
    /* 清理测试文件 */
    unlink(src_file);
    unlink(dst_file);
    
    return (dst_exists && content_match);
}

/* 测试export命令设置环境变量 */
int test_export_command(void) {
    /* 初始化环境变量系统 */
    init_environment();
    
    /* 执行export命令 */
    char *args[] = {"export", "TEST_VAR=test_value", NULL};
    int result = builtin_export(args);
    
    /* 检查返回值 */
    if (result != 0) {
        return 0;
    }
    
    /* 验证环境变量是否被设置 */
    char *value = get_env_var("TEST_VAR");
    return (value != NULL && strcmp(value, "test_value") == 0);
}

/* 测试内部命令识别 */
int test_builtin_recognition(void) {
    /* 测试已知的内部命令 */
    if (!is_builtin("ls")) return 0;
    if (!is_builtin("cd")) return 0;
    if (!is_builtin("pwd")) return 0;
    if (!is_builtin("echo")) return 0;
    if (!is_builtin("cat")) return 0;
    if (!is_builtin("cp")) return 0;
    if (!is_builtin("rm")) return 0;
    if (!is_builtin("touch")) return 0;
    if (!is_builtin("date")) return 0;
    if (!is_builtin("export")) return 0;
    if (!is_builtin("exit")) return 0;
    
    /* 测试非内部命令 */
    if (is_builtin("gcc")) return 0;
    if (is_builtin("make")) return 0;
    if (is_builtin("nonexistent")) return 0;
    
    return 1;
}

/* 测试命令执行接口 */
int test_builtin_execution_interface(void) {
    /* 测试有效命令 */
    char *pwd_args[] = {NULL};
    int result1 = execute_builtin("pwd", pwd_args);
    if (result1 != 0) return 0;
    
    /* 测试无效命令 */
    int result2 = execute_builtin("invalid_command", NULL);
    if (result2 == 0) return 0;  /* 应该返回错误 */
    
    return 1;
}

/* 主测试函数 */
int test_builtin_main(void) {
    printf("=== MyShell Builtin Commands Tests ===\n\n");
    
    /* 初始化全局状态 */
    g_shell_state.running = 1;
    g_shell_state.last_exit_status = 0;
    g_shell_state.env_vars = NULL;
    g_shell_state.current_dir = NULL;
    
    /* 初始化系统 */
    init_memory_tracking();
    init_error_system();
    
    /* 运行测试 */
    TEST(test_pwd_command);
    TEST(test_pwd_different_directory);
    TEST(test_cd_command);
    TEST(test_cd_home_directory);
    TEST(test_cd_invalid_directory);
    TEST(test_ls_command);
    TEST(test_ls_specific_directory);
    TEST(test_ls_invalid_directory);
    TEST(test_echo_command);
    TEST(test_echo_no_args);
    TEST(test_date_command);
    TEST(test_touch_command);
    TEST(test_touch_no_args);
    TEST(test_rm_command);
    TEST(test_rm_nonexistent_file);
    TEST(test_cat_command);
    TEST(test_cat_nonexistent_file);
    TEST(test_cp_command);
    TEST(test_export_command);
    TEST(test_builtin_recognition);
    TEST(test_builtin_execution_interface);
    
    /* 输出测试结果 */
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    /* 清理系统 */
    cleanup_error_system();
    cleanup_memory_tracking();
    
    if (tests_passed == tests_run) {
        printf("All builtin tests PASSED!\n");
        return 0;
    } else {
        printf("Some builtin tests FAILED!\n");
        return 1;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    return test_builtin_main();
}