#include "osdna_compression_core.h"
#include "osdna_utils.h"
#include "osdna_bitwriter.h"
#include "osdna_bitreader.h"
#include <string.h>
#include <math.h>
#include <cstdlib>
#include <math.h>

//#define DEBUG
#define BUFF_SIZE 1024
#define MAX_TRIGGER_SIZE 2000
#define BIT_ENCODE_SIZE 10
#define POS(X) (X == 'A' ? 0 : (X=='C' ? 1 : (X=='G' ? 2 : (X=='T' ? 3 : -1))))


void printBuffer(int8_t *buff, int pointer){
    printf("BUFFER: ");
    for (int yy = 0; yy < pointer ; yy++){
        printf("%d", buff[yy]);
    }
    printf("\n");
}

int getMaxBitLen(OSDNA_opt_param *opt){
    int toReturn = 0;
    toReturn = (opt->opt_bit_A > opt->opt_bit_C)?opt->opt_bit_A:opt->opt_bit_C;
    toReturn = (toReturn > opt->opt_bit_G)?toReturn:opt->opt_bit_G;
    toReturn = (toReturn > opt->opt_bit_T)?toReturn:opt->opt_bit_T;
    return toReturn;
}

int getBitLengthByChar(OSDNA_opt_param *opt, char c){
    switch(c){
        case 'A':{
            return opt->opt_bit_A;
        }
        case 'C':{
            return opt->opt_bit_C;
        }
        case 'G':{
            return opt->opt_bit_G;
        }
        case 'T':{
            return opt->opt_bit_T;
        }
    }
    return -1;
}

int getTriggerSizeByChar(OSDNA_opt_param *opt, char c){
    switch(c){
        case 'A':{
            return opt->opt_trigger_A;
        }
        case 'C':{
            return opt->opt_trigger_C;
        }
        case 'G':{
            return opt->opt_trigger_G;
        }
        case 'T':{
            return opt->opt_trigger_T;
        }
    }
    return -1;
}

void fillBufferNumber(int8_t *buff, char ch, int val, int *pointer, OSDNA_opt_param *opt){

    int n, c, k;
    n = val;
    for (c = getBitLengthByChar(opt, ch) - 1; c >= 0; c--)
    {
        k = n >> c;
        k = k & 1;
        if (k)
            buff[*pointer] = 1;
        else
            buff[*pointer] = 0;

#ifdef DEBUG
        printf("buff[%d]:%d ",*pointer, buff[*pointer]);
#endif
        *pointer = *pointer + 1;
    }
#ifdef DEBUG
    printf("\n");
#endif
}

void fillBuffer(int8_t *buff, char c, int val, bool number, int *pointer, OSDNA_opt_param *opt){
    if(number){
        fillBufferNumber(buff, c, val, pointer, opt);
    }
    else {
        switch (c) {
            case 'A': {
                buff[*pointer] = 0;
                buff[*pointer + 1] = 0;
#ifdef DEBUG
                printf("buff[%d]:%d ",*pointer, buff[*pointer]);
                printf("buff[%d]:%d \n",*pointer + 1, buff[*pointer + 1]);
#endif
                *pointer = *pointer + 2;
                break;
            }
            case 'C': {
                buff[*pointer] = 0;
                buff[*pointer + 1] = 1;
#ifdef DEBUG
                printf("buff[%d]:%d ",*pointer, buff[*pointer]);
                printf("buff[%d]:%d \n",*pointer + 1, buff[*pointer + 1]);
#endif
                *pointer = *pointer + 2;
                break;
            }
            case 'G': {
                buff[*pointer] = 1;
                buff[*pointer + 1] = 0;
#ifdef DEBUG
                printf("buff[%d]:%d ",*pointer, buff[*pointer]);
                printf("buff[%d]:%d \n",*pointer + 1, buff[*pointer + 1]);
#endif
                *pointer = *pointer + 2;
                break;
            }
            case 'T': {
                buff[*pointer] = 1;
                buff[*pointer + 1] = 1;
#ifdef DEBUG
                printf("buff[%d]:%d ",*pointer, buff[*pointer]);
                printf("buff[%d]:%d \n",*pointer + 1, buff[*pointer + 1]);
#endif
                *pointer = *pointer + 2;
                break;
            }
        }
    }
}

