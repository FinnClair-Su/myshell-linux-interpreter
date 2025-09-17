#include "shell.h"

/* 静态函数声明 */
static char* process_escape_sequences(const char *input);
static int is_valid_var_name(const char *name);

/* 内部命令信息结构体 */
typedef struct {
    char *name;
    builtin_func_t func;
    int min_args;       /* 最少参数个数 */
    int max_args;       /* 最多参数个数 (-1表示无限制) */
    char *usage;        /* 使用说明 */
    char *description;  /* 命令描述 */
} builtin_info_t;

/* 内部命令注册表 */
static builtin_info_t builtin_commands[] = {
    {"ls", builtin_ls, 0, 1, "ls [directory]", "List directory contents"},
    {"cat", builtin_cat, 1, -1, "cat <file1> [file2] ...", "Display file contents"},
    {"cp", builtin_cp, 2, 2, "cp <source> <destination>", "Copy files"},
    {"rm", builtin_rm, 1, -1, "rm <file1> [file2] ...", "Remove files"},
    {"touch", builtin_touch, 1, -1, "touch <file1> [file2] ...", "Create empty files"},
    {"date", builtin_date, 0, 0, "date", "Display current date and time"},
    {"pwd", builtin_pwd, 0, 0, "pwd", "Print working directory"},
    {"cd", builtin_cd, 0, 1, "cd [directory]", "Change directory"},
    {"echo", builtin_echo, 0, -1, "echo [text] ...", "Display text"},
    {"export", builtin_export, 1, 1, "export <VAR=value>", "Set environment variable"},
    {"memstat", builtin_memstat, 0, 1, "memstat [leaks]", "Show memory statistics"},
    {"exit", builtin_exit, 0, 1, "exit [code]", "Exit the shell"},
    {"help", builtin_help, 0, 1, "help [command]", "Show help information"},
    {NULL, NULL, 0, 0, NULL, NULL}  /* 结束标记 */
};

/**
 * 计算参数个数
 */
static int count_args(char **args) {
    int count = 0;
    if (args == NULL) {
        return 0;
    }
    
    while (args[count] != NULL) {
        count++;
    }
    return count;
}

/**
 * 验证命令参数
 */
static int validate_args(builtin_info_t *cmd_info, char **args) {
    if (cmd_info == NULL) {
        return 0;
    }
    
    int argc = count_args(args);
    
    /* 检查最少参数个数 */
    if (argc < cmd_info->min_args) {
        print_error("Too few arguments");
        printf("Usage: %s\n", cmd_info->usage);
        return 0;
    }
    
    /* 检查最多参数个数 */
    if (cmd_info->max_args != -1 && argc > cmd_info->max_args) {
        print_error("Too many arguments");
        printf("Usage: %s\n", cmd_info->usage);
        return 0;
    }
    
    return 1;
}

/**
 * 查找内部命令信息
 */
static builtin_info_t* find_builtin_info(char *command) {
    if (command == NULL) {
        return NULL;
    }
    
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(command, builtin_commands[i].name) == 0) {
            return &builtin_commands[i];
        }
    }
    return NULL;
}

/**
 * 检查命令是否为内部命令
 */
int is_builtin(char *command) {
    return find_builtin_info(command) != NULL;
}

/**
 * 执行内部命令
 */
int execute_builtin(char *command, char **args) {
    if (command == NULL) {
        handle_error(ERROR_INVALID_ARGUMENT, "execute_builtin: command is NULL");
        return -1;
    }
    
    builtin_info_t *cmd_info = find_builtin_info(command);
    if (cmd_info == NULL) {
        handle_error(ERROR_COMMAND_NOT_FOUND, command);
        return -1;
    }
    
    /* 验证参数 */
    if (!validate_args(cmd_info, args)) {
        return -1;
    }
    
    /* 执行命令 */
    int result = cmd_info->func(args);
    
    /* 更新最后执行状态 */
    g_shell_state.last_exit_status = result;
    
    return result;
}

/**
 * 获取所有内部命令列表
 */
