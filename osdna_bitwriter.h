#ifndef OSDNA_OSDNA_BITWRITER_H
#define OSDNA_OSDNA_BITWRITER_H

struct osdna_bit_handler {
    FILE *write_stream = NULL;
    char write_buffer[1024];
    char current_window = 0x00;
    short byte_position = 0;
};

osdna_bit_handler * osdna_bit_init(FILE *write_stream);



#endif //OSDNA_OSDNA_BITWRITER_H
