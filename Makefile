########################################################################
## VeriCross: A Rapid Cross-Verification Platform for Soft Processors
########################################################################

# C++ Sources 
SIM_SOURCES = sim/functional_core.cpp sim/loader.cpp sim/memory.cpp sim/riscv.cpp \
	sim/syscall.cpp core_wrapper/kronos_core.cpp core_wrapper/kronos_core_faulty.cpp \
	core_wrapper/rvcorep_core.cpp sim/sim_single.cpp sim/sim_concurrent.cpp sim/main.cpp
SIM_BINARY = vericross

# Kronos Sources
KRONOS_SOURCES = kronos/rtl/core/kronos_types.sv kronos/rtl/core/kronos_agu.sv \
	kronos/rtl/core/kronos_alu.sv kronos/rtl/core/kronos_branch.sv \
	kronos/rtl/core/kronos_core.sv kronos/rtl/core/kronos_counter64.sv \
	kronos/rtl/core/kronos_csr.sv kronos/rtl/core/kronos_EX.sv \
	kronos/rtl/core/kronos_hcu.sv core_patched/kronos/rtl/core/kronos_ID.sv \
	kronos/rtl/core/kronos_IF.sv kronos/rtl/core/kronos_lsu.sv \
	kronos/rtl/core/kronos_RF.sv core_wrapper/kronos_top.sv
KRONOS_TOP = kronos_top

# RVCoreP Sources
RVCOREP_SOURCES = core_patched/rvcorep/config.vh core_patched/rvcorep/proc.v \
	core_wrapper/rvcorep_main.sv
RVCOREP_TOP = rvcorep_main

# Verilator and its flags
VERILATOR = verilator
VERILATOR_INCLUDE = /usr/share/verilator/include
VERILATOR_FLAGS = --cc --Mdir core_verilated

# C++ compiler and its flags
CXX = g++
CXXFLAGS = -O -Wall --std=c++20 -Isim -Icore_wrapper -Icore_verilated -I$(VERILATOR_INCLUDE)
LDFLAGS = -Lcore_verilated -lV$(KRONOS_TOP) -lV$(RVCOREP_TOP) -lverilated -pthread -latomic

########################################################################
# Targets
.PHONY: all benchmark clean verilate_kronos verilate_rvcorep lib_kronos lib_rvcorep sim

all: benchmark verilate_kronos verilate_rvcorep lib_kronos lib_rvcorep sim

########################################################################
# VeriCross Simulator build process
# Phase 0: apply patches to processors
core_patched/%: % core_patch/%.patch
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	patch -o $@ $* core_patch/$*.patch

# Phase 1: Verilate SystemVerilog code into C++
verilate_kronos: core_verilated/V$(KRONOS_TOP).cpp
core_verilated/V$(KRONOS_TOP).cpp: $(KRONOS_SOURCES)
	$(VERILATOR) $(VERILATOR_FLAGS) --top-module $(KRONOS_TOP) $(KRONOS_SOURCES)

verilate_rvcorep: core_verilated/V$(RVCOREP_TOP).cpp
core_verilated/V$(RVCOREP_TOP).cpp: $(RVCOREP_SOURCES)
	$(VERILATOR) $(VERILATOR_FLAGS) --top-module $(RVCOREP_TOP) $(RVCOREP_SOURCES)

# Phase 2: Build the verilated C++ code and get the library
lib_kronos: core_verilated/libV$(KRONOS_TOP).a
core_verilated/libV$(KRONOS_TOP).a: core_verilated/V$(KRONOS_TOP).cpp
	$(MAKE) -C core_verilated -f V$(KRONOS_TOP).mk libV$(KRONOS_TOP)

lib_rvcorep: core_verilated/libV$(RVCOREP_TOP).a
core_verilated/libV$(RVCOREP_TOP).a: core_verilated/V$(RVCOREP_TOP).cpp
	$(MAKE) -C core_verilated -f V$(RVCOREP_TOP).mk libV$(RVCOREP_TOP)

# Phase 3: Build the simulator
sim: $(SIM_BINARY)
$(SIM_BINARY): core_verilated/libV$(KRONOS_TOP).a core_verilated/libV$(RVCOREP_TOP).a \
core_verilated/libverilated.a $(SIM_SOURCES)
	$(CXX) $(CXXFLAGS) -o $(SIM_BINARY) $(SIM_SOURCES) $(LDFLAGS)

########################################################################
benchmark:
	$(MAKE) -C benchmark

clean:
	$(MAKE) -C benchmark clean
	rm -f core_verilated/*
	rm -rf core_patched/*
	rm -f $(SIM_BINARY)
	
########################################################################
