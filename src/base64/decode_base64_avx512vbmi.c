/*
    sources:
    - https://github.com/WojciechMula/base64simd/blob/master/decode/decode.avx512vbmi.cpp
    - https://github.com/WojciechMula/base64simd/blob/master/decode/lookup.avx512vbmi.cpp
    - https://github.com/WojciechMula/base64simd/blob/master/decode/pack.avx512bw.cpp
*/
#include "decode_base64_avx512vbmi.h"
#include "chromiumbase64.h"

#include <assert.h>
#include <immintrin.h>

// Note: constants lookup_lo, lookup_hi, pack were
// generated with scripts/avx512vbmi_decode_lookups.py

size_t decode_base64_avx512vbmi(uint8_t* dst, const uint8_t* src, size_t size) {

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

    uint8_t* start = dst;
    while (size >= 64) {

        // 1. load input
        __m512i input = _mm512_loadu_si512((const __m512i*)src);

        // 2. translate from ASCII into 6-bit values
        __m512i translated = _mm512_permutex2var_epi8(lookup_0, input, lookup_1);

        // 2a. check for errors --- convert MSBs to a mask
        const uint64_t mask = _mm512_movepi8_mask(translated | input);
        if (mask != 0) break;

        // 3. pack four 6-bit values into 24-bit words (all within 32-bit lanes)
        // Note: exactly the same procedure as we have in AVX2 version
        // input:  packed_dword([00dddddd|00cccccc|00bbbbbb|00aaaaaa] x 4)
        // merged: packed_dword([00000000|ddddddcc|ccccbbbb|bbaaaaaa] x 4)
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

        _mm512_storeu_si512((__m512*)dst, shuffled);

        src += 64;
        dst += 48;
        size -= 64;
    }

    size_t scalar = chromium_base64_decode((char*)dst, (const char*)src, size);
    if (scalar == MODP_B64_ERROR) return MODP_B64_ERROR;
    return (dst - start) + scalar;
}
