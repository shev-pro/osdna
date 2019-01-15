#include <cstdio>
#include <cstdlib>
#include "osdna_bitwriter.h"
#include "osdna_utils.h"

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

osdna_error write_window(osdna_bit_handler *handler, bool forced = false);

osdna_bit_handler *osdna_bit_init(FILE *write_stream) {
    osdna_bit_handler *ctx = (osdna_bit_handler *) malloc(sizeof(osdna_bit_handler));
    ctx->current_window = 0x00;
    ctx->bit_position = 0;
    ctx->buffer_position = 0;
    ctx->write_stream = write_stream;
    return ctx;
}

char get_bits_char(char c) {
    if (c == 'A' || c == '0')
        return 0x00;
    if (c == 'C' || c == '1')
        return 0x01;
    if (c == 'G' || c == '2')
        return 0x02;
    if (c == 'T' || c == '3')
        return 0x03;
}

osdna_error osdna_bit_write_char(osdna_bit_handler *handle, char c) {
    char bitchar = get_bits_char(c);
//    printf("%c", c);

    handle->current_window = handle->current_window | bitchar;

//    printf("Added char %c, current window: %c%c %c%c %c%c %c%c\n", c, BYTE_TO_BINARY(handle->current_window));
    handle->bit_position = handle->bit_position + 2;
    if (handle->bit_position == 8) {
        osdna_error error = write_window(handle);
        if (error != OSDNA_OK)
            return error;
        handle->bit_position = 0;
        handle->current_window = 0x00;
    } else {
        handle->current_window = handle->current_window << 2;
    }
}

osdna_error write_window(osdna_bit_handler *handler, bool forced) {
    handler->write_buffer[handler->buffer_position] = handler->current_window;
    handler->buffer_position++;
    if (handler->buffer_position >= WRITE_BUFFER_SIZE || forced) {
        int wrote_bytes = fwrite(handler->write_buffer, 1, handler->buffer_position - 1, handler->write_stream);
        if (wrote_bytes != handler->buffer_position - 1) {
            return OSDNA_IO_ERROR;
        }
        handler->buffer_position = 0;
    }

    return OSDNA_OK;
}

osdna_error osdna_bitwriter_finilize(osdna_bit_handler *handle) {
//    printf("Cache: \n");
//    for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
//        printf("Pos: %d = %c%c %c%c %c%c %c%c\n", i, BYTE_TO_BINARY(handle->write_buffer[i]));
//    }
    if (handle->bit_position != 0) {
        int bits_to_fill = 8 - handle->bit_position;
        while (handle->bit_position != 0) {
            osdna_bit_write_char(handle, 0x00);
        }

        handle->current_window = (unsigned char) bits_to_fill;
    }

    osdna_error error = write_window(handle, true);
    fflush(handle->write_stream);
    fclose(handle->write_stream);

    free(handle);
    return error;

}