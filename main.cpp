/*H**********************************************************************
*       Copyright Ferdinando D'Avino, Lino Sarto, Sergiy Shevchenko 2019.  All rights reserved.
*H*/
#include <ctime>
#include <cstring>
#include <cstdlib>
#include "OSdna.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"
#include "osdna_utils.h"
#include "osdna_compressor.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    clock_t start = clock();
    if (argc != 4 && argc != 5) {
        printf("Usage: <C for compression or D for decompression> <input> <output> [--bwt]\n");
        exit(-2);
    }
    OSDNA_ctx *ctx = osdna_init_ctx();
    ctx->bwt = false;
    if (strncmp(argv[1], "C", 1) == 0) {
        osdna_set_direction(ctx, COMPRESSION);
    }
    if (strncmp(argv[1], "D", 1) == 0) {
        osdna_set_direction(ctx, DECOMPRESSION);
    }
    if (argc == 5) {
        if (strncmp(argv[4], "--bwt", 5) == 0) {
            ctx->bwt = true;
            printf("BWT enabled\n");
        } else {
            printf("Usage: <C for compression or D for decompression> <input> <output> [--bwt]\n");
            exit(-2);
        }
    }

    osdna_set_input_file(ctx, argv[2]);
    osdna_set_output_file(ctx, argv[3]);

    osdna_status status = osdna_process(ctx);

    clock_t end = clock();
    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Time needed %f sec\n", seconds);

    if (status != OSDNA_OK) {
        printf("Exited with code %d", status);
    } else {
        printf("\nOK\n");
    }

    return 0;
}

