include ../../common.mk

FILE = bitcnt_1.c bitcnt_2.c bitcnt_3.c bitcnt_4.c bitcnts.c bitfiles.c bitstrng.c bstr_i.c 

OUTDIR = $(OUTDIR_BASE)/bitcount
PREFIX = $(OUTDIR)/bitcnts
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

$(PREFIX).elf: ${FILE} Makefile
	$(CROSS)-gcc -static -O3 ${FILE} -o $@

clean:
	rm -rf $(TARGET)
