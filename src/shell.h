#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <utime.h>
#include <sys/time.h>
#include <ctype.h>

/* 最大输入长度 */
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_PATH_SIZE 1024
#define MAX_ALLOCATION_SIZE (1024 * 1024 * 10)  /* 10MB 最大分配限制 */

/* 错误代码枚举 - 增强版本 */
typedef enum {
    ERROR_NONE = 0,
    ERROR_COMMAND_NOT_FOUND,
    ERROR_PERMISSION_DENIED,
    ERROR_FILE_NOT_FOUND,
    ERROR_FILE_EXISTS,
    ERROR_DIRECTORY_NOT_EMPTY,
    ERROR_INVALID_ARGUMENT,
    ERROR_INVALID_PATH,
    ERROR_SYSTEM_CALL,
    ERROR_MEMORY_ALLOCATION,
    ERROR_BUFFER_OVERFLOW,
    ERROR_IO_OPERATION,
    ERROR_PROCESS_CREATION,
    ERROR_SIGNAL_HANDLING,
    ERROR_ENVIRONMENT,
    ERROR_PARSING,
    ERROR_TIMEOUT,
    ERROR_RESOURCE_LIMIT
} error_code_t;

/* 日志级别枚举 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

/* 内存分配跟踪结构体 */
typedef struct memory_block {
    void *ptr;
    size_t size;
    const char *context;
    const char *file;
    int line;
    struct memory_block *next;
} memory_block_t;

/* 内存管理状态结构体 */
typedef struct {
    memory_block_t *allocated_blocks;
    size_t total_allocated;
    size_t peak_allocated;
    int allocation_count;
    int deallocation_count;
    int tracking_enabled;
} memory_state_t;

/* 错误状态结构体 */
typedef struct {
    error_code_t last_error;
    int error_count;
    int log_enabled;
    FILE *log_file;
} error_state_t;

/* 命令结构体 */
typedef struct {
    char *command;      /* 命令名 */
    char **args;        /* 参数数组 */
    int argc;           /* 参数个数 */
    char *input_file;   /* 输入重定向文件 */
    char *output_file;  /* 输出重定向文件 */
} command_t;

/* 环境变量结构体 */
typedef struct env_var {
    char *name;
    char *value;
    struct env_var *next;
} env_var_t;

/* Shell状态结构体 */
typedef struct {
    char *current_dir;
    env_var_t *env_vars;
    int last_exit_status;
    int running;
} shell_state_t;

/* 内部命令函数指针类型 */
typedef int (*builtin_func_t)(char **args);

/* 内部命令结构体 */
typedef struct {
    char *name;
    builtin_func_t func;
} builtin_command_t;

/* 全局Shell状态 */
extern shell_state_t g_shell_state;

/* 函数声明 - main.c */
int main(int argc, char *argv[]);
void shell_init(void);
void shell_cleanup(void);
void main_loop(void);
void handle_sigint(int sig);
void handle_sigquit(int sig);
void setup_signal_handlers(void);

/* 函数声明 - parser.c */
command_t* parse_command(char *input);
void free_command(command_t *cmd);
char** tokenize_input(char *input, int *token_count);

/* 函数声明 - builtin.c */
int is_builtin(char *command);
int execute_builtin(char *command, char **args);
void list_builtin_commands(void);
void show_command_help(char *command);
int builtin_ls(char **args);
int builtin_cat(char **args);
int builtin_cp(char **args);
int builtin_rm(char **args);
int builtin_touch(char **args);
int builtin_date(char **args);
int builtin_pwd(char **args);
int builtin_cd(char **args);
int builtin_echo(char **args);
int builtin_export(char **args);
int builtin_memstat(char **args);
int builtin_exit(char **args);
int builtin_help(char **args);

/* 函数声明 - external.c */
int execute_external(char *command, char **args);
char* find_executable(char *command);
int fork_and_exec(char *path, char **args);

/* 函数声明 - environment.c */
void init_environment(void);
char* get_env_var(char *name);
int set_env_var(char *name, char *value);
char* expand_variables(char *input);
char** get_path_dirs(void);
void free_path_dirs(char **dirs);
void cleanup_environment(void);
void print_all_env_vars(void);
int env_var_exists(char *name);
int unset_env_var(char *name);

