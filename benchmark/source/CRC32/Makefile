include ../../common.mk

FILE = crc_32.c

OUTDIR = $(OUTDIR_BASE)/CRC32
PREFIX = $(OUTDIR)/crc
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -std=c90 -static -O3 ${FILE} -o $@

clean:
	rm -rf $(TARGET)