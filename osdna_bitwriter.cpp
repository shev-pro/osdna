#include <cstdio>
#include <cstdlib>
#include "osdna_bitwriter.h"

char get_mask_for_rotation(int i) {
    char mask = 0xFF;
    switch (i) {
        case 0: {
            mask = 0xC0;
            break;
        }
        case 1: {
            mask = 0x30;
            break;
        }
        case 2: {
            mask = 0x0C;
            break;
        }
        case 3: {
            mask = 0x03;
            break;
        }
        default: {
            printf("get_mask_rotation unexpected rotation value");
            mask = 0x03;
        }
    }

    return mask;
}

char get_char_by_mask(char bb, int i) {
    char section = (bb & get_mask_for_rotation(i));
    section = section << 2 * i;
    if (section == 0x00)
        return 'A';
    if (section == 0x40)
        return 'C';
    if (section == 0x80)
        return 'G';
    if (section == 0xC0)
        return 'T';
    return '?';
}

osdna_bit_handler *osdna_bit_init(FILE *write_stream) {
    osdna_bit_handler *ctx = (osdna_bit_handler *) malloc(sizeof(osdna_bit_handler));
    return ctx;
}

osdna_error write_char(osdna_bit_handler *handle, char c) {

}
