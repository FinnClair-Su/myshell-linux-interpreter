#!/bin/bash

# Performance Benchmarking Script
# Comprehensive performance testing and benchmarking

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
BENCHMARK_RESULTS="benchmark_results.txt"
PERFORMANCE_LOG="performance_data.csv"

# Performance thresholds (in milliseconds)
STARTUP_THRESHOLD=1000
COMMAND_THRESHOLD=100
BUILTIN_THRESHOLD=50

# Function to get precise timing
get_precise_time() {
    python3 -c "import time; print(time.time())" 2>/dev/null || date +%s.%N
}

# Function to calculate time difference in milliseconds
calc_time_diff() {
    local start=$1
    local end=$2
    python3 -c "print(($end - $start) * 1000)" 2>/dev/null || echo "scale=3; ($end - $start) * 1000" | bc
}

# Function to run benchmark test
run_benchmark() {
    local test_name="$1"
    local command="$2"
    local iterations="${3:-10}"
    
    echo "  Running $test_name benchmark ($iterations iterations)..."
    
    local times=()
    local successful_runs=0
    
    for i in $(seq 1 $iterations); do
        local start_time=$(get_precise_time)
        
        if eval "$command" > /dev/null 2>&1; then
            local end_time=$(get_precise_time)
            local duration=$(calc_time_diff $start_time $end_time)
            times+=($duration)
            ((successful_runs++))
        fi
    done
    
    if [ $successful_runs -gt 0 ]; then
        # Calculate statistics
        local min_time=${times[0]}
        local max_time=${times[0]}
        local total_time=0
        
        for time in "${times[@]}"; do
            total_time=$(echo "$total_time + $time" | bc)
            if (( $(echo "$time < $min_time" | bc -l) )); then
                min_time=$time
            fi
            if (( $(echo "$time > $max_time" | bc -l) )); then
                max_time=$time
            fi
        done
        
        local avg_time=$(echo "scale=3; $total_time / $successful_runs" | bc)
        
        echo "    Min: ${min_time}ms, Max: ${max_time}ms, Avg: ${avg_time}ms"
        echo "    Success rate: $successful_runs/$iterations"
        
        # Log to CSV
        echo "$test_name,$min_time,$max_time,$avg_time,$successful_runs,$iterations" >> "$PERFORMANCE_LOG"
        
        # Return average time for threshold checking
        echo $avg_time
    else
        echo "    All runs failed"
        echo "$test_name,0,0,0,0,$iterations" >> "$PERFORMANCE_LOG"
        echo "999999"  # Return high value to indicate failure
    fi
}

# Test shell startup performance
test_startup_performance() {
    echo -e "${BLUE}=== Shell Startup Performance ===${NC}"
    
    local avg_time=$(run_benchmark "Shell Startup" "echo 'exit' | timeout 10 ./myshell" 20)
    
    if (( $(echo "$avg_time < $STARTUP_THRESHOLD" | bc -l) )); then
        echo -e "${GREEN}[PASS] Startup performance within threshold${NC}"
        return 0
    else
        echo -e "${RED}[FAIL] Startup performance too slow: ${avg_time}ms > ${STARTUP_THRESHOLD}ms${NC}"
        return 1
    fi
}

# Test builtin command performance
test_builtin_performance() {
    echo -e "${BLUE}=== Builtin Command Performance ===${NC}"
    
    local builtin_commands=("pwd" "echo hello" "date")
    local all_passed=1
    
    for cmd in "${builtin_commands[@]}"; do
        local avg_time=$(run_benchmark "Builtin: $cmd" "echo '$cmd' | timeout 10 ./myshell" 15)
        
        if (( $(echo "$avg_time < $BUILTIN_THRESHOLD" | bc -l) )); then
            echo -e "    ${GREEN}[PASS] $cmd: ${avg_time}ms${NC}"
        else
            echo -e "    ${RED}[FAIL] $cmd: ${avg_time}ms > ${BUILTIN_THRESHOLD}ms${NC}"
            all_passed=0
        fi
    done
    
    return $all_passed
}

