#!/bin/bash

# Comprehensive Shell Stress Test
# Tests shell under various stress conditions and high load scenarios

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Test configuration
STRESS_DURATION=300  # 5 minutes
CONCURRENT_PROCESSES=10
MEMORY_TEST_SIZE=1000
COMMAND_ITERATIONS=1000

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Function to print test results
print_test_result() {
    local test_name="$1"
    local result="$2"
    local details="$3"
    
    ((TESTS_TOTAL++))
    
    if [ "$result" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} $test_name"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} $test_name"
        if [ -n "$details" ]; then
            echo "       $details"
        fi
        ((TESTS_FAILED++))
    fi
}

# Function to get memory usage
get_memory_usage() {
    ps -o pid,vsz,rss,comm -p $$ | tail -n 1 | awk '{print $2}'
}

# Function to monitor process
monitor_process() {
    local pid=$1
    local max_memory=0
    local samples=0
    
    while kill -0 $pid 2>/dev/null; do
        local memory=$(ps -o rss= -p $pid 2>/dev/null || echo 0)
        if [ "$memory" -gt "$max_memory" ]; then
            max_memory=$memory
        fi
        ((samples++))
        sleep 1
    done
    
    echo $max_memory
}

# Test rapid command execution
test_rapid_execution() {
    echo -e "${BLUE}=== Testing Rapid Command Execution ===${NC}"
    
    local start_time=$(date +%s)
    local successful_commands=0
    local failed_commands=0
    
    for i in $(seq 1 100); do
        if echo "echo rapid_test_$i" | timeout 5 ./myshell > /dev/null 2>&1; then
            ((successful_commands++))
        else
            ((failed_commands++))
        fi
        
        # Show progress every 20 commands
        if [ $((i % 20)) -eq 0 ]; then
            echo "  Completed $i/100 commands..."
        fi
    done
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    echo "  Successful commands: $successful_commands"
    echo "  Failed commands: $failed_commands"
    echo "  Total time: ${duration}s"
    echo "  Commands per second: $(( successful_commands / (duration + 1) ))"
    
    local success_rate=$((successful_commands * 100 / 100))
    if [ $success_rate -ge 95 ]; then
        print_test_result "Rapid command execution" 0
    else
        print_test_result "Rapid command execution" 1 "Success rate: ${success_rate}%"
    fi
}

# Test concurrent shell instances
test_concurrent_shells() {
    echo -e "${BLUE}=== Testing Concurrent Shell Instances ===${NC}"
    
    local pids=()
    local successful_processes=0
    
    echo "  Starting $CONCURRENT_PROCESSES concurrent shell processes..."
    
    # Start concurrent processes
    for i in $(seq 1 $CONCURRENT_PROCESSES); do
        (
            for j in $(seq 1 10); do
                echo "pwd" | timeout 10 ./myshell > /dev/null 2>&1 || exit 1
                echo "echo concurrent_test_${i}_${j}" | timeout 10 ./myshell > /dev/null 2>&1 || exit 1
                sleep 0.1
            done
        ) &
        pids+=($!)
    done
    
    # Wait for all processes and count successes
    for pid in "${pids[@]}"; do
        if wait $pid; then
            ((successful_processes++))
        fi
    done
    
    echo "  Successful processes: $successful_processes/$CONCURRENT_PROCESSES"
    
    if [ $successful_processes -eq $CONCURRENT_PROCESSES ]; then
        print_test_result "Concurrent shell execution" 0
    else
        print_test_result "Concurrent shell execution" 1 "Only $successful_processes/$CONCURRENT_PROCESSES succeeded"
    fi
}

# Test memory stress
test_memory_stress() {
    echo -e "${BLUE}=== Testing Memory Stress ===${NC}"
    
    local initial_memory=$(get_memory_usage)
    echo "  Initial memory usage: ${initial_memory}KB"
    
    # Create large command with many arguments
    local large_command="echo "
    for i in $(seq 1 500); do
        large_command+="arg$i "
    done
    
    # Execute memory-intensive commands
    local max_memory=$initial_memory
    for i in $(seq 1 50); do
        echo "$large_command" | timeout 10 ./myshell > /dev/null 2>&1
        
        local current_memory=$(get_memory_usage)
        if [ "$current_memory" -gt "$max_memory" ]; then
            max_memory=$current_memory
        fi
        
        if [ $((i % 10)) -eq 0 ]; then
            echo "  Completed $i/50 memory stress iterations..."
        fi
    done
    
    local final_memory=$(get_memory_usage)
    local memory_growth=$((final_memory - initial_memory))
    
    echo "  Peak memory usage: ${max_memory}KB"
    echo "  Final memory usage: ${final_memory}KB"
    echo "  Memory growth: ${memory_growth}KB"
    
    # Check for excessive memory growth (more than 10MB)
    if [ $memory_growth -lt 10240 ]; then
        print_test_result "Memory stress test" 0
    else
        print_test_result "Memory stress test" 1 "Excessive memory growth: ${memory_growth}KB"
    fi
}