/* 函数声明 - io.c */
void display_prompt(void);
char* read_input(void);
void print_error(char *message);
void print_output(char *message);
void print_warning(char *message);
void print_success(char *message);
void print_formatted(const char *format, ...);
void print_safe(const char *str);
int read_char_noecho(void);
int confirm_action(const char *message);
void clear_screen(void);
void set_cursor_position(int row, int col);
int get_terminal_size(int *rows, int *cols);
void print_separator(char ch, int length);

/* 函数声明 - error.c */
void init_error_system(void);
void cleanup_error_system(void);
void handle_error(error_code_t code, const char *context);
void handle_syscall_error(const char *syscall_name, const char *context);
void handle_memory_error(const char *context, size_t size);
void log_error(const char *message);
void log_error_with_level(log_level_t level, const char *message);
void log_debug(const char *message);
void log_info(const char *message);
void log_warning(const char *message);
const char* get_log_level_string(log_level_t level);
char* get_error_message(error_code_t code);
error_code_t get_last_error(void);
void clear_last_error(void);
int get_error_count(void);
void reset_error_count(void);
void set_logging_enabled(int enabled);
int is_logging_enabled(void);
char* safe_strdup(const char *str, const char *context);
void* safe_malloc(size_t size, const char *context);
void* safe_realloc(void *ptr, size_t size, const char *context);

/* 内存管理函数声明 */
void init_memory_tracking(void);
void cleanup_memory_tracking(void);
void* tracked_malloc(size_t size, const char *context, const char *file, int line);
void* tracked_realloc(void *ptr, size_t size, const char *context, const char *file, int line);
char* tracked_strdup(const char *str, const char *context, const char *file, int line);
void tracked_free(void *ptr, const char *file, int line);
void print_memory_stats(void);
void print_memory_leaks(void);
int check_memory_leaks(void);
void set_memory_tracking(int enabled);
int is_memory_tracking_enabled(void);

/* 错误处理宏 - 增强版本 */
#define HANDLE_SYSCALL_ERROR(call, context, action) \
    do { \
        if ((call) == -1) { \
            handle_syscall_error(#call, context); \
            action; \
        } \
    } while(0)

#define HANDLE_MALLOC_ERROR(ptr, context, size, action) \
    do { \
        if ((ptr) == NULL) { \
            handle_memory_error(context, size); \
            action; \
        } \
    } while(0)

#define SAFE_MALLOC(ptr, size, context) \
    do { \
        (ptr) = safe_malloc(size, context); \
        if ((ptr) == NULL) { \
            return -1; \
        } \
    } while(0)

#define SAFE_STRDUP(dest, src, context) \
    do { \
        (dest) = safe_strdup(src, context); \
        if ((dest) == NULL) { \
            return -1; \
        } \
    } while(0)

/* 内存跟踪宏 */
#define TRACKED_MALLOC(size, context) \
    tracked_malloc(size, context, __FILE__, __LINE__)

#define TRACKED_REALLOC(ptr, size, context) \
    tracked_realloc(ptr, size, context, __FILE__, __LINE__)

#define TRACKED_STRDUP(str, context) \
    tracked_strdup(str, context, __FILE__, __LINE__)

#define TRACKED_FREE(ptr) \
    tracked_free(ptr, __FILE__, __LINE__)

#define SAFE_TRACKED_MALLOC(ptr, size, context) \
    do { \
        (ptr) = TRACKED_MALLOC(size, context); \
        if ((ptr) == NULL) { \
            return -1; \
        } \
    } while(0)

#define SAFE_TRACKED_STRDUP(dest, src, context) \
    do { \
        (dest) = TRACKED_STRDUP(src, context); \
        if ((dest) == NULL) { \
            return -1; \
        } \
    } while(0)

#define CHECK_NULL_PARAM(param, context) \
    do { \
        if ((param) == NULL) { \
            handle_error(ERROR_INVALID_ARGUMENT, context); \
            return -1; \
        } \
    } while(0)

#define CHECK_BUFFER_SIZE(size, max_size, context) \
    do { \
        if ((size) >= (max_size)) { \
            handle_error(ERROR_BUFFER_OVERFLOW, context); \
            return -1; \
        } \
    } while(0)

#define SAFE_FREE(ptr) \
    do { \
        if ((ptr) != NULL) { \
            free(ptr); \
            (ptr) = NULL; \
        } \
    } while(0)

#define LOG_FUNCTION_ENTRY(func_name) \
    log_debug("Entering function: " func_name)

#define LOG_FUNCTION_EXIT(func_name) \
    log_debug("Exiting function: " func_name)

#endif /* SHELL_H */
