#pragma once

#include <stdint.h>
#include <stddef.h>

// Writes exactly 4 * ceil(size / 3) bytes (no terminating NUL).
void encode_base64_avx512vbmi(uint8_t* output, const uint8_t* input, size_t size);
