// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#pragma once
#include "core.hpp"
#include "memory.hpp"
#include <stdint.h>
#include <string>
#include <string_view>

// *********************************************************************************************
// Information of an individual instruction
class RiscVInstruction {
  private:
    int32_t get_imm();

  public:
    // instruction ID
    enum {
        INST_UNKNOWN = 0,
        INST_LUI,
        INST_AUIPC,
        INST_JAL,
        INST_JALR,
        INST_BEQ,
        INST_BNE,
        INST_BLT,
        INST_BGE,
        INST_BLTU,
        INST_BGEU,
        INST_LB,
        INST_LH,
        INST_LW,
        INST_LBU,
        INST_LHU,
        INST_SB,
        INST_SH,
        INST_SW,
        INST_ADDI,
        INST_SLLI,
        INST_SLTI,
        INST_SLTIU,
        INST_XORI,
        INST_SRLI,
        INST_SRAI,
        INST_ORI,
        INST_ANDI,
        INST_ADD,
        INST_SUB,
        INST_SLL,
        INST_SLT,
        INST_SLTU,
        INST_XOR,
        INST_SRL,
        INST_SRA,
        INST_OR,
        INST_AND,
        INST_FENCE,
        INST_FENCEI,
        INST_ECALL,
        INST_EBREAK,
        INST_MRET,
        INST_WFI,
        INST_CSRRW,
        INST_CSRRS,
        INST_CSRRC,
        INST_CSRRWI,
        INST_CSRRSI,
        INST_CSRRCI
    };

    // instruction name
    static constexpr std::string_view instr_table[] = {
        "???",   "lui",   "auipc",  "jal",    "jalr",  "beq",    "bne",  "blt",  "bge",
        "bltu",  "bgeu",  "lb",     "lh",     "lw",    "lbu",    "lhu",  "sb",   "sh",
        "sw",    "addi",  "slli",   "slti",   "sltiu", "xori",   "srli", "srai", "ori",
        "andi",  "add",   "sub",    "sll",    "slt",   "sltu",   "xor",  "srl",  "sra",
        "or",    "and",   "fence",  "fencei", "ecall", "ebreak", "mret", "wfi",  "csrrw",
        "csrrs", "csrrc", "csrrwi", "csrrsi", "csrrci"};

    int              instr_id;
    std::string_view instr;
    int              attr;
    uint32_t         pc, ir;
    int32_t          imm;
    uint32_t         rs1, rs2, rd, shamt, pred, succ, zimm, csr, npc; // operands

    RiscVInstruction(uint32_t pc, uint32_t ir);
};

// *********************************************************************************************
// RISC-V Instruction Information for Logging
class RiscVLogInfo {
  public:
    RiscVInstruction *inst;
    CoreInstructionIF iif;
    uint32_t          mem_data;

    RiscVLogInfo(CoreInstructionIF &coreif, Memory *mem = nullptr);
    ~RiscVLogInfo();

    std::string get_mnemonic();
    std::string get_log();
};

// *********************************************************************************************
// RISC-V Attributes
class RiscV {
  public:
    // instruction attribute (flags)
    static const int FMT_MASK       = 0x0007;
    static const int FMT_R          = 0x0000;
    static const int FMT_I          = 0x0001;
    static const int FMT_S          = 0x0002;
    static const int FMT_B          = 0x0003;
    static const int FMT_U          = 0x0004;
    static const int FMT_J          = 0x0005;
    static const int LOG_MASK       = 0x01f8;
    static const int LOG_NONE       = 0x0000;
    static const int LOG_RD         = 0x0008;
    static const int LOG_LOAD       = 0x0030;
    static const int LOG_LD1B       = 0x0010;
    static const int LOG_LD2B       = 0x0020;
    static const int LOG_LD4B       = 0x0030;
    static const int LOG_LDSGN      = 0x0040;
    static const int LOG_STORE      = 0x0180;
    static const int LOG_ST1B       = 0x0080;
    static const int LOG_ST2B       = 0x0100;
    static const int LOG_ST4B       = 0x0180;
    static const int OPR_MASK       = 0x1e00;
    static const int OPR_NONE       = 0x0000;
    static const int OPR_RDRS1RS2   = 0x0200;
    static const int OPR_RDRS1IMM   = 0x0400;
    static const int OPR_RDRS1SHAMT = 0x0600;
    static const int OPR_RDIMM      = 0x0800;
    static const int OPR_RDOFFRS1   = 0x0a00;
    static const int OPR_RS2OFFRS1  = 0x0c00;
    static const int OPR_RS1RS2NPC  = 0x0e00;
    static const int OPR_RDNPC      = 0x1000;
    static const int OPR_PREDSUCC   = 0x1200;
    static const int OPR_RDCSRRS1   = 0x1400;
    static const int OPR_RDCSRZIMM  = 0x1600;

    // register name
    enum {
        REG_ZERO = 0,
        REG_RA   = 1,
        REG_SP   = 2,
        REG_GP   = 3,
        REG_TP   = 4,
        REG_T0   = 5,
        REG_T1   = 6,
        REG_T2   = 7,
        REG_S0   = 8,
        REG_S1   = 9,
        REG_A0   = 10,
        REG_A1   = 11,
        REG_A2   = 12,
        REG_A3   = 13,
        REG_A4   = 14,
        REG_A5   = 15,
        REG_A6   = 16,
        REG_A7   = 17,
        REG_S2   = 18,
        REG_S3   = 19,
        REG_S4   = 20,
        REG_S5   = 21,
        REG_S6   = 22,
        REG_S7   = 23,
        REG_S8   = 24,
        REG_S9   = 25,
        REG_S10  = 26,
        REG_S11  = 27,
        REG_T3   = 28,
        REG_T4   = 29,
        REG_T5   = 30,
        REG_T6   = 31
    };
};

// *********************************************************************************************