# Test long-running stability
test_long_running_stability() {
    echo -e "${BLUE}=== Testing Long-Running Stability ===${NC}"
    
    local test_duration=120  # 2 minutes for this test
    local start_time=$(date +%s)
    local end_time=$((start_time + test_duration))
    
    local commands_executed=0
    local commands_failed=0
    local memory_samples=()
    
    echo "  Running stability test for ${test_duration} seconds..."
    
    while [ $(date +%s) -lt $end_time ]; do
        local current_time=$(date +%s)
        local elapsed=$((current_time - start_time))
        
        # Execute a variety of commands
        local test_commands=("pwd" "echo stability_test" "ls /tmp" "date")
        local cmd_index=$((commands_executed % 4))
        local command=${test_commands[$cmd_index]}
        
        if echo "$command" | timeout 10 ./myshell > /dev/null 2>&1; then
            ((commands_executed++))
        else
            ((commands_failed++))
        fi
        
        # Sample memory usage every 10 seconds
        if [ $((elapsed % 10)) -eq 0 ] && [ $elapsed -gt 0 ]; then
            local memory=$(get_memory_usage)
            memory_samples+=($memory)
            echo "    ${elapsed}s: ${commands_executed} commands, ${memory}KB memory"
        fi
        
        sleep 1
    done
    
    echo "  Commands executed: $commands_executed"
    echo "  Commands failed: $commands_failed"
    
    if [ $commands_failed -eq 0 ] && [ $commands_executed -gt 50 ]; then
        print_test_result "Long-running stability" 0
    else
        print_test_result "Long-running stability" 1 "Failed: $commands_failed, Executed: $commands_executed"
    fi
}

# Test file system stress
test_filesystem_stress() {
    echo -e "${BLUE}=== Testing File System Stress ===${NC}"
    
    local test_dir="/tmp/shell_stress_test"
    mkdir -p "$test_dir"
    
    # Create many files and test operations
    echo "  Creating and manipulating files..."
    
    local operations_successful=0
    local operations_failed=0
    
    for i in $(seq 1 100); do
        local filename="$test_dir/test_file_$i.txt"
        
        # Create file
        if echo "touch $filename" | timeout 10 ./myshell > /dev/null 2>&1; then
            ((operations_successful++))
        else
            ((operations_failed++))
        fi
        
        # Write to file
        if echo "echo 'test content $i' > $filename" | timeout 10 ./myshell > /dev/null 2>&1; then
            ((operations_successful++))
        else
            ((operations_failed++))
        fi
        
        # Read file
        if echo "cat $filename" | timeout 10 ./myshell > /dev/null 2>&1; then
            ((operations_successful++))
        else
            ((operations_failed++))
        fi
        
        if [ $((i % 20)) -eq 0 ]; then
            echo "    Processed $i/100 files..."
        fi
    done
    
    # Cleanup
    rm -rf "$test_dir"
    
    echo "  Successful operations: $operations_successful"
    echo "  Failed operations: $operations_failed"
    
    local success_rate=$((operations_successful * 100 / (operations_successful + operations_failed)))
    if [ $success_rate -ge 95 ]; then
        print_test_result "File system stress test" 0
    else
        print_test_result "File system stress test" 1 "Success rate: ${success_rate}%"
    fi
}

