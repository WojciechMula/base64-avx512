#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include "benchmark.h"
#include "decode_base64_avx512vbmi_despace.h"

static const int repeat = 50;

uint8_t* email_likedata(size_t lines, size_t line_length, size_t* data_size, size_t* decoded_size);

int main() {
    RDTSC_SET_OVERHEAD(rdtsc_overhead_func(1), repeat);

    const int verbose = 1;
    const size_t lines = 1024;

    for (size_t line_length=50; line_length < 80; line_length++) {
        size_t data_size;
        size_t decoded_size;
        uint8_t* data = email_likedata(lines, line_length, &data_size, &decoded_size);
        uint8_t* dest = (uint8_t*)malloc(data_size);

        printf("data_size = %lu decoded_size = %lu\n", data_size, decoded_size);
        BEST_TIME_NOCHECK("AVX512VBMI (despace)", decode_base64_avx512vbmi_despace(dest, data, decoded_size),
                          /*no-pre*/, repeat, data_size, verbose); 

        free(data);
        free(dest);
    }

    return 0;
}
uint8_t* email_likedata(size_t lines, size_t line_length, size_t* data_size, size_t* decoded_size) {
    const size_t size = (lines + 1) * (line_length + 2); // +2 = '\n' and '\r'
    uint8_t* buffer = (uint8_t*)malloc(size);

    uint8_t* out = buffer;
    *decoded_size = 0;
    for (size_t i=0; i < lines; i++) {
        memset(out, 'V', line_length); // in base64 'V' maps to 0x15 = 0b010101
        *decoded_size += line_length;
        out += line_length;
        *out++ = '\r';
        *out++ = '\n';
    }

    while (*decoded_size % 4 != 0) {
        *out++ = 'V';
        *decoded_size += 1;
    }

    *data_size = (out - buffer);

    return buffer;
}
