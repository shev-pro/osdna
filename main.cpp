#include <ctime>
#include "OSdna.h"
#include "osdna_bitwriter.h"

int main() {
    clock_t start = clock();
    OSDNA_ctx *ctx = osdna_init_ctx();
    osdna_set_direction(ctx, COMPRESSION);
//    osdna_set_input_file(ctx, "../human_g1k_v37.fasta");
    osdna_set_input_file(ctx, "../lambda_virus.dna.fasta");
    osdna_set_output_file(ctx, "../lambda_virus.dna.osdna");

    osdna_error status = osdna_process(ctx);

    if (status != OSDNA_OK) {
        printf("We have an error %d", status);
    }

    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }
    clock_t end = clock();
    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Time needed %f sec", seconds);
    return 0;
}
