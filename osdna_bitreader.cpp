#include <cstdlib>
#include "osdna_bitreader.h"
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

osdna_status read_next_window(osdna_bit_read_handler *handle);

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


osdna_status read_next_window(osdna_bit_read_handler *handle) {
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
            handle->bit_position = handle->read_buffer[handle->current_buffer_read_pos];
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

int8_t read_bit_from_window(osdna_bit_read_handler *handle) {
    unsigned char mask = handle->current_window & 0x80;
#ifdef DEBUG
    printf("Window %c%c %c%c %c%c %c%c\n", BYTE_TO_BINARY(handle->current_window));
    printf("Mask %c%c %c%c %c%c %c%c\n", BYTE_TO_BINARY(mask));
#endif
    handle->current_window = handle->current_window << 1;
    handle->bit_position--;
    if (mask == 0x80) {
        return 1;
    } else {
        return 0;
    }
}

osdna_status osdna_bit_read(osdna_bit_read_handler *handle, int8_t *buffer, int *count) {
    int intcount = *count;
    osdna_status st = OSDNA_OK;
    if (handle->bit_position == 0) {
        st = read_next_window(handle);
        if (st != OSDNA_OK) {
            return st;
        }
    }
    int buffer_pos = 0;
    if (handle->bit_position > intcount) { // Read directly
        for (int i = 0; i < intcount; ++i) {
            buffer[buffer_pos] = read_bit_from_window(handle);
            buffer_pos++;
        }
    } else {
        while (intcount > 0) {
            if (handle->bit_position == 0) {
                st = read_next_window(handle);
                if (st == OSDNA_EOF) {
                    *count = *count - intcount;
                }
                if (st != OSDNA_OK) {
                    return st;
                }
            }
            buffer[buffer_pos] = read_bit_from_window(handle);
            buffer_pos++;
            intcount--;
        }
    }

    return st;
}