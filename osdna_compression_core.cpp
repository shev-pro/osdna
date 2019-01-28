#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"

#define BIT_ADVANTAGE 0
#define FILE_SIZE 1
#define MAX_TRIGGER_SIZE 1024*2
#define POS(X) (X == 'A' ? 0 : (X=='C' ? 1 : (X=='G' ? 2 : (X=='T' ? 3 : -1))))


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

    ctx->trigger_size = (opt_trigger_calc(ctx->read_stream));
    if (ctx->trigger_size < 3) {
        ctx->trigger_size = 3;
    }
    printf("Trigger size %d\n", ctx->trigger_size);
//    ctx->trigger_size = 4;
    while (bytesRead = fread(file_read_buff, 1, 1024, ctx->read_stream)) {
//        if (print_counter % 1024 == 0) {
//            printf("Processed %d Mb\n", print_counter / 1024);
//        }
//        print_counter++;
        for (int i = 0; i < bytesRead; i++) {
            curr_char = file_read_buff[i];
            if (!is_acceptable_char(curr_char))  // are acceptable only AGCT, everything else is skipped
//                return OSDNA_DATA_CHAR_ERROR;
                continue;

            if (last_char == curr_char) {
                if (last_occ_len < ctx->trigger_size) {
                    osdna_bit_write_char(bit_write_handle, curr_char);
                    last_occ_len++;
                } else {
                    last_occ_len++;
                }
            } else {
                if (last_occ_len < ctx->trigger_size) {
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

int opt_trigger_calc(FILE *read_stream) {
    long occ_matr[4][MAX_TRIGGER_SIZE] = {{0}};
    int bit_advantage[MAX_TRIGGER_SIZE] = {0};
    int trigger_size_advantage[MAX_TRIGGER_SIZE][2] = {{0}};
    unsigned long total_byte = 0;
    char buff[102400];
    char last_char = 0, curr_char;
    int bytesRead, count = 1, opt_trigger_size = 0;

    if (read_stream == (FILE *) NULL) {
        printf("Error opening file\n");
        return -1;
    }

    bytesRead = fread(buff, 1, 1, read_stream);
    if (bytesRead == 0) {
        printf("Error empty file\n");
        return -1;
    }
    total_byte++;
    last_char = buff[0];
    while (bytesRead = fread(buff, 1, 102400, read_stream)) {
        for (int i = 0; i < bytesRead; i++) {
            curr_char = buff[i];
            total_byte++;
            if (POS(curr_char) == -1) {
                printf("Bad file\n");
                return -1;
            }
            if (curr_char == last_char)
                count++;
            else {
                if (count > MAX_TRIGGER_SIZE) {
                    printf("Too long trigger size \n");
                    return -1;
                }
                occ_matr[POS(last_char)][count]++;
                count = 1;
                last_char = curr_char;
            }
        }
    }
    occ_matr[POS(last_char)][count]++;

    bit_advantage[0] = -2;
    for (int i = 1; i < MAX_TRIGGER_SIZE; i++)
        bit_advantage[i] = bit_advantage[i - 1] + ((i % 3) > 0) * 2;

    for (int j = 2; j < MAX_TRIGGER_SIZE; j++) {
        for (int i = j; i < MAX_TRIGGER_SIZE; i++) {
            trigger_size_advantage[j][BIT_ADVANTAGE] +=
                    (occ_matr[0][i] + occ_matr[1][i] + occ_matr[2][i] + occ_matr[3][i])
                    * bit_advantage[i - j];
            trigger_size_advantage[j][FILE_SIZE] = total_byte / 4 - (trigger_size_advantage[j][0] / 4) +
                                                   (((total_byte * 2 - trigger_size_advantage[j][0]) % 8) > 0);
        }
    }

    for (int i = 2; i < MAX_TRIGGER_SIZE; i++) {
        printf("Opt trigger update %d %d\n", trigger_size_advantage[i][BIT_ADVANTAGE],
               trigger_size_advantage[opt_trigger_size][BIT_ADVANTAGE]);
        if (trigger_size_advantage[i][BIT_ADVANTAGE] > trigger_size_advantage[opt_trigger_size][BIT_ADVANTAGE]) {
            opt_trigger_size = i;
            printf("Opt trigger update %d\n", opt_trigger_size);
        }
    }

    fseek(read_stream, 0, SEEK_SET);

    printf("original size: %lu\n", total_byte);
    printf("size with 2 bit encode: %lu\n", total_byte / 4);

    for (int j = 2; j < MAX_TRIGGER_SIZE; j++)
        printf("(%d) %d  %d\n", j, trigger_size_advantage[j][BIT_ADVANTAGE], trigger_size_advantage[j][FILE_SIZE]);

    for (int i = 1; i < MAX_TRIGGER_SIZE; i++) {
        printf("%d)\t%ld\t\t%ld\t\t%ld\t\t%ld\n", i, occ_matr[POS('A')][i], occ_matr[POS('C')][i],
               occ_matr[POS('G')][i], occ_matr[POS('T')][i]);
    }

    return opt_trigger_size;
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
//    ctx->trigger_size = 4;
//    int print_counter = 0;
    while ((error = osdna_bit_read_char(handler, &current_char)) == OSDNA_OK) {
//        if (print_counter % 1024 == 0) {
//            printf("Processed %d Mb\n", print_counter / 1024);
//        }
//        print_counter++;

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
            if (iter_count == ctx->trigger_size) {
                reading_seq = true;
            }
            continue;
        }

        if (prev_char == current_char) {
            last_seq++;
            error = write_char(ctx, current_char);
//            printf("%c", current_char);
            if (error != OSDNA_OK) {
                return error;
            }
        } else {
            last_seq = 1;
            error = write_char(ctx, current_char);
//            printf("%c", current_char);
            if (error != OSDNA_OK) {
                return error;
            }
        }
        prev_char = current_char;

        if (last_seq == ctx->trigger_size) {
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
//        printf("%d\n", ctx->out_buff_pos);
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