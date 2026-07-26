// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include <stdint.h>
#include <string>

// *********************************************************************************************
// Program Loader
class Loader {
  private:
    uint32_t make_jal(uint32_t imm);

  public:
    // Section to be written
    class Section {
      public:
        uint32_t  addr, length;
        uint32_t *data;

        Section(uint32_t addr, int length);
        ~Section();
    };

    bool        valid;
    std::string message;
    Section   **sections;
    uint32_t    brk_addr;

    Loader(const char *base_dir, const char *bin_file, const char *dump_file, const char *sp_file);
    ~Loader();
};

// *********************************************************************************************