void list_builtin_commands(void) {
    printf("Available built-in commands:\n");
    printf("%-10s %s\n", "Command", "Description");
    printf("%-10s %s\n", "-------", "-----------");
    
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        printf("%-10s %s\n", builtin_commands[i].name, builtin_commands[i].description);
    }
}

/**
 * 获取命令帮助信息
 */
void show_command_help(char *command) {
    if (command == NULL) {
        list_builtin_commands();
        return;
    }
    
    builtin_info_t *cmd_info = find_builtin_info(command);
    if (cmd_info == NULL) {
        printf("Unknown command: %s\n", command);
        printf("Type 'help' to see available commands.\n");
        return;
    }
    
    printf("Command: %s\n", cmd_info->name);
    printf("Usage: %s\n", cmd_info->usage);
    printf("Description: %s\n", cmd_info->description);
}

/* 内部命令实现 - 占位符函数 */

int builtin_ls(char **args) {
    char *target_dir = ".";  /* 默认为当前目录 */
    
    /* 如果提供了目录参数 */
    if (args != NULL && args[0] != NULL) {
        target_dir = args[0];
    }
    
    /* 使用opendir系统调用打开目录 */
    DIR *dir = opendir(target_dir);
    if (dir == NULL) {
        /* 根据errno提供具体的错误信息 */
        switch (errno) {
            case ENOENT:
                print_error("ls: cannot access: No such file or directory");
                break;
            case ENOTDIR:
                print_error("ls: cannot access: Not a directory");
                break;
            case EACCES:
                print_error("ls: cannot open directory: Permission denied");
                break;
            default:
                handle_error(ERROR_SYSTEM_CALL, "opendir failed");
                break;
        }
        return -1;
    }
    
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH_SIZE];
    
    /* 使用readdir系统调用读取目录项 */
    while ((entry = readdir(dir)) != NULL) {
        /* 跳过隐藏文件（以.开头的文件，除了.和..） */
        if (entry->d_name[0] == '.' && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            continue;
        }
        
        /* 构建完整路径用于获取文件信息 */
        snprintf(full_path, sizeof(full_path), "%s/%s", target_dir, entry->d_name);
        
        /* 获取文件状态信息 */
        if (stat(full_path, &file_stat) == 0) {
            /* 显示文件类型标识 */
            char type_char = '-';
            if (S_ISDIR(file_stat.st_mode)) {
                type_char = 'd';
            } else if (S_ISLNK(file_stat.st_mode)) {
                type_char = 'l';
            } else if (S_ISREG(file_stat.st_mode)) {
                type_char = '-';
            }
            
            /* 显示权限信息 */
            char permissions[10];
            permissions[0] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
            permissions[1] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
            permissions[2] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
            permissions[3] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
            permissions[4] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
            permissions[5] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
            permissions[6] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
            permissions[7] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
            permissions[8] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
            permissions[9] = '\0';
            
            /* 显示文件信息：类型+权限 文件名 */
            printf("%c%s  %s", type_char, permissions, entry->d_name);
            
            /* 如果是目录，添加/标识 */
            if (S_ISDIR(file_stat.st_mode)) {
                printf("/");
            }
            printf("\n");
        } else {
            /* 如果无法获取文件状态，只显示文件名 */
            printf("?---------  %s\n", entry->d_name);
        }
    }
    
    /* 使用closedir系统调用关闭目录 */
    if (closedir(dir) != 0) {
        handle_error(ERROR_SYSTEM_CALL, "closedir failed");
        return -1;
    }
    
    return 0;
}

