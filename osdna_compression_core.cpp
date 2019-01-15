#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"

#define TRIGGER_SIZE 3

osdna_error compress_core(OSDNA_ctx *ctx) {
    printf("compress_core\n");

    int bytesRead;
    char file_read_buff[1024];
    char curr_char;
    char last_char = 'Q';
    int last_occ_len = 0;
    osdna_bit_handler *bit_write_handle = osdna_bit_init(ctx->write_stream);

    while (bytesRead = fread(file_read_buff, 1, 1024, ctx->read_stream)) {
        for (int i = 0; i < bytesRead; i++) {
            curr_char = file_read_buff[i];
            if (!is_acceptable_char(curr_char))  // are acceptable only AGCT, everything else is skipped
                continue;

            osdna_bit_write_char(bit_write_handle, curr_char);
        }
    }

    osdna_bitwriter_finilize(bit_write_handle);

    return OSDNA_OK;
}

osdna_error decompress_core(OSDNA_ctx *ctx) {
    printf("decompress_core");

    return OSDNA_OK;
}

