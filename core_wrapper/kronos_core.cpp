// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "kronos_core.hpp"
#include "Vkronos_top___024root.h"
#include "riscv.hpp"
#include <stdint.h>

// *********************************************************************************************
KronosCore::KronosCore() {
    top        = new Vkronos_top();
    top->clk   = 0;
    top->rst_x = 0;
}

// *********************************************************************************************
KronosCore::~KronosCore() {
    if (top != nullptr)
        delete top;
}

// *********************************************************************************************
uint32_t KronosCore::read_reg(int addr) {
    return top->rootp->kronos_top__DOT__u_dut__DOT__u_if__DOT__u_rf__DOT__REG[addr];
}

// *********************************************************************************************
void KronosCore::write_reg(int addr, uint32_t data) {
    top->rootp->kronos_top__DOT__u_dut__DOT__u_if__DOT__u_rf__DOT__REG[addr] = data;
}

// *********************************************************************************************
void KronosCore::reset() {
    top->rst_x = 0;
    drive();
    update();
    top->rst_x = 1;
    write_reg(RiscV::REG_SP, INITIAL_SP);
}

// *********************************************************************************************
bool KronosCore::drive() {
    if (halt)
        return false;

    top->mem_i_rdata = memif.i_rdata;
    top->mem_d_rdata = memif.d_rdata;

    top->clk = 0;
    top->eval();

    memif.en      = top->mem_en;
    memif.d_re    = top->mem_d_re;
    memif.d_we    = top->mem_d_we;
    memif.i_addr  = top->mem_i_addr;
    memif.d_addr  = top->mem_d_addr;
    memif.d_wdata = top->mem_d_wdata;
    memif.d_wmask = top->mem_d_wmask;
    return true;
}

// *********************************************************************************************
bool KronosCore::update() {
    if (halt)
        return false;

    top->clk = 1;
    top->eval();

    instif.valid   = top->inst_valid;
    instif.pc      = top->inst_pc;
    instif.ir      = top->inst_ir;
    instif.rd      = top->inst_rd;
    instif.result  = top->inst_result;
    instif.d_addr  = top->inst_d_addr;
    instif.d_wdata = top->inst_d_wdata;
    instif.d_wmask = top->inst_d_wmask;
    instif.d_baddr = top->inst_d_baddr;

    memif.en      = top->mem_en;
    memif.d_re    = top->mem_d_re;
    memif.d_we    = top->mem_d_we;
    memif.i_addr  = top->mem_i_addr;
    memif.d_addr  = top->mem_d_addr;
    memif.d_wdata = top->mem_d_wdata;
    memif.d_wmask = top->mem_d_wmask;

    // override mtvec control register after ecall instruction
    if (instif.valid && instif.ir == 0x00000073) {
        top->rootp->kronos_top__DOT__u_dut__DOT__u_ex__DOT__u_csr__DOT__mtvec = top->inst_pc + 4;
    }

    cnt_cycle++;
    if (instif.valid)
        cnt_inst++;
    return true;
}

// *********************************************************************************************