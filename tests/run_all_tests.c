/*
 * Test Runner: Runs all libid tests and reports results
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

typedef struct {
    const char *name;
    const char *executable;
    const char *description;
} test_info_t;

static test_info_t tests[] = {
    {
        "Test 1",
        "./test1_selector_interning",
        "Selector Interning (Basic functionality)"
    },
    {
        "Test 2", 
        "./test2_object_creation",
        "Object Creation and Method Dispatch"
    },
    {
        "Test 3",
        "./test3_memory_allocation", 
        "Memory Allocation and GC Integration"
    },
    {
        "Test 4",
        "./test4_method_installation",
        "Method Installation and Custom Methods"
    },
    {
        "Test 5",
        "./test5_import_export",
        "Import/Export and Global Namespace"
    },
    {
        "Test 6",
        "./test6_arm64_stress",
        "ARM64 Stress Test and Macro Verification"
    },
    {
        "Test 7",
        "./test7_method_dispatch",
        "Method Dispatch and Calling"
    },
    {
        "Test 8",
        "./test8_error_handling",
        "Error Handling and Edge Cases"
    },
    {
        "Test 9",
        "./test9_performance",
        "Performance and Scalability"
    },
    {
        "Test 10",
        "./test10_arm64_specific",
        "ARM64-Specific Features"
    }
};

static int num_tests = sizeof(tests) / sizeof(tests[0]);

int run_test(const test_info_t *test, int quiet_mode)
{
    if (!quiet_mode) {
        printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
        printf("Running %s: %s\n", test->name, test->description);
        printf("Executable: %s\n", test->executable);
        printf("" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
    } else {
        printf("Running %s... ", test->name);
        fflush(stdout);
    }
    
    // Check if executable exists
    if (access(test->executable, X_OK) != 0) {
        if (quiet_mode) {
            printf("SKIP (not found)\n");
        } else {
            printf("❌ SKIP: Executable %s not found or not executable\n", test->executable);
        }
        return -1;  // Skip
    }
    
    // Record start time
    time_t start_time = time(NULL);
    
    // Fork and execute the test
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - execute the test
        if (quiet_mode) {
            // Redirect stdout to /dev/null in quiet mode
            freopen("/dev/null", "w", stdout);
        }
        execl(test->executable, test->executable, NULL);
        // If we get here, exec failed
        if (!quiet_mode) {
            printf("❌ FAIL: Could not execute %s\n", test->executable);
        }
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        time_t end_time = time(NULL);
        double elapsed = difftime(end_time, start_time);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                if (quiet_mode) {
                    printf("PASSED (%.1fs)\n", elapsed);
                } else {
                    printf("\n✅ %s PASSED (%.1f seconds)\n", test->name, elapsed);
                }
                return 1;  // Success
            } else {
                if (quiet_mode) {
                    printf("FAILED (exit code %d, %.1fs)\n", exit_code, elapsed);
                } else {
                    printf("\n❌ %s FAILED with exit code %d (%.1f seconds)\n", test->name, exit_code, elapsed);
                }
                return 0;  // Failure
            }
        } else {
            if (quiet_mode) {
                printf("CRASHED (%.1fs)\n", elapsed);
            } else {
                printf("\n❌ %s CRASHED or was terminated (%.1f seconds)\n", test->name, elapsed);
            }
            return 0;  // Failure
        }
    } else {
        // Fork failed
        if (quiet_mode) {
            printf("FORK FAILED\n");
        } else {
            printf("❌ FAIL: Could not fork process for %s\n", test->name);
        }
        return 0;  // Failure
    }
}

void print_system_info()
{
    printf("🖥️  System Information:\n");
    
    // Get system information
    FILE *fp = popen("uname -a", "r");
    if (fp) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp)) {
            printf("   System: %s", buffer);
        }
        pclose(fp);
    }
    
    // Get architecture
    fp = popen("uname -m", "r");
    if (fp) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), fp)) {
            // Remove newline
            buffer[strcspn(buffer, "\n")] = 0;
            printf("   Architecture: %s\n", buffer);
        }
        pclose(fp);
    }
    
    // Get compiler info
    fp = popen("cc --version 2>/dev/null | head -1", "r");
    if (fp) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp)) {
            printf("   Compiler: %s", buffer);
        }
        pclose(fp);
    }
    
    printf("\n");
}

int main(int argc, char **argv)
{
    int quiet_mode = 0;

    // Check for quiet mode flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            quiet_mode = 1;
            break;
        }
    }

    printf("🧪 Enhanced libid Test Suite");
    if (quiet_mode) {
        printf(" (Quiet Mode)\n");
    } else {
        printf("\n");
    }
    printf("Testing Apple Silicon ARM64 compatibility and numbered macro functionality\n\n");

    if (!quiet_mode) {
        print_system_info();
    }
    
    int passed = 0;
    int failed = 0;
    int skipped = 0;
    
    time_t suite_start = time(NULL);
    
    // Run all tests
    for (int i = 0; i < num_tests; i++) {
        int result = run_test(&tests[i], quiet_mode);

        if (result == 1) {
            passed++;
        } else if (result == 0) {
            failed++;
        } else {
            skipped++;
        }
    }
    
    time_t suite_end = time(NULL);
    double total_time = difftime(suite_end, suite_start);
    
    // Print summary
    printf("\n" "🏁 Test Suite Summary\n");
    printf("" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
    printf("Total Tests: %d\n", num_tests);
    printf("✅ Passed: %d\n", passed);
    printf("❌ Failed: %d\n", failed);
    printf("⏭️  Skipped: %d\n", skipped);
    printf("⏱️  Total Time: %.1f seconds\n", total_time);
    
    if (failed == 0 && passed > 0) {
        printf("\n🎉 ALL TESTS PASSED! Enhanced libid is working correctly on this platform.\n");
        if (skipped > 0) {
            printf("   (Note: %d tests were skipped due to missing executables)\n", skipped);
        }
        return 0;
    } else if (failed > 0) {
        printf("\n💥 %d TESTS FAILED. Please check the output above for details.\n", failed);
        return 1;
    } else {
        printf("\n⚠️  NO TESTS RAN. Please build the test executables first.\n");
        return 2;
    }
}
