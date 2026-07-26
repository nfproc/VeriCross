// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "memory.hpp"
#include <string.h>
#include <stdint.h>

// *********************************************************************************************
Memory::Memory(Core **c, uint32_t nw)
{
    core_prim = c[0];
    core_reps = &c[1];
    num_reps = 0;
    while (core_reps[num_reps] != nullptr)
        num_reps++;

    if (num_reps > 0) {
        if_reps = new CoreMemoryIF[num_reps];
        wb_all = new std::map<uint32_t, uint32_t>*[num_reps + 1];
        for (int i = 0; i < num_reps + 1; i++)
            wb_all[i] = new std::map<uint32_t, uint32_t>();
        wb_prim = wb_all[0];
        wb_reps = &wb_all[1];
    } else {
        if_reps = nullptr;
        wb_all  = nullptr;
        wb_prim = nullptr;
        wb_reps = nullptr;
    }

    NWORDS = nw;
    mem = new uint32_t[NWORDS];
    for (uint32_t i = 0; i < NWORDS; i++)
        mem[i] = 0;
}

// *********************************************************************************************
Memory::~Memory()
{
    if (mem != nullptr)
        delete[] mem;
    if (if_reps != nullptr)
        delete[] if_reps;
    if (wb_all != nullptr) {
        for (int i = 0; i < num_reps + 1; i++)
            if (wb_all[i] != nullptr)
                delete wb_all[i];
        delete[] wb_all;
    }
}

// *********************************************************************************************
void Memory::drive()
{
    if_prim.en      = core_prim->memif.en;
    if_prim.d_re    = core_prim->memif.d_re;
    if_prim.d_we    = core_prim->memif.d_we;
    if_prim.i_addr  = core_prim->memif.i_addr;
    if_prim.d_addr  = core_prim->memif.d_addr;
    if_prim.d_wdata = core_prim->memif.d_wdata;
    if_prim.d_wmask = core_prim->memif.d_wmask;
}

// *********************************************************************************************
void Memory::update()
{
    if (! if_prim.en)
        return;
    if_prim.i_rdata = read_4b(ID_PRIMARY, if_prim.i_addr); // instruction memory read
    if (if_prim.d_re)
        if_prim.d_rdata = read_4b(ID_PRIMARY, if_prim.d_addr); // data memory read
    else if (if_prim.d_we)
        write_mask(ID_PRIMARY, if_prim.d_addr, if_prim.d_wdata, if_prim.d_wmask); 

    core_prim->memif.i_rdata = if_prim.i_rdata;
    core_prim->memif.d_rdata = if_prim.d_rdata;
}

// *********************************************************************************************
void Memory::drive_replica()
{
    for (int i = 0; i < num_reps; i++) {
        if_reps[i].en      = core_reps[i]->memif.en;
        if_reps[i].d_re    = core_reps[i]->memif.d_re;
        if_reps[i].d_we    = core_reps[i]->memif.d_we;
        if_reps[i].i_addr  = core_reps[i]->memif.i_addr;
        if_reps[i].d_addr  = core_reps[i]->memif.d_addr;
        if_reps[i].d_wdata = core_reps[i]->memif.d_wdata;
        if_reps[i].d_wmask = core_reps[i]->memif.d_wmask;
    }
}

// *********************************************************************************************
void Memory::update_replica()
{
    for (int i = 0; i < num_reps; i++) {
        if (! if_reps[i].en)
            continue;
        if_reps[i].i_rdata = read_4b(i, if_reps[i].i_addr); // instruction memory read
        if (if_reps[i].d_re)
            if_reps[i].d_rdata = read_4b(i, if_reps[i].d_addr); // data memory read
        else if (if_reps[i].d_we)
            write_mask(i, if_reps[i].d_addr, if_reps[i].d_wdata, if_reps[i].d_wmask);

        core_reps[i]->memif.i_rdata = if_reps[i].i_rdata;
        core_reps[i]->memif.d_rdata = if_reps[i].d_rdata;
    }
    cleanup_wb();
}

