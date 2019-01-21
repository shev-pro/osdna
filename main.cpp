#include <ctime>
#include "OSdna.h"
#include "osdna_bitwriter.h"

int main() {
    clock_t start = clock();
    OSDNA_ctx *ctx = osdna_init_ctx();
    /* COMPRESSION */
//    osdna_set_direction(ctx, COMPRESSION);
//    osdna_set_input_file(ctx, "../lambda_virus.dna");
//    osdna_set_output_file(ctx, "../lambda_virus.dna.osdna");




    /* DECOMPRESSION */
    osdna_set_direction(ctx, DECOMPRESSION);
    osdna_set_input_file(ctx, "../lambda_virus.dna.osdna");
    osdna_set_output_file(ctx, "../lambda_virus.dna.decompressed");

    osdna_error status = osdna_process(ctx);

    if (status != OSDNA_OK) {
        printf("We have an error %d", status);
    } else {
        printf("\nFinished decompression :) \n");
    }

    osdna_print_statistic(ctx);

    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }
    clock_t end = clock();
    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Time needed %f sec", seconds);
    return 0;
}
