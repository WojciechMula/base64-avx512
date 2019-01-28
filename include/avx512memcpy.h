#ifndef AVX512MEMCPY_H
#define AVX512MEMCPY_H

#include <string.h>
#include <immintrin.h>

static inline void* avx512_memcpy(void *dst, const void * src, size_t n) {
  if(n >= 64) {
    size_t i = 0;
    if(n >= 4*64) {
      for(; i <= n - 4*64; i+=4*64) {
        __m512i x0 = _mm512_loadu_si512((const char*)src + i);
        __m512i x1 = _mm512_loadu_si512((const char*)src + i + 64);
        __m512i x2 = _mm512_loadu_si512((const char*)src + i + 128);
        __m512i x3 = _mm512_loadu_si512((const char*)src + i + 192);
        _mm512_storeu_si512((char*)dst + i, x0);
        _mm512_storeu_si512((char*)dst + i + 64, x1);
        _mm512_storeu_si512((char*)dst + i + 128, x2);
        _mm512_storeu_si512((char*)dst + i + 192, x3);
     }
    }
    if(n>=64) {
      for(; i <= n - 64; i+=64) {
        __m512i x0 = _mm512_loadu_si512((const char*)src + i);
        _mm512_storeu_si512((char*)dst + i, x0);
      }
    }
    size_t leftover = n % 64;
    memcpy((char*)dst + n - leftover, (const char*)src + n - leftover, leftover);
    return dst;
  } else {
    return memcpy(dst,src,n);
  }
}

#endif
