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

static const int repeat = 50;


void testencode(const char * data, size_t datalength, bool verbose) {
  if(verbose) printf("encode a base64 input of  %zu bytes, ",datalength);
  char * buffer = malloc(datalength * 2); // we allocate plenty of memory
  size_t expected =   chromium_base64_encode(buffer, data,  datalength);
  if(verbose) printf("encoded size = %zu \n",expected);
  BEST_TIME_NOCHECK(memcpy(buffer, data, datalength),  , repeat, datalength,verbose);
  BEST_TIME(chromium_base64_encode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_CHECK(fast_avx2_base64_encode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_CHECK(encode_base64_avx512vbmi((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME_CHECK(encode_base64_avx512vl((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);
  free(buffer);
  if(verbose) printf("\n");
}


void testdecode(const char * data, size_t datalength, bool verbose) {
  if(verbose) printf("decoding a base64 input of  %zu bytes, ",datalength);
  if (datalength < 4 || (datalength % 4 != 0)) {
    printf("size should be divisible by 4 bytes.\n");
    return;
  }
  char * buffer = malloc(datalength * 2); // we allocate plenty of memory
  size_t expected =  chromium_base64_decode(buffer, data,  datalength);
  if(verbose) printf("original size = %zu \n",expected);
  BEST_TIME_NOCHECK(memcpy(buffer, data, datalength),  , repeat, datalength,verbose);
  BEST_TIME(chromium_base64_decode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);

  BEST_TIME(fast_avx2_base64_decode(buffer, data, datalength), (int) expected, , repeat, datalength,verbose);
  BEST_TIME(decode_base64_avx512vbmi((uint8_t*)buffer, (const uint8_t*)data, datalength), (int) expected, , repeat, datalength,verbose);

  free(buffer);
  if(verbose) printf("\n");
}

//void test_real_data();

typedef struct RealData {
    const char* description;
    const char* path;
} RealData;

typedef struct MemoryArray {
    char*  bytes;
    size_t size;
} MemoryArray;

void load_file(const char* path, MemoryArray* data);

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
        free(data.bytes);
        item++;
    }
}

void load_file(const char* path, MemoryArray* data) {
    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        printf("Can't open '%s': %s\n", path, strerror(errno));
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    data->size = ftell(f);
    fseek(f, 0, SEEK_SET);

    data->bytes = malloc(data->size);
    if (data->bytes == NULL) {
        puts("allocation failed");
        exit(1);
    }

    if (fread(data->bytes, 1, data->size, f) != data->size) {
        printf("Error reading '%s': %s\n", path, strerror(errno));
        exit(1);
    }


    fclose(f);
}








int main() {
  RDTSC_SET_OVERHEAD(rdtsc_overhead_func(1), repeat);

  printf("Testing first with random data.\n");
  const int N = 2048;
  char randombuffer[N];
  for(int k = 0; k < N; ++k) randombuffer[k] = rand();
  const char * decodingfilename = "decodingperf.txt";
  const char * encodingfilename = "encodingperf.txt";
  printf("See files %s %s ... \n", encodingfilename,decodingfilename);
  if ( freopen(decodingfilename,"w",stdout) == NULL) {
    printf("error opening %s \n", decodingfilename);
  }

  printf("#displaying cycles per input bytes for memcpy and decoders: chromium, AVX2, AVX512VBMI; first column is number of bytes\n");

  for(int l = 8; l <= N; l ++) {
    printf("%d ",l);
    char * code = (char*) malloc(chromium_base64_encode_len(l));
    int codedlen = chromium_base64_encode(code, randombuffer, l);
    testdecode(code, codedlen, false);
    free(code);
    printf("\n");

  }

  if ( freopen(encodingfilename,"w",stdout) == NULL) {
    printf("error opening %s \n", encodingfilename);
  }
  printf("#displaying cycles per input bytes for memcpy and encoders: chromium, AVX2, AVX512VBMI, AVX512VL, first column is number of bytes\n");
  for(int l = 8; l <= N; l ++) {
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


