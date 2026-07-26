include ../../../common.mk

# from the original Makefile (source files)
c_sources = brhist.c formatBitstream.c fft.c get_audio.c l3bitstream.c id3tag.c \
        ieeefloat.c lame.c newmdct.c parse.c portableio.c psymodel.c quantize.c \
        quantize-pvt.c vbrquantize.c reservoir.c tables.c takehiro.c timestatus.c \
        util.c VbrTag.c version.c gtkanal.c gpkplotting.c mpglib/common.c \
        mpglib/dct64_i386.c mpglib/decode_i386.c mpglib/layer3.c mpglib/tabinit.c \
        mpglib/interface.c mpglib/main.c 
OBJ = $(c_sources:.c=.o)
# end of source file definition

FILE = main.o libmp3lame.a
OUTDIR = $(OUTDIR_BASE)/lame
PREFIX = $(OUTDIR)/lame
TARGET = $(PREFIX).elf $(PREFIX).objdump $(PREFIX).bin

all: $(TARGET)

libmp3lame.a: $(OBJ) Makefile
	ar cr libmp3lame.a $(OBJ)

$(PREFIX).elf: $(FILE) Makefile
	$(CROSS)-gcc -static $(FILE) -o $@ -lm

%.o: %.c 
	$(CROSS)-gcc -DHAVEMPGLIB -DLAMEPARSE -DNDEBUG -D__NO_MATH_INLINES -O3 \
	-fomit-frame-pointer -funroll-loops -ffast-math -finline-functions -Wall \
	-DLAMESNDFILE -DBRHIST -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJ) main.o libmp3lame.a

# from the results of gcc -MM (object file dependency)
brhist.o: brhist.c
formatBitstream.o: formatBitstream.c formatBitstream.h machine.h
fft.o: fft.c util.h machine.h encoder.h lame.h l3side.h psymodel.h
get_audio.o: get_audio.c util.h machine.h encoder.h lame.h l3side.h get_audio.h
l3bitstream.o: l3bitstream.c lame.h l3bitstream.h util.h machine.h encoder.h \
        l3side.h quantize.h quantize-pvt.h formatBitstream.h tables.h \
        l3bitstream-pvt.h
id3tag.o: id3tag.c id3tag.h
ieeefloat.o: ieeefloat.c ieeefloat.h
lame.o: lame.c lame.h util.h machine.h encoder.h l3side.h timestatus.h \
        psymodel.h newmdct.h quantize.h quantize-pvt.h l3bitstream.h \
        formatBitstream.h version.h VbrTag.h id3tag.h tables.h brhist.h \
        get_audio.h
newmdct.o: newmdct.c util.h machine.h encoder.h lame.h l3side.h newmdct.h
parse.o: parse.c
portableio.o: portableio.c portableio.h ieeefloat.h
psymodel.o: psymodel.c util.h machine.h encoder.h lame.h l3side.h \
        psymodel.h tables.h fft.h
quantize.o: quantize.c util.h machine.h encoder.h lame.h l3side.h \
        quantize.h l3bitstream.h reservoir.h quantize-pvt.h
quantize-pvt.o: quantize-pvt.c util.h machine.h encoder.h lame.h l3side.h \
        tables.h reservoir.h quantize-pvt.h
vbrquantize.o: vbrquantize.c util.h machine.h encoder.h lame.h l3side.h \
        quantize.h l3bitstream.h reservoir.h quantize-pvt.h
reservoir.o: reservoir.c util.h machine.h encoder.h lame.h l3side.h
tables.o: tables.c util.h machine.h encoder.h lame.h l3side.h tables.h
takehiro.o: takehiro.c util.h machine.h encoder.h lame.h l3side.h \
        tables.h quantize-pvt.h
timestatus.o: timestatus.c timestatus.h util.h machine.h encoder.h lame.h \
        l3side.h
util.o: util.c util.h machine.h encoder.h lame.h l3side.h
VbrTag.o: VbrTag.c machine.h VbrTag.h l3bitstream.h util.h encoder.h \
        lame.h l3side.h version.h
version.o: version.c version.h machine.h lame.h
gtkanal.o: gtkanal.c
gpkplotting.o: gpkplotting.c
mpglib/common.o: mpglib/common.c
mpglib/decode_i386.o: mpglib/decode_i386.c
mpglib/dct64_i386.o: mpglib/dct64_i386.c
mpglib/layer3.o: mpglib/layer3.c
mpglib/tabinit.o: mpglib/tabinit.c
mpglib/interface.o: mpglib/interface.c
mpglib/main.o: mpglib/main.c
main.o: main.c lame.h