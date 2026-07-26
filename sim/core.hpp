// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include <stdint.h>

// *********************************************************************************************
// Processor Core <-> Instruction Logger Interface
class CoreInstructionIF {
  public:
    uint32_t valid;
    uint32_t pc, ir;
    uint32_t rd, result;               // output register operand
    uint32_t d_addr, d_wdata, d_wmask; // memory operand
    uint8_t  d_baddr;
};

// *********************************************************************************************
// Processor Core <-> Memory Interface
class CoreMemoryIF {
  public:
    uint32_t i_addr, d_addr; // word address
    uint32_t i_rdata, d_rdata, d_wdata, d_wmask;
    uint32_t en, d_re, d_we;
};

// *********************************************************************************************
// Processor Core Interface
class Core {
  public:
    virtual ~Core(){};

    bool halt = false;
    // number of instructions/cycles
    uint64_t cnt_inst = 0, cnt_cycle = 0;
    // interface to other classes
    CoreInstructionIF instif;
    CoreMemoryIF      memif;
    virtual uint32_t  read_reg(int addr)                 = 0;
    virtual void      write_reg(int addr, uint32_t data) = 0;
    // reset the core
    virtual void reset() = 0;
    // drive + update = execute one cycle
    virtual bool drive()  = 0;
    virtual bool update() = 0;
};

// *********************************************************************************************