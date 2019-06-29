#pragma once

#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include "c_api_plain.h"
#include "c_api_table.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    EXPORT bool E_InitAll(char const *data_dir);

#ifdef __cplusplus
}
#endif
