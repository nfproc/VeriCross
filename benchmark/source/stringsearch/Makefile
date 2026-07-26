include ../../common.mk

FILE = bmhasrch.c bmhisrch.c bmhsrch.c pbmsrch_small.c 

OUTDIR = $(OUTDIR_BASE)/stringsearch
PREFIX = $(OUTDIR)/search_small
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -std=c90 -static -O3 ${FILE} -o $@

clean:
	rm -rf $(TARGET)
