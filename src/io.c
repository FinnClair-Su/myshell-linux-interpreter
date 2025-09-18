#include "shell.h"

/* 输入缓冲区管理 */
static char input_buffer[MAX_INPUT_SIZE];
static size_t buffer_pos = 0;

/**
 * 显示命令提示符
 * 根据用户权限显示不同的提示符样式
 */
void display_prompt(void) {
    char *user = getenv("USER");
    if (user == NULL) {
        user = "user";
    }
    
    char *hostname = getenv("HOSTNAME");
    if (hostname == NULL) {
        hostname = "localhost";
    }
    
    /* 获取当前目录的基名 */
    char *current_dir = g_shell_state.current_dir;
    char *basename_dir = current_dir;
    if (current_dir) {
        char *last_slash = strrchr(current_dir, '/');
        if (last_slash && *(last_slash + 1) != '\0') {
            basename_dir = last_slash + 1;
        } else if (strcmp(current_dir, "/") != 0) {
            basename_dir = current_dir;
        } else {
            basename_dir = "/";
        }
    }
    
    /* 检查是否为root用户，显示不同的提示符 */
    char prompt_char = (getuid() == 0) ? '#' : '$';
    
    /* 显示彩色提示符（如果终端支持） */
    if (isatty(STDOUT_FILENO)) {
        printf("\033[1;32m[%s@%s \033[1;34m%s\033[1;32m]%c\033[0m ", 
               user, hostname, basename_dir ? basename_dir : "unknown", prompt_char);
    } else {
        printf("[%s@%s %s]%c ", user, hostname, basename_dir ? basename_dir : "unknown", prompt_char);
    }
    
    fflush(stdout);
}

/**
 * 验证输入字符是否安全
 */
static int is_safe_char(char c) {
    /* 允许可打印字符、空格、制表符 */
    return (c >= 32 && c <= 126) || c == '\t';
}

/**
 * 清理输入缓冲区
 */
static void clear_input_buffer(void) {
    buffer_pos = 0;
    memset(input_buffer, 0, MAX_INPUT_SIZE);
}

/**
 * 验证输入安全性
 */
static int validate_input(const char *input) {
    if (input == NULL) {
        return 0;
    }
    
    size_t len = strlen(input);
    
    /* 检查输入长度 */
    if (len == 0) {
        return 1;  /* 空输入是有效的 */
    }
    
    if (len >= MAX_INPUT_SIZE - 1) {
        print_error("Input too long");
        return 0;
    }
    
    /* 检查是否包含不安全字符 */
    for (size_t i = 0; i < len; i++) {
        if (!is_safe_char(input[i])) {
            print_error("Input contains invalid characters");
            return 0;
        }
    }
    
    /* 检查是否包含潜在的注入攻击模式 */
    if (strstr(input, "../") != NULL) {
        print_error("Path traversal attempt detected");
        return 0;
    }
    
    /* 检查是否包含空字节 */
    if (strlen(input) != len) {
        print_error("Null byte in input detected");
        return 0;
    }
    
    return 1;
}

/**
 * 读取用户输入（带缓冲和验证）
 */
char* read_input(void) {
    char *input = TRACKED_MALLOC(MAX_INPUT_SIZE, "read_input: input buffer");
    if (input == NULL) {
        return NULL;
    }
    
    /* 清理输入缓冲区 */
    clear_input_buffer();
    
    /* 读取输入 */
    if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        TRACKED_FREE(input);
        if (feof(stdin)) {
            return NULL;  /* EOF */
        } else {
            print_error("Failed to read input");
            return NULL;  /* 读取错误 */
        }
    }
    
    /* 移除换行符 */
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
        len--;
    }
    
    /* 验证输入安全性 */
    if (!validate_input(input)) {
        TRACKED_FREE(input);
        return NULL;
    }
    
    /* 如果输入过长，清理stdin缓冲区 */
    if (len == MAX_INPUT_SIZE - 2) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            /* 丢弃剩余字符 */
        }
        print_error("Input truncated due to length limit");
    }
    
    return input;
}

/**
 * 格式化并打印错误信息
 */
