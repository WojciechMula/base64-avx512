/*
    This program is meant to be a simple email-like data parser. It's not a
    fully-fledged library, it mimics application of base64 decoder.
    
    When the parser detects beginning of bas64-encoded data it passes control
    to our base64 decoder and then the decoder is responsible for 1)
    **inplace** decode data 2) detect end of base64-encoded data. Then it
    returns back to parser, returning two numbers: how many bytes were read and
    written. Then parser might consume decoded data (in example we write it
    back to disc) and carry on parsing.
 
*/
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include "memalloc.h"
#include "load_file.h"
#include "decode_base64_avx512vbmi_despace.h"
#include <sys/time.h>

static uint64_t clock(void) {
        static struct timeval T;
        gettimeofday(&T, NULL);
        return (T.tv_sec * 1000000) + T.tv_usec;
}


typedef void (*consume_attachment_function)(const uint8_t* input, size_t size, void* extra);

// Note that we're going to modify data-inplace!
// It's way easier and faster than making copies.
void decode_email(char* data, size_t size, consume_attachment_function consume, void* extra) {
    
    char* cursor = data;
    char* end    = data + size;

    int linelen;

#define SKIP_LINE                                       \
    do {                                                \
        char* start = cursor;                           \
        for (/**/; *cursor != '\n'; cursor++);          \
        linelen = (cursor - start);                     \
        if (*cursor == '\n')                            \
            cursor++;                                   \
    } while (0);

    const uint64_t t_start = clock();

    while (cursor < end) {

        // skip empty lines
        while (*cursor == '\n')
            cursor++;

        // skip lines --- in fact here we'd have parsing
        // i.e. detecting keywords, etc.
        do {
            SKIP_LINE;
        } while (linelen > 0);

        // there's an empty line after the attachment header
        if (linelen == 0) {
            puts("detected BASE64 data");
            uint8_t* src_ptr = (uint8_t*)cursor;
            const uint64_t t1 = clock();
            const size_t k = decode_base64_avx512vbmi_despace_email((uint8_t*)cursor, &src_ptr, end - cursor);
            const uint64_t t2 = clock();
            if (k == (size_t)-1) {
                printf("... BASE64 data is broken (the current offset is %lu)\n", cursor - data);
                exit(2);
            } else {
                printf("... decoded %lu bytes in %lu us\n", k, t2 - t1);
                if (consume != NULL) {
                    consume((uint8_t*)cursor, k, extra);
                }
            }

            cursor = (char*)src_ptr;
            if (*cursor == '\n')
                cursor++;

            SKIP_LINE;
            SKIP_LINE;
        }
    }
    
    const uint64_t t_end = clock();

    printf("Whole procedure completed in %lu us\n", t_end - t_start);
}

void save_attachment(const uint8_t* input, size_t size, void* extra) {
    
    int* number = (int*)extra;
    char filename[256];
    snprintf(filename, sizeof(filename), "attachement%d.dat", *number);

    *number += 1;

    printf("Saving attachement as %s\n", filename);
    FILE* f = fopen(filename, "wb");
    fwrite(input, 1, size, f);
    fclose(f);
}

void usage(const char* progname) {
    printf("Usage: %s FILE\n", progname);
    puts("");
    puts("FILE is a e-mail like message produced by scripts/email-generator.py");
}

int main(int argc, char* argv[]) {
    
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* inputfile = argv[1];

    printf("Loading %s ... ", inputfile); fflush(stdout);
    MemoryArray email; 
    load_file(inputfile, &email);
    puts("done");

    int number = 0;
    decode_email(email.bytes, email.size, save_attachment, &number);

    return EXIT_SUCCESS;
}