void initActualParam(int *actual_trigger_size, int *actual_bit_size, char last_char, OSDNA_opt_param *opt_param){
    switch (last_char){
        case 'A':{
            *actual_bit_size = opt_param->opt_bit_A;
            *actual_trigger_size = opt_param->opt_trigger_A;
            break;
        }
        case 'C':{
            *actual_bit_size = opt_param->opt_bit_C;
            *actual_trigger_size = opt_param->opt_trigger_C;
            break;
        }
        case 'G':{
            *actual_bit_size = opt_param->opt_bit_G;
            *actual_trigger_size = opt_param->opt_trigger_G;
            break;
        }
        case 'T':{
            *actual_bit_size = opt_param->opt_bit_T;
            *actual_trigger_size = opt_param->opt_trigger_T;
            break;
        }
    }
}

void manageWrite(int8_t *toWrite, int last_occ_len, int actual_trigger_size, int actual_bit_size, char last_char, OSDNA_opt_param *opt_param,  osdna_bit_write_handler *bit_write_handle){
    int pointer = 0;
    int j;
    int counter = 0;
    initActualParam(&actual_trigger_size, &actual_bit_size, last_char, opt_param); // init actual trigger and bit size

    if (last_occ_len < actual_trigger_size || actual_trigger_size == 0) {
        for(j = 0; j < last_occ_len; j++)
            fillBuffer(toWrite,last_char,-1,false,&pointer,opt_param);
        counter = last_occ_len * 2;
    } else if(last_occ_len == actual_trigger_size){
        for(j = 0; j < last_occ_len; j++){
            fillBuffer(toWrite, last_char, -1, false, &pointer,opt_param);
        }
        fillBuffer(toWrite, last_char, 0, true, &pointer, opt_param);
        counter = (last_occ_len * 2) + actual_bit_size;
    } else{
        for(j = 0; j < actual_trigger_size; j++){
            fillBuffer(toWrite, last_char, -1,false, &pointer, opt_param);
        }
        counter = actual_trigger_size * 2;
        last_occ_len -= actual_trigger_size;
        int max_r = (int) (pow(2, actual_bit_size)) - 1;
        while(last_occ_len >= max_r){
            fillBuffer(toWrite, last_char, max_r, true, &pointer, opt_param);
            last_occ_len -= max_r;
            counter += actual_bit_size;
        }
        fillBuffer(toWrite, last_char, last_occ_len, true, &pointer, opt_param);
        counter += actual_bit_size;
    }
#ifdef DEBUG
    printBuffer(toWrite, pointer);
#endif
    osdna_bit_write(bit_write_handle, toWrite, counter);
}

void readHeader(OSDNA_opt_param *opt_param, osdna_bit_read_handler *hdl){

    opt_param->opt_trigger_A = 8;
    opt_param->opt_trigger_C = 9;
    opt_param->opt_trigger_G = 9;
    opt_param->opt_trigger_T = 8;
    opt_param->opt_bit_A = 3;
    opt_param->opt_bit_C = 2;
    opt_param->opt_bit_G = 2;
    opt_param->opt_bit_T = 3;

}

char decodeBit(int8_t *r){
    if(r[0] == 0) {
        if (r[1] == 0)
            return 'A';
        else
            return 'C';
    }
    else {
        if (r[1] == 0)
            return 'G';
        else
            return 'T';
    }
}