# Test external command performance
test_external_performance() {
    echo -e "${BLUE}=== External Command Performance ===${NC}"
    
    local external_commands=("/bin/echo test" "/bin/pwd" "/bin/date")
    local all_passed=1
    
    for cmd in "${external_commands[@]}"; do
        local avg_time=$(run_benchmark "External: $cmd" "echo '$cmd' | timeout 10 ./myshell" 10)
        
        if (( $(echo "$avg_time < $COMMAND_THRESHOLD" | bc -l) )); then
            echo -e "    ${GREEN}[PASS] $cmd: ${avg_time}ms${NC}"
        else
            echo -e "    ${RED}[FAIL] $cmd: ${avg_time}ms > ${COMMAND_THRESHOLD}ms${NC}"
            all_passed=0
        fi
    done
    
    return $all_passed
}

# Test file operation performance
test_file_operation_performance() {
    echo -e "${BLUE}=== File Operation Performance ===${NC}"
    
    # Create test file
    echo "test content for performance testing" > /tmp/perf_test_file.txt
    
    local file_commands=(
        "cat /tmp/perf_test_file.txt"
        "ls /tmp"
        "touch /tmp/perf_test_new.txt"
        "cp /tmp/perf_test_file.txt /tmp/perf_test_copy.txt"
        "rm /tmp/perf_test_new.txt"
    )
    
    local all_passed=1
    
    for cmd in "${file_commands[@]}"; do
        local avg_time=$(run_benchmark "File Op: $cmd" "echo '$cmd' | timeout 10 ./myshell" 10)
        
        if (( $(echo "$avg_time < $COMMAND_THRESHOLD" | bc -l) )); then
            echo -e "    ${GREEN}[PASS] $cmd: ${avg_time}ms${NC}"
        else
            echo -e "    ${RED}[FAIL] $cmd: ${avg_time}ms > ${COMMAND_THRESHOLD}ms${NC}"
            all_passed=0
        fi
    done
    
    # Cleanup
    rm -f /tmp/perf_test_file.txt /tmp/perf_test_copy.txt /tmp/perf_test_new.txt
    
    return $all_passed
}

# Test command parsing performance
test_parsing_performance() {
    echo -e "${BLUE}=== Command Parsing Performance ===${NC}"
    
    local parsing_tests=(
        "echo simple command"
        "echo command with multiple arguments here"
        "echo 'command with quoted arguments and special chars: !@#$%'"
        "/bin/echo external command with arguments"
    )
    
    local all_passed=1
    
    for cmd in "${parsing_tests[@]}"; do
        local avg_time=$(run_benchmark "Parse: $cmd" "echo '$cmd' | timeout 10 ./myshell" 15)
        
        if (( $(echo "$avg_time < $BUILTIN_THRESHOLD" | bc -l) )); then
            echo -e "    ${GREEN}[PASS] Parse time: ${avg_time}ms${NC}"
        else
            echo -e "    ${RED}[FAIL] Parse time: ${avg_time}ms > ${BUILTIN_THRESHOLD}ms${NC}"
            all_passed=0
        fi
    done
    
    return $all_passed
}

