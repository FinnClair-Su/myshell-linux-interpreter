# MyShell 用户使用手册

## 概述

MyShell 是一个基于Linux内核的命令解释程序（Shell），专为操作系统实验课程设计。它提供了完整的Shell功能，包括内部命令处理、外部命令执行、环境变量管理等核心特性。

## 安装和启动

### 系统要求

- Linux操作系统（推荐Ubuntu 18.04+或CentOS 7+）
- GCC编译器（版本4.8或更高）
- GNU Make（版本3.8或更高）
- 标准C库和头文件

### 编译安装

1. **获取源代码**
   ```bash
   # 如果从Git仓库获取
   git clone <repository-url>
   cd myshell
   ```

2. **编译项目**
   ```bash
   make clean all
   ```

3. **验证安装**
   ```bash
   ./myshell
   ```

4. **系统安装（可选）**
   ```bash
   sudo make install
   ```

### 启动Shell

- **直接启动**: `./myshell`
- **系统安装后**: `myshell`
- **作为登录Shell**: 参见Shell替换指南

## 基本使用

### 命令提示符

MyShell启动后会显示命令提示符，格式为：
```
[用户名@主机名 当前目录]$ 
```

### 命令输入

- 在提示符后输入命令和参数
- 按Enter键执行命令
- 使用Ctrl+C中断当前命令
- 输入`exit`退出Shell

## 内部命令

MyShell提供以下内部命令：

### 文件和目录操作

#### `ls [目录]`
列出目录内容
```bash
ls                    # 列出当前目录
ls /home             # 列出指定目录
ls -l                # 详细列表（如果支持）
```

#### `pwd`
显示当前工作目录
```bash
pwd                  # 输出当前目录路径
```

#### `cd [目录]`
切换工作目录
```bash
cd                   # 切换到用户主目录
cd /home/user        # 切换到指定目录
cd ..                # 切换到上级目录
cd -                 # 切换到上次目录
```

### 文件操作

#### `cat [文件名]`
显示文件内容
```bash
cat file.txt         # 显示文件内容
cat file1.txt file2.txt  # 显示多个文件
```

#### `touch [文件名]`
创建空文件或更新文件时间戳
```bash
touch newfile.txt    # 创建新文件
touch existing.txt   # 更新时间戳
```

#### `cp [源文件] [目标文件]`
复制文件
```bash
cp source.txt dest.txt     # 复制文件
cp file.txt /tmp/          # 复制到目录
```

#### `rm [文件名]`
删除文件
```bash
rm file.txt          # 删除文件
rm -f file.txt       # 强制删除（如果支持）
```

### 系统信息

#### `date`
显示当前日期和时间
```bash
date                 # 显示当前时间
```

#### `echo [文本]`
输出文本
```bash
echo "Hello World"   # 输出文本
echo $HOME           # 输出环境变量
```

### 环境变量

#### `export [变量名=值]`
设置环境变量
```bash
export PATH=/usr/bin:$PATH    # 设置PATH
export MYVAR=value            # 设置自定义变量
```

### Shell控制

#### `exit`
退出Shell
```bash
exit                 # 正常退出
exit 0               # 以指定状态码退出
```

## 外部命令

MyShell可以执行系统中的任何外部程序：

### 系统命令
```bash
whoami              # 显示当前用户
uname -a            # 显示系统信息
ps aux              # 显示进程列表
grep pattern file   # 文本搜索
```

### 程序执行
```bash
./my_program        # 执行当前目录程序
/usr/bin/vim file   # 执行绝对路径程序
python script.py    # 执行脚本
```

## 环境变量

### 预设环境变量

MyShell自动设置以下环境变量：

- **HOME**: 用户主目录
- **PATH**: 可执行文件搜索路径
- **PWD**: 当前工作目录
- **USER**: 当前用户名

### 变量使用

```bash
echo $HOME          # 显示主目录
echo $PATH          # 显示搜索路径
export EDITOR=vim   # 设置编辑器
```

### 变量扩展

MyShell支持基本的变量扩展：
```bash
echo "My home is $HOME"
echo "Current user: $USER"
```

## 错误处理

