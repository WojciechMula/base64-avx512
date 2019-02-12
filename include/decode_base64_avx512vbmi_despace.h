#pragma once

#include <stdint.h>
#include <stddef.h>

// characters skipped: space (' '), new line ('\n'), carriage return ('\r')
size_t decode_base64_avx512vbmi_despace(uint8_t* output, const uint8_t* input, size_t size);
size_t decode_base64_avx512vbmi_despace_email(uint8_t* output, uint8_t** input, size_t size);
