#pragma once

#include <stdint.h>
#include <stddef.h>

void encode_base64_avx512vbmi(uint8_t* output, const uint8_t* input, size_t size);
