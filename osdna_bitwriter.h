/*H**********************************************************************
*       Copyright Ferdinando D'Avino, Lino Sarto, Sergiy Shevchenko 2019.  All rights reserved.
*H*/
#ifndef OSDNA_OSDNA_BITWRITER_H
#define OSDNA_OSDNA_BITWRITER_H

#include "osdna_compressor.h"
#include <stdint.h>
/**
 * Bit writing abstraction, allows character write without handling mask shift
 */

#define WRITE_BUFFER_SIZE 1024
//#define DEBUG
struct osdna_bit_write_handler {
    FILE *write_stream;
    char write_buffer[WRITE_BUFFER_SIZE];
    short bit_position;
    short buffer_position;
    unsigned char current_window;
};

/**
 * Context initialization
 * @param write_stream opened FP to writing stream, MUST be opened as binary eg wb or wb+
 * @return ctx
 */
osdna_bit_write_handler *osdna_bit_init(FILE *write_stream);

/**
 * Writes boolean sequences to file
 * @param handle
 * @param bytes bytes buffer (accepts only 0 or 1)
 * @param count bit count to write
 * @return
 */
osdna_status osdna_bit_write(osdna_bit_write_handler *handle, int8_t *bytes, int count);

/**
 * Finilizes writing handle dumping all memory saved data and removes everything
 * @param handle
 * @return
 */
osdna_status osdna_bitwriter_finilize(osdna_bit_write_handler *handle);


#endif //OSDNA_OSDNA_BITWRITER_H
