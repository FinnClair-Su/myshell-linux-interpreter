#include <stdio.h>

/* 声明测试函数 */
extern int test_main(void);
extern void run_environment_tests(void);
extern void run_parser_tests(void);
extern void run_error_tests(void);
extern void run_integration_tests(void);
extern void run_builtin_tests(void);

int main(void) {
    printf("=== MyShell Test Runner ===\n\n");
    
    /* 运行基础测试 */
    int result = test_main();
    
    /* 运行解析器测试 */
    run_parser_tests();
    
    /* 运行环境变量测试 */
    run_environment_tests();
    
    /* 运行错误处理测试 */
    run_error_tests();
    
    /* 运行内部命令测试 */
    run_builtin_tests();
    
    /* 运行集成测试 */
    run_integration_tests();
    
    return result;
}
