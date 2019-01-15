#include "OSdna.h"
#include "osdna_bitwriter.h"

int main() {
    OSDNA_ctx *ctx = osdna_init_ctx();
    osdna_set_direction(ctx, COMPRESSION);
    osdna_set_input_file(ctx, "../lambda_virus.dna");
    osdna_set_output_file(ctx, "../lambda_virus.dna.osdna");

    osdna_error status = osdna_process(ctx);

    if (status != OSDNA_OK) {
        printf("We have an error %d", status);
    }

    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }
    return 0;
}
