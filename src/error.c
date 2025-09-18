#include "shell.h"

/* 全局错误状态 */
static error_state_t g_error_state = {0};

/* 全局内存管理状态 */
static memory_state_t g_memory_state = {0};

/**
 * 初始化内存跟踪系统
 */
void init_memory_tracking(void) {
    g_memory_state.allocated_blocks = NULL;
    g_memory_state.total_allocated = 0;
    g_memory_state.peak_allocated = 0;
    g_memory_state.allocation_count = 0;
    g_memory_state.deallocation_count = 0;
    g_memory_state.tracking_enabled = 1;
    
    log_info("Memory tracking system initialized");
}

/**
 * 初始化错误处理系统
 */
void init_error_system(void) {
    g_error_state.last_error = ERROR_NONE;
    g_error_state.error_count = 0;
    g_error_state.log_enabled = 1;
    g_error_state.log_file = NULL;
    
    /* 初始化内存跟踪 */
    init_memory_tracking();
    
    /* 尝试打开日志文件 */
    char *home = getenv("HOME");
    if (home) {
        char log_path[MAX_PATH_SIZE];
        snprintf(log_path, sizeof(log_path), "%s/.myshell.log", home);
        g_error_state.log_file = fopen(log_path, "a");
    }
}

/**
 * 清理内存跟踪系统
 */
void cleanup_memory_tracking(void) {
    if (!g_memory_state.tracking_enabled) {
        return;
    }
    
    /* 打印内存统计信息 */
    print_memory_stats();
    
    /* 检查内存泄漏 */
    int leaks = check_memory_leaks();
    if (leaks > 0) {
        log_warning("Memory leaks detected during cleanup");
        print_memory_leaks();
    }
    
    /* 释放所有未释放的内存块 */
    memory_block_t *current = g_memory_state.allocated_blocks;
    while (current) {
        memory_block_t *next = current->next;
        
        /* 记录泄漏信息 */
        char leak_msg[512];
        snprintf(leak_msg, sizeof(leak_msg), 
                "Leaked memory: %zu bytes at %p (%s:%d) - %s",
                current->size, current->ptr, 
                current->file ? current->file : "unknown",
                current->line, 
                current->context ? current->context : "no context");
        log_warning(leak_msg);
        
        /* 释放内存 */
        free(current->ptr);
        free(current);
        current = next;
    }
    
    g_memory_state.allocated_blocks = NULL;
    log_info("Memory tracking system cleaned up");
}

/**
 * 清理错误处理系统
 */
void cleanup_error_system(void) {
    /* 清理内存跟踪 */
    cleanup_memory_tracking();
    
    if (g_error_state.log_file) {
        fclose(g_error_state.log_file);
        g_error_state.log_file = NULL;
    }
}

/**
 * 处理错误 - 增强版本
 */
void handle_error(error_code_t code, const char *context) {
    if (code == ERROR_NONE) {
        return;
    }
    
    g_error_state.last_error = code;
    g_error_state.error_count++;
    
    char *error_msg = get_error_message(code);
    char full_message[MAX_INPUT_SIZE];
    
    /* 构建完整的错误消息 */
    if (context) {
        snprintf(full_message, sizeof(full_message), 
                "Error in %s: %s", context, error_msg);
    } else {
        snprintf(full_message, sizeof(full_message), 
                "Error: %s", error_msg);
    }
    
    /* 如果是系统调用错误，添加errno信息 */
    if (code == ERROR_SYSTEM_CALL && errno != 0) {
        char errno_msg[256];
        snprintf(errno_msg, sizeof(errno_msg), 
                "%s (errno: %d - %s)", full_message, errno, strerror(errno));
        strcpy(full_message, errno_msg);
    }
    
    /* 输出错误到stderr */
    fprintf(stderr, "%s\n", full_message);
    
    /* 记录错误到日志 */
    log_error_with_level(LOG_LEVEL_ERROR, full_message);
}

/**
 * 处理系统调用错误
 */
void handle_syscall_error(const char *syscall_name, const char *context) {
    char error_context[MAX_INPUT_SIZE];
    
    if (context) {
        snprintf(error_context, sizeof(error_context), 
                "%s (syscall: %s)", context, syscall_name);
    } else {
        snprintf(error_context, sizeof(error_context), 
                "syscall: %s", syscall_name);
    }
    
    handle_error(ERROR_SYSTEM_CALL, error_context);
}

