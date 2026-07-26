// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "riscv.hpp"
#include <format>
#include <stdint.h>

// *********************************************************************************************
// instruction constructor: get attributes of instruction
RiscVInstruction::RiscVInstruction(uint32_t pc, uint32_t ir) {
    instr_id = INST_UNKNOWN;
    instr    = instr_table[INST_UNKNOWN];
    attr     = 0;
    this->pc = pc;
    this->ir = ir;
    imm      = 0;
    rs1 = rs2 = rd = shamt = pred = succ = zimm = csr = npc = 0;

    if ((ir & 0x3) != 0x3) // not a 32-bit instruction
        return;

    uint8_t OP     = (ir >> 2) & 0x1f;
    uint8_t funct3 = (ir >> 12) & 0x07;
    uint8_t funct7 = (ir >> 25) & 0x7f;
    int32_t imm_i  = (ir >> 20) & 0xfff;

    // instruction name and attributes
    switch (OP) {
    case 0b01101:
        instr_id = INST_LUI;
        attr     = RiscV::FMT_U | RiscV::LOG_RD | RiscV::OPR_RDIMM;
        break;
    case 0b00101:
        instr_id = INST_AUIPC;
        attr     = RiscV::FMT_U | RiscV::LOG_RD | RiscV::OPR_RDIMM;
        break;
    case 0b11011:
        instr_id = INST_JAL;
        attr     = RiscV::FMT_J | RiscV::LOG_RD | RiscV::OPR_RDNPC;
        break;
    case 0b11001:
        instr_id = INST_JALR;
        attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDOFFRS1;
        break;
    case 0b11000: // branch
        if (funct3 == 0b000) {
            instr_id = INST_BEQ;
        } else if (funct3 == 0b001) {
            instr_id = INST_BNE;
        } else if (funct3 == 0b100) {
            instr_id = INST_BLT;
        } else if (funct3 == 0b101) {
            instr_id = INST_BGE;
        } else if (funct3 == 0b110) {
            instr_id = INST_BLTU;
        } else if (funct3 == 0b111) {
            instr_id = INST_BGEU;
        }
        attr = RiscV::FMT_B | RiscV::LOG_NONE | RiscV::OPR_RS1RS2NPC;
        break;
    case 0b00000: // load
        if (funct3 == 0b000) {
            instr_id = INST_LB;
            attr     = RiscV::LOG_LD1B | RiscV::LOG_LDSGN;
        } else if (funct3 == 0b001) {
            instr_id = INST_LH;
            attr     = RiscV::LOG_LD2B | RiscV::LOG_LDSGN;
        } else if (funct3 == 0b010) {
            instr_id = INST_LW;
            attr     = RiscV::LOG_LD4B;
        } else if (funct3 == 0b100) {
            instr_id = INST_LBU;
            attr     = RiscV::LOG_LD1B;
        } else if (funct3 == 0b101) {
            instr_id = INST_LHU;
            attr     = RiscV::LOG_LD2B;
        }
        attr = attr | RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDOFFRS1;
        break;
    case 0b01000: // store
        if (funct3 == 0b000) {
            instr_id = INST_SB;
            attr     = RiscV::LOG_ST1B;
        } else if (funct3 == 0b001) {
            instr_id = INST_SH;
            attr     = RiscV::LOG_ST2B;
        } else if (funct3 == 0b010) {
            instr_id = INST_SW;
            attr     = RiscV::LOG_ST4B;
        }
        attr = attr | RiscV::FMT_S | RiscV::OPR_RS2OFFRS1;
        break;
    case 0b00100: // arithmetic/logic operation with immediate
        if (funct3 == 0b000) {
            instr_id = INST_ADDI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1IMM;
        } else if (funct3 == 0b001) {
            instr_id = INST_SLLI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1SHAMT;
        } else if (funct3 == 0b010) {
            instr_id = INST_SLTI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1IMM;
        } else if (funct3 == 0b011) {
            instr_id = INST_SLTIU;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1IMM;
        } else if (funct3 == 0b100) {
            instr_id = INST_XORI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1IMM;
        } else if (funct3 == 0b101) {
            instr_id = (funct7 & 0b0100000) ? INST_SRAI : INST_SRLI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1SHAMT;
        } else if (funct3 == 0b110) {
            instr_id = INST_ORI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1IMM;
        } else if (funct3 == 0b111) {
            instr_id = INST_ANDI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDRS1IMM;
        }
        break;
    case 0b01100: // arithmetic/logic operation with registers
        if (funct3 == 0b000) {
            instr_id = (funct7 & 0b0100000) ? INST_SUB : INST_ADD;
        } else if (funct3 == 0b001) {
            instr_id = INST_SLL;
        } else if (funct3 == 0b010) {
            instr_id = INST_SLT;
        } else if (funct3 == 0b011) {
            instr_id = INST_SLTU;
        } else if (funct3 == 0b100) {
            instr_id = INST_XOR;
        } else if (funct3 == 0b101) {
            instr_id = (funct7 & 0b0100000) ? INST_SRA : INST_SRL;
        } else if (funct3 == 0b110) {
            instr_id = INST_OR;
        } else if (funct3 == 0b111) {
            instr_id = INST_AND;
        }
        attr = RiscV::FMT_R | RiscV::LOG_RD | RiscV::OPR_RDRS1RS2;
        break;
    case 0b00011: // misc.
        if (funct3 == 0b000) {
            instr_id = INST_FENCE;
            attr     = RiscV::FMT_I | RiscV::LOG_NONE | RiscV::OPR_PREDSUCC;
        } else if (funct3 == 0b001) {
            instr_id = INST_FENCEI;
            attr     = RiscV::FMT_I | RiscV::LOG_NONE | RiscV::OPR_NONE;
        }
        break;
    case 0b11100: // system
        if (funct3 == 0b000) {
            instr_id = (imm_i == 0x000) ? INST_ECALL :
                       (imm_i == 0x001) ? INST_EBREAK :
                       (imm_i == 0x302) ? INST_MRET :
                       (imm_i == 0x105) ? INST_WFI :
                                          INST_UNKNOWN;
            attr     = RiscV::FMT_I | RiscV::LOG_NONE | RiscV::OPR_NONE;
        } else if (funct3 == 0b001) {
            instr_id = INST_CSRRW;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDCSRRS1;
        } else if (funct3 == 0b010) {
            instr_id = INST_CSRRS;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDCSRRS1;
        } else if (funct3 == 0b011) {
            instr_id = INST_CSRRC;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDCSRRS1;
        } else if (funct3 == 0b101) {
            instr_id = INST_CSRRWI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDCSRZIMM;
        } else if (funct3 == 0b110) {
            instr_id = INST_CSRRSI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDCSRZIMM;
        } else if (funct3 == 0b111) {
            instr_id = INST_CSRRCI;
            attr     = RiscV::FMT_I | RiscV::LOG_RD | RiscV::OPR_RDCSRZIMM;
        }
        break;
    }
    instr = instr_table[instr_id];

    // operands
    imm   = get_imm();
    rs1   = (ir >> 15) & 0x1f;
    rs2   = (ir >> 20) & 0x1f;
    rd    = (ir >> 7) & 0x1f;
    shamt = (ir >> 20) & 0x1f;
    pred  = (ir >> 24) & 0x0f;
    succ  = (ir >> 20) & 0x0f;
    zimm  = (ir >> 15) & 0x1f;
    csr   = (ir >> 20) & 0xfff;
    npc   = pc + imm;
}

