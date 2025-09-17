# MyShell - Linux Shell Interpreter

一个基于Linux内核的命令解释程序（Shell），用于操作系统实验课程。

## 项目结构

```
.
├── src/                    # 源代码目录
│   ├── shell.h            # 主头文件，包含所有数据结构和函数声明
│   ├── main.c             # 主程序入口和Shell主循环
│   ├── parser.c           # 命令解析器
│   ├── builtin.c          # 内部命令实现
│   ├── external.c         # 外部命令执行器
│   ├── environment.c      # 环境变量管理
│   ├── io.c              # 输入输出处理
│   └── error.c           # 错误处理
├── test/                  # 测试代码目录
│   └── test_basic.c      # 基础功能测试
├── obj/                  # 编译生成的目标文件目录（自动创建）
├── Makefile              # 构建系统
└── README.md             # 项目说明文档
```

## 核心数据结构

### command_t - 命令结构体
```c
typedef struct {
    char *command;      // 命令名
    char **args;        // 参数数组
    int argc;           // 参数个数
    char *input_file;   // 输入重定向文件
    char *output_file;  // 输出重定向文件
} command_t;
```

### shell_state_t - Shell状态结构体
```c
typedef struct {
    char *current_dir;
    env_var_t *env_vars;
    int last_exit_status;
    int running;
} shell_state_t;
```

### env_var_t - 环境变量结构体
```c
typedef struct env_var {
    char *name;
    char *value;
    struct env_var *next;
} env_var_t;
```

## 构建和运行

### 编译项目
```bash
make                # 构建Shell
make debug          # 构建调试版本
make release        # 构建发布版本
```

### 运行测试
```bash
make test           # 编译并运行测试
```

### 清理构建文件
```bash
make clean          # 清理所有构建文件
```

### 安装和卸载
```bash
make install        # 安装到 /usr/local/bin
make uninstall      # 从系统卸载
```

### 其他有用的命令
```bash
make format         # 格式化代码
make analyze        # 静态代码分析
make memcheck       # 内存泄漏检查
make help           # 显示帮助信息
```

## 功能特性

### 已实现的框架
- ✅ 项目结构和构建系统
- ✅ 基本数据结构定义
- ✅ 命令解析框架
- ✅ 内部命令注册表
- ✅ 外部命令执行框架
- ✅ 环境变量管理框架
- ✅ 错误处理系统
- ✅ 输入输出处理

### 待实现的功能
- ⏳ 具体内部命令实现（ls, cat, cp, rm, touch, date, pwd, cd, echo, export）
- ⏳ 系统调用直接实现
- ⏳ 完整的环境变量扩展
- ⏳ 信号处理
- ⏳ 更多测试用例

## 开发说明

### 编译要求
- GCC编译器
- Linux操作系统
- 支持C99标准

### 代码规范
- 使用C99标准
- 遵循Linux内核编码风格
- 所有函数都有详细注释
- 严格的内存管理和错误处理

### 测试
项目包含基础的单元测试框架，可以通过 `make test` 运行测试。

## 许可证

本项目用于教育目的，遵循MIT许可证。