int getDecimalByBinary(int8_t const *buff, int count) {
    int toReturn = 0;
    for(int i = 0; i < count; i++){
        if(buff[i] == 1)
            toReturn += (int) pow(2,count - i - 1);
    }//0
    return toReturn;
}

osdna_status opt_param_calc(OSDNA_opt_param *opt_param) {
    long long occ_matr[4][MAX_TRIGGER_SIZE];
    long long bit_advantage[BIT_ENCODE_SIZE][MAX_TRIGGER_SIZE];
    long long trigger_size_advantage[4][BIT_ENCODE_SIZE][MAX_TRIGGER_SIZE];
    int trigger_size_opt[4];
    int bit_enc_opt[4];
    long long total = 0;
    char buff[BUFF_SIZE];
    char last_char = 0, curr_char;
    int bytesRead, cont = 1;

    memset(occ_matr, 0, 4 * MAX_TRIGGER_SIZE * sizeof(unsigned long));
    memset(bit_advantage, 0, BIT_ENCODE_SIZE * MAX_TRIGGER_SIZE * sizeof(long long));
    memset(trigger_size_advantage, 0, 4 * BIT_ENCODE_SIZE * MAX_TRIGGER_SIZE * sizeof(long long));
    memset(trigger_size_opt, 0, 4 * sizeof(int));

    for (int char_curr = 0; char_curr < 4; char_curr++) {
        bit_enc_opt[char_curr] = 2;
    }

    if (opt_param->read_stream == (FILE *) NULL) {
        printf("Error opening file %s\n", opt_param->read_stream);
        return OSDNA_IO_ERROR;
    }

    bytesRead = (int) fread(buff, 1, 1, opt_param->read_stream);
    if (bytesRead == 0) {
        printf("Error empty file\n");
        return OSDNA_IO_ERROR;
    }
    total++;
    last_char = buff[0];
    while (bytesRead = (int)fread(buff, sizeof(char), BUFF_SIZE, opt_param->read_stream)) {
        for (int i = 0; i < bytesRead; i++) {
            curr_char = buff[i];
            total++;

            if (POS(curr_char) == -1) {
                printf("Bad file\n");
                //return -1;
                continue;  //skip bad character occurrences
            }
            if (curr_char == last_char)
                cont++;
            else {
                if (cont > MAX_TRIGGER_SIZE) {
                    printf("Too long trigger size \n");
                    return OSDNA_IO_ERROR;
                }
                occ_matr[POS(last_char)][cont]++;
                cont = 1;
                last_char = curr_char;
            }
        }
    }
    occ_matr[POS(last_char)][cont]++;


    for (int i = 2; i < BIT_ENCODE_SIZE; i++) {
        bit_advantage[i][0] = -i;
    }

    for (int bit_encode = 2; bit_encode < BIT_ENCODE_SIZE; bit_encode++)
        for (int i = 1; i < MAX_TRIGGER_SIZE; i++)
            bit_advantage[bit_encode][i] =
                    bit_advantage[bit_encode][i - 1] + ((i % (int) (pow(2, bit_encode) - 1)) > 0) * 2
                    + ((i % (int) (pow(2, bit_encode) - 1)) == 0) * (2 - bit_encode);

    for (int char_curr = 0; char_curr < 4; char_curr++)
        for (int bit_encode = 2; bit_encode < BIT_ENCODE_SIZE; bit_encode++)
            for (int trigger_size = 1; trigger_size < MAX_TRIGGER_SIZE; trigger_size++)
                for (int i = trigger_size; i < MAX_TRIGGER_SIZE; i++)
                    trigger_size_advantage[char_curr][bit_encode][trigger_size] += occ_matr[char_curr][i]
                                                                                   * bit_advantage[bit_encode][i - trigger_size];


    for (int char_curr = 0; char_curr < 4; char_curr++)
        for (int bit_encode = 2; bit_encode < BIT_ENCODE_SIZE; bit_encode++)
            for (int trigger_size = 1; trigger_size < MAX_TRIGGER_SIZE; trigger_size++) {
                if (trigger_size_advantage[char_curr][bit_encode][trigger_size] >
                    trigger_size_advantage[char_curr][bit_enc_opt[char_curr]][trigger_size_opt[char_curr]]) {
                    trigger_size_opt[char_curr] = trigger_size;
                    bit_enc_opt[char_curr] = bit_encode;
                }
            }

    long long adv_total=0;
    for (int char_curr = 0; char_curr < 4; char_curr++) {
        printf("char: %d trigger_size_opt: %d, bit_enc_opt: %d\n", char_curr, trigger_size_opt[char_curr],
               bit_enc_opt[char_curr]);
        switch (char_curr) {
            case 0:
                opt_param->opt_bit_A = bit_enc_opt[0];
                opt_param->opt_trigger_A = trigger_size_opt[0];
                break;
            case 1:
                opt_param->opt_bit_C = bit_enc_opt[1];
                opt_param->opt_trigger_C = trigger_size_opt[1];
                break;
            case 2:
                opt_param->opt_bit_G = bit_enc_opt[2];
                opt_param->opt_trigger_G = trigger_size_opt[2];
                break;
            case 3:
                opt_param->opt_bit_T = bit_enc_opt[3];
                opt_param->opt_trigger_T = trigger_size_opt[3];
        }

        adv_total += trigger_size_advantage[char_curr][bit_enc_opt[char_curr]][trigger_size_opt[char_curr]];
    }
    printf("adv_total %lli\n", adv_total);

    fseek(opt_param->read_stream, SEEK_SET, 0);

    return OSDNA_OK;
}

