include ../../common.mk

FILE1 = dijkstra_small.c

OUTDIR = $(OUTDIR_BASE)/dijkstra
PREFIX1 = $(OUTDIR)/dijkstra_small
TARGET = $(PREFIX1).elf $(PREFIX1).objdump $(PREFIX1).bin

all: $(TARGET)

$(PREFIX1).elf: ${FILE1} Makefile
	$(CROSS)-gcc -static -O3 ${FILE1} -o $@

clean:
	rm -rf $(TARGET)
