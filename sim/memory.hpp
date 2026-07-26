// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include <stdint.h>
#include <map>
#include "common.hpp"
#include "core.hpp"

// *********************************************************************************************
// Main memory: multiple cores can be bound 
class Memory {
private:
    static const int ID_PRIMARY = -1;
    static const int ID_ALL = -2;

    int num_reps;
    Core *core_prim;  // primary
    Core **core_reps; // replicas
    CoreMemoryIF if_prim;
    CoreMemoryIF *if_reps;
    std::map<uint32_t, uint32_t> **wb_all;  // write buffer
    std::map<uint32_t, uint32_t> *wb_prim;
    std::map<uint32_t, uint32_t> **wb_reps;
    uint32_t *mem;
    uint32_t NWORDS;
    
    // private read/write methods require id
    uint8_t read_1b(int id, uint32_t addr);
    uint32_t read_4b(int id, uint32_t addr);
    void write_1b(int id, uint32_t addr, uint8_t data);
    void write_4b(int id, uint32_t addr, uint32_t data);
    void write_mask(int id, uint32_t addr, uint32_t data, uint32_t mask = 0xf);

    void cleanup_wb();
    
public:
    Memory(Core **c, uint32_t nw = MEM_WORDS);
    ~Memory();
    void drive();
    void update();
    void drive_replica();
    void update_replica();
    // public read/write methods do not require id
    void read(uint32_t addr, void *data, int size = 4);
    void write(uint32_t addr, const void *data, int size = 4);
};

// *********************************************************************************************