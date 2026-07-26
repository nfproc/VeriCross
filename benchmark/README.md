This folder contains source and binary codes of MiBench.
The original MiBench code was available from http://www.eecs.umich.edu/mibench/index.html.
We used the [modified version](https://github.com/pulp-platform/mibench) for RISC-V
by the PULP team as the base version, selected benchmarks, and made some additional
modifications.

We comfirmed that the all programs could be compiled with riscv32-unknown-elf-gcc,
installed using [RISC-V GNU Toolchain](https://github.com/riscv/riscv-gnu-toolchain).
Make sure that it is configured to use the RV32I instruction set (`--with-arch=rv32i`)
and the ILP32 ABI (`--with-abi=ilp32`).

Though most of the modifications are rewriting of Makefile, some of the source files also
have to be slightly modified, mainly because the default installation of the toolchain
misses some libraries. The list of such files and the specific reasons are as follows:
- `lame/lame3.70/brhist.c`: the termcap (or ncurses?) library header is missing
- `patricia/patricia_test.c`: IP-related library headers are missing
- `qsort/qsort.c`: the array is too large to fit the stack

The original license of each program is retained: see `LICENSE` file in the each
source folder for details.