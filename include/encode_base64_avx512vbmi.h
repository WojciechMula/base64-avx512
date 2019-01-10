#pragma once

#include <stdint.h>
#include <stddef.h>

void encode_base64_avx512vbmi(const uint8_t* input, size_t bytes, uint8_t* output);
