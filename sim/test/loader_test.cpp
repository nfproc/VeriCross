#include <stdio.h>
#include "../loader.hpp"

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("usage: ./loader_test base_dir bin_file dump_file sp_file\n");
        return 1;
    }
    Loader ld(argv[1], argv[2], argv[3], argv[4]);
    if (! ld.valid) {
        printf("!! %s. Stop.\n", ld.message.c_str());
        return 1;
    }
    for (int i = 0; ld.sections[i]; i++) {
        for (uint32_t j = 0; j < ld.sections[i]->length / 4; j++) {
            if (j < 3 || j >= ld.sections[i]->length / 4 - 3)
                printf("%08x %08x\n", ld.sections[i]->addr + j * 4, ld.sections[i]->data[j]);
            if (j == 3 && j < ld.sections[i]->length / 4 - 3)
                printf("...\n");
        }
        printf("\n");
    }
    return 0;
}