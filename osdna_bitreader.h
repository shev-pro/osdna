/*H**********************************************************************
*       Copyright Ferdinando D'Avino, Lino Sarto, Sergiy Shevchenko 2019.  All rights reserved.
*H*/

#ifndef OSDNA_OSDNA_BITREADER_H
#define OSDNA_OSDNA_BITREADER_H
#include <stdint.h>
#include "osdna_compressor.h"

#include <cstdio>

#define READ_BUFFER_SIZE 1024

struct osdna_bit_read_handler {
    FILE *read_stream;
    char read_buffer[READ_BUFFER_SIZE];
    short bit_position;
    long to_read_buff_size;
    long current_buffer_read_pos;
    char current_window;
    long file_bytes_remaining;
    bool last_window;
};

osdna_bit_read_handler *osdna_bit_read_init(FILE *read_stream);

osdna_status osdna_bit_read(osdna_bit_read_handler *handle, int8_t* buffer, int* count);

#endif //OSDNA_OSDNA_BITREADER_H
