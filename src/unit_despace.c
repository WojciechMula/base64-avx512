
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


void test_wikidata();
void test_synthetic();

int main() {
    test_wikidata();
    test_synthetic();
    return 0;
}


size_t count_bytes(uint8_t* data, size_t size, uint8_t val) {
    size_t res = 0;
    for (size_t i=0; i < size; i++)
        res += (data[i] == val);

    return res;
}

void test_synthetic() {

    puts("Test sythetic data (might take a while)");

    size_t size = 64 * 3;
    uint8_t buffer[size + 1];
    uint8_t dest[size + 1];

    buffer[size] = '\0'; // make it puts-friendly

    uint8_t BASE64_CHAR = 'V';

    int passed = 0;
    int failed = 0;

    for (size_t i=0; i < size; i++) {
        for (size_t j=i; j < size; j++) {
            for (size_t k=j; k < size; k++) {
                memset(buffer, BASE64_CHAR, size);
                buffer[i] = ' ';
                buffer[j] = ' ';
                buffer[k] = ' ';


                // 1. assure that input with spaces is valid base64 string
                size_t base64_len = count_bytes(buffer, size, BASE64_CHAR);
                uint8_t* input = buffer;
                size_t   input_size = size;
                while (base64_len % 4 != 0) {
                    if (*input == BASE64_CHAR) {
                        input++;
                        base64_len -= 1;
                        input_size -= 1;
                    } else if (input[input_size - 1] == BASE64_CHAR) {
                        base64_len -= 1;
                        input_size -= 1;
                    } else {
                        break;
                    }
                }

                // 2. test
                if (base64_len % 4 == 0) {
                    size_t decoded_len = decode_base64_avx512vbmi_despace(dest, input, input_size);
                    if (decoded_len == (size_t)(-1)) {
                        printf("failed for i = %lu j = %lu k = %lu\n", i, j, k);
                        printf("'%*s'\n", (int)input_size, input);
                        failed += 1;
                    }
                    else
                        passed += 1;
                }
            }
        }
    }

    printf("Summary: %d passed, %d failed\n", passed, failed);
}


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
    puts((const char*)source);
    puts((const char*)dest);
    fail = 1;
  }

  free(dest);

  if (fail) exit(1);
}

void test_wikidata() {

  puts("Test wikidata");

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
