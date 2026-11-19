#pragma once

#include <stdint.h>
#include <stddef.h>

// Decodes whitespace-free base64; same rules as simdutf::base64_to_binary
// except that white space is an error (see decode_base64_scalar.h).
// The output buffer must hold base64_decoded_max_length(input, size) bytes.
// Returns the number of bytes written or (size_t)-1 on error.
size_t decode_base64_avx512vbmi__unrolled(uint8_t* output, const uint8_t* input, size_t size);
