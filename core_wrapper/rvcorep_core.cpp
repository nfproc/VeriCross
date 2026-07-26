// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "rvcorep_core.hpp"
#include "Vrvcorep_main___024root.h"
#include "riscv.hpp"
#include <stdint.h>

// *********************************************************************************************
RVCorePCore::RVCorePCore() {
    top        = new Vrvcorep_main();
    top->clk   = 0;
    top->rst_x = 0;
}

// *********************************************************************************************
RVCorePCore::~RVCorePCore() {
    if (top != nullptr)
        delete top;
}

// *********************************************************************************************
uint32_t RVCorePCore::read_reg(int addr) {
    return top->rootp->rvcorep_main__DOT__p__DOT__regs0__DOT__mem[addr];
}

// *********************************************************************************************
void RVCorePCore::write_reg(int addr, uint32_t data) {
    top->rootp->rvcorep_main__DOT__p__DOT__regs0__DOT__mem[addr] = data;
}

// *********************************************************************************************
void RVCorePCore::reset() {
    top->rst_x = 0;
    drive();
    update();
    top->rst_x = 1;
    write_reg(RiscV::REG_SP, INITIAL_SP);
}

// *********************************************************************************************
bool RVCorePCore::drive() {
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
bool RVCorePCore::update() {
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

    cnt_cycle++;
    if (instif.valid)
        cnt_inst++;
    return true;
}

// *********************************************************************************************