# Test error recovery
test_error_recovery() {
    echo -e "${BLUE}=== Testing Error Recovery ===${NC}"
    
    local recovery_successful=0
    local recovery_failed=0
    
    # Test recovery from various error conditions
    local error_commands=(
        "nonexistent_command_12345"
        "cat /nonexistent/file.txt"
        "cd /nonexistent/directory"
        "rm /nonexistent/file.txt"
        "cp /nonexistent/src.txt /tmp/dst.txt"
    )
    
    for error_cmd in "${error_commands[@]}"; do
        # Execute error command
        echo "$error_cmd" | timeout 10 ./myshell > /dev/null 2>&1
        
        # Test if shell can still execute normal commands after error
        if echo "pwd" | timeout 10 ./myshell > /dev/null 2>&1; then
            ((recovery_successful++))
        else
            ((recovery_failed++))
        fi
    done
    
    echo "  Successful recoveries: $recovery_successful"
    echo "  Failed recoveries: $recovery_failed"
    
    if [ $recovery_failed -eq 0 ]; then
        print_test_result "Error recovery test" 0
    else
        print_test_result "Error recovery test" 1 "Failed to recover from $recovery_failed errors"
    fi
}

# Test signal handling stress
test_signal_handling() {
    echo -e "${BLUE}=== Testing Signal Handling ===${NC}"
    
    local signal_tests_passed=0
    local signal_tests_total=5
    
    for i in $(seq 1 $signal_tests_total); do
        # Start a shell process
        echo "sleep 5" | timeout 10 ./myshell > /dev/null 2>&1 &
        local shell_pid=$!
        
        sleep 1
        
        # Send SIGINT (Ctrl+C)
        kill -INT $shell_pid 2>/dev/null || true
        
        # Wait a bit and check if process is still running
        sleep 1
        if ! kill -0 $shell_pid 2>/dev/null; then
            ((signal_tests_passed++))
        else
            # Force kill if still running
            kill -KILL $shell_pid 2>/dev/null || true
        fi
        
        wait $shell_pid 2>/dev/null || true
    done
    
    echo "  Signal handling tests passed: $signal_tests_passed/$signal_tests_total"
    
    if [ $signal_tests_passed -ge $((signal_tests_total - 1)) ]; then
        print_test_result "Signal handling stress test" 0
    else
        print_test_result "Signal handling stress test" 1 "Only $signal_tests_passed/$signal_tests_total passed"
    fi
}

# Generate stress test report
generate_stress_report() {
    echo
    echo -e "${BLUE}=== Stress Test Report ===${NC}"
    echo "Date: $(date)"
    echo "System: $(uname -a)"
    echo "Shell: $(realpath ./myshell)"
    echo
    echo "Test Configuration:"
    echo "  Stress duration: ${STRESS_DURATION}s"
    echo "  Concurrent processes: $CONCURRENT_PROCESSES"
    echo "  Memory test size: $MEMORY_TEST_SIZE"
    echo "  Command iterations: $COMMAND_ITERATIONS"
    echo
    echo "Results Summary:"
    echo "  Total tests: $TESTS_TOTAL"
    echo "  Passed: $TESTS_PASSED"
    echo "  Failed: $TESTS_FAILED"
    echo "  Success rate: $(( TESTS_PASSED * 100 / TESTS_TOTAL ))%"
    echo
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}[SUCCESS] All stress tests passed!${NC}"
        echo "Shell demonstrates excellent stability under stress conditions."
        return 0
    else
        echo -e "${RED}[FAILURE] Some stress tests failed.${NC}"
        echo "Shell may have stability issues under high load."
        return 1
    fi
}

# Main execution
main() {
    echo "Shell Stress Test Suite"
    echo "======================="
    echo
    
    # Check if shell exists
    if [ ! -f "./myshell" ]; then
        echo -e "${RED}ERROR: myshell not found. Please build it first.${NC}"
        exit 1
    fi
    
    # Make sure shell is executable
    chmod +x ./myshell
    
    echo "Starting comprehensive stress tests..."
    echo "This may take several minutes to complete."
    echo
    
    # Run all stress test suites
    test_rapid_execution
    echo
    test_concurrent_shells
    echo
    test_memory_stress
    echo
    test_long_running_stability
    echo
    test_filesystem_stress
    echo
    test_error_recovery
    echo
    test_signal_handling
    
    # Generate final report
    generate_stress_report
}

# Cleanup function
cleanup() {
    echo "Cleaning up stress test artifacts..."
    rm -rf /tmp/shell_stress_test
    pkill -f "./myshell" 2>/dev/null || true
}

# Set trap for cleanup
trap cleanup EXIT

# Run main function
main "$@"