// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "loader.hpp"
#include "common.hpp"
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdint.h>

// *********************************************************************************************
// Section constructor and deconstructor: initialize and destroy buffer
Loader::Section::Section(uint32_t addr, int length) {
    this->addr   = addr;
    this->length = length;
    this->data   = new uint32_t[length / 4]();
}

Loader::Section::~Section() {
    delete[] this->data;
}

// *********************************************************************************************
// Loader constructor: load files and prepare sections to be written
Loader::Loader(const char *base_dir, const char *bin_file, const char *dump_file,
               const char *sp_file) {
    auto last_wd = std::filesystem::current_path();
    valid        = false;
    brk_addr     = 0;

    // open files
    try {
        std::filesystem::current_path(base_dir);
    } catch (std::filesystem::filesystem_error &e) {
        message = std::format("failed to change working directory ({})", e.what());
        return;
    }

    int             bin_size;
    std::error_code ec;
    bin_size = std::filesystem::file_size(bin_file, ec);
    std::ifstream bin_stream(bin_file, std::ios::binary);
    if (! bin_stream || bin_size == -1) {
        message = std::format("failed to open the binary file ({})", bin_file);
        std::filesystem::current_path(last_wd);
        return;
    }
    bin_size = bin_size & ~0x3;

    std::ifstream dump_stream(dump_file);
    if (! dump_stream) {
        message = std::format("failed to open the objdump file ({})", dump_file);
        std::filesystem::current_path(last_wd);
        return;
    }

    std::ifstream sp_stream(sp_file);
    if (! sp_stream) {
        message = std::format("failed to open the stack image file ({})", sp_file);
        std::filesystem::current_path(last_wd);
        return;
    }

    // check the base address of .text section and _start function
    uint32_t    text_addr = 0, start_addr = 0;
    std::string line;
    std::regex  re_func(R"(([0-9a-f]{8}) <(.+)>:)");
    std::smatch match;
    while (std::getline(dump_stream, line)) {
        if (std::regex_match(line, match, re_func)) {
            text_addr = (text_addr == 0) ? std::stoul(match[1].str(), nullptr, 16) : text_addr;
            if (match[2].str() == "_start") {
                start_addr = std::stoul(match[1].str(), nullptr, 16);
                break;
            }
        }
    }
    if (start_addr == 0) {
        message = std::string("failed to determine the initial PC");
        std::filesystem::current_path(last_wd);
        return;
    }
    dump_stream.close();

    // check if the stack image is valid (having at least 4 words)
    std::regex re_sp(R"(^([0-9a-f]{8}))");
    int        sp_size = 0;
    while (std::getline(sp_stream, line)) {
        if (std::regex_match(line, match, re_sp))
            sp_size += 4;
    }
    if (sp_size < 16) {
        message = std::format("the stack image file ({}) is invalid", sp_file);
        std::filesystem::current_path(last_wd);
        return;
    }
    sp_stream.clear();
    sp_stream.seekg(0, std::ios::beg); // clear EOF and rewind

    // generate the memory image
    sections    = new Section *[4];
    sections[0] = new Section(text_addr, bin_size); // text and data sections
    sections[1] = new Section(INITIAL_SP, sp_size); // stack
    sections[2] = new Section(INITIAL_PC, 4);       // instruction to jump to _start
    sections[3] = nullptr;

    bin_stream.read((char *)sections[0]->data, bin_size);
    bin_stream.close();

    int sp_pos = 0;
    while (std::getline(sp_stream, line)) {
        if (std::regex_match(line, match, re_sp)) {
            sections[1]->data[sp_pos] = std::stoul(match[1].str(), nullptr, 16);
            sp_pos++;
        }
    }
    sp_stream.close();

    sections[2]->data[0] = make_jal(start_addr - INITIAL_PC);

    brk_addr = text_addr + bin_size;
    valid    = true;
    std::filesystem::current_path(last_wd);
}

// *********************************************************************************************
// Loader deconstructor
Loader::~Loader() {
    for (int i = 0; sections[i]; i++)
        delete sections[i];
    delete[] sections;
}

// *********************************************************************************************
// make a jal instruction, written at entry point, to jump to _start
uint32_t Loader::make_jal(uint32_t imm) {
    return ((imm << 20) & 0x7fe00000) | ((imm << 9) & 0x00100000) | ((imm) & 0x000ff000) |
           ((imm << 11) & 0x80000000) | 0x0000006f;
}

// *********************************************************************************************