# Test throughput performance
test_throughput_performance() {
    echo -e "${BLUE}=== Throughput Performance ===${NC}"
    
    local commands_per_test=100
    local start_time=$(get_precise_time)
    local successful_commands=0
    
    echo "  Executing $commands_per_test commands..."
    
    for i in $(seq 1 $commands_per_test); do
        local cmd_type=$((i % 4))
        local command=""
        
        case $cmd_type in
            0) command="pwd" ;;
            1) command="echo throughput_test_$i" ;;
            2) command="date" ;;
            3) command="ls /tmp" ;;
        esac
        
        if echo "$command" | timeout 5 ./myshell > /dev/null 2>&1; then
            ((successful_commands++))
        fi
        
        if [ $((i % 25)) -eq 0 ]; then
            echo "    Completed $i/$commands_per_test commands..."
        fi
    done
    
    local end_time=$(get_precise_time)
    local total_time=$(calc_time_diff $start_time $end_time)
    local commands_per_second=$(echo "scale=2; $successful_commands * 1000 / $total_time" | bc)
    
    echo "  Throughput Results:"
    echo "    Successful commands: $successful_commands/$commands_per_test"
    echo "    Total time: ${total_time}ms"
    echo "    Commands per second: $commands_per_second"
    
    # Log throughput data
    echo "Throughput Test,$total_time,0,$commands_per_second,$successful_commands,$commands_per_test" >> "$PERFORMANCE_LOG"
    
    # Check if throughput meets minimum requirements (>10 commands/second)
    if (( $(echo "$commands_per_second > 10" | bc -l) )); then
        echo -e "${GREEN}[PASS] Throughput performance acceptable${NC}"
        return 0
    else
        echo -e "${RED}[FAIL] Throughput performance too low${NC}"
        return 1
    fi
}

# Test memory performance
test_memory_performance() {
    echo -e "${BLUE}=== Memory Performance ===${NC}"
    
    local initial_memory=$(ps -o rss= -p $$ 2>/dev/null || echo 0)
    echo "  Initial memory usage: ${initial_memory}KB"
    
    # Run memory-intensive operations
    local start_time=$(get_precise_time)
    
    for i in $(seq 1 50); do
        # Create command with many arguments
        local long_cmd="echo "
        for j in $(seq 1 20); do
            long_cmd+="arg${i}_${j} "
        done
        
        echo "$long_cmd" | timeout 10 ./myshell > /dev/null 2>&1
    done
    
    local end_time=$(get_precise_time)
    local execution_time=$(calc_time_diff $start_time $end_time)
    local final_memory=$(ps -o rss= -p $$ 2>/dev/null || echo 0)
    local memory_growth=$((final_memory - initial_memory))
    
    echo "  Memory Performance Results:"
    echo "    Execution time: ${execution_time}ms"
    echo "    Final memory usage: ${final_memory}KB"
    echo "    Memory growth: ${memory_growth}KB"
    
    # Check memory efficiency
    if [ $memory_growth -lt 1024 ] && (( $(echo "$execution_time < 5000" | bc -l) )); then
        echo -e "${GREEN}[PASS] Memory performance efficient${NC}"
        return 0
    else
        echo -e "${RED}[FAIL] Memory performance issues detected${NC}"
        return 1
    fi
}

# Compare with system shell performance
compare_with_system_shell() {
    echo -e "${BLUE}=== Comparison with System Shell ===${NC}"
    
    local test_commands=("pwd" "echo comparison_test" "date")
    
    echo "  Comparing performance with /bin/bash..."
    
    for cmd in "${test_commands[@]}"; do
        # Test myshell
        local myshell_time=$(run_benchmark "MyShell: $cmd" "echo '$cmd' | timeout 10 ./myshell" 5)
        
        # Test bash
        local bash_time=$(run_benchmark "Bash: $cmd" "echo '$cmd' | timeout 10 /bin/bash" 5)
        
        # Calculate ratio
        local ratio=$(echo "scale=2; $myshell_time / $bash_time" | bc)
        
        echo "    $cmd: MyShell=${myshell_time}ms, Bash=${bash_time}ms, Ratio=${ratio}x"
        
        if (( $(echo "$ratio < 3.0" | bc -l) )); then
            echo -e "      ${GREEN}[PASS] Performance comparable to bash${NC}"
        else
            echo -e "      ${YELLOW}[WARN] Significantly slower than bash${NC}"
        fi
    done
}

