// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "core.hpp"
#include "memory.hpp"
#include "riscv.hpp"
#include "syscall.hpp"
#include <stdint.h>
#include <stdio.h>
#include <string>

// *********************************************************************************************
// Main Loop for Single Execution
uint64_t sim_single(Core *core, Memory *mem, SysCall *sys, FILE *log, uint64_t max_insts) {
    uint64_t cycle_count = 0;
    core->reset();
    while (! core->halt) {
        bool fail = false;
        mem->drive();
        fail = fail || ! core->drive();
        mem->drive();
        fail = fail || ! core->update();
        mem->update();

        cycle_count++;
        if (log != nullptr) {
            std::string line = RiscVLogInfo(core->instif, mem).get_log();
            fputs(line.c_str(), log);
        }
        if (core->instif.valid && core->instif.ir == 0x00000073) { // ecall
            sys->emulate();
        }
        if (core->cnt_inst >= max_insts || fail) {
            core->halt = true;
        }
    }
    return cycle_count;
}

// *********************************************************************************************