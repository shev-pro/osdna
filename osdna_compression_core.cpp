#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"

#define TRIGGER_SIZE 3

osdna_error compress_core(OSDNA_ctx *ctx) {
    printf("Initializing compression core\n");

    int bytesRead;
    char file_read_buff[1024]; // for perfomance reasons lets read 1KB at once
    char curr_char;
    char last_char = 'Q'; // Any char excluded AGTC
    int last_occ_len = 0;
    osdna_bit_write_handler *bit_write_handle = osdna_bit_init(ctx->write_stream);

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

    osdna_bit_read_handler *handler = osdna_bit_read_init(ctx->read_stream);
    char current_char = 'Q';
    osdna_error error = OSDNA_OK;
    char prev_char = 'Q';
    int last_seq = 1;
    bool reading_seq = false;
    while ((error = osdna_bit_read_char(handler, &current_char)) == OSDNA_OK) {
        if (reading_seq) {
            reading_seq = false;
            last_seq = 1;
            // Crazy code start
            if (current_char == 'C') {
                printf("%c", prev_char);
            }
            if (current_char == 'G') {
                printf("%c", prev_char);
                printf("%c", prev_char);
            }
            if (current_char == 'T') {
                printf("%c", prev_char);
                printf("%c", prev_char);
                printf("%c", prev_char);
                reading_seq = true;
            }
            // Crazy code end
            continue;
        }

        if (prev_char == current_char) {
            last_seq++;
            printf("%c", current_char);
        } else {
            last_seq = 1;
            printf("%c", current_char);
        }
        prev_char = current_char;

        if (last_seq == 3) {
            reading_seq = true;
            continue;
        }


    }
    if (error == OSDNA_EOF) { // if end of file we finished here!
        return OSDNA_OK;
    } else {
        return error;
    }
}

