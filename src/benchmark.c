#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include "benchmark.h"
#include "chromiumbase64.h"
#include "fastavxbase64.h"
#include "encode_base64_avx512vbmi.h"
#include "encode_base64_avx512vl.h"
#include "decode_base64_avx512vbmi.h"
#include "decode_base64_avx512vbmi__unrolled.h"
#include "decode_base64_avx512vbmi_despace.h"
#include "avx512memcpy.h"
#include "memalloc.h"
#include "load_file.h"

static const int repeat = 100;
static const int alignment = 2048;

void testencode(const char * data, size_t datalength, bool verbose) {
  if(verbose) printf("encode a base64 input of  %zu bytes, ",datalength);
  char * buffer = aligned_malloc(alignment, datalength * 2);
  size_t expected =   chromium_base64_encode(buffer, data,  datalength);
  if(verbose) printf("encoded size = %zu \n",expected);
  BEST_TIME_NOCHECK("memcpy", memcpy(buffer, data, datalength),  , repeat, datalength,verbose);
  BEST_TIME("Google chrome", chromium_base64_encode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_CHECK("AVX2", fast_avx2_base64_encode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_CHECK("AVX512VBMI", encode_base64_avx512vbmi((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_CHECK("AVX512VL", encode_base64_avx512vl((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_NOCHECK("avx512_memcpy", avx512_memcpy(buffer, data, datalength),  , repeat, datalength,verbose);
  if(!verbose) printf("\t\t\t");
  int speedrepeat = repeat ;
  int statspeed = 10;
  MEASURE_SPEED("memcpy", memcpy(buffer, data, datalength), speedrepeat, statspeed, datalength,verbose);
  MEASURE_SPEED("Google chrome", chromium_base64_encode(buffer, data, datalength) , speedrepeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX2", fast_avx2_base64_encode(buffer, data, datalength), speedrepeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX512VBMI", encode_base64_avx512vbmi((uint8_t*)buffer, (const uint8_t*)data, datalength), speedrepeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX512VL", encode_base64_avx512vl((uint8_t*)buffer, (const uint8_t*)data, datalength) , speedrepeat, statspeed, datalength,verbose);
  aligned_free(buffer);
  if(verbose) printf("\n");
}


void testdecode(const char * data, size_t datalength, bool verbose) {
  if(verbose) printf("decoding a base64 input of  %zu bytes, ",datalength);
  if (datalength < 4 || (datalength % 4 != 0)) {
    printf("size should be divisible by 4 bytes.\n");
    return;
  }
  char * buffer = aligned_malloc(alignment, datalength * 2);
  size_t expected =  chromium_base64_decode(buffer, data,  datalength);
  if(verbose) printf("original size = %zu \n",expected);
  BEST_TIME_NOCHECK("memcpy", memcpy(buffer, data, datalength),  , repeat, datalength,verbose);
  int large_repeat = repeat < 1000 ? 1000 : repeat;
  BEST_TIME("Google chrome", chromium_base64_decode(buffer, data, datalength), (int) expected, , large_repeat , datalength,verbose);

  BEST_TIME("AVX2", fast_avx2_base64_decode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME("AVX512VBMI", decode_base64_avx512vbmi((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME("AVX512VBMI (unrolled)", decode_base64_avx512vbmi__unrolled((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_NOCHECK("AVX512VBMI (despacing)", decode_base64_avx512vbmi_despace((uint8_t*)buffer, (const uint8_t*)data, datalength), , repeat, datalength,verbose);
  BEST_TIME_NOCHECK("avx512_memcpy", avx512_memcpy(buffer, data, datalength),  , repeat, datalength,verbose);
  if(!verbose) printf("\t\t\t");
  int statspeed = 10;
  MEASURE_SPEED("memcpy", memcpy(buffer, data, datalength), large_repeat, statspeed,  datalength,verbose);
  MEASURE_SPEED("Google chrome", chromium_base64_decode(buffer, data, datalength) , large_repeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX2", fast_avx2_base64_decode(buffer, data, datalength), large_repeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX512VBMI", decode_base64_avx512vbmi((uint8_t*)buffer, (const uint8_t*)data, datalength), large_repeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX512VBMI (unrolled)", decode_base64_avx512vbmi__unrolled((uint8_t*)buffer, (const uint8_t*)data, datalength), large_repeat, statspeed, datalength,verbose);
  MEASURE_SPEED("AVX512VBMI (despacing)", decode_base64_avx512vbmi_despace((uint8_t*)buffer, (const uint8_t*)data, datalength),  large_repeat, statspeed, datalength,verbose);
  aligned_free(buffer);
  if(verbose) printf("\n");
}

typedef struct RealData {
    const char* description;
    const char* path;
} RealData;

RealData real_data[] = {
    {"lena [jpg]",              "data/lena_color_512.base64"},
    {"peppers [jpg]",           "data/peppers_color.base64"},
    {"mandril [jpg]",           "data/mandril_color.base64"},
    {"moby_dick [text]",        "data/moby_dick.base64"},
    {"google logo [png]",       "data/googlelogo.base64"},
    {"bing.com social icons [png]", "data/bing.base64"},
    {NULL, NULL}
};


static inline size_t despace(char *bytes, size_t howmany) {
  size_t i = 0, pos = 0;
  while (i < howmany) {
    const char c = bytes[i++];
    bytes[pos] = c;
    pos += ((unsigned char)c > 32 ? 1 : 0);
  }
  return pos;
}



void test_real_data(bool removespaces) {
    
    MemoryArray data;
    RealData* item;
    
    for (item = real_data; item->description != NULL; item++) {
        printf("%s\n", item->description);
        printf("loading file %s \n", item->path);
        load_file(item->path, &data);
        if(removespaces) {
          printf("removing spaces (as a preliminary step), init size = %zu, ", data.size);
          data.size = despace(data.bytes,data.size);
          printf(" final size = %zu \n", data.size);
        }
        testdecode(data.bytes, data.size, true);
        aligned_free(data.bytes);
        item++;
    }
}


int main() {
  RDTSC_SET_OVERHEAD(rdtsc_overhead_func(1), repeat);

  printf("Testing first with random data.\n");
  const int N = 2048 * 16;
  char randombuffer[N];
  for(int k = 0; k < N; ++k) randombuffer[k] = rand();
  const char * decodingfilename = "decodingperf.txt";
  const char * encodingfilename = "encodingperf.txt";
  printf("See files %s %s ... \n", encodingfilename,decodingfilename);
  if ( freopen(decodingfilename,"w",stdout) == NULL) {
    printf("error opening %s \n", decodingfilename);
  }

  printf("#displaying cycles per input bytes for memcpy and decoders: chromium, AVX2, AVX512VBMI, AVX512VBMI-unrolled, AVX512VBMI-despacing; first column is number of bytes\n");
  printf("#Following the cycles per inpt bytes, we provide the GB/s (speeds) \n");
  printf("#Each measure is given as a triple (mean, min, max)\n");
  for(int l = 256; l <= N; l+=64) {
   printf("%d ",l);
    char * code = (char*) aligned_malloc(alignment, chromium_base64_encode_len(l));
    int codedlen = chromium_base64_encode(code, randombuffer, l);
    testdecode(code, codedlen, false);
    aligned_free(code);
    printf("\n");
  }

  if ( freopen(encodingfilename,"w",stdout) == NULL) {
    printf("error opening %s \n", encodingfilename);
  }
  printf("#displaying cycles per input bytes for memcpy and encoders: chromium, AVX2, AVX512VBMI, AVX512VL, AVX512-memcpy first column is number of bytes\n");
  printf("#Following the cycles per inpt bytes, we provide the GB/s (speeds) \n");
  printf("#Each measure is given as a triple (mean, min, max)\n");
  for(int l = 256; l <= N; l+=64) {
    printf("%d ",l);
    testencode(randombuffer, l, false);
    printf("\n");
  }
  const char * ttystr = "/dev/tty";
  if ( freopen(ttystr,"w",stdout) == NULL ) {
     printf("error opening %s \n", ttystr);
  }

  printf("Testing with real data.\n");
  bool removespaces = true;
  test_real_data(removespaces);

  return EXIT_SUCCESS;
}