// *********************************************************************************************
// decode the immediate value field
int32_t RiscVInstruction::get_imm() {
    switch (attr & RiscV::FMT_MASK) {
    case RiscV::FMT_I:
        return ((ir & 0x80000000) ? 0xfffff000 : 0) | ((ir >> 20) & 0x00000fff);
    case RiscV::FMT_S:
        return ((ir & 0x80000000) ? 0xfffff000 : 0) | ((ir >> 20) & 0x00000fe0) |
               ((ir >> 7) & 0x0000001f);
    case RiscV::FMT_B:
        return ((ir & 0x80000000) ? 0xfffff000 : 0) | ((ir << 4) & 0x00000800) |
               ((ir >> 20) & 0x000007e0) | ((ir >> 7) & 0x0000001e);
    case RiscV::FMT_U:
        return ir & 0xfffff000;
    case RiscV::FMT_J:
        return ((ir & 0x80000000) ? 0xfff00000 : 0) | ((ir) & 0x000ff000) |
               ((ir >> 9) & 0x00000800) | ((ir >> 20) & 0x000007fe);
    }
    return 0;
}

// *********************************************************************************************
// log information constructor: RiscVInstruction and current value of memory
RiscVLogInfo::RiscVLogInfo(CoreInstructionIF &coreif, Memory *mem) {
    iif  = coreif;
    inst = (iif.valid) ? new RiscVInstruction(iif.pc, iif.ir) : nullptr;
    if (mem != nullptr && inst != nullptr && (inst->attr & (RiscV::LOG_LOAD | RiscV::LOG_STORE)))
        mem->read(iif.d_addr, &mem_data);
}

