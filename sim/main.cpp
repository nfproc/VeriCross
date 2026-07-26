// VeriCross: A Rapid Cross-Verification Platform for Soft Processors
// Copyright (C) 2020-2026 Digital Systems Lab. New BSD License is applied.
// *********************************************************************************************
#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <chrono>
#include <format>
#include <filesystem>
#include "loader.hpp"
#include "memory.hpp"
#include "syscall.hpp"
#include "functional_core.hpp"
#include "kronos_core.hpp"
#include "kronos_core_faulty.hpp"
#include "rvcorep_core.hpp"

uint64_t sim_single(Core *, Memory *, SysCall *, FILE *, uint64_t);
uint64_t sim_concurrent(Core **, Memory *, SysCall *, FILE *, uint64_t, int);

// Known processor core models
static const char *known_cores[] = {"kronos", "kronos_faulty", "rvcorep", nullptr};

// Command Line Argument
struct args_t {
    bool dump_log, proc_only;
    uint64_t max_insts;
    int mismatch_insts;
    std::string core_type;
    std::string base_dir, bin_file, dump_file, sp_file, log_file;
};

// Return values for checkarg
enum class ArgResult {
    Success,      // Normal success, proceed with simulation
    Failed,       // Argument error
    InfoPrinted   // Argument to print information specified, we should exit now
};

// *********************************************************************************************
// Parse a number with optional suffix (k/m/g)
static bool parse_scaled_number(const char *str, uint64_t &result)
{
    char *endptr = nullptr;
    long long val = strtoll(str, &endptr, 10);
    if (endptr == str || val < 0) {
        return false;
    }
    if (*endptr == 'k' || *endptr == 'K') {
        val *= 1000;
        endptr++;
    } else if (*endptr == 'm' || *endptr == 'M') {
        val *= 1000000;
        endptr++;
    } else if (*endptr == 'g' || *endptr == 'G') {
        val *= 1000000000;
        endptr++;
    }
    if (*endptr != '\0') {
        return false; // reject other trailing characters
    }
    result = static_cast<uint64_t>(val);
    return true;
}

// *********************************************************************************************
// Print Usage
void usage(char **argv)
{
    printf("Usage: %s [options] basedir binary stack\n", argv[0]);
    printf("  - options:\n");
    printf("    - -p                    : simulate processor core only\n");
    printf("    - -c core / --core core : processor core model (see --list-core)\n");
    printf("    - --list-core           : list available core models and exit\n");
    printf("    - -l                    : print the instruction trace to stdout\n");
    printf("    - --logfile file        : print the instruction trace to the specified file\n");
    printf("    - -icount               : alias of --instruction\n");
    printf("    - --instruction count   : maximum instructions before halt (default: 10m)\n");
    printf("                              count supports a suffix of k, m, or g\n");
    printf("    - -m N / --mismatch N   : show last N instructions on mismatch\n");
    printf("  - basedir: directory name where the files are stored\n");
    printf("  - binary : file name of binary file without '.bin'\n");
    printf("  - stack  : file name of stack image without '_sp.txt'\n");
}

// *********************************************************************************************
// Check if the given core type is a known model
static bool is_known_core(const std::string &core_type)
{
    for (int i = 0; known_cores[i] != nullptr; i++) {
        if (core_type == known_cores[i]) {
            return true;
        }
    }
    return false;
}

