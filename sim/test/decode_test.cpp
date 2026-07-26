#include <stdio.h>
#include <string>
#include <stdint.h>
#include "../riscv.hpp"


uint32_t insts[38] = {
    0x40000513,
    0x00000593,
    0x01f00613,
    0x010000ef,
    0x044000ef,
    0x00000000,
    0x00000000,
    0x00052283,
    0x40000313,
    0x00160393,
    0x00239393,
    0x007303b3,
    0x02730063,
    0x00528e33,
    0x01c282b3,
    0x00128293,
    0x7ff2f293,
    0x00532023,
    0x00430313,
    0xfe0002e3,
    0x000080e7,
    0x00060313,
    0x02b30c63,
    0x00058293,
    0x02628463,
    0x00229393,
    0x007503b3,
    0x0003ae03,
    0x0043ae83,
    0x01de6663,
    0x01d3a023,
    0x01c3a223,
    0x00128293,
    0xfc000ee3,
    0xfff30313,
    0xfc0006e3,
    0x000080e7,
    0x00000000};

int main() {
    for (int i = 0; i < 38; i++) {
        uint32_t pc = i * 4;
        uint32_t ir = insts[i];
        int attr;
        std::string str = RiscV::decode(pc, ir, &attr);
        printf("%04x %08x %s\n", pc, ir, str.c_str());
    }
    return 0;
}