#include <cstdlib>
#include "osdna_bitreader.h"

#define BYTE_TO_BINARY_PATTERN "%c%c %c%c %c%c %c%c\n"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

osdna_error read_next_window(osdna_bit_read_handler *handle, char *c);

osdna_error read_char_from_window(osdna_bit_read_handler *pHandler, char *c);

osdna_bit_read_handler *osdna_bit_read_init(FILE *read_stream) {
    osdna_bit_read_handler *ctx = (osdna_bit_read_handler *) malloc(sizeof(osdna_bit_read_handler));
    ctx->current_window = 0x00;
    ctx->bit_position = 0;
    ctx->to_read_buff_size = 0;
    ctx->read_stream = read_stream;
    ctx->current_buffer_read_pos = 0;
    ctx->last_window = false;

    fseek(ctx->read_stream, 0L, SEEK_END);
    ctx->file_bytes_remaining = ftell(ctx->read_stream);
    fseek(ctx->read_stream, 0L, SEEK_SET);

    return ctx;
}

char get_char_from_bits(char c) { //Bits-pair encoding protocl
    if (c == 0x00) {
        return 'A'; //0
    }
    if (c == 0x01) {
        return 'C'; //1
    }
    if (c == 0x02) {
        return 'G'; //2
    }
    if (c == 0x03) {
        return 'T'; //3
    }
}

int get_occ_from_char(char c) {
    if (c == 'A')
        return 0;
    if (c == 'C')
        return 1;
    if (c == 'G')
        return 2;
    if (c == 'T')
        return 3;
}

osdna_error osdna_bit_read_char(osdna_bit_read_handler *handle, char *c) {
    if (handle->bit_position > 0) {
        return read_char_from_window(handle, c);
    } else { // finished current window
        osdna_error status = read_next_window(handle, c);
        if (status != OSDNA_OK) {
            return status;
        } else {
            return read_char_from_window(handle, c);
        }
    }
}

osdna_error read_char_from_window(osdna_bit_read_handler *handle, char *c) {
    char mask = (handle->current_window & 0xc0) >> 6;  //takes first 2 bits and shifts right
    handle->current_window = (handle->current_window << 2);
    handle->bit_position = handle->bit_position - 2;
    *c = get_char_from_bits(mask);
    return OSDNA_OK;
}

osdna_error read_next_window(osdna_bit_read_handler *handle, char *c) {
    if (handle->file_bytes_remaining == 2) { // A very, very special case
        handle->to_read_buff_size = fread(handle->read_buffer, 1, READ_BUFFER_SIZE, handle->read_stream);
        handle->to_read_buff_size = handle->to_read_buff_size - 2;
        if (handle->to_read_buff_size != 0) {
            return OSDNA_IO_ERROR;
        }
        handle->current_window = handle->read_buffer[handle->current_buffer_read_pos];
        handle->current_buffer_read_pos++;
        handle->bit_position = 8 - handle->read_buffer[handle->current_buffer_read_pos];
        handle->file_bytes_remaining = 0;
        return OSDNA_OK;
    }
    if (handle->to_read_buff_size <= 0) {
        if (handle->last_window) {
            handle->current_window = handle->read_buffer[handle->current_buffer_read_pos];
            handle->current_buffer_read_pos++;
            handle->bit_position = 8 - handle->read_buffer[handle->current_buffer_read_pos];
            handle->last_window = false;
            return OSDNA_OK;
        }
        if (handle->file_bytes_remaining <= 0) {
            return OSDNA_EOF;
        }
        handle->to_read_buff_size = fread(handle->read_buffer, 1, READ_BUFFER_SIZE, handle->read_stream);
        handle->file_bytes_remaining = handle->file_bytes_remaining - handle->to_read_buff_size;
        handle->current_buffer_read_pos = 0;
        if (handle->file_bytes_remaining == 0) {
            // Last slice
            handle->to_read_buff_size = handle->to_read_buff_size - 2;
            handle->last_window = true;
        }
    }
    handle->current_window = handle->read_buffer[handle->current_buffer_read_pos];
    handle->current_buffer_read_pos++;
    handle->to_read_buff_size--;
    handle->bit_position = 8;
    return OSDNA_OK;
}

//osdna_error read_next_window(osdna_bit_read_handler *handle, char *c) {
//    if (handle->to_read_buff_size > 0) { // There are other data in cache
//        handle->current_buffer_read_pos++;
//        handle->current_window = handle->read_buffer[handle->current_buffer_read_pos];
//
//        handle->to_read_buff_size--;
//        handle->bit_position = 8;
//        return OSDNA_OK;
//    } else {
//        handle->to_read_buff_size = fread(handle->read_buffer, 1, READ_BUFFER_SIZE, handle->read_stream);
//        handle->file_bytes_remaining = handle->file_bytes_remaining - handle->to_read_buff_size;
//
//        if (handle->file_bytes_remaining == 0) { // Last slice was read from file
//            // Substract 2 bytes from the end for padding handling
//            handle->to_read_buff_size = handle->to_read_buff_size - 2;
//        } else {
//            handle->current_buffer_read_pos = 0;
//        }
//        if (handle->to_read_buff_size <= 0 || handle->last_window) {
//            // Non ci dimentichiamo che abbiamo altri belli 2 byte da leggere!
//            int bits = handle->read_buffer[handle->current_buffer_read_pos + 1];
//            handle->bit_position = 8 - bits;
//            handle->current_window = handle->read_buffer[handle->current_buffer_read_pos];
//            if (handle->last_window) {
//                return OSDNA_EOF;
//            }
//            handle->last_window = true;
//            return OSDNA_OK;
//        } else {
//            handle->current_window = handle->read_buffer[handle->current_buffer_read_pos];
//            handle->bit_position = 8;
//            handle->to_read_buff_size--;
//            return OSDNA_OK;
//        }
//    }
//}