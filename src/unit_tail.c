#include "chromiumbase64.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "base64/decode_base64_tail_avx512vbmi.c"

static const char* input_string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#%";

int main() {
    char input[256];
    char encoded[256];
    char decoded[256];
   
    for (int input_size=1; /**/; input_size++) {
        memcpy(input, input_string, strlen(input_string));
        input[input_size] = '\0';

        const int encoded_size = chromium_base64_encode(encoded, input, input_size);
        if (encoded_size > 64)
            break;
       
        puts("");
        printf("input   = '%s', len = %d\n", input, input_size);
        printf("encoded = '%s', len = %d\n", encoded, encoded_size);

        const int decoded_size = decode_base64_tail_avx512vbmi((uint8_t*)decoded,
                                                               (const uint8_t*)encoded,
                                                               encoded_size);
        assert(decoded_size >= 0);

        decoded[decoded_size] = '\0';
        printf("decoded = '%s', len = %d\n", decoded, decoded_size);

        assert(decoded_size == input_size);
        assert(memcmp(decoded, input_string, input_size) == 0);
    }
}
