# we target only AVX512VBMI Cannon Lake CPU

########
# I do not think one should use  '-funroll-loops' as it may harm some operations.
#######
## Similarly, it seems that -ftree-vectorize is not obviously useful.
######
CFLAGS	+= -O3 -std=c99 -Wall -Wextra -pedantic -Iinclude
#FLAGS+=-march=native
CFLAGS	+= -march=cannonlake

BASE64 	= src/base64/chromiumbase64.o \
	  src/base64/fastavxbase64.o \
	  src/base64/encode_base64_avx512vbmi.o \
	  src/base64/decode_base64_avx512vbmi.o \
	  src/base64/encode_base64_avx512vl.o \
	  src/base64/decode_base64_avx512vbmi_despace.o \
	  src/base64/decode_base64_avx512vbmi__unrolled.o
HELPERS = src/load_file.o
BINOBJS	= src/unit.o \
	  src/unit_tail.o \
	  src/unit_despace.o \
	  src/benchmark.o \
	  src/benchmark_despace.o \
	  src/benchmark_email.o

BINS	= unit \
	  unit_despace \
	  benchmark \
	  benchmark_email \
	  benchmark_despace

all: $(BINS)

src/base64/chromiumbase64.o: include/chromiumbase64.h
src/base64/decode_base64_avx512vbmi_despace.o: include/decode_base64_avx512vbmi_despace.h
src/base64/decode_base64_avx512vbmi.o: include/decode_base64_avx512vbmi.h
src/base64/decode_base64_avx512vbmi__unrolled.o: include/decode_base64_avx512vbmi__unrolled.h
src/base64/encode_base64_avx512vbmi.o: include/encode_base64_avx512vbmi.h
src/base64/encode_base64_avx512vl.o: include/encode_base64_avx512vl.h
src/base64/fastavxbase64.o: include/fastavxbase64.h
src/load_file.o: include/load_file.h include/memalloc.h
src/benchmark.o: src/benchmark.h include/memalloc.h include/avx512memcpy.h
src/benchmark_email.o: include/memalloc.h
src/unit_tail.o: src/base64/decode_base64_tail_avx512vbmi.c

benchmark: src/benchmark.o $(BASE64) $(HELPERS)
benchmark_despace: src/benchmark_despace.o $(BASE64)
benchmark_email: src/benchmark_email.o $(BASE64) $(HELPERS)
unit: src/unit.o $(BASE64)
unit_despace: src/unit_despace.o $(BASE64)
unit_tail: src/unit_tail.o \
	src/base64/chromiumbase64.o

$(BINS) unit_tail:
	$(CC) $^ -o $@

clean:
	$(RM) $(BINS)
	$(RM) $(BASE64) $(HELPERS) $(BINOBJS)
