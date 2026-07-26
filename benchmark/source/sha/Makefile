include ../../common.mk

FILE = sha_driver.c sha.c

OUTDIR = $(OUTDIR_BASE)/sha
PREFIX = $(OUTDIR)/sha
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -static -O3 -Wall -DLITTLE_ENDIAN ${FILE} -o $@

clean:
	rm -rf $(TARGET)