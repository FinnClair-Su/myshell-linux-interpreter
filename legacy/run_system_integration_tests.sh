#!/bin/bash

# Master System Integration and Deployment Test Runner
# Executes all system integration tests in the correct order

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m'

# Test configuration
TEST_LOG_DIR="test_logs"
MASTER_LOG="$TEST_LOG_DIR/master_test_log.txt"
SUMMARY_REPORT="$TEST_LOG_DIR/integration_test_summary.txt"

# Test counters
TOTAL_TEST_SUITES=0
PASSED_TEST_SUITES=0
FAILED_TEST_SUITES=0

# Function to print section headers
print_section() {
    local title="$1"
    echo
    echo -e "${BOLD}${BLUE}================================${NC}"
    echo -e "${BOLD}${BLUE} $title${NC}"
    echo -e "${BOLD}${BLUE}================================${NC}"
    echo
}

# Function to print test results
print_test_suite_result() {
    local suite_name="$1"
    local result="$2"
    local details="$3"
    
    ((TOTAL_TEST_SUITES++))
    
    if [ "$result" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} $suite_name"
        ((PASSED_TEST_SUITES++))
        echo "[PASS] $suite_name" >> "$MASTER_LOG"
    else
        echo -e "${RED}[FAIL]${NC} $suite_name"
        if [ -n "$details" ]; then
            echo "       $details"
            echo "       $details" >> "$MASTER_LOG"
        fi
        ((FAILED_TEST_SUITES++))
        echo "[FAIL] $suite_name" >> "$MASTER_LOG"
    fi
}

# Function to run a test suite
run_test_suite() {
    local suite_name="$1"
    local test_command="$2"
    local log_file="$TEST_LOG_DIR/${suite_name,,}_test.log"
    
    echo "Running $suite_name..."
    echo "Command: $test_command" >> "$MASTER_LOG"
    echo "Log file: $log_file" >> "$MASTER_LOG"
    
    if eval "$test_command" > "$log_file" 2>&1; then
        print_test_suite_result "$suite_name" 0
        return 0
    else
        local exit_code=$?
        print_test_suite_result "$suite_name" 1 "Exit code: $exit_code, Check: $log_file"
        return 1
    fi
}

# Function to setup test environment
setup_test_environment() {
    print_section "Setting Up Test Environment"
    
    # Create log directory
    mkdir -p "$TEST_LOG_DIR"
    
    # Initialize master log
    echo "System Integration and Deployment Tests" > "$MASTER_LOG"
    echo "Date: $(date)" >> "$MASTER_LOG"
    echo "System: $(uname -a)" >> "$MASTER_LOG"
    echo "========================================" >> "$MASTER_LOG"
    
    # Check if shell exists and is built
    if [ ! -f "./myshell" ]; then
        echo -e "${RED}ERROR: myshell executable not found.${NC}"
        echo "Please build the shell first with: make clean all"
        exit 1
    fi
    
    # Make shell executable
    chmod +x ./myshell
    
    # Check system requirements
    echo "Checking system requirements..."
    
    local requirements_met=1
    
    # Check for required tools
    local required_tools=("gcc" "make" "timeout")
    for tool in "${required_tools[@]}"; do
        if command -v "$tool" >/dev/null 2>&1; then
            echo "  ✓ $tool available"
        else
            echo -e "  ${RED}✗ $tool missing${NC}"
            requirements_met=0
        fi
    done
    
    # Check for optional tools
    local optional_tools=("valgrind" "bc" "python3")
    for tool in "${optional_tools[@]}"; do
        if command -v "$tool" >/dev/null 2>&1; then
            echo "  ✓ $tool available (optional)"
        else
            echo -e "  ${YELLOW}! $tool not available (optional)${NC}"
        fi
    done
    
    if [ $requirements_met -eq 0 ]; then
        echo -e "${RED}ERROR: Missing required tools. Please install them first.${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Test environment setup complete.${NC}"
}

# Function to run build verification tests
run_build_verification() {
    print_section "Build System Verification (Task 15.1)"
    
    # Compile build verification test
    if gcc -o test_build_verification test_build_verification.c 2>/dev/null; then
        run_test_suite "Build Verification Test" "./test_build_verification"
    else
        print_test_suite_result "Build Verification Test" 1 "Failed to compile test program"
    fi
    
    # Run simple build test
    if gcc -o simple_build_test simple_build_test.c 2>/dev/null; then
        run_test_suite "Simple Build Test" "./simple_build_test"
    else
        print_test_suite_result "Simple Build Test" 1 "Failed to compile test program"
    fi
    
    # Test Makefile features
    if gcc -o test_makefile test_makefile_features.c 2>/dev/null; then
        run_test_suite "Makefile Features Test" "./test_makefile"
    else
        print_test_suite_result "Makefile Features Test" 1 "Failed to compile test program"
    fi
    
    # Test system compatibility
    if gcc -o test_compatibility test_system_compatibility.c 2>/dev/null; then
        run_test_suite "System Compatibility Test" "./test_compatibility"
    else
        print_test_suite_result "System Compatibility Test" 1 "Failed to compile test program"
    fi
}

