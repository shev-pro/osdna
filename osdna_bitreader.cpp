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

osdna_bit_read_handler *osdna_bit_read_init(FILE *read_stream) {
    osdna_bit_read_handler *ctx = (osdna_bit_read_handler *) malloc(sizeof(osdna_bit_read_handler));
    ctx->current_window = 0x00;
    ctx->bit_position = 0;
    ctx->buffer_to_read = 0;
    ctx->read_stream = read_stream;

    fseek(ctx->read_stream, 0L, SEEK_END);
    ctx->bytes_to_read = ftell(ctx->read_stream) - 2;
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

osdna_error osdna_bit_read_char(osdna_bit_read_handler *handle, char *c) {
    if (handle->bit_position > 0) {
        char mask = (handle->current_window & 0xc0) >> 6;  //takes first 2 bits and shifts right
        handle->current_window = (handle->current_window << 2);
        handle->bit_position = handle->bit_position - 2;
        *c = get_char_from_bits(mask);
        return OSDNA_OK;
    } else { // finished current window
        if (handle->buffer_to_read > 0) {
//            printf("reading next window %d\n", handle->buffer_to_read);
            handle->current_window = handle->read_buffer[READ_BUFFER_SIZE - handle->buffer_to_read];
            handle->buffer_to_read--;
            handle->bit_position = 8;
            return osdna_bit_read_char(handle, c);
        } else {
            handle->buffer_to_read = fread(handle->read_buffer, 1, READ_BUFFER_SIZE, handle->read_stream);
//            printf("reading next buffer %d\n", handle->buffer_to_read);
            if (handle->buffer_to_read == 0) {
                return OSDNA_EOF;
            } else {
                handle->current_window = handle->read_buffer[READ_BUFFER_SIZE - handle->buffer_to_read];
                handle->bit_position = 8;
                handle->buffer_to_read--;
                return osdna_bit_read_char(handle, c);
            }
        }
    }
}