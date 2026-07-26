// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "functional_core.hpp"
#include "riscv.hpp"
#include <stdint.h>

// *********************************************************************************************
FunctionalCore::FunctionalCore() {
    memory = nullptr;
    for (int i = 0; i < 32; i++)
        reg[i] = 0;
    reg[RiscV::REG_SP] = INITIAL_SP;
    pc                 = INITIAL_PC;
}

// *********************************************************************************************
uint32_t FunctionalCore::read_reg(int addr) {
    return reg[addr];
}

// *********************************************************************************************
void FunctionalCore::write_reg(int addr, uint32_t data) {
    reg[addr] = data;
}

// *********************************************************************************************
bool FunctionalCore::drive() {
    if (halt || ! memory) {
        return false; // memory must be bound
    }

    memif.en     = 1;
    memif.d_re   = 0;
    memif.d_we   = 0;
    instif.valid = false;

    // instruction fetch
    uint32_t ir;
    memory->read(pc, &ir);

    // instruction decode
    RiscVInstruction inst(pc, ir);

    // execute, memory access, and write back
    uint32_t npc = pc + 4;
    uint8_t  buf_byte;
    uint16_t buf_half;
    uint32_t addr  = reg[inst.rs1] + inst.imm;
    uint8_t  baddr = 0;

    switch (inst.instr_id) {
    case RiscVInstruction::INST_LUI:
        reg[inst.rd] = inst.imm;
        break;
    case RiscVInstruction::INST_AUIPC:
        reg[inst.rd] = inst.npc;
        break;
    case RiscVInstruction::INST_JAL:
        reg[inst.rd] = pc + 4;
        npc          = inst.npc;
        break;
    case RiscVInstruction::INST_JALR:
        reg[inst.rd] = pc + 4;
        npc          = reg[inst.rs1] + inst.imm;
        break;
    case RiscVInstruction::INST_BEQ:
        if (reg[inst.rs1] == reg[inst.rs2])
            npc = inst.npc;
        break;
    case RiscVInstruction::INST_BNE:
        if (reg[inst.rs1] != reg[inst.rs2])
            npc = inst.npc;
        break;
    case RiscVInstruction::INST_BLT:
        if ((int32_t)reg[inst.rs1] < (int32_t)reg[inst.rs2])
            npc = inst.npc;
        break;
    case RiscVInstruction::INST_BGE:
        if ((int32_t)reg[inst.rs1] >= (int32_t)reg[inst.rs2])
            npc = inst.npc;
        break;
    case RiscVInstruction::INST_BLTU:
        if (reg[inst.rs1] < reg[inst.rs2])
            npc = inst.npc;
        break;
    case RiscVInstruction::INST_BGEU:
        if (reg[inst.rs1] >= reg[inst.rs2])
            npc = inst.npc;
        break;
    case RiscVInstruction::INST_LB:
        memif.d_re   = 1;
        memif.d_addr = addr & ~0x3;
        baddr        = addr & 0x3;
        memory->read(addr, &buf_byte, 1);
        reg[inst.rd] = buf_byte | ((buf_byte & 0x80) ? 0xffffff00 : 0);
        break;
    case RiscVInstruction::INST_LH:
        memif.d_re   = 1;
        memif.d_addr = addr & ~0x3;
        baddr        = addr & 0x2;
        memory->read(addr, &buf_half, 2);
        reg[inst.rd] = buf_half | ((buf_half & 0x8000) ? 0xffff0000 : 0);
        break;
    case RiscVInstruction::INST_LW:
        memif.d_re   = 1;
        memif.d_addr = addr & ~0x3;
        memory->read(memif.d_addr, &reg[inst.rd], 4);
        break;
    case RiscVInstruction::INST_LBU:
        memif.d_re   = 1;
        memif.d_addr = addr & ~0x3;
        baddr        = addr & 0x3;
        memory->read(addr, &buf_byte, 1);
        reg[inst.rd] = buf_byte;
        break;
    case RiscVInstruction::INST_LHU:
        memif.d_re   = 1;
        memif.d_addr = addr & ~0x3;
        baddr        = addr & 0x2;
        memory->read(addr, &buf_half, 2);
        reg[inst.rd] = buf_half;
        break;
    case RiscVInstruction::INST_SB:
        memif.d_we    = 1;
        memif.d_addr  = addr & ~0x3;
        memif.d_wdata = reg[inst.rs2] << ((addr & 0x3) * 8);
        memif.d_wmask = 0x1 << (addr & 0x3);
        break;
    case RiscVInstruction::INST_SH:
        memif.d_we    = 1;
        memif.d_addr  = addr & ~0x3;
        memif.d_wdata = reg[inst.rs2] << ((addr & 0x2) * 8);
        memif.d_wmask = 0x3 << (addr & 0x2);
        break;
    case RiscVInstruction::INST_SW:
        memif.d_we    = 1;
        memif.d_addr  = addr & ~0x3;
        memif.d_wdata = reg[inst.rs2];
        memif.d_wmask = 0xf;
        break;
    case RiscVInstruction::INST_ADDI:
        reg[inst.rd] = reg[inst.rs1] + inst.imm;
        break;
    case RiscVInstruction::INST_SLLI:
        reg[inst.rd] = reg[inst.rs1] << inst.imm;
        break;
    case RiscVInstruction::INST_SLTI:
        reg[inst.rd] = ((int32_t)reg[inst.rs1] < inst.imm) ? 1 : 0;
        break;
    case RiscVInstruction::INST_SLTIU:
        reg[inst.rd] = (reg[inst.rs1] < (uint32_t)inst.imm) ? 1 : 0;
        break;
    case RiscVInstruction::INST_XORI:
        reg[inst.rd] = reg[inst.rs1] ^ inst.imm;
        break;
    case RiscVInstruction::INST_SRLI:
        reg[inst.rd] = reg[inst.rs1] >> inst.shamt;
        break;
    case RiscVInstruction::INST_SRAI:
        reg[inst.rd] = (uint32_t)((int32_t)reg[inst.rs1] >> inst.shamt);
        break;
    case RiscVInstruction::INST_ORI:
        reg[inst.rd] = reg[inst.rs1] | inst.imm;
        break;
    case RiscVInstruction::INST_ANDI:
        reg[inst.rd] = reg[inst.rs1] & inst.imm;
        break;
    case RiscVInstruction::INST_ADD:
        reg[inst.rd] = reg[inst.rs1] + reg[inst.rs2];
        break;
    case RiscVInstruction::INST_SUB:
        reg[inst.rd] = reg[inst.rs1] - reg[inst.rs2];
        break;
    case RiscVInstruction::INST_SLL:
        reg[inst.rd] = reg[inst.rs1] << (reg[inst.rs2] & 0x1f);
        break;
    case RiscVInstruction::INST_SLT:
        reg[inst.rd] = ((int32_t)reg[inst.rs1] < (int32_t)reg[inst.rs2]) ? 1 : 0;
        break;
    case RiscVInstruction::INST_SLTU:
        reg[inst.rd] = (reg[inst.rs1] < reg[inst.rs2]) ? 1 : 0;
        break;
    case RiscVInstruction::INST_XOR:
        reg[inst.rd] = reg[inst.rs1] ^ reg[inst.rs2];
        break;
    case RiscVInstruction::INST_SRL:
        reg[inst.rd] = reg[inst.rs1] >> (reg[inst.rs2] & 0x1f);
        break;
    case RiscVInstruction::INST_SRA:
        reg[inst.rd] = (uint32_t)((int32_t)reg[inst.rs1] >> (reg[inst.rs2] & 0x1f));
        break;
    case RiscVInstruction::INST_OR:
        reg[inst.rd] = reg[inst.rs1] | reg[inst.rs2];
        break;
    case RiscVInstruction::INST_AND:
        reg[inst.rd] = reg[inst.rs1] & reg[inst.rs2];
        break;
    // ignore system instuctions so far
    case RiscVInstruction::INST_FENCE:
    case RiscVInstruction::INST_FENCEI:
    case RiscVInstruction::INST_ECALL:
    case RiscVInstruction::INST_EBREAK:
    case RiscVInstruction::INST_MRET:
    case RiscVInstruction::INST_WFI:
    case RiscVInstruction::INST_CSRRW:
    case RiscVInstruction::INST_CSRRS:
    case RiscVInstruction::INST_CSRRC:
    case RiscVInstruction::INST_CSRRWI:
    case RiscVInstruction::INST_CSRRSI:
    case RiscVInstruction::INST_CSRRCI:
        break;
    default: // unknown opcode
        return false;
    }

    // instruction logging
    instif.valid   = true;
    instif.pc      = pc;
    instif.ir      = ir;
    instif.rd      = inst.rd;
    instif.result  = reg[inst.rd];
    instif.d_addr  = memif.d_addr;
    instif.d_baddr = baddr;
    instif.d_wdata = memif.d_wdata;
    instif.d_wmask = memif.d_wmask;

    // set next PC
    pc                   = npc;
    reg[RiscV::REG_ZERO] = 0;
    cnt_inst++;
    cnt_cycle++;
    return true;
}

// *********************************************************************************************
bool FunctionalCore::update() {
    if (halt || ! memory) {
        return false; // memory must be bound
    }
    return true;
}

// *********************************************************************************************