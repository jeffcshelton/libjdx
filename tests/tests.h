#pragma once

#include "libjdx.h"
#include <stdbool.h>

typedef enum {
    STATE_SUCCESS,
    STATE_FAILURE,
    STATE_NOEXECUTE
} TestState;

// Constant testing environment variables
extern JDXDataset example_dataset;

// Variables that tests set
extern TestState final_state;

// Test definitions
void Test_ReadHeaderFromPath(void);
void Test_ReadDatasetFromPath(void);
void Test_WriteDatasetToPath(void);
void Test_CopyDataset(void);
void Test_AppendDataset(void);
