// copy from: https://github.com/WojciechMula/base64simd/blob/master/encode/encode.avx512vbmi.cpp
#include "encode_base64_avx512vbmi.h"
#include "chromiumbase64.h"

#include <immintrin.h>

void encode_base64_avx512vbmi(uint8_t* dst, const uint8_t* src, size_t size) {

    static const char* lookup_tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    const __m512i shuffle_input = _mm512_setr_epi32(
                0x01020001, 0x04050304, 0x07080607, 0x0a0b090a,
                0x0d0e0c0d, 0x10110f10, 0x13141213, 0x16171516,
                0x191a1819, 0x1c1d1b1c, 0x1f201e1f, 0x22232122,
                0x25262425, 0x28292728, 0x2b2c2a2b, 0x2e2f2d2e);
    const __m512i lookup = _mm512_loadu_si512((const __m512i*)(lookup_tbl));

    while (size >= 64) {
        const __m512i v = _mm512_loadu_si512((const __m512i*)src);

        // reorder bytes
        const __m512i in = _mm512_permutexvar_epi8(shuffle_input, v);

        // in    = [bbbbcccc|ccdddddd|aaaaaabb|bbbbcccc]
        // t0    = [0000cccc|cc000000|aaaaaa00|00000000]
        const __m512i t0 = _mm512_and_si512(in, _mm512_set1_epi32(0x0fc0fc00));
        // t1    = [00000000|00cccccc|00000000|00aaaaaa] (c >> 6, a >> 10)
        const __m512i t1 = _mm512_srlv_epi16(t0, _mm512_set1_epi32(0x0006000a));
        // t2    = [ccdddddd|00000000|aabbbbbb|cccc0000]
        const __m512i t2 = _mm512_sllv_epi16(in, _mm512_set1_epi32(0x00080004));

        // indices = 0x3f003f00 ? t2 : t1
        //         = [00dddddd|00cccccc|00bbbbbb|00aaaaaa]
        const __m512i indices = _mm512_ternarylogic_epi32(_mm512_set1_epi32(0x3f003f00), t2, t1, 0xca);

        // translation into ASCII
        const __m512i result = _mm512_permutexvar_epi8(indices, lookup);

        _mm512_storeu_si512((__m512i*)dst, result);

        dst += 64;
        src += 48;
        size -= 48;
    }

    if (size > 0) {
        chromium_base64_encode((char*)dst, (const char*)src, size);
    }
}
