include ../../common.mk

FILE1 = basicmath_small.c   rad2deg.c  cubic.c   isqrt.c
FILE2 = basicmath_large.c   rad2deg.c  cubic.c   isqrt.c

OUTDIR = $(OUTDIR_BASE)/basicmath
PREFIX1 = $(OUTDIR)/basicmath_small
TARGET = $(PREFIX1).elf $(PREFIX1).objdump $(PREFIX1).bin

all: $(TARGET)

$(PREFIX1).elf: ${FILE1} Makefile
	$(CROSS)-gcc -static -O3 ${FILE1} -o $@ -lm

clean:
	rm -rf $(TARGET)
