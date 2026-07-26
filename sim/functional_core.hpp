// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include "core.hpp"
#include "memory.hpp"
#include <stdint.h>


// *********************************************************************************************
// RV32I Functional Simulator Core
class FunctionalCore : public Core {
  private:
    Memory  *memory; // to allow instructions/data being read directly
    uint32_t reg[32];
    uint32_t pc;

  public:
    FunctionalCore();
    ~FunctionalCore(){};

    void     bind_memory(Memory *mem) { memory = mem; }
    uint32_t read_reg(int addr);
    void     write_reg(int addr, uint32_t data);
    void     reset() {};
    bool     drive();
    bool     update();
};

// *********************************************************************************************