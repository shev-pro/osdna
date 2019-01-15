#ifndef OSDNA_OSDNA_BITWRITER_H
#define OSDNA_OSDNA_BITWRITER_H

#include "osdna_compressor.h"

/**
 * Bit writing abstraction, allows character write without handling mask shift
 */

#define WRITE_BUFFER_SIZE 1024

struct osdna_bit_handler {
    FILE *write_stream = NULL;
    char write_buffer[WRITE_BUFFER_SIZE];
    short bit_position;
    short buffer_position;
    char current_window;
};

/**
 * Context initialization
 * @param write_stream opened FP to writing stream, MUST be opened as binary eg wb or wb+
 * @return ctx
 */
osdna_bit_handler *osdna_bit_init(FILE *write_stream);

/**
 * Writes single char c to file in binary rappresentation.
 * Organizes each 4 chars as 1 single byte, permitted charset is AGCT0123
 *
 * Uses WRITE_BUFFER_SIZE bytes cache to make bulk writes
 *
 * @param handle writing handle for context discovery
 * @param c char to write one of AGCT0123
 * @return error
 */
osdna_error osdna_bit_write_char(osdna_bit_handler *handle, char c);

/**
 * Finilizes writing handle dumping all memory saved data and removes everything
 * @param handle
 * @return
 */
osdna_error osdna_bitwriter_finilize(osdna_bit_handler *handle);


#endif //OSDNA_OSDNA_BITWRITER_H
