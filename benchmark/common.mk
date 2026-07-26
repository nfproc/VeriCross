CROSS = riscv32-unknown-elf
THIS_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
OUTDIR_BASE = $(THIS_DIR)/binary

%.objdump: %.elf
	$(CROSS)-objdump -D $< > $@

%.bin: %.elf
	$(CROSS)-objcopy -O binary --set-section-flags=.bss=contents,alloc,load $< $@