osdna_status compress_core(OSDNA_ctx *ctx) {
    printf("Initializing compression core\n");

    osdna_status status;
    int bytesRead;
    char file_read_buff[1024]; // for perfomance reasons lets read 1KB at once
    char curr_char = '#';
    char last_char;// Any char excluded AGTC
    osdna_bit_write_handler *bit_write_handle = osdna_bit_init(ctx->write_stream);
    OSDNA_opt_param *opt_param = (OSDNA_opt_param *) malloc(sizeof(OSDNA_opt_param));
    opt_param->read_stream = ctx->read_stream;
    if((status = opt_param_calc(opt_param)) != OSDNA_OK)
       return status;
/*
    opt_param->opt_bit_A = 3;
    opt_param->opt_bit_C = 3;
    opt_param->opt_bit_T = 2;
    opt_param->opt_bit_G = 3;
    opt_param->opt_trigger_A = 2;
    opt_param->opt_trigger_C = 2;
    opt_param->opt_trigger_G = 2;
    opt_param->opt_trigger_T = 2;*/

    int actual_bit_size;
    int actual_trigger_size;
    auto *toWrite = (int8_t *) malloc(sizeof(int8_t)*1024);
    int greatestBitLen = getMaxBitLen(opt_param);
    fread(file_read_buff, sizeof(char), 1, ctx->read_stream);

    curr_char = file_read_buff[0];
    last_char = curr_char;
    int last_occ_len = 1;

    memset(file_read_buff, '\0', sizeof(char)*1024);

    while ((bytesRead = (int) fread(file_read_buff, sizeof(char), 1024, ctx->read_stream)) != 0) {

        for (int i = 0; i < bytesRead; i++) {
            curr_char = file_read_buff[i];
            if (!is_acceptable_char(curr_char))  // are acceptable only AGCT, everything else is skipped
                continue;

            if (last_char == curr_char) {
                last_occ_len++;
            } else {
                toWrite = (int8_t*) realloc(toWrite, (sizeof(int8_t)*last_occ_len*2)+greatestBitLen); // 2 location for every char + 0 in bit_len: WROST CASE
                initActualParam(&actual_trigger_size, &actual_bit_size, last_char, opt_param); // init actual trigger and bit size
                manageWrite(toWrite, last_occ_len, actual_trigger_size, actual_bit_size, last_char, opt_param, bit_write_handle);
                last_occ_len = 1;
            }
            last_char = curr_char;
        }

        memset(file_read_buff, '\0', sizeof(char));
    }

    toWrite = (int8_t*) realloc(toWrite, (sizeof(int8_t)*last_occ_len*2)+greatestBitLen); // 2 location for every char + 0 in bit_len: WROST CASE
    initActualParam(&actual_trigger_size, &actual_bit_size, curr_char, opt_param); // init actual trigger and bit size
    manageWrite(toWrite, last_occ_len, actual_trigger_size, actual_bit_size, last_char, opt_param, bit_write_handle);

    return osdna_bitwriter_finilize(bit_write_handle);
}

