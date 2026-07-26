// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include "core.hpp"
#include "memory.hpp"
#include <map>
#include <stdint.h>
#include <sys/stat.h>

// *********************************************************************************************
// System Call Emulation
class SysCall {
  private:
    // maximum length of path
    static const int SIM_PATH_MAX = 4096;

    Core  **cores;
    Memory *mem;

    std::map<int, int> fd_map;
    int                fd_next;
    uint32_t           brk_addr;

    void     halt_cores();
    uint32_t read_reg(int addr);
    void     write_reg(int addr, uint32_t data);
    int      read_path(char *path_buf, uint32_t base_addr);
    void     postprocess_stat(uint32_t *dst, struct stat *src, uint32_t vtime, uint32_t vntime);

  public:
    static const int SC_CLOSE        = 57;
    static const int SC_LSEEK        = 62;
    static const int SC_READ         = 63;
    static const int SC_WRITE        = 64;
    static const int SC_FSTAT        = 80;
    static const int SC_EXIT         = 93;
    static const int SC_GETTIMEOFDAY = 169;
    static const int SC_SBRK         = 214;
    static const int SC_OPEN         = 1024;
    static const int SC_STAT         = 1038;

    SysCall(Core **, Memory *, int, int, int, uint32_t);
    void emulate();
};

// *********************************************************************************************