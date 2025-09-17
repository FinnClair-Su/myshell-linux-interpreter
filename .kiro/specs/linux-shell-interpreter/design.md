# Design Document

## Overview

本设计文档描述了Linux命令解释程序（Shell）的架构和实现方案。该Shell采用模块化设计，包含命令解析器、内部命令处理器、外部命令执行器、环境变量管理器等核心组件。

## Architecture

### 系统架构图

```
┌─────────────────────────────────────────────────────────┐
│                    Main Shell Loop                      │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │
│  │   Input     │  │   Command   │  │    Output       │  │
│  │  Handler    │  │   Parser    │  │   Handler       │  │
│  └─────────────┘  └─────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │
│  │  Internal   │  │  External   │  │  Environment    │  │
│  │  Commands   │  │  Commands   │  │   Variables     │  │
│  └─────────────┘  └─────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │
│  │   System    │  │    File     │  │     Error       │  │
│  │    Calls    │  │   System    │  │    Handler      │  │
│  └─────────────┘  └─────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### 核心设计原则

1. **模块化设计**: 每个功能模块独立实现，便于测试和维护
2. **错误处理**: 完善的错误检查和恢复机制
3. **内存管理**: 严格的内存分配和释放管理
4. **系统调用**: 直接使用Linux系统调用实现核心功能

## Components and Interfaces

### 1. 主程序模块 (main.c)

**职责**: 程序入口点，初始化Shell环境，启动主循环

**接口**:
```c
int main(int argc, char *argv[]);
void shell_init(void);
void shell_cleanup(void);
void main_loop(void);
```

### 2. 命令解析器 (parser.c)

**职责**: 解析用户输入，分离命令和参数

**接口**:
```c
typedef struct {
    char *command;
    char **args;
    int argc;
} command_t;

command_t* parse_command(char *input);
void free_command(command_t *cmd);
char** tokenize_input(char *input, int *token_count);
```

### 3. 内部命令处理器 (builtin.c)

**职责**: 实现所有内部命令的功能

**接口**:
```c
typedef int (*builtin_func_t)(char **args);

typedef struct {
    char *name;
    builtin_func_t func;
} builtin_command_t;

int is_builtin(char *command);
int execute_builtin(char *command, char **args);

// 内部命令实现函数
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
int builtin_exit(char **args);
```

### 4. 外部命令执行器 (external.c)

**职责**: 执行外部程序，管理进程创建和等待

**接口**:
```c
int execute_external(char *command, char **args);
char* find_executable(char *command);
int fork_and_exec(char *path, char **args);
```

### 5. 环境变量管理器 (environment.c)

**职责**: 管理环境变量的设置、获取和初始化

**接口**:
```c
void init_environment(void);
char* get_env_var(char *name);
int set_env_var(char *name, char *value);
char* expand_variables(char *input);
char** get_path_dirs(void);
```

### 6. 输入输出处理器 (io.c)

**职责**: 处理用户输入输出，显示提示符

**接口**:
```c
void display_prompt(void);
char* read_input(void);
void print_error(char *message);
void print_output(char *message);
```

### 7. 错误处理器 (error.c)

**职责**: 统一的错误处理和日志记录

**接口**:
```c
typedef enum {
    ERROR_NONE = 0,
    ERROR_COMMAND_NOT_FOUND,
    ERROR_PERMISSION_DENIED,
    ERROR_FILE_NOT_FOUND,
    ERROR_INVALID_ARGUMENT,
    ERROR_SYSTEM_CALL,
    ERROR_MEMORY_ALLOCATION
} error_code_t;

void handle_error(error_code_t code, char *context);
void log_error(char *message);
char* get_error_message(error_code_t code);
```

## Data Models

### 命令结构体
```c
typedef struct {
    char *command;      // 命令名
    char **args;        // 参数数组
    int argc;           // 参数个数
    char *input_file;   // 输入重定向文件
    char *output_file;  // 输出重定向文件
} command_t;
```

### 环境变量结构体
```c
typedef struct env_var {
    char *name;
    char *value;
    struct env_var *next;
} env_var_t;
```

### Shell状态结构体
```c
typedef struct {
    char *current_dir;
    env_var_t *env_vars;
    int last_exit_status;
    int running;
} shell_state_t;
```

## Error Handling

### 错误处理策略

1. **系统调用错误**: 使用errno检查系统调用返回值，提供详细错误信息
2. **内存分配错误**: 检查malloc/calloc返回值，失败时优雅退出
3. **文件操作错误**: 检查文件权限和存在性，提供用户友好的错误信息
4. **命令解析错误**: 验证命令语法，提供使用帮助

### 错误恢复机制

```c
// 错误处理宏
#define HANDLE_SYSCALL_ERROR(call, action) \
    do { \
        if ((call) == -1) { \
            perror(#call); \
            action; \
        } \
    } while(0)

#define HANDLE_MALLOC_ERROR(ptr, action) \
    do { \
        if ((ptr) == NULL) { \
            fprintf(stderr, "Memory allocation failed\n"); \
            action; \
        } \
    } while(0)
```

## Testing Strategy

### 单元测试

1. **命令解析测试**: 测试各种输入格式的解析正确性
2. **内部命令测试**: 测试每个内部命令的功能和边界条件
3. **环境变量测试**: 测试环境变量的设置和获取
4. **错误处理测试**: 测试各种错误情况的处理

### 集成测试

1. **完整命令流程测试**: 从输入到输出的完整流程
2. **外部命令执行测试**: 测试各种外部程序的执行
3. **环境变量继承测试**: 测试子进程的环境变量继承

### 系统测试

1. **替换系统Shell测试**: 在真实Linux环境中替换默认Shell
2. **长时间运行测试**: 测试Shell的稳定性和内存泄漏
3. **并发测试**: 测试多个命令同时执行的情况

## Implementation Details

### 关键系统调用使用

1. **进程管理**:
   - `fork()`: 创建子进程执行外部命令
   - `execvp()`: 执行外部程序
   - `waitpid()`: 等待子进程结束

2. **文件系统操作**:
   - `opendir()`, `readdir()`, `closedir()`: 实现ls命令
   - `getcwd()`: 实现pwd命令
   - `chdir()`: 实现cd命令
   - `open()`, `read()`, `write()`, `close()`: 实现文件操作

3. **环境变量**:
   - `getenv()`: 获取环境变量
   - `setenv()`: 设置环境变量
   - `environ`: 访问环境变量数组

### 内存管理策略

1. **动态内存分配**: 使用malloc/free管理动态内存
2. **字符串处理**: 使用strdup复制字符串，确保内存安全
3. **数组管理**: 动态分配参数数组，根据需要扩展

### 信号处理

```c
void setup_signal_handlers(void) {
    signal(SIGINT, handle_sigint);   // Ctrl+C
    signal(SIGQUIT, handle_sigquit); // Ctrl+\
    signal(SIGTSTP, handle_sigtstp); // Ctrl+Z
}
```

## Build System

### Makefile设计

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g
SRCDIR = src
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = myshell

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

test: $(TARGET)
	./test/run_tests.sh

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
```

## Security Considerations

1. **输入验证**: 验证所有用户输入，防止缓冲区溢出
2. **路径安全**: 验证文件路径，防止目录遍历攻击
3. **权限检查**: 在执行文件操作前检查权限
4. **环境变量安全**: 限制敏感环境变量的修改