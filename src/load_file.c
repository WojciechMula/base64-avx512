#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const int alignment = 2048;

typedef struct MemoryArray {
    char*  bytes;
    size_t size;
} MemoryArray;

void load_file(const char* path, MemoryArray* data) {
    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        printf("Can't open '%s': %s\n", path, strerror(errno));
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    data->size = ftell(f);
    fseek(f, 0, SEEK_SET);

    data->bytes = aligned_malloc(alignment, data->size);
    if (data->bytes == NULL) {
        puts("allocation failed");
        exit(1);
    }

    if (fread(data->bytes, 1, data->size, f) != data->size) {
        printf("Error reading '%s': %s\n", path, strerror(errno));
        exit(1);
    }

    fclose(f);
}
