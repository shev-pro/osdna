#include "OSdna.h"

int main() {
    OSDNA_ctx *ctx = osdna_init_ctx();
    osdna_set_direction(ctx, COMPRESSION);
    osdna_set_input_file(ctx, "../lambdavirus.dna");
    osdna_set_output_file(ctx, "../lambdavirus.dna.osdna");

    osdna_error status = osdna_process(ctx);



    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }
    return 0;
}
