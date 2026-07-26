// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "kronos_core_faulty.hpp"
#include <stdint.h>

// *********************************************************************************************
// Fault Injection: Flip a randomly selected bit of register file
void KronosCoreFaulty::inject_fault() {
    // register number is selected from x1 to x31 (x0 is always zero)
    std::uniform_int_distribution<int> reg_dist(1, 31);
    std::uniform_int_distribution<int> bit_dist(0, 31);

    int reg_num = reg_dist(rng);
    int bit_pos = bit_dist(rng);

    // Read from the selected register, flip the selected bit, and write back
    uint32_t value = read_reg(reg_num);
    value ^= (1u << bit_pos);
    write_reg(reg_num, value);
}

// *********************************************************************************************
bool KronosCoreFaulty::update() {
    if (! KronosCore::update()) {
        return false;
    }
    // Every 1,000,000 instructions, inject a fault
    if (cnt_inst > 0 && cnt_inst % 1000000 == 0) {
        // if (rng() < 100) { // or, one in ~43m (useful for the sake of demonstration)
        inject_fault();
    }
    return true;
}

// *********************************************************************************************