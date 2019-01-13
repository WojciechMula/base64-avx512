/*
    sources:
    - https://github.com/WojciechMula/base64simd/blob/master/decode/decode.avx512vbmi.cpp
    - https://github.com/WojciechMula/base64simd/blob/master/decode/lookup.avx512vbmi.cpp
    - https://github.com/WojciechMula/base64simd/blob/master/decode/pack.avx512bw.cpp

    despace code by Zach Wegner --- see https://news.ycombinator.com/item?id=18834741
*/
#include "decode_base64_avx512vbmi_despace.h"
#include "chromiumbase64.h"

#include <assert.h>
#include <immintrin.h>

// Note: constants lookup_lo, lookup_hi, pack were
// generated with scripts/avx512vbmi_decode_lookups.py

size_t despace(uint8_t* dst, const uint8_t* src, size_t size);

size_t decode_base64_avx512vbmi_despace(uint8_t* dst, const uint8_t* src, size_t size) {

    // lookup:
    // - 'A'-'Z', 'a'-'z', '0'-'9', '+', '/'  : base64 6-bit values
    // - ' ', '\n', '\r'                      : 0x40 [6th bit set]
    // - all other chars                      : 0x80 [7th bit set]

    const __m512i lookup_0 = _mm512_setr_epi32(
                                0x80808080, 0x80808080, 0x80408080, 0x80804080,
                                0x80808080, 0x80808080, 0x80808080, 0x80808080,
                                0x80808040, 0x80808080, 0x3e808080, 0x3f808080,
                                0x37363534, 0x3b3a3938, 0x80803d3c, 0x80808080);
    const __m512i lookup_1 = _mm512_setr_epi32(
                                0x02010080, 0x06050403, 0x0a090807, 0x0e0d0c0b,
                                0x1211100f, 0x16151413, 0x80191817, 0x80808080,
                                0x1c1b1a80, 0x201f1e1d, 0x24232221, 0x28272625,
                                0x2c2b2a29, 0x302f2e2d, 0x80333231, 0x80808080);

    // despace constants
    const uint64_t index_masks[6] = {
        0xaaaaaaaaaaaaaaaa,
        0xcccccccccccccccc,
        0xf0f0f0f0f0f0f0f0,
        0xff00ff00ff00ff00,
        0xffff0000ffff0000,
        0xffffffff00000000,
    };

    const __m512i index_bits[6] = {
        _mm512_set1_epi8(1),
        _mm512_set1_epi8(2),
        _mm512_set1_epi8(4),
        _mm512_set1_epi8(8),
        _mm512_set1_epi8(16),
        _mm512_set1_epi8(32),
    };

    uint8_t* start = dst;
    size_t scalar = 0;

    while (size >= 64) {

        // 1. load input
        __m512i input = _mm512_loadu_si512((const __m512i*)src);

        // 2. translate from ASCII into 6-bit values
        __m512i translated = _mm512_permutex2var_epi8(lookup_0, input, lookup_1);

        // 2a. check for errors --- convert MSBs to a mask
        const uint64_t error_mask = _mm512_movepi8_mask(translated | input);
        if (error_mask != 0) return MODP_B64_ERROR;

        // 3. check if we need there are spaces (bit 6th)
        uint64_t whitespace_mask = _mm512_movepi8_mask(_mm512_add_epi8(translated, translated));
        if (whitespace_mask == 0) {
            // no despacing
            const __m512i merge_ab_and_bc = _mm512_maddubs_epi16(translated, _mm512_set1_epi32(0x01400140));
            const __m512i merged = _mm512_madd_epi16(merge_ab_and_bc, _mm512_set1_epi32(0x00011000));

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
        } else {
            // maybe we should explicitly handle case popcount(whitespace_mask) < 4? [Wojciech]

            // despace --- Zach's algorithm starts here
            uint64_t characters_mask = ~whitespace_mask;

            __m512i indices = _mm512_set1_epi8(0);
            for (size_t index = 0; index < 6; index++) {
                uint64_t m = _pext_u64(index_masks[index], characters_mask);
                indices = _mm512_mask_add_epi8(indices, m, indices, index_bits[index]);
            }

            translated = _mm512_permutexvar_epi8(indices, translated);
            // end of despace

            // base64 algorithm
            const __m512i merge_ab_and_bc = _mm512_maddubs_epi16(translated, _mm512_set1_epi32(0x01400140));
            const __m512i merged = _mm512_madd_epi16(merge_ab_and_bc, _mm512_set1_epi32(0x00011000));

            const __m512i pack = _mm512_setr_epi32(
                                    0x06000102, 0x090a0405, 0x0c0d0e08, 0x16101112,
                                    0x191a1415, 0x1c1d1e18, 0x26202122, 0x292a2425,
                                    0x2c2d2e28, 0x36303132, 0x393a3435, 0x3c3d3e38,
                                    0x00000000, 0x00000000, 0x00000000, 0x00000000);
            const __m512i shuffled = _mm512_permutexvar_epi8(pack, merged);

            _mm512_storeu_si512((__m512*)dst, shuffled);

            /*
                Getting number of bytes to skip is a bit tricky.

                Despacing yields arbitrary number of bytes, but in base64 we
                process groups of 4 bytes. Thus the number of **despaced** bytes
                that contributed to output is 4 * popcount(characters_mask) / 4.
                However, the number of **input** bytes that have to be skipped
                depends on the spaces appeared before the last consumed byte.

                Solution [in examples we assume input = 32 bytes, 64 is just too wide]

                The input data we already have from despacing:

                whitespace_mask   = 0010_0101_0010_0010_0100_0000_1000_0110
                characters_mask   = 1101_1010_1101_1101_1011_1111_0111_1001 - popcnt = 23
                characters_compr  = 1111_1111_1111_1111_1111_1110_0000_0000
                                    ^                                     ^
                                    LSB                                 MSB

                1. We trim `characters_compr` to have exactly 4 * (23/4) + 1 = 21 bits
                   This extra bit is needed to skip also trailing whitespaces.

                characters_converted = 1111_1111_1111_1111_1111_1000_0000_0000

                2. The converted characters is expanded using PDEP with
                   `character_mask` as a pattern:

                expanded = _pdep_u64(characters_converted, characters_mask) =>
                           1101_1010_1101_1101_1011_1111_0111_1001
                           1111_1111_1111_1111_1111_1000_0000_0000
                         = 1101_1010_1101_1101_1011_1111_0111_0000

                3. The last step is to get position of last bit set

                input_skip = 64 - __builtin_clzll(expanded) - !!(count != rounded);
                           = 1101_1010_1101_1101_1011_1111_0111_0000
                             ^                                ^
                             LSB                             28

                The correction is needed for cases when count != rounded, otherwise
                for cases when all converted bytes are consumed we'd end up with
                wrong skip amounts.
            */

            const size_t count   = __builtin_popcountll(characters_mask);
            const size_t rounded = 4 * (count / 4);

            const uint64_t characters_converted = (uint64_t)(-1) >> (64 - rounded - 1);
            const uint64_t expanded             = _pdep_u64(characters_converted, characters_mask);
            const size_t   input_skip           = 64 - __builtin_clzll(expanded) - !!(count != rounded);

            src  += input_skip;
            size -= input_skip;
            dst  += 3 * (count / 4);
        }
    }

    // this is a really slow part
    if (size > 0) {
        uint8_t tmp[128];

        size = despace(tmp, src, size);
        scalar = chromium_base64_decode((char*)dst, (const char*)tmp, size);
        if (scalar == MODP_B64_ERROR) return MODP_B64_ERROR;
    }

    return (dst - start) + scalar;
}


size_t despace(uint8_t* dst, const uint8_t* src, size_t size) {
    uint8_t* orig = dst;
    for (size_t i=0; i < size; i++) {
        if (src[i] == ' ' || src[i] == '\n' || src[i] == '\r')
            continue;

        *dst++ = src[i];
    }

    return dst - orig;
}
