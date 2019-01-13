
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "chromiumbase64.h"
#include "encode_base64_avx512vbmi.h"
#include "decode_base64_avx512vbmi_despace.h"

char* insert_spaces(const char* data, double prob);

typedef size_t (*decode_base64_function)(uint8_t* output, const uint8_t* input, size_t size);

void test(
    const char* name,
    decode_base64_function decode,
    const char* source,
    const char* coded)
{
  printf("%s\n", name);

  size_t size = strlen(source);
  size_t encoded_len;
  size_t decoded_len_ret;

  encoded_len = chromium_base64_encode_len(size);

  uint8_t* dest = (uint8_t*)malloc(encoded_len);
  decoded_len_ret = decode(dest, (const uint8_t*)coded, strlen(coded));
  int fail = 0;
  if (decoded_len_ret != strlen(source)) {
    printf("wrong length: result = %lu, expected = %lu\n", decoded_len_ret, strlen(source));
    fail = 1;
  }

  if (memcmp(dest, source, size) != 0) {
    puts("different results");
    puts(source);
    puts(dest);
    fail = 1;
  }

  free(dest);

  if (fail) exit(1);
}

int main() {

  // from Wikipedia page
  const char * wikipediasource =
    "Man is distinguished, not only by his reason, but by this singular passion from "
    "other animals, which is a lust of the mind, that by a perseverance of delight "
    "in the continued and indefatigable generation of knowledge, exceeds the short "
    "vehemence of any carnal pleasure.";

  const char * wikipediacoded =
    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
    "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
    "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
    "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
    "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";


    for (double prob = 0.1; prob < 1.0; prob += 0.1) {
        char* encoded_with_spaces = insert_spaces(wikipediacoded, prob);
        
        test("AVX512VBMI (despace)",
             decode_base64_avx512vbmi_despace,
             wikipediasource,
             encoded_with_spaces);

        free(encoded_with_spaces);
    }

    return 0;
}

double randd() {
    return (double)rand() / RAND_MAX;
}

char* insert_spaces(const char* data, double prob) {
    const size_t size = strlen(data);

    size_t newsize = size * 2;
    char*  newdata = (char*)malloc(newsize);
    size_t j = 0;

    for (size_t i=0; i < size; i++) {
        if (randd() > prob) {
            newdata[j] = ' ';
            j += 1;
            if (j == newsize) {
                newsize *= 2;
                newdata  = realloc(newdata, newsize);
            }
        }

        newdata[j] = data[i];
        j += 1;
        if (j == newsize) {
            newsize *= 2;
            newdata  = realloc(newdata, newsize);
        }
    }

    newdata[j] = '\0';

    return newdata;
}
