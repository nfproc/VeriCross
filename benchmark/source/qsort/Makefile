include ../../common.mk

FILE = qsort_small.c

OUTDIR = $(OUTDIR_BASE)/qsort
PREFIX = $(OUTDIR)/qsort_small
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -static -O3 ${FILE} -o $@

clean:
	rm -rf $(TARGET)