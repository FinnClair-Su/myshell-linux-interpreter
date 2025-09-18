#include "shell.h"

/**
 * 初始化环境变量
 */
void init_environment(void) {
    /* 初始化HOME环境变量 */
    char *home = getenv("HOME");
    if (home == NULL) {
        home = "/tmp";  /* 默认值 */
    }
    set_env_var("HOME", home);
    
    /* 初始化PATH环境变量 */
    char *path = getenv("PATH");
    if (path == NULL) {
        path = "/bin:/usr/bin:/usr/local/bin";  /* 默认PATH */
    }
    set_env_var("PATH", path);
    
    /* 设置PWD为当前目录 */
    if (g_shell_state.current_dir) {
        set_env_var("PWD", g_shell_state.current_dir);
    }
}

/**
 * 获取环境变量值
 */
char* get_env_var(char *name) {
    if (name == NULL) {
        return NULL;
    }
    
    /* 首先检查内部环境变量表 */
    env_var_t *current = g_shell_state.env_vars;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    
    /* 如果内部表中没有，检查系统环境变量 */
    return getenv(name);
}

/**
 * 设置环境变量
 */
int set_env_var(char *name, char *value) {
    if (name == NULL || value == NULL) {
        return -1;
    }
    
    /* 检查是否已存在 */
    env_var_t *current = g_shell_state.env_vars;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            /* 更新现有变量 */
            TRACKED_FREE(current->value);
            current->value = TRACKED_STRDUP(value, "set_env_var: update value");
            if (current->value == NULL) {
                return -1;
            }
            
            /* 同时更新系统环境变量 */
            setenv(name, value, 1);
            return 0;
        }
        current = current->next;
    }
    
    /* 创建新的环境变量节点 */
    env_var_t *new_var = TRACKED_MALLOC(sizeof(env_var_t), "set_env_var: new variable");
    if (new_var == NULL) {
        return -1;
    }
    
    new_var->name = TRACKED_STRDUP(name, "set_env_var: variable name");
    if (new_var->name == NULL) {
        TRACKED_FREE(new_var);
        return -1;
    }
    
    new_var->value = TRACKED_STRDUP(value, "set_env_var: variable value");
    if (new_var->value == NULL) {
        TRACKED_FREE(new_var->name);
        TRACKED_FREE(new_var);
        return -1;
    }
    
    /* 插入到链表头部 */
    new_var->next = g_shell_state.env_vars;
    g_shell_state.env_vars = new_var;
    
    /* 同时设置系统环境变量 */
    setenv(name, value, 1);
    
    return 0;
}

/**
 * 展开环境变量（完整实现）
 * 支持 $VAR 和 ${VAR} 语法
 */
char* expand_variables(char *input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t input_len = strlen(input);
    size_t result_size = input_len * 2;  /* 初始分配更大的空间 */
    char *result = TRACKED_MALLOC(result_size, "expand_variables: result buffer");
    if (result == NULL) {
        return NULL;
    }
    
    size_t result_pos = 0;
    size_t i = 0;
    
    while (i < input_len) {
        if (input[i] == '$') {
            i++;  /* 跳过 $ */
            
            char var_name[256] = {0};
            size_t var_name_len = 0;
            int is_braced = 0;
            
            /* 检查是否是 ${VAR} 格式 */
            if (i < input_len && input[i] == '{') {
                is_braced = 1;
                i++;  /* 跳过 { */
            }
            
            /* 提取变量名 */
            while (i < input_len && var_name_len < sizeof(var_name) - 1) {
                char c = input[i];
                
                if (is_braced) {
                    if (c == '}') {
                        i++;  /* 跳过 } */
                        break;
                    }
                    var_name[var_name_len++] = c;
                } else {
                    /* 变量名只能包含字母、数字和下划线 */
                    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || 
                        (c >= '0' && c <= '9') || c == '_') {
                        var_name[var_name_len++] = c;
                    } else {
                        break;
                    }
                }
                i++;
            }
            
            /* 获取变量值 */
            if (var_name_len > 0) {
                var_name[var_name_len] = '\0';
                char *var_value = get_env_var(var_name);
                
                if (var_value != NULL) {
                    size_t var_value_len = strlen(var_value);
                    
                    /* 检查是否需要扩展结果缓冲区 */
                    while (result_pos + var_value_len >= result_size) {
                        result_size *= 2;
                        char *new_result = TRACKED_REALLOC(result, result_size, "expand_variables: expand buffer");
                        if (new_result == NULL) {
                            TRACKED_FREE(result);
                            return NULL;
                        }
                        result = new_result;
                    }
                    
                    /* 复制变量值到结果中 */
                    strcpy(result + result_pos, var_value);
                    result_pos += var_value_len;
                }
                /* 如果变量不存在，则不添加任何内容（变量被替换为空字符串） */
            } else {
                /* 如果没有有效的变量名，保留原始的 $ */
                if (result_pos + 1 >= result_size) {
                    result_size *= 2;
                    char *new_result = TRACKED_REALLOC(result, result_size, "expand_variables: expand for dollar");
                    if (new_result == NULL) {
                        TRACKED_FREE(result);
                        return NULL;
                    }
                    result = new_result;
                }
                result[result_pos++] = '$';
            }
        } else {
            /* 普通字符，直接复制 */
            if (result_pos + 1 >= result_size) {
                result_size *= 2;
                char *new_result = TRACKED_REALLOC(result, result_size, "expand_variables: expand for char");
                if (new_result == NULL) {
                    TRACKED_FREE(result);
                    return NULL;
                }
                result = new_result;
            }
            result[result_pos++] = input[i];
            i++;
        }
    }
    
    /* 添加字符串结束符 */
    if (result_pos >= result_size) {
        result_size++;
        char *new_result = TRACKED_REALLOC(result, result_size, "expand_variables: null terminator");
        if (new_result == NULL) {
            TRACKED_FREE(result);
            return NULL;
        }
        result = new_result;
    }
    result[result_pos] = '\0';
    
    return result;
}

