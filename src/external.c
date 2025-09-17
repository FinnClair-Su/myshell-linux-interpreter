#include "shell.h"

/**
 * 执行外部命令
 */
int execute_external(char *command, char **args) {
    if (command == NULL) {
        return -1;
    }
    
    /* 查找可执行文件 */
    char *executable_path = find_executable(command);
    if (executable_path == NULL) {
        fprintf(stderr, "%s: command not found\n", command);
        return 127;  /* 命令未找到的标准退出码 */
    }
    
    /* 创建子进程并执行 */
    int exit_status = fork_and_exec(executable_path, args);
    
    free(executable_path);
    return exit_status;
}

/**
 * 在PATH中查找可执行文件
 */
char* find_executable(char *command) {
    if (command == NULL) {
        return NULL;
    }
    
    /* 如果命令包含路径分隔符，直接检查 */
    if (strchr(command, '/') != NULL) {
        if (access(command, X_OK) == 0) {
            return strdup(command);
        }
        return NULL;
    }
    
    /* 在PATH中搜索 */
    char **path_dirs = get_path_dirs();
    if (path_dirs == NULL) {
        return NULL;
    }
    
    char *full_path = malloc(MAX_PATH_SIZE);
    HANDLE_MALLOC_ERROR(full_path, return NULL);
    
    for (int i = 0; path_dirs[i] != NULL; i++) {
        snprintf(full_path, MAX_PATH_SIZE, "%s/%s", path_dirs[i], command);
        
        if (access(full_path, X_OK) == 0) {
            /* 释放path_dirs */
            for (int j = 0; path_dirs[j] != NULL; j++) {
                free(path_dirs[j]);
            }
            free(path_dirs);
            
            return full_path;
        }
    }
    
    /* 清理资源 */
    for (int i = 0; path_dirs[i] != NULL; i++) {
        free(path_dirs[i]);
    }
    free(path_dirs);
    free(full_path);
    
    return NULL;
}

/**
 * 创建子进程并执行程序
 */
int fork_and_exec(char *path, char **args) {
    if (path == NULL) {
        return -1;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        /* 子进程 */
        if (execv(path, args) == -1) {
            perror("execv");
            exit(EXIT_FAILURE);
        }
    } else {
        /* 父进程 */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
    }
    
    return 0;
}
