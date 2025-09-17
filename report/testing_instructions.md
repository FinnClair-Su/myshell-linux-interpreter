# MyShell 测试指令和截图说明

本文档提供了MyShell项目的测试指令和需要截图的位置，用于完善实验报告。

## 1. 基本功能测试截图

### 1.1 Shell启动界面
**运行指令：**
```bash
./myshell
```

**截图要求：**
- 显示Shell启动后的命令提示符
- 包含用户名和当前目录信息
- 截图保存为：`shell_startup.png`

**在报告中的位置：** 图 \ref{fig:integration} - 系统集成测试结果

---

### 1.2 内部命令执行演示
**运行指令序列：**
```bash
./myshell
pwd
ls
echo "Hello MyShell"
date
cd /tmp
pwd
ls
exit
```

**截图要求：**
- 显示完整的命令执行过程
- 每个命令的输出结果清晰可见
- 截图保存为：`builtin_commands.png`

**在报告中的位置：** 表格 \ref{tab:builtin_commands} 的补充说明

---

### 1.3 文件操作命令演示
**运行指令序列：**
```bash
./myshell
touch test_file.txt
echo "This is a test file" > test_file.txt  # 注意：如果不支持重定向，手动创建文件内容
cat test_file.txt
cp test_file.txt test_copy.txt
ls -l test*
rm test_file.txt
ls -l test*
exit
```

**截图要求：**
- 显示文件创建、复制、删除的完整过程
- 文件内容显示正确
- 截图保存为：`file_operations.png`

---

## 2. 性能测试截图

### 2.1 单元测试运行结果
**运行指令：**
```bash
make test
```

**截图要求：**
- 显示所有测试用例的执行结果
- 包含测试通过率和执行时间
- 截图保存为：`unit_tests.png`

**在报告中的位置：** 图 \ref{fig:performance} - Shell性能对比分析图

---

### 2.2 内存检查结果
**运行指令：**
```bash
valgrind --leak-check=full --show-leak-kinds=all ./myshell
# 在Shell中执行几个命令后退出
pwd
ls
echo "test"
exit
```

**截图要求：**
- 显示Valgrind的完整输出
- 重点显示"no leaks are possible"信息
- 截图保存为：`memory_check.png`

---

### 2.3 压力测试运行
**运行指令：**
```bash
# 创建压力测试脚本
cat > stress_test.sh << 'EOF'
#!/bin/bash
echo "Starting stress test..."
start_time=$(date +%s)

for i in {1..1000}; do
    ./myshell -c "pwd; echo test; date" > /dev/null
    if [ $((i % 100)) -eq 0 ]; then
        echo "Completed $i commands"
    fi
done

end_time=$(date +%s)
duration=$((end_time - start_time))
echo "Completed 1000 commands in ${duration}s"
echo "Throughput: $((1000 / duration)) commands/second"
EOF

chmod +x stress_test.sh
./stress_test.sh
```

**截图要求：**
- 显示压力测试的进度和最终结果
- 包含吞吐量统计信息
- 截图保存为：`stress_test.png`

---

## 3. 系统集成测试截图

### 3.1 外部命令执行
**运行指令：**
```bash
./myshell
whoami
uname -a
ps aux | head -5  # 如果不支持管道，使用 ps aux
date
which gcc
exit
```

**截图要求：**
- 显示外部命令的正确执行
- 输出结果与系统Shell一致
- 截图保存为：`external_commands.png`

---

### 3.2 环境变量测试
**运行指令：**
```bash
./myshell
echo $HOME
echo $PATH
export MYVAR=test_value
echo $MYVAR
export PATH=/usr/bin:$PATH
echo $PATH
exit
```

**截图要求：**
- 显示环境变量的设置和获取
- 变量扩展功能正常工作
- 截图保存为：`environment_vars.png`

---

## 4. 编译和构建截图

### 4.1 项目编译过程
**运行指令：**
```bash
make clean
make all
ls -la myshell
```

**截图要求：**
- 显示完整的编译过程
- 无编译错误和警告
- 最终生成可执行文件
- 截图保存为：`build_process.png`

---

### 4.2 项目结构展示
**运行指令：**
```bash
tree . -I 'obj|*.o|*.dSYM'
# 如果没有tree命令，使用：
find . -type f -name "*.c" -o -name "*.h" -o -name "Makefile" -o -name "README.md" | sort
```

**截图要求：**
- 显示项目的完整目录结构
- 包含所有源代码文件
- 截图保存为：`project_structure.png`

---

## 5. 代码质量检查截图

### 5.1 静态代码分析
**运行指令：**
```bash
# 如果有cppcheck
cppcheck --enable=all src/

# 或者使用gcc的静态分析
gcc -Wall -Wextra -Wpedantic -fsyntax-only src/*.c
```

**截图要求：**
- 显示代码质量检查结果
- 无严重警告或错误
- 截图保存为：`code_analysis.png`

---

### 5.2 代码覆盖率测试
**运行指令：**
```bash
# 编译带覆盖率信息的版本
gcc -fprofile-arcs -ftest-coverage -o myshell_coverage src/*.c
./test/run_all_tests.sh
gcov src/*.c
```

**截图要求：**
- 显示代码覆盖率统计
- 各模块的覆盖率百分比
- 截图保存为：`code_coverage.png`

---

## 6. 性能基准测试截图

### 6.1 启动时间测试
**运行指令：**
```bash
# 测试启动时间
for i in {1..10}; do
    time ./myshell -c "exit"
done
```

**截图要求：**
- 显示多次启动时间的统计
- 平均启动时间计算
- 截图保存为：`startup_time.png`

---

### 6.2 命令执行效率测试
**运行指令：**
```bash
# 测试命令执行效率
time for i in {1..100}; do ./myshell -c "pwd > /dev/null"; done
time for i in {1..100}; do ./myshell -c "echo test > /dev/null"; done
time for i in {1..100}; do ./myshell -c "date > /dev/null"; done
```

**截图要求：**
- 显示不同命令的执行效率
- 包含时间统计信息
- 截图保存为：`command_efficiency.png`

---

## 截图文件清单

完成所有测试后，应该有以下截图文件：

1. `shell_startup.png` - Shell启动界面
2. `builtin_commands.png` - 内部命令执行
3. `file_operations.png` - 文件操作演示
4. `unit_tests.png` - 单元测试结果
5. `memory_check.png` - 内存检查结果
6. `stress_test.png` - 压力测试结果
7. `external_commands.png` - 外部命令执行
8. `environment_vars.png` - 环境变量测试
9. `build_process.png` - 编译过程
10. `project_structure.png` - 项目结构
11. `code_analysis.png` - 代码分析
12. `code_coverage.png` - 代码覆盖率
13. `startup_time.png` - 启动时间测试
14. `command_efficiency.png` - 命令执行效率

## 注意事项

1. **截图质量**：确保截图清晰，文字可读
2. **完整性**：截图应包含完整的命令和输出
3. **一致性**：使用相同的终端设置和字体
4. **文件命名**：严格按照指定的文件名保存
5. **存储位置**：所有截图保存在 `report/images/` 目录下

## 报告更新

获得截图后，需要在LaTeX报告中更新以下部分：

1. 取消图片占位符的注释
2. 添加实际的图片路径
3. 更新图片说明文字
4. 确保图片编号和引用正确

例如：
```latex
\begin{figure}[H]
    \centering
    \includegraphics[width=\columnwidth]{images/unit_tests.png}
    \caption{单元测试执行结果}
    \label{fig:performance}
\end{figure}
```