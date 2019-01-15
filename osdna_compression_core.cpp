#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"

#define TRIGGER_SIZE 3

osdna_error compress_core(OSDNA_ctx *ctx) {
    printf("Initializing compression core\n");

    int bytesRead;
    char file_read_buff[1024]; // for perfomance reasons lets read 1KB at once
    char curr_char;
    char last_char = 'Q'; // Any char excluded AGTC
    int last_occ_len = 0;
    osdna_bit_handler *bit_write_handle = osdna_bit_init(ctx->write_stream);

    int print_counter = 0;
    while (bytesRead = fread(file_read_buff, 1, 1024, ctx->read_stream)) {
        if (print_counter % 1024 == 0) {
            printf("Processed %d Mb\n", print_counter / 1024);
        }
        print_counter++;
        for (int i = 0; i < bytesRead; i++) {
            curr_char = file_read_buff[i];
            if (!is_acceptable_char(curr_char))  // are acceptable only AGCT, everything else is skipped
                continue;

            if (last_char == curr_char) {
                if (last_occ_len < TRIGGER_SIZE) {
                    osdna_bit_write_char(bit_write_handle, curr_char);
                    last_occ_len++;
                } else {
                    last_occ_len++;
                }
            } else {
                if (last_occ_len < TRIGGER_SIZE) {
                    osdna_bit_write_char(bit_write_handle, curr_char);
                    last_occ_len = 1;
                } else {
                    last_occ_len -= 3;
                    while (last_occ_len >= 3) {
                        osdna_bit_write_char(bit_write_handle, '3');
                        last_occ_len -= 3;
                    }
                    if (last_occ_len == 2) {
                        osdna_bit_write_char(bit_write_handle, '2');
                    }
                    if (last_occ_len == 1) {
                        osdna_bit_write_char(bit_write_handle, '1');
                    }
                    if (last_occ_len == 0) {
                        osdna_bit_write_char(bit_write_handle, '0');
                    }
                    last_occ_len = 1;
                    osdna_bit_write_char(bit_write_handle, curr_char);
                }
            }
            last_char = curr_char;
        }
    }

    return osdna_bitwriter_finilize(bit_write_handle);
}

osdna_error decompress_core(OSDNA_ctx *ctx) {
    printf("Initializing compression core\n");

    int bytesRead;
    char file_read_buff[1024];

    int print_counter = 0;
    while (bytesRead = fread(file_read_buff, 1, 1024, ctx->read_stream)) {
        if (print_counter % 1024 == 0) {
            printf("Processed %d Mb\n", print_counter / 1024);
        }
        print_counter++;
        for (int i = 0; i < bytesRead; i++) {
            char curr_char = file_read_buff[i];

        }
    }


    return OSDNA_OK;
}

