# Makefile for MyShell - Linux Shell Interpreter

# 编译器和编译选项
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -g -O2
LDFLAGS = 

# 目录定义
SRCDIR = src
OBJDIR = obj
TESTDIR = test

# 源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = myshell

# 测试相关
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(OBJDIR)/test_%.o)
TEST_TARGET = test_runner

# 默认目标
.PHONY: all clean test install uninstall help debug release

all: $(TARGET)

# 构建主程序
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# 编译源文件
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 编译测试文件
$(OBJDIR)/test_%.o: $(TESTDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "Compiling test $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 构建测试程序
test: $(TEST_TARGET)

$(TEST_TARGET): $(filter-out $(OBJDIR)/main.o, $(OBJECTS)) $(TEST_OBJECTS)
	@echo "Linking test runner..."
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "Running tests..."
	./$(TEST_TARGET)

# 清理构建文件
clean:
	@echo "Cleaning build files..."
	rm -rf $(OBJDIR)
	rm -f $(TARGET) $(TEST_TARGET)
	@echo "Clean complete."

# 安装到系统
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin/..."
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Installation complete."

# 从系统卸载
uninstall:
	@echo "Uninstalling $(TARGET)..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete."

# 调试版本
debug: CFLAGS += -DDEBUG -g3
debug: clean $(TARGET)

# 发布版本
release: CFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

# 代码格式化
format:
	@echo "Formatting code..."
	find $(SRCDIR) -name "*.c" -o -name "*.h" | xargs clang-format -i
	@echo "Format complete."

# 静态分析
analyze:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c99 $(SRCDIR)/
	@echo "Analysis complete."

# 内存检查
memcheck: $(TARGET)
	@echo "Running memory check..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# 显示帮助信息
help:
	@echo "Available targets:"
	@echo "  all       - Build the shell (default)"
	@echo "  clean     - Remove build files"
	@echo "  test      - Build and run tests"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  debug     - Build debug version"
	@echo "  release   - Build optimized release version"
	@echo "  format    - Format source code"
	@echo "  analyze   - Run static analysis"
	@echo "  memcheck  - Run memory leak check"
	@echo "  help      - Show this help message"

# 依赖关系
$(OBJDIR)/main.o: $(SRCDIR)/shell.h
$(OBJDIR)/parser.o: $(SRCDIR)/shell.h
$(OBJDIR)/builtin.o: $(SRCDIR)/shell.h
$(OBJDIR)/external.o: $(SRCDIR)/shell.h
$(OBJDIR)/environment.o: $(SRCDIR)/shell.h
$(OBJDIR)/io.o: $(SRCDIR)/shell.h
$(OBJDIR)/error.o: $(SRCDIR)/shell.h