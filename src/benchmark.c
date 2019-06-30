#define _POSIX_C_SOURCE 200212L // enable posix_memalign

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>

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

static const int repeat = 10;
static const int alignment = 16384;
static const int minimal_volume = 16384;

// to void inlining!
// just a wrapper around memcpy...
__attribute__((noinline)) void *copy(void *restrict dst,
                                     const void *restrict src, size_t n) {
  return memcpy(dst, src, n);
}

void testencode(const char *data, size_t datalength, bool verbose) {
  if (verbose)
    printf("encode a base64 input of  %zu bytes, ", datalength);
  char *buffer = aligned_malloc(alignment, datalength * 2);
  char *tmp = aligned_malloc(alignment, datalength * 2);
  size_t expected = chromium_base64_encode(buffer, data, datalength);
  if (verbose)
    printf("encoded size = %zu \n", expected);
  int speedrepeat = repeat * 10;
  // ensure we have a 16K volume at all times
  while (speedrepeat * datalength < minimal_volume) {
    speedrepeat *= 2;
  }
  int statspeed = 30;
  MEASURE_SPEED("memcpy (base64)", copy(tmp, buffer, expected), speedrepeat,
                statspeed, expected, verbose);
  MEASURE_SPEED("Google chrome",
                chromium_base64_encode(buffer, data, datalength), speedrepeat,
                statspeed, datalength, verbose);
  MEASURE_SPEED("AVX2", fast_avx2_base64_encode(buffer, data, datalength),
                speedrepeat, statspeed, datalength, verbose);
  MEASURE_SPEED("AVX512VL",
                encode_base64_avx512vl((uint8_t *)buffer, (const uint8_t *)data,
                                       datalength),
                speedrepeat, statspeed, datalength, verbose);
  aligned_free(buffer);
  aligned_free(tmp);
  if (verbose)
    printf("\n");
}

void testdecode(const char *data, size_t datalength, bool verbose) {
  if (verbose)
    printf("decoding a base64 input of  %zu bytes, ", datalength);
  if (datalength < 4 || (datalength % 4 != 0)) {
    printf("size should be divisible by 4 bytes.\n");
    return;
  }
  char *buffer = aligned_malloc(alignment, datalength * 2);
  size_t expected = chromium_base64_decode(buffer, data, datalength);
  if (verbose)
    printf("original size = %zu \n", expected);
  int speedrepeat = repeat * 10;
  while (speedrepeat * datalength < minimal_volume) {
    speedrepeat *= 2;
  }
  int statspeed = 30;
  MEASURE_SPEED_WARMUP("memcpy (base64)", copy(buffer, data, datalength),
                       speedrepeat, statspeed * 10, datalength, false);
  MEASURE_SPEED("memcpy (base64)", copy(buffer, data, datalength), speedrepeat,
                statspeed, datalength, verbose);
  MEASURE_SPEED("Google chrome",
                chromium_base64_decode(buffer, data, datalength), speedrepeat,
                statspeed, datalength, verbose);
  MEASURE_SPEED("AVX2", fast_avx2_base64_decode(buffer, data, datalength),
                speedrepeat, statspeed, datalength, verbose);
  MEASURE_SPEED("AVX512VBMI (unrolled)",
                decode_base64_avx512vbmi__unrolled(
                    (uint8_t *)buffer, (const uint8_t *)data, datalength),
                speedrepeat, statspeed, datalength, verbose);
  aligned_free(buffer);
  if (verbose)
    printf("\n");
}

typedef struct RealData {
  const char *description;
  const char *path;
} RealData;

