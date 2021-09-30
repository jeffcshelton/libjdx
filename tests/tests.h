#pragma once

#include "jdx/jdx.h"
#include <stdbool.h>

// Constant testing environment variables
extern JDXDataset example_dataset;

// Variables that tests set
extern bool did_fail;

// Test definitions
void Test_ReadHeaderFromPath(void);
void Test_ReadDatasetFromPath(void);
void Test_WriteDatasetToPath(void);
