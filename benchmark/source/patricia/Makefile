include ../../common.mk

FILE = patricia.c patricia_test.c

OUTDIR = $(OUTDIR_BASE)/patricia
PREFIX = $(OUTDIR)/patricia
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -static -O3 ${FILE} -o $@

clean:
	rm -rf $(TARGET)