/**
 * 处理内存分配错误
 */
void handle_memory_error(const char *context, size_t size) {
    char error_context[MAX_INPUT_SIZE];
    
    snprintf(error_context, sizeof(error_context), 
            "%s (requested size: %zu bytes)", 
            context ? context : "memory allocation", size);
    
    handle_error(ERROR_MEMORY_ALLOCATION, error_context);
}

/**
 * 记录错误到日志 - 基本版本
 */
void log_error(const char *message) {
    log_error_with_level(LOG_LEVEL_ERROR, message);
}

/**
 * 记录错误到日志 - 带级别版本
 */
void log_error_with_level(log_level_t level, const char *message) {
    if (!g_error_state.log_enabled || !message) {
        return;
    }
    
    time_t now;
    struct tm *tm_info;
    char timestamp[64];
    
    /* 获取当前时间 */
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    const char *level_str = get_log_level_string(level);
    
    /* 输出到stderr */
    fprintf(stderr, "[%s] %s: %s\n", timestamp, level_str, message);
    
    /* 输出到日志文件 */
    if (g_error_state.log_file) {
        fprintf(g_error_state.log_file, "[%s] %s: %s\n", 
                timestamp, level_str, message);
        fflush(g_error_state.log_file);
    }
}

/**
 * 记录调试信息
 */
void log_debug(const char *message) {
    log_error_with_level(LOG_LEVEL_DEBUG, message);
}

/**
 * 记录信息
 */
void log_info(const char *message) {
    log_error_with_level(LOG_LEVEL_INFO, message);
}

/**
 * 记录警告
 */
void log_warning(const char *message) {
    log_error_with_level(LOG_LEVEL_WARNING, message);
}

/**
 * 获取日志级别字符串
 */
const char* get_log_level_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_ERROR:   return "ERROR";
        case LOG_LEVEL_FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

/**
 * 获取错误信息字符串 - 增强版本
 */
char* get_error_message(error_code_t code) {
    switch (code) {
        case ERROR_NONE:
            return "No error";
        case ERROR_COMMAND_NOT_FOUND:
            return "Command not found";
        case ERROR_PERMISSION_DENIED:
            return "Permission denied";
        case ERROR_FILE_NOT_FOUND:
            return "File or directory not found";
        case ERROR_FILE_EXISTS:
            return "File already exists";
        case ERROR_DIRECTORY_NOT_EMPTY:
            return "Directory not empty";
        case ERROR_INVALID_ARGUMENT:
            return "Invalid argument";
        case ERROR_INVALID_PATH:
            return "Invalid path";
        case ERROR_SYSTEM_CALL:
            return "System call failed";
        case ERROR_MEMORY_ALLOCATION:
            return "Memory allocation failed";
        case ERROR_BUFFER_OVERFLOW:
            return "Buffer overflow detected";
        case ERROR_IO_OPERATION:
            return "Input/output operation failed";
        case ERROR_PROCESS_CREATION:
            return "Process creation failed";
        case ERROR_SIGNAL_HANDLING:
            return "Signal handling error";
        case ERROR_ENVIRONMENT:
            return "Environment variable error";
        case ERROR_PARSING:
            return "Command parsing error";
        case ERROR_TIMEOUT:
            return "Operation timeout";
        case ERROR_RESOURCE_LIMIT:
            return "Resource limit exceeded";
        default:
            return "Unknown error";
    }
}

/**
 * 获取最后一个错误代码
 */
error_code_t get_last_error(void) {
    return g_error_state.last_error;
}

/**
 * 清除最后一个错误
 */
void clear_last_error(void) {
    g_error_state.last_error = ERROR_NONE;
    errno = 0;
}

/**
 * 获取错误统计信息
 */
int get_error_count(void) {
    return g_error_state.error_count;
}

/**
 * 重置错误统计
 */
void reset_error_count(void) {
    g_error_state.error_count = 0;
}

/**
 * 启用/禁用日志记录
 */
void set_logging_enabled(int enabled) {
    g_error_state.log_enabled = enabled;
}

/**
 * 检查是否启用了日志记录
 */
int is_logging_enabled(void) {
    return g_error_state.log_enabled;
}

/**
 * 安全的字符串复制，防止缓冲区溢出
 */