int builtin_cat(char **args) {
    if (args == NULL || args[0] == NULL) {
        print_error("cat: missing file operand");
        return -1;
    }
    
    int overall_result = 0;
    
    /* 处理多个文件参数 */
    for (int i = 0; args[i] != NULL; i++) {
        char *filename = args[i];
        
        /* 检查文件是否存在并获取文件信息 */
        struct stat file_stat;
        if (stat(filename, &file_stat) != 0) {
            switch (errno) {
                case ENOENT:
                    print_error("cat: cannot access file: No such file or directory");
                    break;
                case EACCES:
                    print_error("cat: cannot access file: Permission denied");
                    break;
                default:
                    handle_error(ERROR_SYSTEM_CALL, "stat failed");
                    break;
            }
            overall_result = -1;
            continue;
        }
        
        /* 检查是否为目录 */
        if (S_ISDIR(file_stat.st_mode)) {
            print_error("cat: is a directory");
            overall_result = -1;
            continue;
        }
        
        /* 检查是否为常规文件 */
        if (!S_ISREG(file_stat.st_mode)) {
            print_error("cat: not a regular file");
            overall_result = -1;
            continue;
        }
        
        /* 使用open系统调用打开文件 */
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            switch (errno) {
                case ENOENT:
                    print_error("cat: cannot open file: No such file or directory");
                    break;
                case EACCES:
                    print_error("cat: cannot open file: Permission denied");
                    break;
                case EISDIR:
                    print_error("cat: cannot open file: Is a directory");
                    break;
                default:
                    handle_error(ERROR_SYSTEM_CALL, "open failed");
                    break;
            }
            overall_result = -1;
            continue;
        }
        
        /* 读取并输出文件内容 */
        char buffer[4096];  /* 4KB缓冲区，适合处理大文件 */
        ssize_t bytes_read;
        
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            /* 使用write系统调用输出到标准输出 */
            ssize_t bytes_written = 0;
            ssize_t total_written = 0;
            
            while (total_written < bytes_read) {
                bytes_written = write(STDOUT_FILENO, buffer + total_written, 
                                    bytes_read - total_written);
                if (bytes_written == -1) {
                    if (errno == EINTR) {
                        /* 被信号中断，继续写入 */
                        continue;
                    }
                    handle_error(ERROR_SYSTEM_CALL, "write to stdout failed");
                    close(fd);
                    return -1;
                }
                total_written += bytes_written;
            }
        }
        
        /* 检查读取是否出错 */
        if (bytes_read == -1) {
            handle_error(ERROR_SYSTEM_CALL, "read failed");
            close(fd);
            overall_result = -1;
            continue;
        }
        
        /* 关闭文件 */
        if (close(fd) != 0) {
            handle_error(ERROR_SYSTEM_CALL, "close failed");
            overall_result = -1;
        }
    }
    
    return overall_result;
}