# Generate performance report
generate_performance_report() {
    echo
    echo -e "${BLUE}=== Performance Benchmark Report ===${NC}"
    echo "Date: $(date)"
    echo "System: $(uname -a)"
    echo "Shell: $(realpath ./myshell)"
    echo
    
    if [ -f "$PERFORMANCE_LOG" ]; then
        echo "Performance Data Summary:"
        echo "========================"
        
        # Process CSV data
        echo "Test Name,Min Time (ms),Max Time (ms),Avg Time (ms),Success Rate"
        echo "================================================================"
        
        while IFS=',' read -r test_name min_time max_time avg_time success_count total_count; do
            if [ "$test_name" != "Test Name" ]; then
                local success_rate=$(echo "scale=1; $success_count * 100 / $total_count" | bc)
                printf "%-30s %8.2f %12.2f %12.2f %10.1f%%\n" \
                    "$test_name" "$min_time" "$max_time" "$avg_time" "$success_rate"
            fi
        done < "$PERFORMANCE_LOG"
        
        echo
        echo "Performance Thresholds:"
        echo "  Shell startup: < ${STARTUP_THRESHOLD}ms"
        echo "  Builtin commands: < ${BUILTIN_THRESHOLD}ms"
        echo "  External commands: < ${COMMAND_THRESHOLD}ms"
        echo
        echo "Raw data available in: $PERFORMANCE_LOG"
    fi
    
    echo
    echo "Performance Recommendations:"
    echo "1. Monitor startup time in production"
    echo "2. Profile slow commands for optimization"
    echo "3. Consider caching for frequently used operations"
    echo "4. Optimize memory allocation patterns"
}

# Main execution
main() {
    echo "Performance Benchmarking Suite"
    echo "=============================="
    echo
    
    # Check if shell exists
    if [ ! -f "./myshell" ]; then
        echo -e "${RED}ERROR: myshell not found. Please build it first.${NC}"
        exit 1
    fi
    
    # Make sure shell is executable
    chmod +x ./myshell
    
    # Initialize performance log
    echo "Test Name,Min Time,Max Time,Avg Time,Success Count,Total Count" > "$PERFORMANCE_LOG"
    
    echo "Starting performance benchmarks..."
    echo "This may take several minutes to complete."
    echo
    
    # Run all benchmark suites
    local tests_passed=0
    local tests_total=0
    
    # Startup performance
    ((tests_total++))
    if test_startup_performance; then
        ((tests_passed++))
    fi
    echo
    
    # Builtin command performance
    ((tests_total++))
    if test_builtin_performance; then
        ((tests_passed++))
    fi
    echo
    
    # External command performance
    ((tests_total++))
    if test_external_performance; then
        ((tests_passed++))
    fi
    echo
    
    # File operation performance
    ((tests_total++))
    if test_file_operation_performance; then
        ((tests_passed++))
    fi
    echo
    
    # Parsing performance
    ((tests_total++))
    if test_parsing_performance; then
        ((tests_passed++))
    fi
    echo
    
    # Throughput performance
    ((tests_total++))
    if test_throughput_performance; then
        ((tests_passed++))
    fi
    echo
    
    # Memory performance
    ((tests_total++))
    if test_memory_performance; then
        ((tests_passed++))
    fi
    echo
    
    # Comparison with system shell
    compare_with_system_shell
    echo
    
    # Generate report
    generate_performance_report
    
    # Final summary
    echo "Benchmark Summary:"
    echo "  Tests passed: $tests_passed/$tests_total"
    echo "  Success rate: $(( tests_passed * 100 / tests_total ))%"
    
    if [ $tests_passed -eq $tests_total ]; then
        echo -e "${GREEN}All performance benchmarks passed!${NC}"
        return 0
    else
        echo -e "${RED}Some performance benchmarks failed.${NC}"
        return 1
    fi
}

# Cleanup function
cleanup() {
    echo "Cleaning up benchmark artifacts..."
    rm -f /tmp/perf_test_*.txt
}

# Set trap for cleanup
trap cleanup EXIT

# Check for required tools
if ! command -v bc >/dev/null 2>&1; then
    echo -e "${YELLOW}Warning: 'bc' calculator not found. Some calculations may be inaccurate.${NC}"
fi

# Run main function
main "$@"