RealData real_data[] = { { "lena [jpg]", "data/lena_color_512.base64" },
                         { "peppers [jpg]", "data/peppers_color.base64" },
                         { "mandril [jpg]", "data/mandril_color.base64" },
                         { "moby_dick [text]", "data/moby_dick.base64" },
                         { "google logo [png]", "data/googlelogo.base64" },
                         { "bing.com social icons [png]", "data/bing.base64" },
                         { NULL, NULL } };

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
  RealData *item;

  for (item = real_data; item->description != NULL; item++) {
    printf("%s\n", item->description);
    printf("loading file %s \n", item->path);
    load_file(item->path, &data);
    if (removespaces) {
      printf("removing spaces (as a preliminary step), init size = %zu, ",
             data.size);
      data.size = despace(data.bytes, data.size);
      printf(" final size = %zu \n", data.size);
    }
    testdecode(data.bytes, data.size, true);
    char *buffer = aligned_malloc(alignment, data.size * 2);
    size_t origsize = chromium_base64_decode(buffer, data.bytes, data.size);
    testencode(buffer, origsize, true);
    aligned_free(buffer);
    aligned_free(data.bytes);
    item++;
  }
}

int main(int argc, char *argv[]) {
  RDTSC_SET_OVERHEAD(rdtsc_overhead_func(1), repeat);
  char *outputdir = ".";
  if (argc > 1) {
    outputdir = argv[1];
    printf("Outputting results to directory %s.\n", outputdir);
    DIR *dir = opendir(outputdir);
    if (dir) {
      closedir(dir);
    } else {
      printf("I can't open the directory.\n");
      return EXIT_FAILURE;
    }
  }
  printf("Testing first with random data.\n");
  const int N = 2048 * 32;
  const int step = 2048;
  char randombuffer[N];
  for (int k = 0; k < N; ++k)
    randombuffer[k] = rand();
  size_t dirlen = strlen(outputdir);
  if (dirlen > 128) {
    printf("Your directory path is too long.\n");
    return EXIT_FAILURE;
  }
  char decodingfilename[256];
  const char *decodingfilenamej = "decodingperf.txt";
  strcpy(decodingfilename, outputdir);
  decodingfilename[dirlen] = '/';
  strcpy(decodingfilename + dirlen + 1, decodingfilenamej);
  char encodingfilename[256];
  const char *encodingfilenamej = "encodingperf.txt";
  strcpy(encodingfilename, outputdir);
  encodingfilename[dirlen] = '/';
  strcpy(encodingfilename + dirlen + 1, encodingfilenamej);
  char realfilename[256];
  const char *realfilenamej = "realperf.txt";
  strcpy(realfilename, outputdir);
  realfilename[dirlen] = '/';
  strcpy(realfilename + dirlen + 1, realfilenamej);

  printf("See files %s %s %s... \n", encodingfilename, decodingfilename,
         realfilename);
  if (freopen(decodingfilename, "w", stdout) == NULL) {
    printf("error opening %s \n", decodingfilename);
  }

  printf("#displaying speed (GB/s) based on input bytes for memcpy and "
         "decoders: memcpy base64, chromium, AVX2, AVX512; first column is "
         "number of bytes\n");
  printf("#Each measure is given as a triple (median, min, max)\n");
  for (int l = 1024; l <= N; l += step) {
    printf("%d ", l);
    char *code =
        (char *)aligned_malloc(alignment, chromium_base64_encode_len(l));
    int codedlen = chromium_base64_encode(code, randombuffer, l);
    testdecode(code, codedlen, false);
    aligned_free(code);
    printf("\n");
  }

  if (freopen(encodingfilename, "w", stdout) == NULL) {
    printf("error opening %s \n", encodingfilename);
  }
  printf("#displaying speed (GB/s) based on input bytes for memcpy and "
         "encoders: memcpy base64, chromium, AVX2, AVX512 first column is "
         "number of bytes\n");
  printf("#Each measure is given as a triple (median, min, max)\n");
  for (int l = 1024; l <= N; l += step) {
    printf("%d ", l);
    testencode(randombuffer, l, false);
    printf("\n");
  }
  if (freopen(realfilename, "w", stdout) == NULL) {
    printf("error opening %s \n", realfilename);
  }

  printf("Testing with real data.\n");
  bool removespaces = true;
  test_real_data(removespaces);
  const char *ttystr = "/dev/tty";
  if (freopen(ttystr, "w", stdout) == NULL) {
    printf("error opening %s \n", ttystr);
  }
  printf("Done.\n");
  return EXIT_SUCCESS;
}