# Function to run shell replacement tests
run_shell_replacement_tests() {
    print_section "Shell Replacement Testing (Task 15.2)"
    
    # Compile shell replacement test
    if gcc -o test_shell_replacement test_shell_replacement.c 2>/dev/null; then
        run_test_suite "Shell Replacement Test" "./test_shell_replacement"
    else
        print_test_suite_result "Shell Replacement Test" 1 "Failed to compile test program"
    fi
    
    # Run comprehensive system replacement test
    if [ -f "test_shell_as_system_replacement.sh" ]; then
        chmod +x test_shell_as_system_replacement.sh
        run_test_suite "System Replacement Test" "./test_shell_as_system_replacement.sh"
    else
        print_test_suite_result "System Replacement Test" 1 "Test script not found"
    fi
}

# Function to run performance and stability tests
run_performance_stability_tests() {
    print_section "Performance and Stability Testing (Task 15.3)"
    
    # Compile performance test
    if gcc -o test_performance test_performance_stability.c 2>/dev/null; then
        run_test_suite "Performance Stability Test" "./test_performance"
    else
        print_test_suite_result "Performance Stability Test" 1 "Failed to compile test program"
    fi
    
    # Run stress tests
    if [ -f "stress_test_shell.sh" ]; then
        chmod +x stress_test_shell.sh
        run_test_suite "Stress Test Suite" "./stress_test_shell.sh"
    else
        print_test_suite_result "Stress Test Suite" 1 "Test script not found"
    fi
    
    # Run memory leak tests (if valgrind available)
    if command -v valgrind >/dev/null 2>&1 && [ -f "test_memory_leaks.sh" ]; then
        chmod +x test_memory_leaks.sh
        run_test_suite "Memory Leak Test" "./test_memory_leaks.sh"
    else
        echo -e "${YELLOW}Skipping memory leak tests (valgrind not available)${NC}"
    fi
    
    # Run performance benchmarks
    if [ -f "benchmark_performance.sh" ]; then
        chmod +x benchmark_performance.sh
        run_test_suite "Performance Benchmark" "./benchmark_performance.sh"
    else
        print_test_suite_result "Performance Benchmark" 1 "Test script not found"
    fi
}

# Function to run additional integration tests
run_additional_tests() {
    print_section "Additional Integration Tests"
    
    # Run existing unit tests if available
    if [ -f "run_unit_tests.sh" ]; then
        chmod +x run_unit_tests.sh
        run_test_suite "Unit Tests" "./run_unit_tests.sh"
    fi
    
    # Run existing integration tests if available
    if [ -f "run_integration_tests.sh" ]; then
        chmod +x run_integration_tests.sh
        run_test_suite "Integration Tests" "./run_integration_tests.sh"
    fi
    
    # Test with existing test programs
    if [ -f "test_runner" ]; then
        run_test_suite "Test Runner" "./test_runner"
    fi
}

# Function to generate comprehensive report
generate_comprehensive_report() {
    print_section "Generating Comprehensive Report"
    
    # Create summary report
    cat > "$SUMMARY_REPORT" << EOF
System Integration and Deployment Test Summary
==============================================

Test Execution Details:
  Date: $(date)
  System: $(uname -a)
  Shell: $(realpath ./myshell)
  Test Duration: $(($(date +%s) - START_TIME)) seconds

Test Suite Results:
  Total test suites: $TOTAL_TEST_SUITES
  Passed: $PASSED_TEST_SUITES
  Failed: $FAILED_TEST_SUITES
  Success rate: $(( PASSED_TEST_SUITES * 100 / TOTAL_TEST_SUITES ))%

Test Categories:
  ✓ Build System Verification (Task 15.1)
  ✓ Shell Replacement Testing (Task 15.2)  
  ✓ Performance and Stability Testing (Task 15.3)
  ✓ Additional Integration Tests

Log Files:
  Master log: $MASTER_LOG
  Individual test logs: $TEST_LOG_DIR/

EOF
    
    # Add detailed results
    echo "Detailed Test Results:" >> "$SUMMARY_REPORT"
    echo "=====================" >> "$SUMMARY_REPORT"
    echo >> "$SUMMARY_REPORT"
    
    # Extract results from master log
    grep "^\[PASS\]\|\[FAIL\]" "$MASTER_LOG" >> "$SUMMARY_REPORT"
    
    # Add recommendations
    cat >> "$SUMMARY_REPORT" << EOF

Recommendations:
===============

EOF
    
    if [ $FAILED_TEST_SUITES -eq 0 ]; then
        cat >> "$SUMMARY_REPORT" << EOF
✓ All system integration tests passed successfully!
✓ Shell is ready for deployment and production use.
✓ Consider running periodic regression tests.
✓ Monitor performance in production environments.

Next Steps:
- Deploy shell to target systems
- Set up monitoring and logging
- Create user documentation
- Plan maintenance procedures
EOF
    else
        cat >> "$SUMMARY_REPORT" << EOF
⚠ Some tests failed - review before deployment:
- Check individual test logs for failure details
- Fix identified issues and re-run tests
- Consider partial deployment with known limitations
- Implement additional monitoring for problem areas

Critical Actions:
- Review failed test logs in $TEST_LOG_DIR/
- Address performance or stability issues
- Re-run tests after fixes
- Document any known limitations
EOF
    fi
    
    echo "Comprehensive report generated: $SUMMARY_REPORT"
}

