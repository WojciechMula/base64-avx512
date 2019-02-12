#pragma once

#include <stddef.h>

typedef struct MemoryArray {
    char*  bytes;
    size_t size;
} MemoryArray;

void load_file(const char* path, MemoryArray* data);

