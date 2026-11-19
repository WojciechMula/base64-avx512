#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BASE64_DECODE_ERROR ((size_t)-1)

/*
 * Upper bound on the number of bytes any decoder in this library writes when
 * given `size` bytes of base64 input, valid or not. Output buffers must hold
 * at least this many bytes. It is the same bound as
 * simdutf::maximal_binary_length_from_base64 (trailing '=' characters do not
 * count). chromium_base64_decode_len() is always at least as large.
 */
static inline size_t base64_decoded_max_length(const uint8_t* src, size_t size) {
    size_t padding = 0;
    if (size > 0 && src[size - 1] == '=') {
        padding++;
        if (size > 1 && src[size - 2] == '=') {
            padding++;
        }
    }
    const size_t actual = size - padding;
    return (actual / 4) * 3 + ((actual % 4 <= 1) ? 0 : (actual % 4) - 1);
}

/*
 * Reference scalar decoder; accepts exactly what simdutf::base64_to_binary
 * accepts with default options (standard alphabet, loose last chunk):
 * - padding is optional, but when present it must complete the last
 *   4-character quantum ("AA==", "AAA="),
 * - a final quantum of a single character is an error,
 * - non-zero trailing bits are ignored.
 * When `ignore_whitespace` is non-zero, ASCII white space (space, '\t', '\n',
 * '\f', '\r') is skipped anywhere, including between the '=' characters;
 * otherwise any white space is an error.
 *
 * Returns the number of bytes written or BASE64_DECODE_ERROR. It never writes
 * more than base64_decoded_max_length(src, size) bytes. It may decode in place
 * (dst <= src).
 */
size_t decode_base64_scalar(uint8_t* dst, const uint8_t* src, size_t size, int ignore_whitespace);

#ifdef __cplusplus
}
#endif