// *********************************************************************************************
// clean up write buffer: detect words written by all cores' buffers and write them to memory
void Memory::cleanup_wb()
{
    if (num_reps == 0)
        return;

    std::map<uint32_t, uint32_t>::iterator it = wb_prim->begin();
    while (it != wb_prim->end()) {
        bool written_all = true;
        for (int i = 0; i < num_reps; i++) {
            written_all = written_all && wb_reps[i]->contains(it->first);
        } 
        if (written_all) {
            mem[it->first] = it->second;
            for (int i = 0; i < num_reps; i++)
                if ((*wb_reps[i])[it->first] == it->second)
                    wb_reps[i]->erase(it->first);
            it = wb_prim->erase(it);              
        } else {
            it++;
        }
    }
}

// *********************************************************************************************
// Public read() alway read from primary core's memory
void Memory::read(uint32_t addr, void *data, int size)
{
    uint8_t *ptr = static_cast<uint8_t *>(data);
    if (addr % 4 == 0 && size % 4 == 0) {
        for (int i = 0; i < size / 4; i++) {
            uint32_t value = read_4b(ID_PRIMARY, addr + i * 4);
            memcpy(ptr + i * 4, &value, sizeof(uint32_t));
        }
    } else {
        for (int i = 0; i < size; i++) {
            uint8_t value = read_1b(ID_PRIMARY, addr + i);
            memcpy(ptr + i, &value, sizeof(uint8_t));
        }
    }
}

// *********************************************************************************************
uint8_t Memory::read_1b(int id, uint32_t addr)
{
    return (read_4b(id, addr) >> ((addr % 4) * 8)) & 0xff;
}

// *********************************************************************************************
uint32_t Memory::read_4b(int id, uint32_t addr)
{
    uint32_t word_addr = (addr >> 2) & (NWORDS - 1);
    if (num_reps == 0) {
        // in single execution, simply return memory content
        return mem[word_addr];
    } else {
        // read from write buffer if data is not yet written to memory
        std::map<uint32_t, uint32_t> &wb = (id == ID_ALL) ? *wb_prim : *wb_all[id + 1];
        return wb.contains(word_addr) ? wb[word_addr] : mem[word_addr];
    }
}

// *********************************************************************************************
void Memory::write(uint32_t addr, const void *data, int size)
{
    const uint8_t *ptr = static_cast<const uint8_t *>(data);
    if (addr % 4 == 0 && size % 4 == 0) {
        uint32_t value;
        for (int i = 0; i < size / 4; i++) {
            memcpy(&value, ptr + i * 4, sizeof(uint32_t));
            write_4b(ID_ALL, addr + i * 4, value);
        }
    } else {
        uint8_t value;
        for (int i = 0; i < size; i++) {
            memcpy(&value, ptr + i, sizeof(uint8_t));
            write_1b(ID_ALL, addr + i, value);
        }
    }
}

// *********************************************************************************************
void Memory::write_1b(int id, uint32_t addr, uint8_t data)
{
    uint32_t wmask = 0xff << ((addr % 4) * 8);
    uint32_t wdata = 0x01010101 * data;
    write_4b(id, addr, (read_4b(id, addr) & ~wmask) | (wdata & wmask));
}

// *********************************************************************************************
void Memory::write_4b(int id, uint32_t addr, uint32_t data)
{
    uint32_t word_addr = (addr >> 2) & (NWORDS - 1);
    if (num_reps == 0) {
        // in single execution, simply write to memory
        mem[word_addr] = data;
    } else if (id == ID_ALL) {
        // write to memory and erase corresponding entry from all write buffers
        mem[word_addr] = data;
        for (int i = 0; i < num_reps + 1; i++)
            if (wb_all[i]->contains(word_addr))
                wb_all[i]->erase(word_addr);
    } else {
        // write to each core's write buffer
        std::map<uint32_t, uint32_t> &wb = *wb_all[id + 1];
        wb[word_addr] = data;
    }
}

// *********************************************************************************************
void Memory::write_mask(int id, uint32_t addr, uint32_t data, uint32_t mask)
{
    uint32_t wmask = expand_wmask(mask);
    write_4b(id, addr, (read_4b(id, addr) & ~wmask) | (data & wmask));
}

// *********************************************************************************************