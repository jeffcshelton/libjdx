#include "tests.h"

#include <stdio.h>
#include <time.h>

// Typedef that all test function signatures must follow
typedef struct {
    void (*func)(void);
    const char *name;
} Test;

// Starting and ending times of tests
struct timespec start, end;

// Constant environment variables accessible by tests
JDXDataset example_dataset;

// Variable set by tests to indicate if they passed, failed, or not executed (declared in header)
TestState final_state;

static void print_duration(void) {
    long duration = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

    if (duration < 1000) {
        printf("%ldμs\n", duration);
    } else if (duration < 1000000) {
        printf("%ldms\n", duration / 1000);
    } else {
        printf("%fs\n", (double) duration / 1000000.0);
    }
}

static void print_pass(void) {
    printf("\x1b[32mpassed\x1b[0m | ");
    print_duration();
}

static void print_fail(void) {
    printf("\x1b[31mfailed\x1b[0m | ");
    print_duration();
}

static void print_na(void) {
    printf("\x1b[34mN/A\x1b[0m\n");
}

static void setup_testing_environment(void) {
    example_dataset = JDX_ReadDatasetFromPath("./res/example.jdx");
}

static void destroy_testing_environment(void) {
    JDX_FreeDataset(example_dataset);
}

int main(void) {
    // List of tests that need to be executed
    Test tests[] = {
        { Test_ReadHeaderFromPath, "ReadHeaderFromPath" },
        { Test_ReadDatasetFromPath, "ReadDatasetFromPath" },
        { Test_WriteDatasetToPath, "WriteDatasetToPath" }
    };

    setup_testing_environment();

    int test_count = sizeof(tests) / sizeof(Test);
    int pass_count = 0;
    int na_count = 0;

    // Run each test and print according to whether it passed or failed
    for (int t = 0; t < test_count; t++) {
        final_state = STATE_FAILURE;

        printf("\x1b[33m[\x1b[1m%s\x1b[0;33m]\x1b[0m ", tests[t].name);

        clock_gettime(CLOCK_REALTIME, &start);
        tests[t].func();
        clock_gettime(CLOCK_REALTIME, &end);

        if (final_state == STATE_SUCCESS) {
            print_pass();
            pass_count++;
        } else if (final_state == STATE_FAILURE) {
            print_fail();
        } else if (final_state == STATE_NOEXECUTE) {
            print_na();
            na_count++;
        }
    }

    // Corresponds to green ASCII color code if failed 0, red otherwise
    int fail_color_code = 31 + !!(pass_count + na_count);

    printf("\nPassed \x1b[32m%d\x1b[0m tests.\n", pass_count);
    printf("Failed \x1b[%dm%d\x1b[0m tests.\n", fail_color_code, test_count - pass_count - na_count);

    if (na_count > 0)
        printf("Did not execute \x1b[34m%d\x1b[0m tests.\n", na_count);

    destroy_testing_environment();
}
