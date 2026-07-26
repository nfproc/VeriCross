// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include "syscall.hpp"
#include "riscv.hpp"
#include <cerrno>
#include <fcntl.h>
#include <map>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// *********************************************************************************************
// constructor: initialize internal state for emulation
SysCall::SysCall(Core **cores, Memory *mem, int fd_in, int fd_out, int fd_err, uint32_t brk_addr) {
    this->cores    = cores;
    this->mem      = mem;
    this->brk_addr = brk_addr;
    fd_map[0]      = fd_in;  // stdin
    fd_map[1]      = fd_out; // stdout
    fd_map[2]      = fd_err; // stderr
    fd_next        = 3;
}

// *********************************************************************************************
// stop all the cores (when exit() is called)
void SysCall::halt_cores() {
    for (int i = 0; cores[i] != nullptr; i++) {
        cores[i]->halt = true;
    }
}

// *********************************************************************************************
// read register value from one of the cores
uint32_t SysCall::read_reg(int addr) {
    return cores[0]->read_reg(addr);
}

// *********************************************************************************************
// write register value to all the cores
void SysCall::write_reg(int addr, uint32_t data) {
    for (int i = 0; cores[i] != nullptr; i++) {
        cores[i]->write_reg(addr, data);
    }
}

// *********************************************************************************************
// read NUL-terminated path string from the memory
int SysCall::read_path(char *path_buf, uint32_t base_addr) {
    for (int i = 0; i < SIM_PATH_MAX; i++) {
        mem->read(base_addr + i, path_buf + i, 1);
        if (path_buf[i] == '\0')
            return i;
    }
    return -1;
}

// *********************************************************************************************
// postprocessing of struct stat for stat() and fstat()
void SysCall::postprocess_stat(uint32_t *dst, struct stat *src, uint32_t vtime,
                               uint32_t vtime_nsec) {
    // modify timestamps for deterministic result
    src->st_atim.tv_sec = src->st_mtim.tv_sec = src->st_ctim.tv_sec = vtime;
    src->st_atim.tv_nsec = src->st_mtim.tv_nsec = src->st_ctim.tv_nsec = vtime_nsec;
    // re-calculate st_blocks in a deterministic way
    src->st_blocks = src->st_size / 512;
    memcpy(dst, src, sizeof(uint32_t) * 30);
    // reorder members of stat
    dst[5] = dst[4];
    dst[4] = dst[6];
    dst[6] = dst[7];
}

