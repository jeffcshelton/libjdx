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

// Variable set by tests to indicate if they failed
bool did_fail;

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

int main(void) {
    // List of tests that need to be executed
    Test tests[] = {
        { Test_ReadHeaderFromPath, "ReadHeaderFromPath" },
        { Test_ReadObjectFromPath, "ReadObjectFromPath" }
    };

    int test_count = sizeof(tests) / sizeof(Test);
    int fail_count = 0;

    // Run each test and print according to whether it passed or failed
    for (int t = 0; t < test_count; t++) {
        did_fail = false;

        printf("\x1b[33m[\x1b[1m%s\x1b[0;33m]\x1b[0m ", tests[t].name);

        clock_gettime(CLOCK_REALTIME, &start);
        tests[t].func();
        clock_gettime(CLOCK_REALTIME, &end);

        if (did_fail) {
            print_fail();
            fail_count++;
        } else {
            print_pass();
        }
    }

    // Corresponds to green ASCII color code if failed 0, red otherwise
    int color_code = 32 - !!fail_count;

    printf("\nPassed \x1b[32m%d\x1b[0m tests.\n", test_count - fail_count);
    printf("Failed \x1b[%dm%d\x1b[0m tests.\n", color_code, fail_count);
}
