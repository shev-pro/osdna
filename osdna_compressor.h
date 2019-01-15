#ifndef OSDNA_OSDNA_COMPRESSOR_H
#define OSDNA_OSDNA_COMPRESSOR_H

#include <cstdio>

/**
 * OSDNA COMPRESSOR PIMPL INTERFACE
 */

enum direct {
    COMPRESSION, DECOMPRESSION
};
enum osdna_error {
    OSDNA_OK = 0,
    OSDNA_IO_ERROR,
    OSDNA_CONFIG_ERROR
};
struct OSDNA_ctx {
    char input_file[4096];
    char output_file[4096];
    direct direction = COMPRESSION;
    FILE *read_stream = NULL;
    FILE *write_stream = NULL;
};
OSDNA_ctx *osdna_init_ctx();

osdna_error osdna_set_input_file(OSDNA_ctx *ctx, const char *input_file);

osdna_error osdna_set_output_file(OSDNA_ctx *ctx, const char *output_file);

osdna_error osdna_set_direction(OSDNA_ctx *ctx, direct direction);

osdna_error osdna_process(OSDNA_ctx *ctx);

osdna_error osdna_print_statistic(OSDNA_ctx *ctx);

void osdna_free_ctx(OSDNA_ctx *ctx);

#endif //OSDNA_OSDNA_COMPRESSOR_H
