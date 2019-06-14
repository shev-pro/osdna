/*H**********************************************************************
*       Copyright Ferdinando D'Avino, Lino Sarto, Sergiy Shevchenko 2019.  All rights reserved.
*H*/
#include <cstdio>
#include <cstdlib>
#include "osdna_bitwriter.h"
#include "osdna_utils.h"
//#define DEBUG
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

osdna_status write_window(osdna_bit_write_handler *handler, bool forced = false);

osdna_status write_bits_to_window(osdna_bit_write_handler *handle, const int8_t *bytes, int count);

osdna_bit_write_handler *osdna_bit_init(FILE *write_stream) {
    osdna_bit_write_handler *ctx = (osdna_bit_write_handler *) malloc(sizeof(osdna_bit_write_handler));
    ctx->current_window = 0x00;
    ctx->bit_position = 0;
    ctx->buffer_position = 0;
    ctx->write_stream = write_stream;
    return ctx;
}

osdna_status write_window(osdna_bit_write_handler *handler, bool forced) {
    handler->write_buffer[handler->buffer_position] = handler->current_window;
    handler->buffer_position++;
    if (handler->buffer_position >= WRITE_BUFFER_SIZE || forced) {
        int wrote_bytes = fwrite(handler->write_buffer, 1, handler->buffer_position, handler->write_stream);
        if (wrote_bytes != handler->buffer_position) {
            return OSDNA_IO_ERROR;
        }
        handler->buffer_position = 0;
    }
    handler->bit_position = 0;
    handler->current_window = 0x00;

    return OSDNA_OK;
}

osdna_status osdna_bitwriter_finilize(osdna_bit_write_handler *handle) {
    int8_t padding_size = (int8_t) handle->bit_position;
//    handle->buffer_position++;
    osdna_status status = write_window(handle);
    handle->current_window = padding_size;
    if (status != OSDNA_OK) {
        return status;
    }
    short buffer_size = handle->buffer_position + 1;
    status = write_window(handle, true);
    if (status != OSDNA_OK) {
        return status;
    }

#ifdef DEBUG
    for (int i = 0; i < buffer_size; i++) {
        printf("%c%c %c%c %c%c %c%c ", BYTE_TO_BINARY(handle->write_buffer[i]));
    }
#endif

    fflush(handle->write_stream);
    fclose(handle->write_stream);

    free(handle);
    return status;
}

unsigned char bit_pos_2_mask(int pos) {
    unsigned char c = 0x01;
    return c << 7 - pos;
}

osdna_status osdna_bit_write(osdna_bit_write_handler *handle, int8_t *bytes, int count) {
#ifdef DEBUG
    printf("Bit to write: ");
    for (int i = 0; i < count; i++) {
        printf("%d", bytes[i]);
    }
    printf("\n");

    printf("Pre update window \t%c%c %c%c %c%c %c%c\n", BYTE_TO_BINARY(handle->current_window));
#endif
    osdna_status st = OSDNA_OK;
    if (8 - handle->bit_position >= count) { //There is space in windows to write
        st = write_bits_to_window(handle, bytes, count);
        if (st != OSDNA_OK) {
            return st;
        }
    } else {
        while (count > 0) {
            int bitspace_remained = 8 - handle->bit_position;
            st = write_bits_to_window(handle, bytes, MIN(bitspace_remained, count));
            count = count - bitspace_remained;
            if (st != OSDNA_OK) {
                return st;
            }
            bytes = bytes + bitspace_remained;
        }
    }

#ifdef DEBUG
    printf("Post update window \t%c%c %c%c %c%c %c%c\n", BYTE_TO_BINARY(handle->current_window));
    printf("Buffer pointer     \t");
    for (int j = 0; j < handle->bit_position; ++j) {
        if (j % 2 != 0) {
            printf(" ");
        }
        printf(" ");
    }
    printf("^\n");
#endif
    return OSDNA_OK;
}

osdna_status write_bits_to_window(osdna_bit_write_handler *handle, const int8_t *bytes, int count) {
    for (int i = 0; i < count; i++) {
        if (bytes[i] == 1) {
            unsigned char mask = bit_pos_2_mask(handle->bit_position);
            handle->current_window = handle->current_window | mask;
        }
        handle->bit_position++;
    }
    if (handle->bit_position == 8) {
        osdna_status error = write_window(handle);
        if (error != OSDNA_OK)
            return error;
    }
    return OSDNA_OK;
}