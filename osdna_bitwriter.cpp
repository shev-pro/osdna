#include <cstdio>
#include <cstdlib>
#include "osdna_bitwriter.h"

osdna_bit_handler *osdna_bit_init(FILE *write_stream) {
    osdna_bit_handler *ctx = (osdna_bit_handler *) malloc(sizeof(osdna_bit_handler));
    return ctx;
}