// *********************************************************************************************
// Check arguments
ArgResult checkarg(int argc, char **argv, struct args_t &args)
{
    static struct option opts[] = {
        {"core",        required_argument, 0, 'c'},
        {"logfile",     required_argument, 0, 0},
        {"instruction", required_argument, 0, 'i'},
        {"list-core",   no_argument,       0, 0},
        {"mismatch",    required_argument, 0, 'm'},
        {"version",     no_argument,       0, 'v'},
        {0,             0,                 0, 0}
    };
    int c, long_index;

    args.dump_log = false;
    args.proc_only = false;
    args.max_insts = 10000000; // default
    args.mismatch_insts = 0;
    args.core_type = "";

    while ((c = getopt_long(argc, argv, "c:lpi:m:v", opts, &long_index)) != -1) {
        switch (c) {
        case 0:
            if (long_index == 1) { // logfile
                args.dump_log = true;
                args.log_file = std::string(optarg);
            } else if (long_index == 3) { // list-core
                printf("Available core models:\n");
                for (int i = 0; known_cores[i] != nullptr; i++) {
                    printf("  %s\n", known_cores[i]);
                }
                return ArgResult::InfoPrinted;
            }
            break;
        case 'c':
            args.core_type = std::string(optarg);
            if (!is_known_core(args.core_type)) {
                fprintf(stderr, "!! Unknown core model: %s\n", optarg);
                return ArgResult::Failed;
            }
            break;
        case 'l':
            args.dump_log = true;
            break;
        case 'p':
            args.proc_only = true;
            break;
        case 'i':
            if (!parse_scaled_number(optarg, args.max_insts)) {
                fprintf(stderr, "!! Invalid instruction count: %s\n", optarg);
                return ArgResult::Failed;
            }
            break;
        case 'm':
            args.mismatch_insts = atoi(optarg);
            if (args.mismatch_insts < 1) {
                fprintf(stderr, "!! Invalid mismatch count: %s\n", optarg);
                return ArgResult::Failed;
            }
            break;
        case 'v':
            printf("## %s\n##%s\n", PROG_NAME, PROG_VER);
            return ArgResult::InfoPrinted;
        default:
            return ArgResult::Failed;
        }
    }
    if (argc - optind != 3) {
        return ArgResult::Failed;
    } else if (args.proc_only && args.core_type.empty()) {
        fprintf(stderr, "!! core model must be specified in -p mode\n");
        return ArgResult::Failed;
    }


    args.base_dir = std::string(argv[optind]);
    args.bin_file = std::format("{}.bin", argv[optind + 1]);
    args.dump_file = std::format("{}.objdump", argv[optind + 1]);
    args.sp_file = std::format("{}_sp.txt", argv[optind + 2]);
    return ArgResult::Success;
}

// *********************************************************************************************
// Main function for cross-simulation
int main(int argc, char **argv)
{
    struct args_t args;
    FILE *log = nullptr;
    ArgResult result = checkarg(argc, argv, args);
    if (result == ArgResult::Failed) {
        usage(argv);
        return 1;
    } else if (result == ArgResult::InfoPrinted) {
        return 0;
    }
    
    // instantiation
    Core *cores[3] = {nullptr};
    int proc_idx = 0;

    if (! args.proc_only) {
        cores[proc_idx++] = new FunctionalCore();
    }
    if (args.core_type == "kronos") {
        cores[proc_idx++] = new KronosCore();
    } else if (args.core_type == "kronos_faulty") {
        cores[proc_idx++] = new KronosCoreFaulty();
    } else if (args.core_type == "rvcorep") {
        cores[proc_idx++] = new RVCorePCore();
    }
    bool concurrent = (proc_idx > 1);

    Memory *mem = new Memory(cores);
    if (FunctionalCore *fc = dynamic_cast<FunctionalCore *>(cores[0])) {
        fc->bind_memory(mem);
    }
    Loader *ld = new Loader(args.base_dir.c_str(), args.bin_file.c_str(),
                            args.dump_file.c_str(), args.sp_file.c_str());
    if (! ld->valid) {
        fprintf(stderr, "!! %s. Stop.\n", ld->message.c_str());
        return 1;
    }

    SysCall *sys = new SysCall(cores, mem, 0, 1, 2, ld->brk_addr);

    // memory initialization
    for (Loader::Section **s = ld->sections; *s != nullptr; s++) {
        for (uint32_t i = 0; i < (*s)->length / 4; i++) {
            mem->write((*s)->addr + i * 4, &(*s)->data[i]);
        }
    }

    // open log file if needed
    if (! args.log_file.empty()) {
        log = fopen(args.log_file.c_str(), "w");
        if (log == nullptr) {
            fprintf(stderr, "!! Failed to open log file(%s). Stop.\n", args.log_file.c_str());
            return 1;
        }
    } else if (args.dump_log) {
        log = stdout;
    }

    // change directory for simulation
    try {
        std::filesystem::current_path(args.base_dir);
    } catch (std::filesystem::filesystem_error &e) {
        fprintf(stderr, "!! failed to change working directory (%s)", e.what());
        return 1;
    }

    // main loop
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    if (concurrent) {
        sim_concurrent(cores, mem, sys, log, args.max_insts, args.mismatch_insts);
    } else {
        sim_single(cores[0], mem, sys, log, args.max_insts);
    }
    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

    // display results
    double elapsed = std::chrono::duration<double>(end_time - start_time).count();
    double kips = (double) cores[0]->cnt_inst / elapsed / 1000.0;
    printf("## Processor stopped after executing %lu insts.\n", cores[0]->cnt_inst);
    printf("## Simulation took %.3f sec.\n", elapsed);
    printf("## Simulation speed was %.2f KIPS.\n", kips);
    return 0;
}

// *********************************************************************************************