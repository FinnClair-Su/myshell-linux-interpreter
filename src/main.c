#include "shell.h"

/* 全局Shell状态 */
shell_state_t g_shell_state;

/**
 * 主程序入口点
 */
int main(int argc, char *argv[]) {
    (void)argc;  /* 避免未使用参数警告 */
    (void)argv;
    
    /* 初始化Shell */
    shell_init();
    
    /* 启动主循环 */
    main_loop();
    
    /* 清理资源 */
    shell_cleanup();
    
    return g_shell_state.last_exit_status;
}

/**
 * 信号处理函数 - 处理Ctrl+C (SIGINT)
 */
void handle_sigint(int sig) {
    (void)sig;  /* 避免未使用参数警告 */
    
    /* 如果在主循环中，显示新的提示符 */
    printf("\n");
    display_prompt();
    fflush(stdout);
}

/**
 * 信号处理函数 - 处理Ctrl+\ (SIGQUIT)
 */
void handle_sigquit(int sig) {
    (void)sig;  /* 避免未使用参数警告 */
    
    /* 忽略SIGQUIT，不退出Shell */
    printf("\nUse 'exit' to quit the shell.\n");
    display_prompt();
    fflush(stdout);
}

/**
 * 设置信号处理器
 */
void setup_signal_handlers(void) {
    /* 设置SIGINT处理器 (Ctrl+C) */
    struct sigaction sa_int;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;  /* 重启被中断的系统调用 */
    
    HANDLE_SYSCALL_ERROR(sigaction(SIGINT, &sa_int, NULL), "setup_signal_handlers: SIGINT", {
        handle_error(ERROR_SIGNAL_HANDLING, "setup_signal_handlers: failed to set SIGINT handler");
    });
    
    /* 设置SIGQUIT处理器 (Ctrl+\) */
    struct sigaction sa_quit;
    sa_quit.sa_handler = handle_sigquit;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = SA_RESTART;
    
    HANDLE_SYSCALL_ERROR(sigaction(SIGQUIT, &sa_quit, NULL), "setup_signal_handlers: SIGQUIT", {
        handle_error(ERROR_SIGNAL_HANDLING, "setup_signal_handlers: failed to set SIGQUIT handler");
    });
    
    /* 忽略SIGTSTP (Ctrl+Z) - Shell不应该被挂起 */
    signal(SIGTSTP, SIG_IGN);
}

/**
 * 初始化Shell环境
 */
void shell_init(void) {
    /* 初始化错误处理系统 */
    init_error_system();
    
    /* 初始化Shell状态 */
    g_shell_state.current_dir = NULL;
    g_shell_state.env_vars = NULL;
    g_shell_state.last_exit_status = 0;
    g_shell_state.running = 1;
    
    /* 获取当前工作目录 */
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        handle_syscall_error("getcwd", "shell_init");
        /* 尝试使用默认目录 */
        g_shell_state.current_dir = TRACKED_STRDUP("/tmp", "shell_init: default directory");
        if (g_shell_state.current_dir == NULL) {
            handle_error(ERROR_MEMORY_ALLOCATION, "shell_init: critical memory allocation failure");
            exit(EXIT_FAILURE);
        }
        /* 尝试切换到默认目录 */
        HANDLE_SYSCALL_ERROR(chdir(g_shell_state.current_dir), "shell_init", {
            handle_error(ERROR_SYSTEM_CALL, "shell_init: failed to change to default directory");
            exit(EXIT_FAILURE);
        });
    } else {
        /* 使用跟踪的字符串复制 */
        g_shell_state.current_dir = TRACKED_STRDUP(cwd, "shell_init: current directory");
        free(cwd);  /* 释放getcwd分配的内存 */
        if (g_shell_state.current_dir == NULL) {
            handle_error(ERROR_MEMORY_ALLOCATION, "shell_init: critical memory allocation failure");
            exit(EXIT_FAILURE);
        }
    }
    
    /* 初始化环境变量 */
    init_environment();
    
    /* 设置信号处理 */
    setup_signal_handlers();
    
    printf("MyShell v1.0 - Linux Shell Interpreter\n");
    printf("Type 'exit' to quit.\n");
    printf("Press Ctrl+C to interrupt, Ctrl+D to exit.\n\n");
}

/**
 * Shell主循环
 */
void main_loop(void) {
    char *input;
    command_t *cmd;
    
    while (g_shell_state.running) {
        /* 显示提示符 */
        display_prompt();
        
        /* 读取用户输入 */
        input = read_input();
        if (input == NULL) {
            /* EOF (Ctrl+D) 或读取错误 */
            printf("\n");
            break;
        }
        
        /* 跳过空输入 */
        if (strlen(input) == 0) {
            TRACKED_FREE(input);
            continue;
        }
        
        /* 解析命令 */
        cmd = parse_command(input);
        if (cmd == NULL) {
            /* 解析错误，显示错误信息并继续 */
            print_error("Invalid command syntax");
            TRACKED_FREE(input);
            continue;
        }
        
        /* 执行命令 */
        if (is_builtin(cmd->command)) {
            /* 对于内部命令，传递参数时跳过命令名 */
            char **builtin_args = (cmd->argc > 1) ? &cmd->args[1] : NULL;
            g_shell_state.last_exit_status = execute_builtin(cmd->command, builtin_args);
        } else {
            g_shell_state.last_exit_status = execute_external(cmd->command, cmd->args);
        }
        
        /* 清理资源 */
        free_command(cmd);
        TRACKED_FREE(input);
    }
}

/**
 * 清理Shell资源
 */
void shell_cleanup(void) {
    log_info("Starting shell cleanup");
    
    /* 释放当前目录字符串 */
    if (g_shell_state.current_dir) {
        TRACKED_FREE(g_shell_state.current_dir);
        g_shell_state.current_dir = NULL;
    }
    
    /* 释放环境变量链表 */
    cleanup_environment();
    
    /* 打印内存统计信息 */
    if (is_memory_tracking_enabled()) {
        print_memory_stats();
    }
    
    /* 清理错误处理系统（包括内存跟踪） */
    cleanup_error_system();
    
    printf("Shell exited.\n");
}
