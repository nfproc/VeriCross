// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include "Vrvcorep_main.h"
#include "core.hpp"
#include <stdint.h>

// *********************************************************************************************
// RVCoreP Core Model
class RVCorePCore : public Core {
  private:
    Vrvcorep_main *top;

  public:
    RVCorePCore();
    ~RVCorePCore();

    uint32_t read_reg(int addr);
    void     write_reg(int addr, uint32_t data);
    void     reset();
    bool     drive();
    bool     update();
};

// *********************************************************************************************