### 常见错误信息

- **"command not found"**: 命令不存在或不在PATH中
- **"Permission denied"**: 权限不足
- **"No such file or directory"**: 文件或目录不存在
- **"Invalid argument"**: 参数错误

### 错误恢复

- 大多数错误不会导致Shell退出
- 检查命令拼写和参数
- 验证文件路径和权限
- 使用`pwd`确认当前目录

## 高级功能

### 信号处理

- **Ctrl+C**: 中断当前命令
- **Ctrl+Z**: 暂停当前命令（基本支持）
- **Ctrl+D**: EOF信号（在某些情况下）

### 命令历史

MyShell提供基本的命令执行，但不包含历史记录功能。

### 自动补全

当前版本不支持Tab自动补全功能。

## 性能和限制

### 性能特征

- **启动时间**: < 1秒
- **命令执行**: 内部命令 < 50ms，外部命令 < 100ms
- **内存使用**: 启动时 < 5MB，正常使用 < 50MB
- **吞吐量**: > 10命令/秒

### 已知限制

- 不支持管道（|）和重定向（>, <）
- 不支持命令历史和自动补全
- 不支持作业控制（后台任务）
- 不支持别名和函数定义
- 不支持复杂的Shell脚本语法

## 故障排除

### 常见问题

1. **Shell无法启动**
   - 检查可执行权限：`chmod +x myshell`
   - 验证依赖库：`ldd myshell`
   - 检查系统兼容性

2. **命令不工作**
   - 验证PATH环境变量：`echo $PATH`
   - 检查命令拼写
   - 确认文件权限

3. **文件操作失败**
   - 检查文件是否存在
   - 验证读写权限
   - 确认磁盘空间

4. **性能问题**
   - 检查系统资源使用
   - 验证内存泄漏
   - 监控CPU使用率

### 调试模式

编译调试版本：
```bash
make debug
./myshell
```

使用调试工具：
```bash
gdb ./myshell
valgrind --leak-check=full ./myshell
strace -o trace.log ./myshell
```

## 最佳实践

### 日常使用

1. **定期检查环境变量**
   ```bash
   echo $PATH
   echo $HOME
   ```

2. **使用绝对路径执行程序**
   ```bash
   /usr/bin/vim file.txt
   ```

3. **验证文件操作**
   ```bash
   ls -l file.txt    # 检查文件权限
   pwd               # 确认当前目录
   ```

### 安全考虑

1. **避免执行未知程序**
2. **检查文件权限**
3. **使用相对路径时要小心**
4. **定期备份重要文件**

## 与其他Shell的比较

### 与Bash的差异

| 功能 | MyShell | Bash |
|------|---------|------|
| 基本命令 | ✅ | ✅ |
| 管道重定向 | ❌ | ✅ |
| 命令历史 | ❌ | ✅ |
| 自动补全 | ❌ | ✅ |
| 脚本支持 | ❌ | ✅ |
| 作业控制 | ❌ | ✅ |

### 适用场景

MyShell适合：
- 操作系统学习和实验
- 基本的命令行操作
- 系统调用学习
- Shell原理理解

不适合：
- 复杂的脚本编写
- 生产环境使用
- 高级Shell功能需求

## 技术支持

### 获取帮助

1. **查看文档**
   - 用户手册（本文档）
   - 技术实现文档
   - 测试指南

2. **调试信息**
   - 使用调试版本
   - 查看错误日志
   - 运行测试套件

3. **社区支持**
   - 查看项目文档
   - 提交问题报告
   - 参与讨论

### 报告问题

报告问题时请包含：
- 操作系统版本
- 编译器版本
- 错误信息
- 重现步骤
- 系统环境信息

## 版本信息

- **当前版本**: 1.0.0
- **发布日期**: 2024年
- **兼容性**: Linux 2.6+
- **许可证**: MIT

## 更新日志

### 版本1.0.0
- 实现基本Shell功能
- 支持内部命令
- 外部命令执行
- 环境变量管理
- 错误处理系统
- 完整测试套件

---

*本手册描述了MyShell的基本使用方法。如需了解技术实现细节，请参考技术实现文档。*