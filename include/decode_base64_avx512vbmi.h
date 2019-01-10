#pragma once

#include <stdint.h>
#include <stddef.h>

size_t decode_base64_avx512vbmi(const uint8_t* input, size_t size, uint8_t* output);
