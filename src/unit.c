
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "chromiumbase64.h"
#include "fastavxbase64.h"


void print_example(const char * source) {
  char * dest1 = (char*) malloc(chromium_base64_encode_len(strlen(source)));
  unsigned int len = chromium_base64_decode(dest1, source,strlen(source));
  unsigned int i = 0;
  for(; i != len; i++) printf("%u ",dest1[i]&0xFF);
  printf("\n");
  free(dest1);
}
void chromium_checkExample(const char * source, const char * coded) {
  printf("chromium codec check.\n");
  unsigned int len;
  unsigned int codedlen;

  char * dest1 = (char*) malloc(chromium_base64_encode_len(strlen(source)));
  codedlen = chromium_base64_encode(dest1, source, strlen(source));
  assert(strncmp(dest1,coded,codedlen) == 0);
  char *dest2 = (char*) malloc(chromium_base64_decode_len(codedlen));
  len = chromium_base64_decode(dest2, coded, codedlen);
  assert(len == strlen(source));
  assert(strncmp(dest2,source,strlen(source)) == 0);
  char *dest3 = (char*) malloc(chromium_base64_decode_len(codedlen));
  len = chromium_base64_decode(dest3, dest1, codedlen);
  assert(len == strlen(source));
  assert(strncmp(dest3,source,strlen(source)) == 0);
  free(dest1);
  free(dest2);
  free(dest3);
}

void fast_avx2_checkExample(const char * source, const char * coded) {
  printf("fast_avx2 codec check.\n");
  size_t len;
  size_t codedlen;

  char * dest1 = (char*) malloc(chromium_base64_encode_len(strlen(source)));
  codedlen = fast_avx2_base64_encode(dest1, source, strlen(source));
  assert(strncmp(dest1,coded,codedlen) == 0);
  char *dest2 = (char*) malloc(chromium_base64_decode_len(codedlen));
  len = fast_avx2_base64_decode(dest2, coded, codedlen);
  assert(len == strlen(source));
  assert(strncmp(dest2,source,strlen(source)) == 0);
  char *dest3 = (char*) malloc(chromium_base64_decode_len(codedlen));
  len = fast_avx2_base64_decode(dest3, dest1, codedlen);
  assert(len == strlen(source));
  assert(strncmp(dest3,source,strlen(source)) == 0);
  free(dest1);
  free(dest2);
  free(dest3);
}

static const uint8_t base64_table_enc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";
void fast_avx2_checkError() {
  printf("fast_avx2 codec error check.\n");
  char source[64];
  char dest[48];
  for(unsigned int z = 0; z < 64; ++z) {
    for(int i = 0; i < 64; ++i) source[i] = base64_table_enc[z];
    int len = fast_avx2_base64_decode(dest, source, 64);
    assert(len == 48);
  }
  for(int z = 0; z < 256 ; ++z) {
    bool in_list = false;
    for(unsigned int zz = 0; zz < 64 ; ++zz)
      if(base64_table_enc[zz] == z) in_list = true;
    if(! in_list) {
      for(int pos = 0; pos < 32; ++pos) {
        for(int i = 0; i < 64; ++i) source[i] = 'A';
        source[pos] = z;
        int len = fast_avx2_base64_decode(dest, source, 64);
        assert(len == -1);
      }
    }
  }
}


int main() {
  fast_avx2_checkError();

  // from Wikipedia page
  const char * wikipediasource = "Man is distinguished, not only by his reason, but by this singular passion from \
other animals, which is a lust of the mind, that by a perseverance of delight \
in the continued and indefatigable generation of knowledge, exceeds the short \
vehemence of any carnal pleasure.";
  const char * wikipediacoded = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz\
IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg\
dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu\
dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo\
ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

  // from https://gobyexample.com/base64-encoding
  const char * gosource = "abc123!?$*&()'-=@~";
  const char * gocoded = "YWJjMTIzIT8kKiYoKSctPUB+";

  // from https://www.tutorialspoint.com/java8/java8_base64.htm
  const char * tutosource = "TutorialsPoint?java8";
  const char * tutocoded = "VHV0b3JpYWxzUG9pbnQ/amF2YTg=";

  chromium_checkExample(wikipediasource,wikipediacoded);
  chromium_checkExample(gosource,gocoded);
  chromium_checkExample(tutosource,tutocoded);

  fast_avx2_checkExample(wikipediasource,wikipediacoded);
  fast_avx2_checkExample(gosource,gocoded);
  fast_avx2_checkExample(tutosource,tutocoded);

  print_example("R0lGODlhAQABAIAAAP///wAAACwAAAAAAQABAAACAkQBADs=");
	printf("Code looks ok.\n");
  return 0;
}
