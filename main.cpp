#include <ctime>
#include <cstring>
#include <cstdlib>
#include "OSdna.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"

// C program to find Burrows Wheeler transform of
// a given text
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define DEBUG
// Structure to store data of a rotation
struct rotation {
    int index;
    char *suffix;
};

// Compares the rotations and
// sorts the rotations alphabetically
int cmpfunc(const void *x, const void *y) {
    struct rotation *rx = (struct rotation *) x;
    struct rotation *ry = (struct rotation *) y;
    return strcmp(rx->suffix, ry->suffix);
}

// Takes text to be transformed and its length as
// arguments and returns the corresponding suffix array
int *computeSuffixArray(char *input_text, int len_text) {
    // Array of structures to store rotations and
    // their indexes
    struct rotation suff[len_text];

    // Structure is needed to maintain old indexes of
    // rotations after sorting them
    for (int i = 0; i < len_text; i++) {
        suff[i].index = i;
        suff[i].suffix = (input_text + i);
    }

    // Sorts rotations using comparison function defined above
    qsort(suff, len_text, sizeof(struct rotation), cmpfunc);

    // Stores the indexes of sorted rotations
    int *suffix_arr = (int *) malloc(len_text * sizeof(int));
    for (int i = 0; i < len_text; i++)
        suffix_arr[i] = suff[i].index;

    // Returns the computed suffix array
    return suffix_arr;
}

// Takes suffix array and its size as arguments and returns
// the Burrows - Wheeler Transform of given text
char *findLastChar(char *input_text, int *suffix_arr, int n) {
    // Iterates over the suffix array to find
    // the last char of each cyclic rotation
    char *bwt_arr = (char *) malloc(n * sizeof(char));
    int i;
    for (i = 0; i < n; i++) {
        // Computes the last char which is given by
        // input_text[(suffix_arr[i] + n - 1) % n]
        int j = suffix_arr[i] - 1;
        if (j < 0)
            j = j + n;

        bwt_arr[i] = input_text[j];
    }

    bwt_arr[i] = '\0';

    // Returns the computed Burrows - Wheeler Transform
    return bwt_arr;
}

#define SLICE 360

//// Driver program to test functions above
//int main() {
//    clock_t start = clock();
//    char input_text[1024 * 1024 + 1];
//    memset(input_text, 0x00, 1024 * 1024 + 1);
//
//    FILE *fp = fopen("/Users/sergio/ClionProjects/osdna/human_g1k_v37.fasta", "r");
//    fread(input_text, 1, 1024 * SLICE, fp);
//
//    input_text[1024 * SLICE] = '$';
////    printf("SRC:%d\n", strlen(input_text));
////    printf("%s", input_text);
//    int len_text = strlen(input_text);
//
//    // Computes the suffix array of our text
//    int *suffix_arr = computeSuffixArray(input_text, len_text);
//
//    // Adds to the output array the last char of each rotation
//    char *bwt_arr = findLastChar(input_text, suffix_arr, len_text);
//
////    printf("Input text : %s\n", input_text);
////    printf("Burrows - Wheeler Transform : \n%s\n", bwt_arr);
//
//    printf("SRC:%d\n", strlen(input_text));
//    printf("DST:%d\n", strlen(bwt_arr));
//    clock_t end = clock();
//    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
//    printf("Seconds %f\n", seconds);
//    return 0;
//}


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

    osdna_status status = osdna_process(ctx);

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

//    osdna_print_statistic(ctx);

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
//        remove(compressed_f);
//        remove(decompressed_f);
    } else {
        printf("1. File sizes are not the same. Epic fail! %d vs %d\n", ftell(f1), ftell(f2));
    }
}

int main(int argc, char *argv[]) {
    clock_t start = clock();
    if (argc != 4) {
        printf("Usage: <C for compression or D for decompression> <input> <output>\n");
        exit(-2);
    }
    OSDNA_ctx *ctx = osdna_init_ctx();
    if (strncmp(argv[1], "C", 1) == 0) {
        osdna_set_direction(ctx, COMPRESSION);
    }
    if (strncmp(argv[1], "D", 1) == 0) {
        osdna_set_direction(ctx, DECOMPRESSION);
    }

    osdna_set_input_file(ctx, argv[2]);
    osdna_set_output_file(ctx, argv[3]);

    osdna_status status = osdna_process(ctx);

    clock_t end = clock();
    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("Time needed %f sec\n", seconds);

    if (status != OSDNA_OK) {
        printf("Exited with code %d", status);
    } else {
        printf("\nOK\n");
    }

//    test("/Users/sergio/ClionProjects/osdna/lambda_virus.dna");

    return 0;
}

int bitwriter() {
    clock_t start = clock();


    //WRITING
    osdna_status status = OSDNA_OK;
    FILE *src = fopen("/tmp/test.wr", "wb");
    osdna_bit_write_handler *handler = osdna_bit_init(src);

    int8_t buffer[9] = {1, 0, 1, 0, 1, 1, 1, 1, 0};
    status = osdna_bit_write(handler, buffer, 5);
    if (status != OSDNA_OK) {
        printf("Write error %d\n", status);
        return -1;
    }
    status = osdna_bitwriter_finilize(handler);
    if (status != OSDNA_OK) {
        printf("Write error %d\n", status);
        return -2;
    }

    fclose(src);
//
    printf("\n");

    //READING
    FILE *reading = fopen("/tmp/test.wr", "rb");
    osdna_bit_read_handler *handler2 = osdna_bit_read_init(reading);
    int8_t buffer2[1024];
    int toread = 5;
    status = osdna_bit_read(handler2, buffer, &toread);
    if (status == OSDNA_EOF) { //Returns EOF and updates toread
        printf("FINISHED\n");
    } else if (status != OSDNA_OK) {
        printf("Write error %d\n", status);
        return -2;
    }
    fclose(reading);

    clock_t end = clock();

    float seconds = (float) (end - start) / CLOCKS_PER_SEC;
    printf("\nTime needed %f sec\n", seconds);
}
