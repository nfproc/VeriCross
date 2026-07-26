// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "common.hpp"
#include "core.hpp"
#include "memory.hpp"
#include "riscv.hpp"
#include "syscall.hpp"
#include <stdint.h>
#include <stdio.h>
#include <string>

// *********************************************************************************************
// Compare the outcomes of two cores and store the difference if differ
class Validator {
  public:
    bool     address_mis, write_data_mis;
    uint32_t faddr, vaddr;
    uint32_t fdata, vdata;

    bool validate(CoreInstructionIF &fif, CoreInstructionIF &vif) {
        address_mis = write_data_mis = false;
        faddr = vaddr = fdata = vdata = 0;
        if (fif.pc != vif.pc || fif.ir != vif.ir) {
            return false;
        }
        RiscVInstruction info(fif.pc, fif.ir);
        if (info.attr & RiscV::LOG_LOAD) {
            faddr       = (fif.d_addr & ~0x3) | fif.d_baddr;
            vaddr       = (vif.d_addr & ~0x3) | vif.d_baddr;
            address_mis = (faddr != vaddr);
            return (! address_mis);
        } else if (info.attr & RiscV::LOG_STORE) {
            faddr          = fif.d_addr;
            vaddr          = vif.d_addr;
            address_mis    = (faddr != vaddr);
            fdata          = fif.d_wdata & expand_wmask(fif.d_wmask);
            vdata          = vif.d_wdata & expand_wmask(vif.d_wmask);
            write_data_mis = (fdata != vdata);
            return (! address_mis && ! write_data_mis);
        } else if (info.attr & RiscV::LOG_RD) {
            return (fif.result == vif.result);
        }
        return true;
    }
};

// *********************************************************************************************
// Main Loop for Concurrent Execution
uint64_t sim_concurrent(Core **cores, Memory *mem, SysCall *sys, FILE *log, uint64_t max_insts,
                        int mismatch_insts) {
    Core     *fcore = cores[0];
    Core     *vcore = cores[1];
    Validator vld;

    // Ring buffer for keeping the recent instruction logs from fcore
    RiscVLogInfo **log_buffer = nullptr;
    if (mismatch_insts > 0) {
        log_buffer = new RiscVLogInfo *[mismatch_insts];
        for (int i = 0; i < mismatch_insts; i++) {
            log_buffer[i] = nullptr;
        }
    }

    uint64_t cycle_count = 0;
    fcore->reset();
    vcore->reset();
    while (! fcore->halt && ! vcore->halt) {
        bool fail = false;
        // first, execute exactly one instruction with functional core
        fail = fail || ! fcore->drive();
        mem->drive();
        fail = fail || ! fcore->update();
        mem->update();
        if (fcore->cnt_inst >= max_insts || fail) {
            fcore->halt = true;
        }

        // then, run verilated core until valid instruction comes
        fail = false;
        for (int i = 0; i < VCORE_TIMEOUT; i++) { // avoid infinite loop
            fail = fail || ! vcore->drive();
            mem->drive_replica();
            fail = fail || ! vcore->update();
            mem->update_replica();
            if (vcore->instif.valid || fail) {
                break;
            }
            cycle_count++;
        }
        if (vcore->cnt_inst >= max_insts || fail || ! vcore->instif.valid) {
            vcore->halt = true;
        }

        // check if the executed instructions are the same
        if (! vld.validate(fcore->instif, vcore->instif)) {
            printf("## failure: detected mismatch @ instruction %ld\n", fcore->cnt_inst);
            if (vld.address_mis) {
                printf("## (address mismatch   : Functional %08x, Verilated %08x)\n", vld.faddr,
                       vld.vaddr);
            }
            if (vld.write_data_mis) {
                printf("## (write data mismatch: Functional %08x, Verilated %08x)\n", vld.fdata,
                       vld.vdata);
            }
            // show last N instructions using ring buffer
            if (mismatch_insts > 0) {
                int start = fcore->cnt_inst % mismatch_insts;
                for (int i = 0; i < mismatch_insts; i++) {
                    int idx = (start + i) % mismatch_insts;
                    if (log_buffer[idx] != nullptr) {
                        std::string logstr = log_buffer[idx]->get_log();
                        printf("## %9d : %s", mismatch_insts - i, logstr.c_str());
                        delete log_buffer[idx];
                    }
                }
            }
            // show failed instruction
            printf("## Functional: ");
            fputs(RiscVLogInfo(fcore->instif, mem).get_log().c_str(), stdout);
            printf("## Verilated : ");
            fputs(RiscVLogInfo(vcore->instif, mem).get_log().c_str(), stdout);
            fcore->halt = vcore->halt = true;
            break;
        }

        // save log information to ring buffer
        if (mismatch_insts > 0) {
            int idx = fcore->cnt_inst % mismatch_insts;
            if (log_buffer[idx] != nullptr) {
                delete log_buffer[idx];
            }
            log_buffer[idx] = new RiscVLogInfo(fcore->instif, mem);
        }

        // logging & system call emulation
        if (log != nullptr) {
            fputs(RiscVLogInfo(fcore->instif, mem).get_log().c_str(), log);
        }
        if (fcore->instif.ir == 0x00000073) { // ecall
            sys->emulate();
        }
    }
    if (fcore->halt ^ vcore->halt) {
        const char *halted = (fcore->halt) ? "Functional" : "Verilated";
        printf("## failure: %s core stopped but the other didn't.\n", halted);
    }
    delete[] log_buffer;
    return cycle_count;
}

// *********************************************************************************************