# MyShell Integration Tests

This document describes the integration tests implemented for the MyShell project.

## Overview

The integration tests verify the complete functionality of the shell by testing:
1. Complete command flow from input to output
2. External command execution and process management
3. Environment variable handling
4. Error handling and recovery
5. Memory management across operations

## Test Files

### test_integration.c
Main integration test file containing:

#### Complete Command Flow Tests (Task 14.1)
- `test_complete_command_flow_pwd()` - Tests pwd command end-to-end
- `test_complete_command_flow_ls()` - Tests ls command execution
- `test_complete_command_flow_echo()` - Tests echo command with arguments
- `test_file_operations_sequence()` - Tests touch, cat, cp, rm sequence
- `test_directory_operations_sequence()` - Tests cd, pwd, ls sequence
- `test_environment_operations_sequence()` - Tests export and echo with variables
- `test_error_handling_flow()` - Tests error conditions and recovery
- `test_command_parsing_edge_cases()` - Tests parsing boundary conditions
- `test_mixed_command_execution()` - Tests internal and external command mix
- `test_command_argument_passing()` - Tests argument handling correctness
- `test_memory_management_in_flow()` - Tests memory cleanup in operations

#### External Command Tests (Task 14.2)
- `test_external_command_basic()` - Basic external command execution
- `test_external_command_path_search()` - PATH environment variable usage
- `test_external_command_arguments()` - Argument passing to external commands
- `test_external_command_environment()` - Environment variable inheritance
- `test_external_command_error_handling()` - Error handling for external commands
- `test_external_command_exit_status()` - Exit status handling
- `test_external_command_process_management()` - Fork/exec/wait functionality
- `test_multiple_external_commands()` - Sequential external command execution
- `test_external_command_io()` - Input/output handling
- `test_external_command_resource_cleanup()` - Resource management
- `test_external_command_signal_handling()` - Signal handling in child processes

## Requirements Coverage

### Requirement 7.3 (Testing and Validation)
The integration tests cover:
- Complete command execution flow
- Internal and external command interaction
- Environment variable propagation
- Error handling and recovery
- Memory management verification
- Process management validation

## Compilation and Execution

### Method 1: Using the provided script
```bash
bash run_integration_tests.sh
```

### Method 2: Manual compilation
```bash
# Ensure object files are compiled
make

# Compile integration tests
gcc -Wall -Wextra -std=c99 -pedantic -g \
    test/test_integration.c \
    obj/parser.o obj/builtin.o obj/external.o obj/environment.o obj/io.o obj/error.o \
    -o test_integration

# Run tests
./test_integration
```

### Method 3: Using simple script
```bash
bash compile_integration_test.sh
./test_integration
```

## Test Structure

Each test follows this pattern:
1. **Setup** - Initialize test environment and variables
2. **Execute** - Run the command or sequence being tested
3. **Verify** - Check results against expected outcomes
4. **Cleanup** - Clean up any created files or resources

## Expected Behavior

### Successful Tests
- All tests should pass in a standard Linux environment
- Tests are designed to be robust across different systems
- Some external command tests may skip if commands are not available

### Test Output
```
=== MyShell Integration Tests ===

=== Complete Command Flow Integration Tests ===
Running integration test: complete command flow - pwd... PASSED
Running integration test: complete command flow - ls... PASSED
...

=== External Command Execution Integration Tests ===
Running integration test: external command basic execution... PASSED
...

=== Integration Test Results ===
Total tests run: XX
Total tests passed: XX
Total tests failed: 0

✓ All Integration Tests Passed!
```

## Troubleshooting

### Common Issues

1. **Compilation Errors**
   - Ensure all source files are compiled first: `make`
   - Check that obj/ directory contains all .o files

2. **External Command Tests Failing**
   - Some tests depend on standard Unix commands (true, false, echo, etc.)
   - Tests are designed to gracefully handle missing commands

3. **Permission Issues**
   - Ensure test scripts have execute permissions: `chmod +x *.sh`
   - Some tests create temporary files in the current directory

4. **Memory Tracking**
   - Tests use the shell's built-in memory tracking system
   - Memory leaks will be reported at the end of test execution

## Test Coverage

The integration tests verify:
- ✅ Command parsing and execution pipeline
- ✅ Internal command functionality
- ✅ External command execution via fork/exec
- ✅ Environment variable handling and inheritance
- ✅ Error handling and recovery mechanisms
- ✅ Memory management and resource cleanup
- ✅ Process management and signal handling
- ✅ File system operations and I/O handling

## Future Enhancements

Potential additions to the test suite:
- Pipeline and redirection testing
- Concurrent command execution
- Performance and stress testing
- Signal handling edge cases
- Advanced error recovery scenarios