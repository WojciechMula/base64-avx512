// This file is meant to be included into another .c file in
// order to allow inlining.
//
// Procedure decodes tail of whitespace-free base64-encoded
// message, and deals with trailing '='.
#include <stddef.h>
#include <stdint.h>
#include <immintrin.h>

int decode_base64_tail_avx512vbmi(uint8_t* dst, const uint8_t* src, size_t size) {

    const char BASE64_PAD = '=';

    assert(size <= 64);
    if (size == 0) {
        return 0;
    }

    if (size % 4 != 0) {
        return -1;
    }

    uint64_t input_mask  = ((uint64_t)-1) >> (64 - size);
    int output_size = (size / 4) * 3;
    if (src[size - 1] == BASE64_PAD) {
        output_size -= 1;
        input_mask >>= 1;
        if (src[size - 2] == BASE64_PAD) {
            output_size -= 1;
            input_mask >>= 1;
        }
    }

    const uint64_t output_mask = (0x0000ffffffffffffllu) >> (48 - output_size);

    const __m512i lookup_0 = _mm512_setr_epi32(
                                0x80808080, 0x80808080, 0x80808080, 0x80808080,
                                0x80808080, 0x80808080, 0x80808080, 0x80808080,
                                0x80808080, 0x80808080, 0x3e808080, 0x3f808080,
                                0x37363534, 0x3b3a3938, 0x80803d3c, 0x80808080);
    const __m512i lookup_1 = _mm512_setr_epi32(
                                0x02010080, 0x06050403, 0x0a090807, 0x0e0d0c0b,
                                0x1211100f, 0x16151413, 0x80191817, 0x80808080,
                                0x1c1b1a80, 0x201f1e1d, 0x24232221, 0x28272625,
                                0x2c2b2a29, 0x302f2e2d, 0x80333231, 0x80808080);

    // 1. load input, fill past-end bytes with a valid base64 character
    __m512i input = _mm512_mask_loadu_epi8(_mm512_set1_epi8('a'), input_mask, src);

    // 2. translate from ASCII into 6-bit values
    __m512i translated = _mm512_permutex2var_epi8(lookup_0, input, lookup_1);

    // 2a. check for errors --- convert MSBs to a mask
    const uint64_t mask = _mm512_test_epi8_mask(translated | input, _mm512_set1_epi8((int8_t)0x80));
    if (mask != 0)
        return -1;

    // 3. pack four 6-bit values into 24-bit words (all within 32-bit lanes)
    // Note: exactly the same procedure as we have in AVX2 version
    // input:  packed_dword([00dddddd|00cccccc|00bbbbbb|00aaaaaa] x 4)
    // merged: packed_dword([00000000|aaaaabbb|bbbbcccc|ccdddddd] x 4)
    const __m512i merge_ab_and_bc = _mm512_maddubs_epi16(translated,
                                                         _mm512_set1_epi32(0x01400140));

    __m512i merged = _mm512_madd_epi16(merge_ab_and_bc, _mm512_set1_epi32(0x00011000));


    // 4. pack 24-bit values into continous array of 48 bytes
    const __m512i pack = _mm512_setr_epi32(
                            0x06000102, 0x090a0405, 0x0c0d0e08, 0x16101112,
                            0x191a1415, 0x1c1d1e18, 0x26202122, 0x292a2425,
                            0x2c2d2e28, 0x36303132, 0x393a3435, 0x3c3d3e38,
                            0x00000000, 0x00000000, 0x00000000, 0x00000000);
    const __m512i shuffled = _mm512_permutexvar_epi8(pack, merged);

    _mm512_mask_storeu_epi8((__m512*)dst, output_mask, shuffled);

    return output_size;
}
