#ifndef MEMALIGN_H
#define MEMALIGN_H

#include <stddef.h>
#include <stdlib.h>

// portable version of  posix_memalign
static inline void *aligned_malloc(size_t alignment, size_t size) {
	void *p;
#ifdef _MSC_VER
	p = _aligned_malloc(size, alignment);
#elif defined(__MINGW32__) || defined(__MINGW64__)
	p = __mingw_aligned_malloc(size, alignment);
#else
	// somehow, if this is used before including "x86intrin.h", it creates an
	// implicit defined warning.
	if (posix_memalign(&p, alignment, size) != 0) return NULL;
#endif
	return p;
}

static inline void aligned_free(void *memblock) {
    if(memblock == NULL) return;
#ifdef _MSC_VER
    _aligned_free(memblock);
#elif defined(__MINGW32__) || defined(__MINGW64__)
    __mingw_aligned_free(memblock);
#else
    free(memblock);
#endif
}

#endif
