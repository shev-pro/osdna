#include <ctime>
#include <cstring>
#include "OSdna.h"
#include "osdna_bitwriter.h"

int test(char *test_file) {
    /* COMPRESSION */

    OSDNA_ctx *ctx = osdna_init_ctx();
    osdna_set_direction(ctx, COMPRESSION);
    char compressed_f[1024];
    char decompressed_f[1024];
    sprintf(compressed_f, "%s.tmp.compressed", test_file);
    sprintf(decompressed_f, "%s.tmp.decompressed", test_file);

    osdna_set_input_file(ctx, test_file);
    osdna_set_output_file(ctx, compressed_f);

    osdna_error status = osdna_process(ctx);

    if (status != OSDNA_OK) {
        printf("We have an error %d in compression \n", status);
    } else {
        printf("\nFinished decompression :) \n");
    }

    osdna_print_statistic(ctx);

    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }

    /* DECOMPRESSION */
    ctx = osdna_init_ctx();
    osdna_set_direction(ctx, DECOMPRESSION);
    osdna_set_input_file(ctx, compressed_f);
    osdna_set_output_file(ctx, decompressed_f);

    status = osdna_process(ctx);

    if (status != OSDNA_OK) {
        printf("We have an error %d in decompression \n", status);
    } else {
        printf("\nFinished decompression :) \n");
    }

    osdna_print_statistic(ctx);

    if (ctx) {
        osdna_free_ctx(ctx);
        ctx = NULL;
    }

    printf("Start checking files\n");
    FILE *f1 = fopen(test_file, "r");
    FILE *f2 = fopen(decompressed_f, "r");
    fseek(f1, 0L, SEEK_END);
    fseek(f2, 0L, SEEK_END);

    if (ftell(f1) == ftell(f2)) {
        printf("1. File sizes are the same! We are hopefull!\n");
        fseek(f1, 0L, SEEK_SET);
        fseek(f2, 0L, SEEK_SET);
        char buff1[1024];
        char buff2[1024];
        while (fread(buff1, 1, 1024, f1) != 0) {
            int read = fread(buff2, 1, 1024, f2);
            if (memcmp(buff1, buff2, read) != 0) {
                printf("2. Files are not the same. Oops\n");
                goto finally; // We need this goto
            }
        }
        printf("2. Two files are ridiculously same. Win!\n");
        finally:
        fclose(f1);
        fclose(f2);
        remove(compressed_f);
        remove(decompressed_f);
    } else {
        printf("1. File sizes are not the same. Epic fail!\n");
    }
}

int main() {
    clock_t start = clock();
    OSDNA_ctx *ctx = osdna_init_ctx();
    /* COMPRESSION */
    osdna_set_direction(ctx, COMPRESSION);
    osdna_set_input_file(ctx, "../lambda_virus.dna");
    osdna_set_output_file(ctx, "../lambda_virus.dna.osdna");

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

    ctx = osdna_init_ctx();
    /* DECOMPRESSION */
    osdna_set_direction(ctx, DECOMPRESSION);
    osdna_set_input_file(ctx, "../lambda_virus.dna.osdna");
    osdna_set_output_file(ctx, "../lambda_virus.dna.decompressed");

    status = osdna_process(ctx);

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