char* safe_strdup(const char *str, const char *context) {
    if (!str) {
        handle_error(ERROR_INVALID_ARGUMENT, context);
        return NULL;
    }
    
    size_t len = strlen(str);
    if (len > MAX_INPUT_SIZE - 1) {
        handle_error(ERROR_BUFFER_OVERFLOW, context);
        return NULL;
    }
    
    char *result = malloc(len + 1);
    if (!result) {
        handle_memory_error(context, len + 1);
        return NULL;
    }
    
    strcpy(result, str);
    return result;
}

/**
 * 安全的内存分配
 */
void* safe_malloc(size_t size, const char *context) {
    if (size == 0) {
        handle_error(ERROR_INVALID_ARGUMENT, context);
        return NULL;
    }
    
    if (size > MAX_ALLOCATION_SIZE) {
        handle_error(ERROR_RESOURCE_LIMIT, context);
        return NULL;
    }
    
    void *ptr = malloc(size);
    if (!ptr) {
        handle_memory_error(context, size);
        return NULL;
    }
    
    return ptr;
}

/**
 * 安全的内存重新分配
 */
void* safe_realloc(void *ptr, size_t size, const char *context) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    if (size > MAX_ALLOCATION_SIZE) {
        handle_error(ERROR_RESOURCE_LIMIT, context);
        return NULL;
    }
    
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        handle_memory_error(context, size);
        return NULL;
    }
    
    return new_ptr;
}
/**
 * 跟踪内存分配
 */
void* tracked_malloc(size_t size, const char *context, const char *file, int line) {
    if (!g_memory_state.tracking_enabled) {
        return safe_malloc(size, context);
    }
    
    if (size == 0) {
        handle_error(ERROR_INVALID_ARGUMENT, context);
        return NULL;
    }
    
    if (size > MAX_ALLOCATION_SIZE) {
        handle_error(ERROR_RESOURCE_LIMIT, context);
        return NULL;
    }
    
    void *ptr = malloc(size);
    if (!ptr) {
        handle_memory_error(context, size);
        return NULL;
    }
    
    /* 创建内存块跟踪记录 */
    memory_block_t *block = malloc(sizeof(memory_block_t));
    if (!block) {
        free(ptr);
        handle_memory_error("tracked_malloc: block tracking", sizeof(memory_block_t));
        return NULL;
    }
    
    block->ptr = ptr;
    block->size = size;
    block->context = context;
    block->file = file;
    block->line = line;
    block->next = g_memory_state.allocated_blocks;
    
    g_memory_state.allocated_blocks = block;
    g_memory_state.total_allocated += size;
    g_memory_state.allocation_count++;
    
    /* 更新峰值内存使用 */
    if (g_memory_state.total_allocated > g_memory_state.peak_allocated) {
        g_memory_state.peak_allocated = g_memory_state.total_allocated;
    }
    
    return ptr;
}

/**
 * 跟踪内存重新分配
 */
void* tracked_realloc(void *ptr, size_t size, const char *context, const char *file, int line) {
    if (!g_memory_state.tracking_enabled) {
        return safe_realloc(ptr, size, context);
    }
    
    if (size == 0) {
        if (ptr) {
            tracked_free(ptr, file, line);
        }
        return NULL;
    }
    
    if (size > MAX_ALLOCATION_SIZE) {
        handle_error(ERROR_RESOURCE_LIMIT, context);
        return NULL;
    }
    
    /* 如果ptr为NULL，相当于malloc */
    if (!ptr) {
        return tracked_malloc(size, context, file, line);
    }
    
    /* 查找原始内存块 */
    memory_block_t *block = g_memory_state.allocated_blocks;
    
    while (block && block->ptr != ptr) {
        block = block->next;
    }
    
    if (!block) {
        handle_error(ERROR_INVALID_ARGUMENT, "tracked_realloc: pointer not found");
        return NULL;
    }
    
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        handle_memory_error(context, size);
        return NULL;
    }
    
    /* 更新内存统计 */
    g_memory_state.total_allocated = g_memory_state.total_allocated - block->size + size;
    
    /* 更新块信息 */
    block->ptr = new_ptr;
    block->size = size;
    block->context = context;
    block->file = file;
    block->line = line;
    
    /* 更新峰值内存使用 */
    if (g_memory_state.total_allocated > g_memory_state.peak_allocated) {
        g_memory_state.peak_allocated = g_memory_state.total_allocated;
    }
    
    return new_ptr;
}

/**
 * 跟踪字符串复制
 */