osdna_status decompress_core(OSDNA_ctx *ctx){
    printf("Initializing decompression core\n");

    osdna_status status;
    int nToRead = 2;
    int max_r;
    int last_occ_len = 1;
    int actual_trigger_size;
    int actual_bit_size;
    int8_t buff[1024]; // for perfomance reasons lets read 1KB at once
    char toWrite[1024];
    int writePointer = 0;
    char curr_char;
    char last_char = '#';// Any char excluded AGTC
    osdna_bit_read_handler *bit_read_handle = osdna_bit_read_init(ctx->read_stream);
    OSDNA_opt_param *opt = (OSDNA_opt_param *) malloc(sizeof(OSDNA_opt_param));
    readHeader(opt, bit_read_handle);
    status = osdna_bit_read(bit_read_handle, buff, &nToRead);

    curr_char = decodeBit(buff);
    while(status == OSDNA_OK ) {

        if (curr_char != last_char)
            last_occ_len = 1;
        else
            last_occ_len++;
        actual_bit_size = getBitLengthByChar(opt, curr_char);
        actual_trigger_size = getTriggerSizeByChar(opt, curr_char);
        if (actual_trigger_size == 0 || last_occ_len < actual_trigger_size) {
            toWrite[writePointer++] = curr_char;
            if (writePointer % 1024 == 0) {
                fwrite(toWrite, sizeof(char), 1024, ctx->write_stream);
                writePointer = 0;
            }
        } else {
            toWrite[writePointer++] = curr_char;
            if (writePointer % 1024 == 0) {
                fwrite(toWrite, sizeof(char), 1024, ctx->write_stream);
                writePointer = 0;
            }
            nToRead = actual_bit_size;
            osdna_bit_read(bit_read_handle, buff, &nToRead);
            last_occ_len = getDecimalByBinary(buff, actual_bit_size);
            max_r = (int) (pow(2, actual_bit_size)) - 1;
            while (last_occ_len == max_r) {
                for (int i = 0; i < max_r; i++) {
                    toWrite[writePointer++] = curr_char;
                    if (writePointer % 1024 == 0) {
                        fwrite(toWrite, sizeof(char), 1024, ctx->write_stream);
                        writePointer = 0;
                    }
                }
                osdna_bit_read(bit_read_handle, buff, &nToRead);
                last_occ_len = getDecimalByBinary(buff, actual_bit_size);
            }
            for (int i = 0; i < last_occ_len; i++) {
                toWrite[writePointer++] = curr_char;
                if (writePointer % 1024 == 0) {
                    fwrite(toWrite, sizeof(char), 1024, ctx->write_stream);
                    writePointer = 0;
                }
            }
        }

        nToRead = 2; // last char write
        last_char = curr_char;
        status = osdna_bit_read(bit_read_handle, buff, &nToRead);
        curr_char = decodeBit(buff);

    }

    if(writePointer != 0){
        fwrite(toWrite, sizeof(char), (size_t)writePointer, ctx->write_stream);
    }

    fflush(ctx->write_stream);
    fclose(ctx->write_stream);
    return OSDNA_OK;
    }

