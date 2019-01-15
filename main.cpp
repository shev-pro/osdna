#include "OSdna.h"
#include "osdna_bitwriter.h"

int main() {
    OSDNA_ctx *ctx = osdna_init_ctx();
    osdna_set_direction(ctx, COMPRESSION);
    osdna_set_input_file(ctx, "../lambdavirus.dna");
    osdna_set_output_file(ctx, "../lambdavirus.dna.osdna");

//    osdna_error status = osdna_process(ctx);

    FILE *test = fopen("../lambdavirus.dna", "wb");
    osdna_bit_handler *handle = osdna_bit_init(test);
    write_char(handle, 'A');
    write_char(handle, 'A');
    write_char(handle, 'A');
    write_char(handle, 'A');

    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }
    return 0;
}
