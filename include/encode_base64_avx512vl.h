#pragma once

#include <stdint.h>
#include <stddef.h>

void encode_base64_avx512vl(const uint8_t* input, size_t bytes, uint8_t* output);
