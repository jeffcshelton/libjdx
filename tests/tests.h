#pragma once

#include "jdx/jdx.h"
#include <stdbool.h>

// Constant testing environment variables
extern JDXObject example_obj;

// Variables that tests set
extern bool did_fail;

// Test definitions
void Test_ReadHeaderFromPath(void);
void Test_ReadObjectFromPath(void);