int builtin_cp(char **args) {
    if (args == NULL || args[0] == NULL || args[1] == NULL) {
        print_error("cp: missing file operand");
        printf("Usage: cp <source> <destination>\n");
        return -1;
    }
    
    char *source = args[0];
    char *destination = args[1];
    
    /* 检查源文件是否存在并获取文件信息 */
    struct stat source_stat;
    if (stat(source, &source_stat) != 0) {
        switch (errno) {
            case ENOENT:
                print_error("cp: cannot stat source file: No such file or directory");
                break;
            case EACCES:
                print_error("cp: cannot access source file: Permission denied");
                break;
            default:
                handle_error(ERROR_SYSTEM_CALL, "stat failed on source file");
                break;
        }
        return -1;
    }
    
    /* 检查源文件是否为常规文件 */
    if (!S_ISREG(source_stat.st_mode)) {
        if (S_ISDIR(source_stat.st_mode)) {
            print_error("cp: source is a directory (use cp -r for directories)");
        } else {
            print_error("cp: source is not a regular file");
        }
        return -1;
    }
    
    /* 检查目标文件是否存在 */
    struct stat dest_stat;
    int dest_exists = (stat(destination, &dest_stat) == 0);
    
    if (dest_exists) {
        /* 检查是否尝试复制到目录 */
        if (S_ISDIR(dest_stat.st_mode)) {
            print_error("cp: destination is a directory");
            return -1;
        }
        
        /* 检查是否尝试复制到自己 */
        if (source_stat.st_dev == dest_stat.st_dev && 
            source_stat.st_ino == dest_stat.st_ino) {
            print_error("cp: source and destination are the same file");
            return -1;
        }
        
        /* 询问是否覆盖现有文件 */
        char confirm_msg[512];
        snprintf(confirm_msg, sizeof(confirm_msg), 
                "cp: overwrite '%s'?", destination);
        
        if (!confirm_action(confirm_msg)) {
            printf("cp: not overwriting '%s'\n", destination);
            return 0;
        }
    }
    
    /* 使用open系统调用打开源文件 */
    int source_fd = open(source, O_RDONLY);
    if (source_fd == -1) {
        switch (errno) {
            case ENOENT:
                print_error("cp: cannot open source file: No such file or directory");
                break;
            case EACCES:
                print_error("cp: cannot open source file: Permission denied");
                break;
            default:
                handle_error(ERROR_SYSTEM_CALL, "open failed on source file");
                break;
        }
        return -1;
    }
    
    /* 使用open系统调用创建/打开目标文件 */
    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 
                      source_stat.st_mode & 0777);  /* 保持源文件权限 */
    if (dest_fd == -1) {
        switch (errno) {
            case EACCES:
                print_error("cp: cannot create destination file: Permission denied");
                break;
            case ENOSPC:
                print_error("cp: cannot create destination file: No space left on device");
                break;
            case ENOENT:
                print_error("cp: cannot create destination file: No such file or directory");
                break;
            default:
                handle_error(ERROR_SYSTEM_CALL, "open failed on destination file");
                break;
        }
        close(source_fd);
        return -1;
    }
    
    /* 复制文件内容 */
    char buffer[8192];  /* 8KB缓冲区，提高复制效率 */
    ssize_t bytes_read, bytes_written;
    int copy_result = 0;
    
    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t total_written = 0;
        
        while (total_written < bytes_read) {
            bytes_written = write(dest_fd, buffer + total_written, 
                                bytes_read - total_written);
            if (bytes_written == -1) {
                if (errno == EINTR) {
                    /* 被信号中断，继续写入 */
                    continue;
                }
                switch (errno) {
                    case ENOSPC:
                        print_error("cp: write error: No space left on device");
                        break;
                    case EIO:
                        print_error("cp: write error: Input/output error");
                        break;
                    default:
                        handle_error(ERROR_SYSTEM_CALL, "write failed");
                        break;
                }
                copy_result = -1;
                goto cleanup;
            }
            total_written += bytes_written;
        }
    }
    
    /* 检查读取是否出错 */
    if (bytes_read == -1) {
        handle_error(ERROR_SYSTEM_CALL, "read failed");
        copy_result = -1;
        goto cleanup;
    }
    
    /* 同步文件数据到磁盘 */
    if (fsync(dest_fd) != 0) {
        handle_error(ERROR_SYSTEM_CALL, "fsync failed");
        copy_result = -1;
    }
    
cleanup:
    /* 关闭文件描述符 */
    if (close(source_fd) != 0) {
        handle_error(ERROR_SYSTEM_CALL, "close failed on source file");
        copy_result = -1;
    }
    
    if (close(dest_fd) != 0) {
        handle_error(ERROR_SYSTEM_CALL, "close failed on destination file");
        copy_result = -1;
    }
    
    /* 如果复制失败，删除不完整的目标文件 */
    if (copy_result != 0) {
        unlink(destination);
        return -1;
    }
    
    /* 保持文件时间戳 */
    struct utimbuf times;
    times.actime = source_stat.st_atime;
    times.modtime = source_stat.st_mtime;
    
    if (utime(destination, &times) != 0) {
        /* 时间戳设置失败不是致命错误，只是警告 */
        print_warning("cp: failed to preserve timestamps");
    }
    
    printf("cp: copied '%s' to '%s'\n", source, destination);
    return 0;
}

