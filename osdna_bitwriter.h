#ifndef OSDNA_OSDNA_BITWRITER_H
#define OSDNA_OSDNA_BITWRITER_H

#include "osdna_compressor.h"

#define WRITE_BUFFER_SIZE 1024

struct osdna_bit_handler {
    FILE *write_stream = NULL;
    char write_buffer[WRITE_BUFFER_SIZE];
    short bit_position;
    short buffer_position;
    char current_window;
};

osdna_bit_handler *osdna_bit_init(FILE *write_stream);

osdna_error osdna_bit_write_char(osdna_bit_handler *handle, char c);

osdna_error osdna_bitwriter_finilize(osdna_bit_handler *handle);


#endif //OSDNA_OSDNA_BITWRITER_H
