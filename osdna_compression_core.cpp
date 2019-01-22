#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"

#define TRIGGER_SIZE 3

void dump_occurence(char curr_char, int last_occ_len, osdna_bit_write_handler *bit_write_handle);

osdna_error write_char(OSDNA_ctx *ctx, char c, bool forced = false);

int char_to_count(char c);

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
                    dump_occurence(curr_char, last_occ_len, bit_write_handle);
                    last_occ_len = 1;
                }
            }
            last_char = curr_char;
        }
    }
    if (last_occ_len > 1) {
        dump_occurence(curr_char, last_occ_len - 1, bit_write_handle);
    }

    return osdna_bitwriter_finilize(bit_write_handle);
}

void dump_occurence(char curr_char, int last_occ_len, osdna_bit_write_handler *bit_write_handle) {
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
    osdna_bit_write_char(bit_write_handle, curr_char);
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
            int iter_count = char_to_count(current_char);
            for (int i = 0; i < iter_count; i++) {
//                printf("%c", prev_char);
                error = write_char(ctx, prev_char);
                if (error != OSDNA_OK) {
                    return error;
                }
            }
            if (iter_count == 3) {
                reading_seq = true;
            }
            continue;
        }

        if (prev_char == current_char) {
            last_seq++;
            error = write_char(ctx, current_char);
            if (error != OSDNA_OK) {
                return error;
            }
        } else {
            last_seq = 1;
            error = write_char(ctx, current_char);
            if (error != OSDNA_OK) {
                return error;
            }
        }
        prev_char = current_char;

        if (last_seq == 3) {
            reading_seq = true;
            continue;
        }


    }
    if (error == OSDNA_EOF) { // if end of file we finished here!
        return write_char(ctx, prev_char, true);
    } else {
        return error;
    }
}

int char_to_count(char c) {
    if (c == 'A')
        return 0;
    if (c == 'C')
        return 1;
    if (c == 'G')
        return 2;
    if (c == 'T')
        return 3;
}

osdna_error write_char(OSDNA_ctx *ctx, char c, bool forced) {
    if (ctx->out_buff_pos == 4096 - 1 || forced) {
        int bytes = fwrite(ctx->output_buffer, 1, ctx->out_buff_pos, ctx->write_stream);
        if (bytes != ctx->out_buff_pos) {
            return OSDNA_IO_ERROR;
        }
        ctx->out_buff_pos = 0;
    }
    ctx->output_buffer[ctx->out_buff_pos] = c;
    ctx->out_buff_pos++;
    return OSDNA_OK;
}