// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include <stdint.h>

const char *const PROG_NAME = "VeriCross: A Rapid Cross-Verification Platform for Soft Processors";
const char *const PROG_VER  = "1.0.0";

const uint32_t MEM_WORDS     = 0x00080000; // 512 Ki words = 2 MiB
const uint32_t INITIAL_PC    = 0x00010000;
const uint32_t INITIAL_SP    = 0x001e6400;
const int      VCORE_TIMEOUT = 1000; // Verilated Core Timeout (in cycles)

// expand bit-wise write mask to byte-wise
inline uint32_t expand_wmask(uint32_t mask) {
    return ((mask & 8) ? 0xff000000 : 0) | ((mask & 4) ? 0x00ff0000 : 0) |
           ((mask & 2) ? 0x0000ff00 : 0) | ((mask & 1) ? 0x000000ff : 0);
}