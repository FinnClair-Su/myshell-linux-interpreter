#include "shell.h"

/**
 * 解析命令行输入
 */
command_t* parse_command(char *input) {
    LOG_FUNCTION_ENTRY("parse_command");
    
    if (input == NULL || strlen(input) == 0) {
        handle_error(ERROR_INVALID_ARGUMENT, "parse_command: empty input");
        return NULL;
    }
    
    /* 检查输入长度 */
    if (strlen(input) >= MAX_INPUT_SIZE) {
        handle_error(ERROR_BUFFER_OVERFLOW, "parse_command: input too long");
        return NULL;
    }
    
    command_t *cmd = TRACKED_MALLOC(sizeof(command_t), "parse_command: command structure");
    if (cmd == NULL) {
        return NULL;
    }
    
    /* 初始化命令结构体 */
    cmd->command = NULL;
    cmd->args = NULL;
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    
    /* 分词处理 */
    int token_count;
    char **tokens = tokenize_input(input, &token_count);
    if (tokens == NULL || token_count == 0) {
        handle_error(ERROR_PARSING, "parse_command: tokenization failed");
        free(cmd);
        return NULL;
    }
    
    /* 设置命令名 */
    cmd->command = TRACKED_STRDUP(tokens[0], "parse_command: command name");
    if (cmd->command == NULL) {
        TRACKED_FREE(cmd);
        /* 释放tokens */
        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        return NULL;
    }
    
    /* 设置参数 */
    cmd->argc = token_count;
    cmd->args = TRACKED_MALLOC((token_count + 1) * sizeof(char*), "parse_command: arguments array");
    if (cmd->args == NULL) {
        TRACKED_FREE(cmd->command);
        TRACKED_FREE(cmd);
        /* 释放tokens */
        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        return NULL;
    }
    
    for (int i = 0; i < token_count; i++) {
        cmd->args[i] = TRACKED_STRDUP(tokens[i], "parse_command: argument");
        if (cmd->args[i] == NULL) {
            /* 清理已分配的内存 */
            for (int j = 0; j < i; j++) {
                TRACKED_FREE(cmd->args[j]);
            }
            TRACKED_FREE(cmd->args);
            TRACKED_FREE(cmd->command);
            TRACKED_FREE(cmd);
            /* 释放tokens */
            for (int k = 0; k < token_count; k++) {
                free(tokens[k]);
            }
            free(tokens);
            return NULL;
        }
    }
    cmd->args[token_count] = NULL;  /* NULL终止 */
    
    /* 释放临时tokens */
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    LOG_FUNCTION_EXIT("parse_command");
    return cmd;
}

/**
 * 释放命令结构体内存
 */
void free_command(command_t *cmd) {
    if (cmd == NULL) {
        return;
    }
    
    LOG_FUNCTION_ENTRY("free_command");
    
    /* 使用跟踪释放函数 */
    if (cmd->command) {
        TRACKED_FREE(cmd->command);
        cmd->command = NULL;
    }
    
    if (cmd->args) {
        for (int i = 0; i < cmd->argc; i++) {
            if (cmd->args[i]) {
                TRACKED_FREE(cmd->args[i]);
                cmd->args[i] = NULL;
            }
        }
        TRACKED_FREE(cmd->args);
        cmd->args = NULL;
    }
    
    if (cmd->input_file) {
        TRACKED_FREE(cmd->input_file);
        cmd->input_file = NULL;
    }
    
    if (cmd->output_file) {
        TRACKED_FREE(cmd->output_file);
        cmd->output_file = NULL;
    }
    
    TRACKED_FREE(cmd);
    
    LOG_FUNCTION_EXIT("free_command");
}

/**
 * 将输入字符串分词
 */
char** tokenize_input(char *input, int *token_count) {
    LOG_FUNCTION_ENTRY("tokenize_input");
    
    if (input == NULL) {
        handle_error(ERROR_INVALID_ARGUMENT, "tokenize_input: input is NULL");
        return NULL;
    }
    if (token_count == NULL) {
        handle_error(ERROR_INVALID_ARGUMENT, "tokenize_input: token_count is NULL");
        return NULL;
    }
    
    *token_count = 0;
    
    /* 检查输入长度 */
    size_t input_len = strlen(input);
    if (input_len >= MAX_INPUT_SIZE) {
        handle_error(ERROR_BUFFER_OVERFLOW, "tokenize_input: input too long");
        return NULL;
    }
    
    char **tokens = TRACKED_MALLOC(MAX_ARGS * sizeof(char*), "tokenize_input: tokens array");
    if (tokens == NULL) {
        return NULL;
    }
    
    char *input_copy = TRACKED_STRDUP(input, "tokenize_input: input copy");
    if (input_copy == NULL) {
        TRACKED_FREE(tokens);
        return NULL;
    }
    
    int count = 0;
    char *token = strtok(input_copy, " \t\n\r");
    
    while (token != NULL && count < MAX_ARGS - 1) {
        /* 检查token长度 */
        if (strlen(token) >= MAX_INPUT_SIZE) {
            handle_error(ERROR_BUFFER_OVERFLOW, "tokenize_input: token too long");
            /* 清理已分配的内存 */
            for (int i = 0; i < count; i++) {
                TRACKED_FREE(tokens[i]);
            }
            TRACKED_FREE(tokens);
            TRACKED_FREE(input_copy);
            return NULL;
        }
        
        tokens[count] = TRACKED_STRDUP(token, "tokenize_input: token");
        if (tokens[count] == NULL) {
            /* 清理已分配的内存 */
            for (int i = 0; i < count; i++) {
                TRACKED_FREE(tokens[i]);
            }
            TRACKED_FREE(tokens);
            TRACKED_FREE(input_copy);
            return NULL;
        }
        count++;
        token = strtok(NULL, " \t\n\r");
    }
    
    TRACKED_FREE(input_copy);
    *token_count = count;
    
    LOG_FUNCTION_EXIT("tokenize_input");
    return tokens;
}