int builtin_rm(char **args) {
    if (args == NULL || args[0] == NULL) {
        print_error("rm: missing file operand");
        return -1;
    }
    
    int overall_result = 0;
    
    /* 处理多个文件参数 */
    for (int i = 0; args[i] != NULL; i++) {
        char *filename = args[i];
        
        /* 检查文件是否存在并获取文件信息 */
        struct stat file_stat;
        if (stat(filename, &file_stat) != 0) {
            switch (errno) {
                case ENOENT:
                    print_error("rm: cannot remove file: No such file or directory");
                    break;
                case EACCES:
                    print_error("rm: cannot access file: Permission denied");
                    break;
                default:
                    handle_error(ERROR_SYSTEM_CALL, "stat failed");
                    break;
            }
            overall_result = -1;
            continue;
        }
        
        /* 检查是否为目录 */
        if (S_ISDIR(file_stat.st_mode)) {
            print_error("rm: cannot remove directory (use rmdir for directories)");
            overall_result = -1;
            continue;
        }
        
        /* 检查文件权限 */
        if (access(filename, W_OK) != 0) {
            /* 文件没有写权限，询问用户确认 */
            char confirm_msg[512];
            snprintf(confirm_msg, sizeof(confirm_msg), 
                    "rm: remove write-protected file '%s'?", filename);
            
            if (!confirm_action(confirm_msg)) {
                printf("rm: skipping '%s'\n", filename);
                continue;
            }
        }
        
        /* 使用unlink系统调用删除文件 */
        if (unlink(filename) != 0) {
            switch (errno) {
                case ENOENT:
                    print_error("rm: cannot remove file: No such file or directory");
                    break;
                case EACCES:
                    print_error("rm: cannot remove file: Permission denied");
                    break;
                case EPERM:
                    print_error("rm: cannot remove file: Operation not permitted");
                    break;
                case EISDIR:
                    print_error("rm: cannot remove directory (use rmdir for directories)");
                    break;
                case EBUSY:
                    print_error("rm: cannot remove file: Device or resource busy");
                    break;
                default:
                    handle_error(ERROR_SYSTEM_CALL, "unlink failed");
                    break;
            }
            overall_result = -1;
            continue;
        }
        
        /* 成功删除文件 */
        printf("rm: removed '%s'\n", filename);
    }
    
    return overall_result;
}

int builtin_touch(char **args) {
    if (args == NULL || args[0] == NULL) {
        print_error("touch: missing file operand");
        return -1;
    }
    
    int overall_result = 0;
    
    /* 处理多个文件参数 */
    for (int i = 0; args[i] != NULL; i++) {
        char *filename = args[i];
        
        /* 检查文件是否已存在 */
        struct stat file_stat;
        int file_exists = (stat(filename, &file_stat) == 0);
        
        if (file_exists) {
            /* 文件已存在，更新访问和修改时间 */
            if (utime(filename, NULL) != 0) {
                switch (errno) {
                    case EACCES:
                        print_error("touch: cannot touch file: Permission denied");
                        break;
                    case ENOENT:
                        print_error("touch: cannot touch file: No such file or directory");
                        break;
                    default:
                        handle_error(ERROR_SYSTEM_CALL, "utime failed");
                        break;
                }
                overall_result = -1;
                continue;
            }
        } else {
            /* 文件不存在，创建新文件 */
            /* 使用open系统调用创建文件，设置适当的权限 */
            int fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, 
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  /* 644权限 */
            
            if (fd == -1) {
                switch (errno) {
                    case EEXIST:
                        /* 文件在检查后被创建，尝试更新时间戳 */
                        if (utime(filename, NULL) != 0) {
                            print_error("touch: cannot touch file: Permission denied");
                            overall_result = -1;
                        }
                        break;
                    case EACCES:
                        print_error("touch: cannot create file: Permission denied");
                        overall_result = -1;
                        break;
                    case ENOENT:
                        print_error("touch: cannot create file: No such file or directory");
                        overall_result = -1;
                        break;
                    case ENOSPC:
                        print_error("touch: cannot create file: No space left on device");
                        overall_result = -1;
                        break;
                    default:
                        handle_error(ERROR_SYSTEM_CALL, "open failed");
                        overall_result = -1;
                        break;
                }
                continue;
            }
            
            /* 关闭文件描述符 */
            if (close(fd) != 0) {
                handle_error(ERROR_SYSTEM_CALL, "close failed");
                overall_result = -1;
            }
        }
    }
    
    return overall_result;
}

