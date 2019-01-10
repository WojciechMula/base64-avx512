# we target only AVX512VBMI Cannon Lake CPU

FLAGS=$(CFLAGS) -O3 -std=c99 -Wall -Wextra -pedantic -march=cannonlake -I./include

BASE64=obj/chromiumbase64.o\
       obj/fastavxbase64.o

ALL=$(BASE64)

# ------------------------------------------------------------

all: $(ALL)

obj/chromiumbase64.o: src/base64/chromiumbase64.c include/chromiumbase64.h
	$(CC) $(FLAGS) $< -c -o $@

obj/fastavxbase64.o: src/base64/fastavxbase64.c include/fastavxbase64.h
	$(CC) $(FLAGS) $< -c -o $@

# ------------------------------------------------------------

clean:
	$(RM) $(ALL)