// *********************************************************************************************
// body of system call emulation
void SysCall::emulate() {
    // for deterministic current time
    uint32_t vtime      = 1640995200 + cores[0]->cnt_inst / 1000000;
    uint32_t vtime_nsec = (cores[0]->cnt_inst % 1000000) * 1000;
    // actual file descriptor and return value of syscall
    int fd, ret;

    switch (read_reg(RiscV::REG_A7)) {
    case SC_CLOSE:
        fd = fd_map[read_reg(RiscV::REG_A0)];
        if (fd >= 3) {
            ret = close(fd);
        } else { // do not close the simulator's stdin/stdout/stderr
            ret = 0;
        }
        write_reg(RiscV::REG_A0, (ret == 0) ? 0 : -errno);
        break;

    case SC_LSEEK:
        fd  = fd_map[read_reg(RiscV::REG_A0)];
        ret = lseek(fd, read_reg(RiscV::REG_A1), read_reg(RiscV::REG_A2));
        write_reg(RiscV::REG_A0, (ret >= 0) ? ret : -errno);
        break;

    case SC_READ: {
        fd        = fd_map[read_reg(RiscV::REG_A0)];
        char *buf = new char[read_reg(RiscV::REG_A2)];
        ret       = read(fd, buf, read_reg(RiscV::REG_A2));
        if (ret > 0) {
            mem->write(read_reg(RiscV::REG_A1), buf, ret);
        }
        write_reg(RiscV::REG_A0, (ret >= 0) ? ret : -errno);
        delete[] buf;
        break;
    }
    case SC_WRITE: {
        fd        = fd_map[read_reg(RiscV::REG_A0)];
        char *buf = new char[read_reg(RiscV::REG_A2)];
        mem->read(read_reg(RiscV::REG_A1), buf, read_reg(RiscV::REG_A2));
        ret = write(fd, buf, read_reg(RiscV::REG_A2));
        write_reg(RiscV::REG_A0, (ret >= 0) ? ret : -errno);
        delete[] buf;
        break;
    }
    case SC_FSTAT: {
        fd = fd_map[read_reg(RiscV::REG_A0)];
        struct stat st;
        uint32_t   *st_buf = new uint32_t[30]; // newlib stat has 30 words
        ret                = fstat(fd, &st);
        if (ret == 0) {
            write_reg(RiscV::REG_A0, 0);
            postprocess_stat(st_buf, &st, vtime, vtime_nsec);
            mem->write(read_reg(RiscV::REG_A1), st_buf, sizeof(uint32_t) * 30);
        } else {
            write_reg(RiscV::REG_A0, -errno);
        }
        delete[] st_buf;
        break;
    }
    case SC_EXIT:
        halt_cores();
        break;

    case SC_GETTIMEOFDAY: {
        uint32_t *tv_buf = new uint32_t[4];
        tv_buf[0]        = vtime;
        tv_buf[1]        = 0;
        tv_buf[2]        = vtime_nsec;
        tv_buf[3]        = 0;
        mem->write(read_reg(RiscV::REG_A0), tv_buf, sizeof(uint32_t) * 4);
        write_reg(RiscV::REG_A0, 0); // never fails: gettimeofday() is not actually called
        delete[] tv_buf;
        break;
    }
    case SC_SBRK:
        if (read_reg(RiscV::REG_A0) == 0) {
            // sbrk(0) returns the current break address
            write_reg(RiscV::REG_A0, brk_addr);
        } else {
            // sbrk(b) (where b != 0) replaces the break address with b and returns b
            brk_addr = read_reg(RiscV::REG_A0);
        }
        break;

    case SC_OPEN: {
        // read file name
        uint32_t base_addr    = read_reg(RiscV::REG_A0);
        char    *filename_buf = new char[SIM_PATH_MAX];
        if (read_path(filename_buf, base_addr) == -1) {
            write_reg(RiscV::REG_A0, (uint32_t)-36); // ENAMETOOLONG
            delete[] filename_buf;
            break;
        }

        // conversion of flags of open()
        int flag_rv = read_reg(RiscV::REG_A1);
        int flag_lx = 0;
        // clang-format off
        if (flag_rv & 0x000008) flag_lx |= 0x000400; // O_APPEND
        if (flag_rv & 0x040000) flag_lx |= 0x080000; // O_CLOEXEC
        if (flag_rv & 0x000200) flag_lx |= 0x000040; // O_CREAT
        if (flag_rv & 0x200000) flag_lx |= 0x010000; // O_DIRECTORY
        if (flag_rv & 0x000800) flag_lx |= 0x000080; // O_EXCL
        if (flag_rv & 0x008000) flag_lx |= 0x000100; // O_NOCTTY
        if (flag_rv & 0x100000) flag_lx |= 0x020000; // O_NOFOLLOW
        if (flag_rv & 0x004000) flag_lx |= 0x000800; // O_NONBLOCK
        if (flag_rv & 0x002000) flag_lx |= 0x101000; // O_SYNC
        if (flag_rv & 0x000400) flag_lx |= 0x000200; // O_TRUNC
        // clang-format on
        flag_lx |= (flag_rv & 0x3); // O_RDONLY, O_WRONLY, O_RDWR

        // open the file and returns a virtual file descriptor
        fd = open(filename_buf, flag_lx, read_reg(RiscV::REG_A2));
        if (fd >= 0) {
            fd_map[fd_next] = fd;
            write_reg(RiscV::REG_A0, fd_next);
            fd_next++;
        } else {
            write_reg(RiscV::REG_A0, -errno);
        }
        delete[] filename_buf;
        break;
    }
    case SC_STAT: {
        // read file name
        uint32_t base_addr    = read_reg(RiscV::REG_A0);
        char    *filename_buf = new char[SIM_PATH_MAX];
        if (read_path(filename_buf, base_addr) == -1) {
            write_reg(RiscV::REG_A0, (uint32_t)-36); // ENAMETOOLONG
            delete[] filename_buf;
            break;
        }
        struct stat st;
        uint32_t   *st_buf = new uint32_t[30]; // newlib stat has 30 words
        ret                = stat(filename_buf, &st);
        if (ret == 0) {
            write_reg(RiscV::REG_A0, 0);
            postprocess_stat(st_buf, &st, vtime, vtime_nsec);
            mem->write(read_reg(RiscV::REG_A1), st_buf, sizeof(uint32_t) * 30);
        } else {
            write_reg(RiscV::REG_A0, -errno);
        }
        delete[] st_buf;
        delete[] filename_buf;
        break;
    }
    }
}

// *********************************************************************************************