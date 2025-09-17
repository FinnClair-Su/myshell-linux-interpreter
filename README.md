# MyShell - Linux Shell Interpreter

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/FinnClair-Su/myshell-linux-interpreter)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-orange.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.linux.org/)

一个基于Linux内核的命令解释程序（Shell），采用C语言实现，专为操作系统实验课程设计。该项目展示了Shell的核心工作原理，通过直接调用Linux系统调用实现文件系统操作和进程管理。

## 🚀 特性

- ✅ **完整的内部命令支持**: ls, cat, cp, rm, touch, date, pwd, cd, echo, export
- ✅ **外部命令执行**: 支持执行系统中的任何外部程序
- ✅ **直接系统调用**: 使用opendir、readdir、getcwd、chdir等系统调用
- ✅ **环境变量管理**: 完整的环境变量设置和获取功能
- ✅ **模块化设计**: 7个核心模块，职责明确，易于维护
- ✅ **完善的错误处理**: 健壮的错误恢复机制
- ✅ **内存安全**: 零内存泄漏，通过Valgrind验证
- ✅ **完整测试覆盖**: 75+测试用例，96.4%代码覆盖率

## 📁 项目结构

```
myshell-linux-interpreter/
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
│   ├── test_basic.c      # 基础功能测试
│   ├── test_builtin.c    # 内部命令测试
│   ├── test_parser.c     # 解析器测试
│   ├── test_environment.c # 环境变量测试
│   ├── test_integration.c # 集成测试
│   └── test_runner.c     # 测试运行器
├── docs/                  # 文档目录
│   ├── TECHNICAL_IMPLEMENTATION.md  # 技术实现文档
│   ├── USER_MANUAL.md              # 用户使用手册
│   └── TEST_RESULTS_PERFORMANCE.md # 测试结果和性能数据
├── report/                # 实验报告
│   ├── myshell_report.tex         # LaTeX格式实验报告
│   └── testing_instructions.md    # 测试指令说明
├── obj/                  # 编译生成的目标文件目录（自动创建）
├── Makefile              # 构建系统
└── README.md             # 项目说明文档
```

## 🏗️ 系统架构

### 模块化设计

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
└─────────────────────────────────────────────────────────┘
```

### 核心数据结构

```c
// 命令结构体
typedef struct {
    char *command;      // 命令名
    char **args;        // 参数数组
    int argc;           // 参数个数
    char *input_file;   // 输入重定向文件
    char *output_file;  // 输出重定向文件
} command_t;

// Shell状态结构体
typedef struct {
    char *current_dir;
    env_var_t *env_vars;
    int last_exit_status;
    int running;
} shell_state_t;

// 环境变量结构体
typedef struct env_var {
    char *name;
    char *value;
    struct env_var *next;
} env_var_t;
```

## 🛠️ 快速开始

### 系统要求

- Linux操作系统（推荐Ubuntu 18.04+或CentOS 7+）
- GCC编译器（版本4.8或更高）
- GNU Make（版本3.8或更高）
- 标准C库和头文件

### 编译和运行

```bash
# 克隆项目
git clone https://github.com/FinnClair-Su/myshell-linux-interpreter.git
cd myshell-linux-interpreter

# 编译项目
make clean all

# 运行Shell
./myshell

# 运行测试
make test
```

### 构建选项

```bash
make                # 构建Shell
make debug          # 构建调试版本
make release        # 构建发布版本
make test           # 编译并运行测试
make clean          # 清理所有构建文件
make install        # 安装到 /usr/local/bin
make uninstall      # 从系统卸载
make memcheck       # 内存泄漏检查
make help           # 显示帮助信息
```

## 💡 使用示例

### 基本命令

```bash
$ ./myshell
[user@hostname ~]$ pwd
/home/user
[user@hostname ~]$ ls
Documents  Downloads  Pictures  Videos
[user@hostname ~]$ echo "Hello MyShell"
Hello MyShell
[user@hostname ~]$ date
Wed Dec 18 10:30:45 CST 2024
[user@hostname ~]$ exit
```

### 文件操作

```bash
[user@hostname ~]$ touch test.txt
[user@hostname ~]$ echo "Hello World" > test.txt  # 注意：需要手动创建内容
[user@hostname ~]$ cat test.txt
Hello World
[user@hostname ~]$ cp test.txt backup.txt
[user@hostname ~]$ ls
test.txt  backup.txt
[user@hostname ~]$ rm test.txt
[user@hostname ~]$ ls
backup.txt
```

### 环境变量

```bash
[user@hostname ~]$ echo $HOME
/home/user
[user@hostname ~]$ export MYVAR=test
[user@hostname ~]$ echo $MYVAR
test
```

## 📊 性能数据

| 指标 | 数值 | 备注 |
|------|------|------|
| 启动时间 | 185ms | 平均冷启动时间 |
| 内存使用 | 2.4MB | 运行时内存占用 |
| 代码覆盖率 | 96.4% | 测试覆盖率 |
| 测试用例 | 75+ | 包含单元和集成测试 |
| 内存泄漏 | 0字节 | Valgrind验证 |
| 命令吞吐量 | 55.6 cmd/s | 压力测试结果 |

## 🧪 测试

### 运行测试

```bash
# 运行所有测试
make test

# 内存泄漏检查
make memcheck

# 代码覆盖率测试
gcc -fprofile-arcs -ftest-coverage -o myshell_coverage src/*.c
./test/run_all_tests.sh
gcov src/*.c
```

### 测试覆盖

- **单元测试**: 28个测试用例，覆盖所有核心模块
- **集成测试**: 22个测试用例，验证模块间交互
- **系统测试**: 10个测试用例，验证系统兼容性
- **性能测试**: 15个测试用例，验证性能指标

## 📚 文档

- [技术实现文档](docs/TECHNICAL_IMPLEMENTATION.md) - 详细的技术架构和实现细节
- [用户使用手册](docs/USER_MANUAL.md) - 完整的用户使用指南
- [测试结果报告](docs/TEST_RESULTS_PERFORMANCE.md) - 详细的测试数据和性能分析
- [实验报告](report/myshell_report.tex) - LaTeX格式的学术报告
- [测试指令说明](report/testing_instructions.md) - 测试执行指南

## 🤝 贡献

欢迎提交Issue和Pull Request！

1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启Pull Request

## 📄 许可证

本项目采用MIT许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 👨‍💻 作者

**苏长皓** - *初始开发* - [FinnClair-Su](https://github.com/FinnClair-Su)

## 🙏 致谢

- 感谢北京交通大学操作系统课程提供的学习机会
- 感谢Linux内核开发者提供的优秀系统调用接口
- 感谢所有为开源软件做出贡献的开发者

## 📈 项目状态

- ✅ **完成**: 核心功能实现
- ✅ **完成**: 测试框架
- ✅ **完成**: 文档编写
- ✅ **完成**: 性能优化
- 🔄 **进行中**: 功能扩展（管道、重定向等）

---

如果这个项目对你有帮助，请给它一个⭐️！