#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"
#include <string.h>
#include <math.h>

#define BUFF_SIZE 1024
#define MAX_TRIGGER_SIZE 2000
#define BIT_ENCODE_SIZE 10
#define POS(X) (X == 'A' ? 0 : (X=='C' ? 1 : (X=='G' ? 2 : (X=='T' ? 3 : -1))))


void dump_occurence(char curr_char, int last_occ_len, osdna_bit_write_handler *bit_write_handle);

osdna_status write_char(OSDNA_ctx *ctx, char c, bool forced = false);

int char_to_count(char c);


osdna_status compress_core(OSDNA_ctx *ctx) {
    printf("Initializing compression core\n");

    int bytesRead;
    char file_read_buff[1024]; // for perfomance reasons lets read 1KB at once
    char curr_char;
    char last_char = 'Q'; // Any char excluded AGTC
    int last_occ_len = 0;
    osdna_bit_write_handler *bit_write_handle = osdna_bit_init(ctx->write_stream);

    int print_counter = 0;

//    osdna_status error = (opt_trigger_calc(ctx->read_stream, &ctx->trigger_size, &ctx->bit_per_num));
//    if (error != OSDNA_OK) {
//        printf("FAILED with %d\n", error);
//        return error;
//    }
    if (ctx->trigger_size < 3) {
        ctx->trigger_size = 3;
    }
    printf("Trigger size %d\n", ctx->trigger_size);
    printf("Bits per num %d\n", ctx->bit_per_num);
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
//                    osdna_bit_write_char(bit_write_handle, curr_char);
                    last_occ_len++;
                } else {
                    last_occ_len++;
                }
            } else {
                if (last_occ_len < ctx->trigger_size) {
//                    osdna_bit_write_char(bit_write_handle, curr_char);
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

osdna_status opt_param_calc(OSDNA_opt_param *opt_param) {
    long long occ_matr[4][MAX_TRIGGER_SIZE];
    long long bit_advantage[BIT_ENCODE_SIZE][MAX_TRIGGER_SIZE];
    long long trigger_size_advantage[4][BIT_ENCODE_SIZE][MAX_TRIGGER_SIZE];
    int trigger_size_opt[4];
    int bit_enc_opt[4];
    long long total = 0;
    char buff[BUFF_SIZE];
    char last_char = 0, curr_char;
    int bytesRead, cont = 1;

    memset(occ_matr, 0, 4 * MAX_TRIGGER_SIZE * sizeof(unsigned long));
    memset(bit_advantage, 0, BIT_ENCODE_SIZE * MAX_TRIGGER_SIZE * sizeof(long long));
    memset(trigger_size_advantage, 0, 4 * BIT_ENCODE_SIZE * MAX_TRIGGER_SIZE * sizeof(long long));
    memset(trigger_size_opt, 0, 4 * sizeof(int));

    for (int char_curr = 0; char_curr < 4; char_curr++) {
        bit_enc_opt[char_curr] = 2;
    }

    if (opt_param->read_stream == (FILE *) NULL) {
        printf("Error opening file %s\n", opt_param->read_stream);
        return OSDNA_IO_ERROR;
    }

    bytesRead = fread(buff, 1, 1, opt_param->read_stream);
    if (bytesRead == 0) {
        printf("Error empty file\n");
        return OSDNA_IO_ERROR;
    }
    total++;
    last_char = buff[0];
    while (bytesRead = fread(buff, 1, BUFF_SIZE, opt_param->read_stream)) {
        for (int i = 0; i < bytesRead; i++) {
            curr_char = buff[i];
            total++;
            if (POS(curr_char) == -1) {
                printf("Bad file\n");
                //return -1;
                continue;  //skip bad character occurrences
            }
            if (curr_char == last_char)
                cont++;
            else {
                if (cont > MAX_TRIGGER_SIZE) {
                    printf("Too long trigger size \n");
                    return OSDNA_IO_ERROR;
                }
                occ_matr[POS(last_char)][cont]++;
                cont = 1;
                last_char = curr_char;
            }
        }
    }
    occ_matr[POS(last_char)][cont]++;


    for (int i = 2; i < BIT_ENCODE_SIZE; i++) {
        bit_advantage[i][0] = -i;
    }

    for (int bit_encode = 2; bit_encode < BIT_ENCODE_SIZE; bit_encode++)
        for (int i = 1; i < MAX_TRIGGER_SIZE; i++)
            bit_advantage[bit_encode][i] =
                    bit_advantage[bit_encode][i - 1] + ((i % (int) (pow(2, bit_encode) - 1)) > 0) * 2
                    + ((i % (int) (pow(2, bit_encode) - 1)) == 0) * (2 - bit_encode);

    for (int char_curr = 0; char_curr < 4; char_curr++)
        for (int bit_encode = 2; bit_encode < BIT_ENCODE_SIZE; bit_encode++)
            for (int trigger_size = 1; trigger_size < MAX_TRIGGER_SIZE; trigger_size++)
                for (int i = trigger_size; i < MAX_TRIGGER_SIZE; i++)
                    trigger_size_advantage[char_curr][bit_encode][trigger_size] += occ_matr[char_curr][i]
                                                                                   * bit_advantage[bit_encode][i - trigger_size];


    for (int char_curr = 0; char_curr < 4; char_curr++)
        for (int bit_encode = 2; bit_encode < BIT_ENCODE_SIZE; bit_encode++)
            for (int trigger_size = 1; trigger_size < MAX_TRIGGER_SIZE; trigger_size++) {
                if (trigger_size_advantage[char_curr][bit_encode][trigger_size] >
                    trigger_size_advantage[char_curr][bit_enc_opt[char_curr]][trigger_size_opt[char_curr]]) {
                    trigger_size_opt[char_curr] = trigger_size;
                    bit_enc_opt[char_curr] = bit_encode;
                }
            }

    long long adv_total=0;
    for (int char_curr = 0; char_curr < 4; char_curr++) {
        printf("char: %d trigger_size_opt: %d, bit_enc_opt: %d\n", char_curr, trigger_size_opt[char_curr],
               bit_enc_opt[char_curr]);
        switch (char_curr) {
            case 0:
                opt_param->opt_bit_A = bit_enc_opt[0];
                opt_param->opt_trigger_A = trigger_size_opt[0];
                break;
            case 1:
                opt_param->opt_bit_C = bit_enc_opt[1];
                opt_param->opt_trigger_C = trigger_size_opt[1];
                break;
            case 2:
                opt_param->opt_bit_G = bit_enc_opt[2];
                opt_param->opt_trigger_G = trigger_size_opt[2];
                break;
            case 3:
                opt_param->opt_bit_T = bit_enc_opt[3];
                opt_param->opt_trigger_T = trigger_size_opt[3];
        }

        adv_total += trigger_size_advantage[char_curr][bit_enc_opt[char_curr]][trigger_size_opt[char_curr]];
    }
    printf("adv_total %lli\n", adv_total);

    return OSDNA_OK;
}

void dump_occurence(char curr_char, int last_occ_len, osdna_bit_write_handler *bit_write_handle) {
//    last_occ_len -= 3;
//    while (last_occ_len >= 3) {
//        osdna_bit_write_char(bit_write_handle, '3');
//        last_occ_len -= 3;
//    }
//    if (last_occ_len == 2) {
//        osdna_bit_write_char(bit_write_handle, '2');
//    }
//    if (last_occ_len == 1) {
//        osdna_bit_write_char(bit_write_handle, '1');
//    }
//    if (last_occ_len == 0) {
//        osdna_bit_write_char(bit_write_handle, '0');
//    }
//    osdna_bit_write_char(bit_write_handle, curr_char);
}

osdna_status decompress_core(OSDNA_ctx *ctx) {
//    printf("Initializing decompression core\n");
//
//    osdna_bit_read_handler *handler = osdna_bit_read_init(ctx->read_stream);
//    char current_char = 'Q';
//    osdna_status error = OSDNA_OK;
//    char prev_char = 'Q';
//    int last_seq = 1;
//    bool reading_seq = false;
////    ctx->trigger_size = 4;
////    int print_counter = 0;
//    while ((error = osdna_bit_read_char(handler, &current_char)) == OSDNA_OK) {
////        if (print_counter % 1024 == 0) {
////            printf("Processed %d Mb\n", print_counter / 1024);
////        }
////        print_counter++;
//
//        if (reading_seq) {
//            reading_seq = false;
//            last_seq = 1;
//            int iter_count = char_to_count(current_char);
//            for (int i = 0; i < iter_count; i++) {
////                printf("%c", prev_char);
//                error = write_char(ctx, prev_char);
//                if (error != OSDNA_OK) {
//                    return error;
//                }
//            }
//            if (iter_count == ctx->trigger_size) {
//                reading_seq = true;
//            }
//            continue;
//        }
//
//        if (prev_char == current_char) {
//            last_seq++;
//            error = write_char(ctx, current_char);
////            printf("%c", current_char);
//            if (error != OSDNA_OK) {
//                return error;
//            }
//        } else {
//            last_seq = 1;
//            error = write_char(ctx, current_char);
////            printf("%c", current_char);
//            if (error != OSDNA_OK) {
//                return error;
//            }
//        }
//        prev_char = current_char;
//
//        if (last_seq == ctx->trigger_size) {
//            reading_seq = true;
//            continue;
//        }
//
//
//    }
//    if (error == OSDNA_EOF) { // if end of file we finished here!
//        return write_char(ctx, prev_char, true);
//    } else {
//        return error;
//    }
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

osdna_status write_char(OSDNA_ctx *ctx, char c, bool forced) {
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