void print_error(char *message) {
    if (message == NULL) {
        return;
    }
    
    /* 使用红色文本显示错误（如果终端支持） */
    if (isatty(STDERR_FILENO)) {
        fprintf(stderr, "\033[1;31mError:\033[0m %s\n", message);
    } else {
        fprintf(stderr, "Error: %s\n", message);
    }
    
    fflush(stderr);
}

/**
 * 格式化并打印输出信息
 */
void print_output(char *message) {
    if (message == NULL) {
        return;
    }
    
    printf("%s\n", message);
    fflush(stdout);
}

/**
 * 打印警告信息
 */
void print_warning(char *message) {
    if (message == NULL) {
        return;
    }
    
    /* 使用黄色文本显示警告（如果终端支持） */
    if (isatty(STDERR_FILENO)) {
        fprintf(stderr, "\033[1;33mWarning:\033[0m %s\n", message);
    } else {
        fprintf(stderr, "Warning: %s\n", message);
    }
    
    fflush(stderr);
}

/**
 * 打印成功信息
 */
void print_success(char *message) {
    if (message == NULL) {
        return;
    }
    
    /* 使用绿色文本显示成功信息（如果终端支持） */
    if (isatty(STDOUT_FILENO)) {
        printf("\033[1;32m%s\033[0m\n", message);
    } else {
        printf("%s\n", message);
    }
    
    fflush(stdout);
}

/**
 * 格式化输出（类似printf）
 */
void print_formatted(const char *format, ...) {
    if (format == NULL) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    fflush(stdout);
}

/**
 * 安全地打印字符串（防止格式字符串攻击）
 */
void print_safe(const char *str) {
    if (str == NULL) {
        return;
    }
    
    /* 使用%s格式化字符串，防止格式字符串攻击 */
    printf("%s", str);
    fflush(stdout);
}

/**
 * 读取单个字符（无回显）
 */
int read_char_noecho(void) {
    struct termios old_termios, new_termios;
    int ch;
    
    /* 获取当前终端设置 */
    if (tcgetattr(STDIN_FILENO, &old_termios) != 0) {
        return -1;
    }
    
    /* 设置新的终端模式（无回显，无缓冲） */
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ECHO | ICANON);
    
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) != 0) {
        return -1;
    }
    
    /* 读取字符 */
    ch = getchar();
    
    /* 恢复原始终端设置 */
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    
    return ch;
}

/**
 * 确认用户操作（y/n）
 */
int confirm_action(const char *message) {
    if (message == NULL) {
        return 0;
    }
    
    printf("%s (y/n): ", message);
    fflush(stdout);
    
    int ch = read_char_noecho();
    printf("%c\n", ch);  /* 显示用户输入的字符 */
    
    return (ch == 'y' || ch == 'Y');
}

/**
 * 清屏函数
 */
void clear_screen(void) {
    if (isatty(STDOUT_FILENO)) {
        printf("\033[2J\033[H");  /* ANSI转义序列清屏 */
    } else {
        /* 如果不是终端，输出换行符 */
        for (int i = 0; i < 50; i++) {
            printf("\n");
        }
    }
    fflush(stdout);
}

/**
 * 设置光标位置
 */
void set_cursor_position(int row, int col) {
    if (isatty(STDOUT_FILENO)) {
        printf("\033[%d;%dH", row, col);
        fflush(stdout);
    }
}

/**
 * 获取终端大小
 */
int get_terminal_size(int *rows, int *cols) {
    struct winsize ws;
    
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        /* 如果获取失败，使用默认值 */
        if (rows) *rows = 24;
        if (cols) *cols = 80;
        return -1;
    }
    
    if (rows) *rows = ws.ws_row;
    if (cols) *cols = ws.ws_col;
    
    return 0;
}

/**
 * 打印分隔线
 */
void print_separator(char ch, int length) {
    if (length <= 0) {
        int cols;
        get_terminal_size(NULL, &cols);
        length = cols > 0 ? cols : 80;
    }
    
    for (int i = 0; i < length; i++) {
        putchar(ch);
    }
    putchar('\n');
    fflush(stdout);
}
/* End of io.c */
