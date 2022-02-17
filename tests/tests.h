#pragma once

#include "libjdx.h"
#include <stdbool.h>

typedef enum {
	STATE_SUCCESS,
	STATE_FAILURE,
	STATE_NOEXECUTE
} TestState;

// Constant testing environment variables
extern JDXDataset *example_dataset;

// Variables that tests set
extern TestState final_state;

// For declaring new test functions
#define TEST_FUNC(name) void Test_##name (void)

// Test definitions
TEST_FUNC(CompareVersions);
TEST_FUNC(ReadHeaderFromPath);
TEST_FUNC(CopyHeader);
TEST_FUNC(ReadDatasetFromPath);
TEST_FUNC(WriteDatasetToPath);
TEST_FUNC(CopyDataset);
TEST_FUNC(AppendDataset);