// *********************************************************************************************
RiscVLogInfo::~RiscVLogInfo() {
    if (inst != nullptr)
        delete inst;
}

// *********************************************************************************************
std::string RiscVLogInfo::get_mnemonic() {
    switch (inst->attr & RiscV::OPR_MASK) {
    case RiscV::OPR_NONE:
        return std::format("{:<7}", inst->instr);
    case RiscV::OPR_RDRS1RS2:
        return std::format("{:<7} x{}, x{}, x{}", inst->instr, inst->rd, inst->rs1, inst->rs2);
    case RiscV::OPR_RDRS1IMM:
        return std::format("{:<7} x{}, x{}, {}", inst->instr, inst->rd, inst->rs1, inst->imm);
    case RiscV::OPR_RDRS1SHAMT:
        return std::format("{:<7} x{}, x{}, {}", inst->instr, inst->rd, inst->rs1, inst->shamt);
    case RiscV::OPR_RDIMM:
        return std::format("{:<7} x{}, 0x{:x}", inst->instr, inst->rd, inst->imm);
    case RiscV::OPR_RDOFFRS1:
        return std::format("{:<7} x{}, {}(x{})", inst->instr, inst->rd, inst->imm, inst->rs1);
    case RiscV::OPR_RS2OFFRS1:
        return std::format("{:<7} x{}, {}(x{})", inst->instr, inst->rs2, inst->imm, inst->rs1);
    case RiscV::OPR_RS1RS2NPC:
        return std::format("{:<7} x{}, x{}, {} # 0x{:x}", inst->instr, inst->rs1, inst->rs2,
                           inst->imm, inst->npc);
    case RiscV::OPR_RDNPC:
        return std::format("{:<7} x{}, {} # 0x{:x}", inst->instr, inst->rd, inst->imm, inst->npc);
    case RiscV::OPR_PREDSUCC:
        return std::format("{:<7} {}, {}", inst->instr, inst->pred, inst->succ);
    case RiscV::OPR_RDCSRRS1:
        return std::format("{:<7} x{}, 0x{:x}, x{}", inst->instr, inst->rd, inst->csr, inst->rs1);
    case RiscV::OPR_RDCSRZIMM:
        return std::format("{:<7} x{}, 0x{:x}, 0x{:x}", inst->instr, inst->rd, inst->csr,
                           inst->zimm);
    }
    return std::string("???");
}

// *********************************************************************************************
std::string RiscVLogInfo::get_log() {
    uint32_t word_addr = iif.d_addr >> 2;

    if (! iif.valid)
        return std::string();

    std::string inst_str = std::format("{:8x}: {:08x}  {}", iif.pc, iif.ir, get_mnemonic());

    // simulate load/store (as logging might be done in the EX stage)
    if (inst->attr & RiscV::LOG_STORE) {
        uint32_t mem_mask = expand_wmask(iif.d_wmask);
        mem_data          = (mem_data & ~mem_mask) | (iif.d_wdata & mem_mask);
    } else if (inst->attr & RiscV::LOG_LOAD) {
        uint32_t mem_mask = ((inst->attr & RiscV::LOG_LOAD) == RiscV::LOG_LD1B) ? 0xff :
                            ((inst->attr & RiscV::LOG_LOAD) == RiscV::LOG_LD2B) ? 0xffff :
                                                                                  0xffffffff;
        iif.result        = (mem_data >> (iif.d_baddr * 8)) & mem_mask;
        if (inst->attr & RiscV::LOG_LDSGN) {
            if (inst->attr & RiscV::LOG_LD1B)
                iif.result = iif.result | ((iif.result & 0x80) ? 0xffffff00 : 0);
            if (inst->attr & RiscV::LOG_LD2B)
                iif.result = iif.result | ((iif.result & 0x8000) ? 0xffff0000 : 0);
        }
    }

    // generate log string
    if (inst->attr & RiscV::LOG_STORE)
        return std::format("{:<55} mem[0x{:x}] <- 0x{:x}\n", inst_str, word_addr, mem_data);
    if (inst->attr & RiscV::LOG_RD)
        return std::format("{:<55} x{:<2} <- 0x{:x}\n", inst_str, iif.rd, iif.result);
    return std::format("{}\n", inst_str);
}

// *********************************************************************************************