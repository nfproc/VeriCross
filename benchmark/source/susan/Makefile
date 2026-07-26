include ../../common.mk

FILE = susan.c

OUTDIR = $(OUTDIR_BASE)/susan
PREFIX = $(OUTDIR)/susan
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -std=c90 -static -O3 ${FILE} -o $@ -lm

clean:
	rm -rf $(TARGET)