# Function to cleanup test artifacts
cleanup_test_artifacts() {
    echo "Cleaning up test artifacts..."
    
    # Remove compiled test programs
    rm -f test_build_verification simple_build_test test_makefile test_compatibility
    rm -f test_shell_replacement test_performance
    
    # Keep log files but clean up temporary files
    rm -f /tmp/shell_test_* /tmp/valgrind_test_* /tmp/fragmentation_test.sh
    
    echo "Cleanup complete."
}

# Function to display final summary
display_final_summary() {
    print_section "Final Summary"
    
    echo "System Integration and Deployment Tests Complete"
    echo
    echo "Results Summary:"
    echo "  Total test suites: $TOTAL_TEST_SUITES"
    echo "  Passed: $PASSED_TEST_SUITES"
    echo "  Failed: $FAILED_TEST_SUITES"
    echo "  Success rate: $(( PASSED_TEST_SUITES * 100 / TOTAL_TEST_SUITES ))%"
    echo
    
    if [ $FAILED_TEST_SUITES -eq 0 ]; then
        echo -e "${GREEN}${BOLD}🎉 ALL TESTS PASSED! 🎉${NC}"
        echo -e "${GREEN}Shell is ready for deployment!${NC}"
        echo
        echo "The shell has successfully passed all system integration tests:"
        echo "✓ Build system verification"
        echo "✓ Shell replacement capability"
        echo "✓ Performance and stability requirements"
        echo
        echo "You can now:"
        echo "1. Install the shell using: ./install_shell_replacement.sh"
        echo "2. Deploy to production systems"
        echo "3. Set up monitoring and maintenance procedures"
        
        return 0
    else
        echo -e "${RED}${BOLD}❌ SOME TESTS FAILED ❌${NC}"
        echo -e "${RED}Shell requires fixes before deployment.${NC}"
        echo
        echo "Failed test suites: $FAILED_TEST_SUITES"
        echo "Please review the test logs and fix identified issues."
        echo
        echo "Check detailed logs in: $TEST_LOG_DIR/"
        echo "Review summary report: $SUMMARY_REPORT"
        
        return 1
    fi
}

# Main execution function
main() {
    local START_TIME=$(date +%s)
    
    echo -e "${BOLD}System Integration and Deployment Test Suite${NC}"
    echo "============================================="
    echo
    echo "This comprehensive test suite validates:"
    echo "• Build system verification (Task 15.1)"
    echo "• Shell replacement capability (Task 15.2)"
    echo "• Performance and stability (Task 15.3)"
    echo
    
    # Setup test environment
    setup_test_environment
    
    # Run all test categories
    run_build_verification
    run_shell_replacement_tests
    run_performance_stability_tests
    run_additional_tests
    
    # Generate comprehensive report
    generate_comprehensive_report
    
    # Display final summary and return appropriate exit code
    if display_final_summary; then
        echo
        echo "All system integration tests completed successfully!"
        echo "Detailed results available in: $SUMMARY_REPORT"
        exit 0
    else
        echo
        echo "Some tests failed. Please review and fix issues."
        echo "Detailed results available in: $SUMMARY_REPORT"
        exit 1
    fi
}

# Cleanup function for script termination
cleanup() {
    echo
    echo "Test execution interrupted. Cleaning up..."
    cleanup_test_artifacts
    exit 130
}

# Set trap for cleanup on script termination
trap cleanup INT TERM

# Run main function
main "$@"