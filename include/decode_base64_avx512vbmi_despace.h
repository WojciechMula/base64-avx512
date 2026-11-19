#pragma once

#include <stdint.h>
#include <stddef.h>

// Decodes base64 ignoring ASCII white space: space, '\t', '\n', '\f', '\r'.
// Same rules as simdutf::base64_to_binary (see decode_base64_scalar.h).
// The output buffer must hold base64_decoded_max_length(input, size) bytes.
// Returns the number of bytes written or (size_t)-1 on error.
size_t decode_base64_avx512vbmi_despace(uint8_t* output, const uint8_t* input, size_t size);

// Decodes, possibly in place (output == *input), the base64 data of an e-mail
// attachment: it ends at the MIME boundary (a line starting with "--") or at
// the end of the input. On success, *input points at the boundary (or at the
// end of the input) and the number of decoded bytes is returned; on error,
// (size_t)-1 is returned and *input is unchanged.
size_t decode_base64_avx512vbmi_despace_email(uint8_t* output, uint8_t** input, size_t size);
