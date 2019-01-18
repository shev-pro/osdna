#ifndef OSDNA_OSDNA_BITREADER_H
#define OSDNA_OSDNA_BITREADER_H

#include "osdna_compressor.h"

#include <cstdio>

#define READ_BUFFER_SIZE 1024

struct osdna_bit_read_handler {
    FILE *read_stream;
    char read_buffer[READ_BUFFER_SIZE];
    short bit_position;
    long buffer_to_read;
    char current_window;
    long bytes_to_read;
};

osdna_bit_read_handler *osdna_bit_read_init(FILE *read_stream);

osdna_error osdna_bit_read_char(osdna_bit_read_handler *handle, char *c);

int get_occ_from_char(char c);

#endif //OSDNA_OSDNA_BITREADER_H
