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

osdna_bit_handler *osdna_bit_init(FILE *write_stream) {
    osdna_bit_handler *ctx = (osdna_bit_handler *) malloc(sizeof(osdna_bit_handler));
    ctx->current_window = 0x00;
    ctx->bit_position = 0;
    ctx->buffer_position = 0;
    return ctx;
}

char get_bits_char(char c) {
    if (c == 'A')
        return 0x00;
    if (c == 'C')
        return 0x01;
    if (c == 'G')
        return 0x02;
    if (c == 'T')
        return 0x03;
}

osdna_error write_char(osdna_bit_handler *handle, char c) {
    char bitchar = get_bits_char(c);
    handle->current_window = handle->current_window | bitchar;

    printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(handle->current_window));
    if(handle->bit_position == 8){

    }
    a = a << 2;


}
