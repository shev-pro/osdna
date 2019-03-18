#ifndef OSDNA_OSDNA_COMPRESSOR_H
#define OSDNA_OSDNA_COMPRESSOR_H

#include <cstdio>

/**
 * OSDNA COMPRESSOR PIMPL INTERFACE
 */

enum direct {
    COMPRESSION, DECOMPRESSION
};
enum osdna_status {
    OSDNA_OK = 0,
    OSDNA_IO_ERROR,
    OSDNA_CONFIG_ERROR,
    OSDNA_DATA_CHAR_ERROR,
    OSDNA_EOF
};
struct OSDNA_opt_param {
    FILE *read_stream = NULL;
    int opt_trigger_A;
    int opt_trigger_C;
    int opt_trigger_G;
    int opt_trigger_T;
    int opt_bit_A;
    int opt_bit_C;
    int opt_bit_G;
    int opt_bit_T;
    long total_read_bytes;
};

struct OSDNA_opt {
    FILE *read_stream = NULL;
    int opt_trigger;
    int opt_bit;
};

struct OSDNA_ctx {
    char input_file[4096];
    char output_file[4096];
    direct direction = COMPRESSION;
    FILE *read_stream = NULL;
    FILE *write_stream = NULL;
    char output_buffer[4096];
    int out_buff_pos;
    long total_read_bytes;
};


OSDNA_ctx *osdna_init_ctx();

osdna_status osdna_set_input_file(OSDNA_ctx *ctx, const char *input_file);

osdna_status osdna_set_output_file(OSDNA_ctx *ctx, const char *output_file);

osdna_status osdna_set_direction(OSDNA_ctx *ctx, direct direction);

osdna_status osdna_process(OSDNA_ctx *ctx);

osdna_status osdna_print_statistic(OSDNA_ctx *ctx);

void osdna_free_ctx(OSDNA_ctx *ctx);

//osdna_status opt_trigger_calc(FILE *read_stream, int *trigger_size, int *bit_per_num);
osdna_status opt_param_calc(OSDNA_opt_param *opt_param);

#endif //OSDNA_OSDNA_COMPRESSOR_H
