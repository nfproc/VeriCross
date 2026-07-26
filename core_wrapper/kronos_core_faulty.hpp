// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include "kronos_core.hpp"
#include <random>

// *********************************************************************************************
// Kronos Core Model, with faulty register file
class KronosCoreFaulty : public KronosCore {
  private:
    std::mt19937 rng;
    void         inject_fault();

  public:
    KronosCoreFaulty() : rng(std::random_device{}()) {}
    ~KronosCoreFaulty() {}

    bool update();
};

// *********************************************************************************************