# we target only AVX512VBMI Cannon Lake CPU

FLAGS=$(CFLAGS) -O3 -ftree-vectorize -funroll-loops -std=c99 -Wall -Wextra -pedantic -I./include
FLAGS+=-march=native
#FLAGS+=-march=cannonlake

BASE64=obj/chromiumbase64.o\
       obj/fastavxbase64.o\
       obj/encode_base64_avx512vbmi.o\
       obj/decode_base64_avx512vbmi.o\
       obj/encode_base64_avx512vl.o\
       obj/decode_base64_avx512vbmi_despace.o\
       obj/decode_base64_avx512vbmi__unrolled.o

ALL=$(BASE64)\
    unit\
    unit_despace\
    benchmark\
    benchmark_despace

# ------------------------------------------------------------

all: $(ALL)

obj/chromiumbase64.o: src/base64/chromiumbase64.c include/chromiumbase64.h
	$(CC) $(FLAGS) $< -c -o $@

obj/fastavxbase64.o: src/base64/fastavxbase64.c include/fastavxbase64.h
	$(CC) $(FLAGS) $< -c -o $@

obj/encode_base64_avx512vbmi.o: src/base64/encode_base64_avx512vbmi.c include/encode_base64_avx512vbmi.h
	$(CC) $(FLAGS) $< -c -o $@

obj/encode_base64_avx512vl.o: src/base64/encode_base64_avx512vl.c include/encode_base64_avx512vl.h
	$(CC) $(FLAGS) $< -c -o $@

obj/decode_base64_avx512vbmi.o: src/base64/decode_base64_avx512vbmi.c include/decode_base64_avx512vbmi.h
	$(CC) $(FLAGS) $< -c -o $@

obj/decode_base64_avx512vbmi__unrolled.o: src/base64/decode_base64_avx512vbmi__unrolled.c include/decode_base64_avx512vbmi__unrolled.h
	$(CC) $(FLAGS) $< -c -o $@

obj/decode_base64_avx512vbmi_despace.o: src/base64/decode_base64_avx512vbmi_despace.c include/decode_base64_avx512vbmi_despace.h
	$(CC) $(FLAGS) $< -c -o $@

unit: src/unit.c $(BASE64)
	$(CC) $(FLAGS) $< $(BASE64) -o $@

unit_despace: src/unit_despace.c $(BASE64)
	$(CC) $(FLAGS) $< $(BASE64) -o $@

benchmark: src/benchmark.c src/benchmark.h $(BASE64)
	$(CC) $(FLAGS) $< $(BASE64) -o $@

benchmark_despace: src/benchmark_despace.c src/benchmark.h $(BASE64)
	$(CC) $(FLAGS) $< $(BASE64) -o $@

# ------------------------------------------------------------

clean:
	$(RM) $(ALL)

