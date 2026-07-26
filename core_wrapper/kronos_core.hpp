// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include "Vkronos_top.h"
#include "core.hpp"
#include <stdint.h>

// *********************************************************************************************
// Kronos Core Model (destructor and update are virtual; overridden by KronosCoreFaulty)
class KronosCore : public Core {
  private:
    Vkronos_top *top;

  public:
    KronosCore();
    virtual ~KronosCore();

    uint32_t     read_reg(int addr);
    void         write_reg(int addr, uint32_t data);
    void         reset();
    bool         drive();
    virtual bool update();
};

// *********************************************************************************************