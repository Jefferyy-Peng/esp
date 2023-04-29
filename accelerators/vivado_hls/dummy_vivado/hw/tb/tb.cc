// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../inc/input.h"

int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */
	 const unsigned t = 1;
	 const unsigned q = 48;
	 const unsigned p = 12;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;
    const int b = 1;


    in_words_adj = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD);
    out_words_adj = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD);
    in_size = in_words_adj * (1);
    out_size = out_words_adj * (1);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff=(word_t*) malloc(in_size * sizeof(word_t));
    word_t *outbuff=(word_t*) malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold= (word_t*) malloc(out_size * sizeof(word_t));
    dma_info_t load;
    dma_info_t store;

    // Prepare input data
    for(unsigned i = 0; i < b; i++){
        int j;
        int x;
        int y;
        for(j = 0; j < (p+q)*10; j++){
            for(unsigned k = 0; k < 32; k++){
                inbuff[i * in_words_adj + j][k] = input_idx[j][k];
            }

                #ifndef __SYNTHESIS__
                    std::cout << "inbuff[" << i * in_words_adj + j << "]=" << inbuff[i * in_words_adj + j] << std::endl;
                #endif
        }
        for(x = 0; x < 41; x++){
            for(unsigned k = 0; k < 32; k++){
                inbuff[i * in_words_adj + j + x][k] = input_idx[j + i*41 + x][k];
            }
                #ifndef __SYNTHESIS__
                    std::cout << "inbuff[" << i * in_words_adj + j + x << "]=" << inbuff[i * in_words_adj + j + x] << std::endl;
                #endif
        }
        for(y = 0; y < (p+q)*t; y++){
            inbuff[i * in_words_adj + j + x + y] = input[y*462];
            #ifndef __SYNTHESIS__
                std::cout << "inbuff[" << i * in_words_adj + j + x + y << "]=" << inbuff[i * in_words_adj + j + x + y] << std::endl;
            #endif
        }
    }

    for(unsigned i = 0; i < dma_in_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	    mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];

    // Set golden output
    for(unsigned i = 0; i < b; i++)
        for(unsigned j = 0; j < (p+q)*10 + 41; j++){
            outbuff_gold[i * out_words_adj + j] = input_idx[j];
            #ifndef __SYNTHESIS__
                std::cout << "out_gold[" << i * out_words_adj + j << "]=" << outbuff_gold[i * out_words_adj + j] << std::endl;
            #endif
            }
        

    for(unsigned i = 0; i < b; i++)
        for(unsigned j = 0; j < (p+q); j++){
            for(int y = 0; y < t; y++){
            outbuff_gold[i * out_words_adj + (p+q)*10 + 41 + j * t + y] = input[j*462 + y];
            #ifndef __SYNTHESIS__
                std::cout << "out_gold[" << i * out_words_adj + j << "]=" << outbuff_gold[i * out_words_adj + j] << std::endl;
            #endif
            }
        }


    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
	 	 t,
	 	 q,
	 	 p,
        load, store);

    // Validate
    uint32_t out_offset = dma_in_size;
    for(unsigned i = 0; i < dma_out_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	    outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < (q+p)*t; j++)
	    if (outbuff[i * out_words_adj + j] != outbuff_gold[i * out_words_adj + j])
		errors++;

    if (errors)
	std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    else
	std::cout << "Test PASSED." << std::endl;

    // Free memory

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