int builtin_date(char **args) {
    (void)args;  /* 避免未使用参数警告 */
    
    /* 使用time系统调用获取当前时间 */
    time_t current_time = time(NULL);
    if (current_time == (time_t)-1) {
        handle_error(ERROR_SYSTEM_CALL, "time system call failed");
        return -1;
    }
    
    /* 将时间转换为本地时间结构 */
    struct tm *local_time = localtime(&current_time);
    if (local_time == NULL) {
        handle_error(ERROR_SYSTEM_CALL, "localtime failed");
        return -1;
    }
    
    /* 格式化时间输出 - 使用标准的date命令格式 */
    char time_buffer[256];
    if (strftime(time_buffer, sizeof(time_buffer), 
                "%a %b %d %H:%M:%S %Z %Y", local_time) == 0) {
        print_error("date: failed to format time");
        return -1;
    }
    
    /* 输出格式化的时间 */
    printf("%s\n", time_buffer);
    
    return 0;
}

int builtin_pwd(char **args) {
    (void)args;  /* 避免未使用参数警告 */
    
    LOG_FUNCTION_ENTRY("builtin_pwd");
    
    char *cwd = NULL;
    size_t size = MAX_PATH_SIZE;
    
    /* 使用安全内存分配 */
    cwd = safe_malloc(size, "builtin_pwd: buffer allocation");
    if (cwd == NULL) {
        return -1;
    }
    
    /* 调用getcwd系统调用获取当前工作目录 */
    if (getcwd(cwd, size) == NULL) {
        /* 如果缓冲区太小，动态增加大小 */
        if (errno == ERANGE) {
            SAFE_FREE(cwd);
            size *= 2;
            cwd = safe_malloc(size, "builtin_pwd: larger buffer allocation");
            if (cwd == NULL) {
                return -1;
            }
            
            if (getcwd(cwd, size) == NULL) {
                handle_syscall_error("getcwd", "builtin_pwd");
                SAFE_FREE(cwd);
                return -1;
            }
        } else {
            handle_syscall_error("getcwd", "builtin_pwd");
            SAFE_FREE(cwd);
            return -1;
        }
    }
    
    /* 输出当前工作目录 */
    printf("%s\n", cwd);
    
    /* 释放内存 */
    SAFE_FREE(cwd);
    
    LOG_FUNCTION_EXIT("builtin_pwd");
    return 0;
}

int builtin_cd(char **args) {
    LOG_FUNCTION_ENTRY("builtin_cd");
    
    char *target_dir = NULL;
    
    /* 如果没有参数，切换到HOME目录 */
    if (args == NULL || args[0] == NULL) {
        target_dir = get_env_var("HOME");
        if (target_dir == NULL) {
            handle_error(ERROR_ENVIRONMENT, "builtin_cd: HOME environment variable not set");
            return -1;
        }
    } else {
        target_dir = args[0];
        
        /* 验证路径长度 */
        if (strlen(target_dir) >= MAX_PATH_SIZE) {
            handle_error(ERROR_INVALID_PATH, "builtin_cd: path too long");
            return -1;
        }
    }
    
    /* 使用chdir系统调用切换目录 */
    if (chdir(target_dir) != 0) {
        /* 根据errno提供具体的错误信息 */
        switch (errno) {
            case ENOENT:
                handle_error(ERROR_FILE_NOT_FOUND, "builtin_cd: directory not found");
                break;
            case ENOTDIR:
                handle_error(ERROR_INVALID_PATH, "builtin_cd: not a directory");
                break;
            case EACCES:
                handle_error(ERROR_PERMISSION_DENIED, "builtin_cd: permission denied");
                break;
            default:
                handle_syscall_error("chdir", "builtin_cd");
                break;
        }
        return -1;
    }
    
    /* 更新PWD环境变量 */
    char *new_cwd = safe_malloc(MAX_PATH_SIZE, "builtin_cd: new directory buffer");
    if (new_cwd == NULL) {
        return -1;
    }
    
    if (getcwd(new_cwd, MAX_PATH_SIZE) == NULL) {
        handle_syscall_error("getcwd", "builtin_cd: after chdir");
        SAFE_FREE(new_cwd);
        return -1;
    }
    
    /* 设置PWD环境变量 */
    if (set_env_var("PWD", new_cwd) != 0) {
        handle_error(ERROR_ENVIRONMENT, "builtin_cd: failed to update PWD");
        SAFE_FREE(new_cwd);
        return -1;
    }
    
    /* 更新Shell状态中的当前目录 */
    SAFE_FREE(g_shell_state.current_dir);
    g_shell_state.current_dir = new_cwd;  /* 转移所有权 */
    
    LOG_FUNCTION_EXIT("builtin_cd");
    return 0;
}

