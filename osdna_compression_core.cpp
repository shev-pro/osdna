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
    printf("Initializing decompression core\n");

    int bytesRead;
    char file_read_buff[1024];
    char file_write_buff[1024];
    int bytes_to_write = 0;

    fseek(ctx->read_stream, 0L, SEEK_END);
    long ssize = ftell(ctx->read_stream);
    ssize = ssize - 2; // last 2 bytes are special once cause used for padding
    fseek(ctx->read_stream, 0L, SEEK_SET);

    int print_counter = 0;
    while (bytesRead = fread(file_read_buff, 1, 1024, ctx->read_stream)) {
        if (print_counter % 1024 == 0) {
            printf("Processed %d Mb\n", print_counter / 1024);
        }
        print_counter++;

        for (int i = 0; i < bytesRead; i++) {
            ssize--;
            char curr_char = file_read_buff[i];
            if (ssize == 0) { //special case - padding
                printf("handle padding");
            } else {
                if (is_acceptable_char(curr_char)) { //its a plain char - write to output
                    file_write_buff[bytes_to_write] = curr_char;
                    bytes_to_write++;
                    if (bytes_to_write == 1024) { // probably i miss +1 here
                        fwrite(file_write_buff, sizeof(char), 1024, ctx->write_stream);
                        bytes_to_write = 0;
                    }
                } else {
                    // continue here
                }
            }

        }
    }


    return OSDNA_OK;
}

