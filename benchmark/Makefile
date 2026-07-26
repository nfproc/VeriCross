all:
	$(MAKE) -C source/basicmath
	$(MAKE) -C source/bitcount
	$(MAKE) -C source/CRC32
	$(MAKE) -C source/dijkstra
	$(MAKE) -C source/jpeg
	$(MAKE) -C source/lame
	$(MAKE) -C source/patricia
	$(MAKE) -C source/qsort
	$(MAKE) -C source/sha
	$(MAKE) -C source/stringsearch
	$(MAKE) -C source/susan

input: vc_mibench_input.tgz
	tar xvzmf $<

vc_mibench_input.tgz:
	wget https://aitech.ac.jp/~dslab/nf/pub/vc_mibench_input.tgz

clean:
	$(MAKE) -C source/basicmath clean
	$(MAKE) -C source/bitcount clean
	$(MAKE) -C source/CRC32 clean
	$(MAKE) -C source/dijkstra clean
	$(MAKE) -C source/jpeg clean
	$(MAKE) -C source/lame clean
	$(MAKE) -C source/patricia clean
	$(MAKE) -C source/qsort clean
	$(MAKE) -C source/sha clean
	$(MAKE) -C source/stringsearch clean
	$(MAKE) -C source/susan clean