/**
 * 获取PATH目录数组
 */
char** get_path_dirs(void) {
    char *path = get_env_var("PATH");
    if (path == NULL) {
        return NULL;
    }
    
    char *path_copy = TRACKED_STRDUP(path, "get_path_dirs: path copy");
    if (path_copy == NULL) {
        return NULL;
    }
    
    /* 计算目录数量 */
    int dir_count = 1;
    for (char *p = path_copy; *p; p++) {
        if (*p == ':') {
            dir_count++;
        }
    }
    
    /* 分配目录数组 */
    char **dirs = TRACKED_MALLOC((dir_count + 1) * sizeof(char*), "get_path_dirs: directory array");
    if (dirs == NULL) {
        TRACKED_FREE(path_copy);
        return NULL;
    }
    
    /* 分割PATH */
    int i = 0;
    char *dir = strtok(path_copy, ":");
    while (dir != NULL && i < dir_count) {
        dirs[i] = TRACKED_STRDUP(dir, "get_path_dirs: directory path");
        if (dirs[i] == NULL) {
            /* 清理已分配的内存 */
            for (int j = 0; j < i; j++) {
                TRACKED_FREE(dirs[j]);
            }
            TRACKED_FREE(dirs);
            TRACKED_FREE(path_copy);
            return NULL;
        }
        i++;
        dir = strtok(NULL, ":");
    }
    dirs[i] = NULL;  /* NULL终止 */
    
    TRACKED_FREE(path_copy);
    return dirs;
}
/**
 * 释放PATH目录数组
 */
void free_path_dirs(char **dirs) {
    if (dirs == NULL) {
        return;
    }
    
    for (int i = 0; dirs[i] != NULL; i++) {
        TRACKED_FREE(dirs[i]);
    }
    TRACKED_FREE(dirs);
}

/**
 * 清理所有环境变量
 */
void cleanup_environment(void) {
    log_info("Cleaning up environment variables");
    
    env_var_t *current = g_shell_state.env_vars;
    int count = 0;
    
    while (current) {
        env_var_t *next = current->next;
        TRACKED_FREE(current->name);
        TRACKED_FREE(current->value);
        TRACKED_FREE(current);
        current = next;
        count++;
    }
    
    g_shell_state.env_vars = NULL;
    
    char cleanup_msg[128];
    snprintf(cleanup_msg, sizeof(cleanup_msg), "Cleaned up %d environment variables", count);
    log_info(cleanup_msg);
}

/**
 * 打印所有环境变量（用于调试）
 */
void print_all_env_vars(void) {
    env_var_t *current = g_shell_state.env_vars;
    printf("Internal environment variables:\n");
    while (current) {
        printf("%s=%s\n", current->name, current->value);
        current = current->next;
    }
}

/**
 * 检查环境变量是否存在
 */
int env_var_exists(char *name) {
    if (name == NULL) {
        return 0;
    }
    
    env_var_t *current = g_shell_state.env_vars;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return 1;
        }
        current = current->next;
    }
    
    /* 检查系统环境变量 */
    return getenv(name) != NULL;
}

/**
 * 删除环境变量
 */
int unset_env_var(char *name) {
    if (name == NULL) {
        return -1;
    }
    
    env_var_t *current = g_shell_state.env_vars;
    env_var_t *prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            /* 从链表中移除 */
            if (prev) {
                prev->next = current->next;
            } else {
                g_shell_state.env_vars = current->next;
            }
            
            /* 释放内存 */
            TRACKED_FREE(current->name);
            TRACKED_FREE(current->value);
            TRACKED_FREE(current);
            
            /* 从系统环境变量中删除 */
            unsetenv(name);
            
            return 0;
        }
        prev = current;
        current = current->next;
    }
    
    /* 如果内部表中没有，尝试从系统环境变量中删除 */
    unsetenv(name);
    return 0;
}