char* tracked_strdup(const char *str, const char *context, const char *file, int line) {
    if (!str) {
        handle_error(ERROR_INVALID_ARGUMENT, context);
        return NULL;
    }
    
    size_t len = strlen(str);
    if (len > MAX_INPUT_SIZE - 1) {
        handle_error(ERROR_BUFFER_OVERFLOW, context);
        return NULL;
    }
    
    char *result = tracked_malloc(len + 1, context, file, line);
    if (!result) {
        return NULL;
    }
    
    strcpy(result, str);
    return result;
}

/**
 * 跟踪内存释放
 */
void tracked_free(void *ptr, const char *file, int line) {
    if (!ptr) {
        return;
    }
    
    if (!g_memory_state.tracking_enabled) {
        free(ptr);
        return;
    }
    
    /* 查找内存块 */
    memory_block_t *block = g_memory_state.allocated_blocks;
    memory_block_t *prev = NULL;
    
    while (block && block->ptr != ptr) {
        prev = block;
        block = block->next;
    }
    
    if (!block) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "tracked_free: attempting to free untracked pointer %p at %s:%d", 
                ptr, file ? file : "unknown", line);
        log_warning(error_msg);
        free(ptr);  /* 仍然释放内存，避免泄漏 */
        return;
    }
    
    /* 从链表中移除 */
    if (prev) {
        prev->next = block->next;
    } else {
        g_memory_state.allocated_blocks = block->next;
    }
    
    /* 更新统计信息 */
    g_memory_state.total_allocated -= block->size;
    g_memory_state.deallocation_count++;
    
    /* 释放内存 */
    free(ptr);
    free(block);
}

/**
 * 打印内存统计信息
 */
void print_memory_stats(void) {
    if (!g_memory_state.tracking_enabled) {
        printf("Memory tracking is disabled\n");
        return;
    }
    
    printf("\n=== Memory Statistics ===\n");
    printf("Total allocations: %d\n", g_memory_state.allocation_count);
    printf("Total deallocations: %d\n", g_memory_state.deallocation_count);
    printf("Current allocated: %zu bytes\n", g_memory_state.total_allocated);
    printf("Peak allocated: %zu bytes\n", g_memory_state.peak_allocated);
    printf("Outstanding blocks: %d\n", 
           g_memory_state.allocation_count - g_memory_state.deallocation_count);
    
    /* 计算当前分配的块数 */
    int block_count = 0;
    memory_block_t *block = g_memory_state.allocated_blocks;
    while (block) {
        block_count++;
        block = block->next;
    }
    printf("Tracked blocks: %d\n", block_count);
    printf("========================\n\n");
}

/**
 * 打印内存泄漏信息
 */
void print_memory_leaks(void) {
    if (!g_memory_state.tracking_enabled) {
        return;
    }
    
    memory_block_t *block = g_memory_state.allocated_blocks;
    int leak_count = 0;
    
    if (!block) {
        printf("No memory leaks detected.\n");
        return;
    }
    
    printf("\n=== Memory Leaks Detected ===\n");
    
    while (block) {
        leak_count++;
        printf("Leak #%d:\n", leak_count);
        printf("  Address: %p\n", block->ptr);
        printf("  Size: %zu bytes\n", block->size);
        printf("  Context: %s\n", block->context ? block->context : "unknown");
        printf("  Location: %s:%d\n", 
               block->file ? block->file : "unknown", block->line);
        printf("\n");
        block = block->next;
    }
    
    printf("Total leaks: %d blocks, %zu bytes\n", 
           leak_count, g_memory_state.total_allocated);
    printf("=============================\n\n");
}

/**
 * 检查内存泄漏
 */
int check_memory_leaks(void) {
    if (!g_memory_state.tracking_enabled) {
        return 0;
    }
    
    int leak_count = 0;
    memory_block_t *block = g_memory_state.allocated_blocks;
    
    while (block) {
        leak_count++;
        block = block->next;
    }
    
    return leak_count;
}

/**
 * 启用/禁用内存跟踪
 */
void set_memory_tracking(int enabled) {
    g_memory_state.tracking_enabled = enabled;
    
    if (enabled) {
        log_info("Memory tracking enabled");
    } else {
        log_info("Memory tracking disabled");
    }
}

/**
 * 检查内存跟踪是否启用
 */
int is_memory_tracking_enabled(void) {
    return g_memory_state.tracking_enabled;
}
