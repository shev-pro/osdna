#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "osdna_compressor.h"

osdna_error preflight_checks(OSDNA_ctx *ctx);

OSDNA_ctx *osdna_init_ctx() {
    OSDNA_ctx *ctx = (OSDNA_ctx *) malloc(sizeof(OSDNA_ctx));
    return ctx;
}

osdna_error osdna_set_input_file(OSDNA_ctx *ctx, const char *input_file) {
    printf("set_input_file[%s]\n", input_file);
    strncpy(ctx->input_file, input_file, 4096);
    return OSDNA_OK;
}

osdna_error osdna_set_output_file(OSDNA_ctx *ctx, const char *output_file) {
    printf("set_input_file[%s]\n", output_file);
    strncpy(ctx->output_file, output_file, 4096);
    return OSDNA_OK;
}

osdna_error osdna_set_direction(OSDNA_ctx *ctx, const direct direction) {
    ctx->direction = direction;
    return OSDNA_OK;
}

osdna_error osdna_process(OSDNA_ctx *ctx) {
    osdna_error st = preflight_checks(ctx);
    if (st != OSDNA_OK) {
        return st;
    }

    int bytesRead;
    char buffer[1024];
    while (bytesRead = fread(buffer, 1, 1024, ctx->read_stream)) {
        for (int i = 0; i < bytesRead; i++) {
            printf("%c", buffer[i]);
        }
    }

    return OSDNA_OK;
}

void osdna_free_ctx(OSDNA_ctx *ctx) {
    if (ctx->read_stream != NULL) {
        fclose(ctx->read_stream);
    }
    if (ctx->write_stream != NULL) {
        fclose(ctx->write_stream);
    }
    free(ctx);
}

osdna_error preflight_checks(OSDNA_ctx *ctx) {
    ctx->read_stream = fopen(ctx->input_file, "rb");
    if (ctx->read_stream == NULL) {
        return OSDNA_IO_ERROR;
    }

    ctx->write_stream = fopen(ctx->output_file, "wb");
    if (ctx->write_stream == NULL) {
        return OSDNA_IO_ERROR;
    }
}