int builtin_echo(char **args) {
    int newline = 1;  /* 默认输出换行符 */
    int start_index = 0;
    
    /* 检查是否有-n选项（不输出换行符） */
    if (args != NULL && args[0] != NULL && strcmp(args[0], "-n") == 0) {
        newline = 0;
        start_index = 1;
    }
    
    /* 如果没有参数（除了可能的-n选项），只输出换行或什么都不输出 */
    if (args == NULL || args[start_index] == NULL) {
        if (newline) {
            printf("\n");
        }
        return 0;
    }
    
    /* 输出所有参数，用空格分隔 */
    for (int i = start_index; args[i] != NULL; i++) {
        if (i > start_index) {
            printf(" ");
        }
        
        /* 处理环境变量扩展和转义字符 */
        char *expanded_arg = expand_variables(args[i]);
        if (expanded_arg != NULL) {
            /* 处理转义字符 */
            char *processed_arg = process_escape_sequences(expanded_arg);
            if (processed_arg != NULL) {
                printf("%s", processed_arg);
                free(processed_arg);
            } else {
                printf("%s", expanded_arg);
            }
            free(expanded_arg);
        } else {
            /* 如果环境变量扩展失败，直接输出原字符串 */
            char *processed_arg = process_escape_sequences(args[i]);
            if (processed_arg != NULL) {
                printf("%s", processed_arg);
                free(processed_arg);
            } else {
                printf("%s", args[i]);
            }
        }
    }
    
    if (newline) {
        printf("\n");
    }
    
    return 0;
}

/**
 * 处理转义字符序列
 * 支持常见的转义字符：\n, \t, \r, \\, \", \'
 */
static char* process_escape_sequences(const char *input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t input_len = strlen(input);
    char *output = malloc(input_len + 1);  /* 最多和输入一样长 */
    if (output == NULL) {
        return NULL;
    }
    
    size_t out_pos = 0;
    for (size_t i = 0; i < input_len; i++) {
        if (input[i] == '\\' && i + 1 < input_len) {
            /* 处理转义字符 */
            switch (input[i + 1]) {
                case 'n':
                    output[out_pos++] = '\n';
                    i++;  /* 跳过下一个字符 */
                    break;
                case 't':
                    output[out_pos++] = '\t';
                    i++;
                    break;
                case 'r':
                    output[out_pos++] = '\r';
                    i++;
                    break;
                case '\\':
                    output[out_pos++] = '\\';
                    i++;
                    break;
                case '"':
                    output[out_pos++] = '"';
                    i++;
                    break;
                case '\'':
                    output[out_pos++] = '\'';
                    i++;
                    break;
                case 'a':
                    output[out_pos++] = '\a';  /* 响铃 */
                    i++;
                    break;
                case 'b':
                    output[out_pos++] = '\b';  /* 退格 */
                    i++;
                    break;
                case 'f':
                    output[out_pos++] = '\f';  /* 换页 */
                    i++;
                    break;
                case 'v':
                    output[out_pos++] = '\v';  /* 垂直制表符 */
                    i++;
                    break;
                default:
                    /* 不识别的转义序列，保持原样 */
                    output[out_pos++] = input[i];
                    break;
            }
        } else {
            /* 普通字符 */
            output[out_pos++] = input[i];
        }
    }
    
    output[out_pos] = '\0';
    return output;
}

