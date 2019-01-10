# we target only AVX512VBMI Cannon Lake CPU

FLAGS=$(CFLAGS) -O3 -std=c99 -Wall -Wextra -pedantic -march=cannonlake -I./include

BASE64=obj/chromiumbase64.o\
       obj/fastavxbase64.o

ALL=$(BASE64)\
    unit\
    benchmark

# ------------------------------------------------------------

all: $(ALL)

obj/chromiumbase64.o: src/base64/chromiumbase64.c include/chromiumbase64.h
	$(CC) $(FLAGS) $< -c -o $@

obj/fastavxbase64.o: src/base64/fastavxbase64.c include/fastavxbase64.h
	$(CC) $(FLAGS) $< -c -o $@

unit: src/unit.c $(BASE64)
	$(CC) $(FLAGS) $< $(BASE64) -o $@

benchmark: src/benchmark.c src/benchmark.h $(BASE64)
	$(CC) $(FLAGS) $< $(BASE64) -o $@

# ------------------------------------------------------------

clean:
	$(RM) $(ALL)

