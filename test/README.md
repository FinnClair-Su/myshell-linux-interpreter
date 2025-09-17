# Unit Tests for Linux Shell Interpreter

This directory contains comprehensive unit tests for the Linux Shell Interpreter project.

## Test Structure

### 1. Command Parser Tests (`test_parser.c`)
Tests the command parsing functionality including:
- Basic command parsing with arguments
- Empty input handling
- Whitespace and special character handling
- Tokenization functionality
- Memory management
- Boundary conditions and error cases

### 2. Internal Command Tests (`test_builtin.c`)
Tests all internal (builtin) commands including:
- `pwd` - Print working directory
- `cd` - Change directory
- `ls` - List directory contents
- `cat` - Display file contents
- `cp` - Copy files
- `rm` - Remove files
- `touch` - Create empty files
- `echo` - Display text
- `date` - Show current date/time
- `export` - Set environment variables
- Command recognition and execution interface

### 3. Environment Variable Tests (`test_environment.c`)
Tests environment variable management including:
- Setting and getting environment variables
- Variable expansion functionality
- PATH directory parsing
- Environment initialization
- Boundary conditions and error handling
- Variable existence checking
- Variable deletion (unset)

## Running Tests

### Run All Tests
```bash
./run_unit_tests.sh
```

### Run Individual Test Suites
```bash
# Parser tests
gcc -Wall -Wextra -std=c99 -g -I. -o test_parser test/test_parser.c obj/parser.o obj/error.o obj/environment.o obj/io.o obj/builtin.o obj/external.o
./test_parser

# Builtin command tests
gcc -Wall -Wextra -std=c99 -g -I. -o test_builtin test/test_builtin.c obj/parser.o obj/error.o obj/environment.o obj/io.o obj/builtin.o obj/external.o
./test_builtin

# Environment variable tests
gcc -Wall -Wextra -std=c99 -g -I. -o test_environment test/test_environment.c obj/parser.o obj/error.o obj/environment.o obj/io.o obj/builtin.o obj/external.o
./test_environment
```

## Test Requirements Coverage

These unit tests verify the following requirements from the specification:

### Requirement 7.3 - Testing and Validation
- ✅ Command parsing functionality with various input formats
- ✅ Internal command execution and error handling
- ✅ Environment variable management and PATH searching
- ✅ System call usage verification
- ✅ Memory management and error handling
- ✅ Boundary condition testing

### Key Test Categories

1. **Functional Tests**: Verify that each component works as expected
2. **Error Handling Tests**: Ensure proper error handling for invalid inputs
3. **Boundary Tests**: Test edge cases and limits
4. **Integration Tests**: Verify components work together correctly
5. **Memory Tests**: Check for memory leaks and proper cleanup

## Test Framework

The tests use a custom lightweight testing framework with:
- Test macros for assertions (`ASSERT_TRUE`, `ASSERT_STR_EQUAL`, etc.)
- Test statistics tracking
- Colored output for better readability
- Automatic cleanup and memory tracking

## Expected Output

When all tests pass, you should see:
```
=== Test Summary ===
Total test suites: 3
Passed: 3
Failed: 0

🎉 All unit tests passed!
```

## Troubleshooting

If tests fail:
1. Check that the main project builds successfully with `make`
2. Ensure all source files are present and up to date
3. Review the specific test failure messages
4. Check for memory leaks or initialization issues

## Adding New Tests

To add new tests:
1. Create test functions following the naming convention `test_*`
2. Use the provided test macros for assertions
3. Add the test to the appropriate test runner function
4. Update this README if adding new test categories