int builtin_export(char **args) {
    /* 如果没有参数，显示所有环境变量 */
    if (args == NULL || args[0] == NULL) {
        print_all_env_vars();
        return 0;
    }
    
    int overall_result = 0;
    
    /* 处理多个变量赋值 */
    for (int i = 0; args[i] != NULL; i++) {
        char *arg = args[i];
        
        /* 查找等号 */
        char *equals = strchr(arg, '=');
        if (equals == NULL) {
            /* 没有等号，只是导出现有变量（如果存在） */
            if (env_var_exists(arg)) {
                /* 变量已存在，无需操作 */
                continue;
            } else {
                /* 变量不存在，设置为空值 */
                if (set_env_var(arg, "") != 0) {
                    print_error("export: failed to export variable");
                    overall_result = -1;
                }
                continue;
            }
        }
        
        /* 分离变量名和值 */
        *equals = '\0';
        char *name = arg;
        char *value = equals + 1;
        
        /* 验证变量名 */
        if (strlen(name) == 0) {
            print_error("export: empty variable name");
            *equals = '=';  /* 恢复原字符串 */
            overall_result = -1;
            continue;
        }
        
        /* 验证变量名格式（只能包含字母、数字和下划线，且不能以数字开头） */
        if (!is_valid_var_name(name)) {
            print_error("export: invalid variable name");
            *equals = '=';  /* 恢复原字符串 */
            overall_result = -1;
            continue;
        }
        
        /* 处理值中的环境变量扩展 */
        char *expanded_value = expand_variables(value);
        char *final_value = (expanded_value != NULL) ? expanded_value : value;
        
        /* 设置环境变量 */
        int result = set_env_var(name, final_value);
        
        /* 清理内存 */
        if (expanded_value != NULL) {
            free(expanded_value);
        }
        
        *equals = '=';  /* 恢复原字符串 */
        
        if (result != 0) {
            print_error("export: failed to set environment variable");
            overall_result = -1;
        }
    }
    
    return overall_result;
}

/**
 * 验证变量名是否有效
 * 变量名只能包含字母、数字和下划线，且不能以数字开头
 */
static int is_valid_var_name(const char *name) {
    if (name == NULL || strlen(name) == 0) {
        return 0;
    }
    
    /* 第一个字符必须是字母或下划线 */
    if (!isalpha(name[0]) && name[0] != '_') {
        return 0;
    }
    
    /* 其余字符必须是字母、数字或下划线 */
    for (size_t i = 1; i < strlen(name); i++) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

/**
 * 内存统计命令
 */
int builtin_memstat(char **args) {
    if (!is_memory_tracking_enabled()) {
        print_error("Memory tracking is disabled");
        return -1;
    }
    
    /* 检查是否要显示内存泄漏 */
    if (args != NULL && args[0] != NULL && strcmp(args[0], "leaks") == 0) {
        int leaks = check_memory_leaks();
        if (leaks > 0) {
            print_memory_leaks();
        } else {
            print_success("No memory leaks detected");
        }
    } else {
        /* 显示内存统计信息 */
        print_memory_stats();
    }
    
    return 0;
}

int builtin_exit(char **args) {
    int exit_code = 0;
    
    /* 如果提供了退出码参数 */
    if (args != NULL && args[0] != NULL) {
        char *endptr;
        long code = strtol(args[0], &endptr, 10);
        
        /* 检查转换是否成功 */
        if (*endptr != '\0' || code < 0 || code > 255) {
            print_error("Invalid exit code. Must be a number between 0 and 255.");
            return -1;
        }
        exit_code = (int)code;
    }
    
    printf("Exiting shell with code %d...\n", exit_code);
    g_shell_state.running = 0;
    g_shell_state.last_exit_status = exit_code;
    return exit_code;
}

int builtin_help(char **args) {
    if (args == NULL || args[0] == NULL) {
        /* 显示所有命令 */
        list_builtin_commands();
        printf("\nType 'help <command>' for detailed information about a specific command.\n");
    } else {
        /* 显示特定命令的帮助 */
        show_command_help(args[0]);
    }
    return 0;
}
