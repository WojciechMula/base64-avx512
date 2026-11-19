#pragma once

#include <stdint.h>
#include <stddef.h>
// note: this scheme is named avx512vl but it is not really related to avx512vl per se.
// Writes exactly 4 * ceil(size / 3) bytes (no terminating NUL).
void encode_base64_avx512vl(uint8_t* output, const uint8_t* input, size_t size);
