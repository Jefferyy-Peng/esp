// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "../inc/input.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */
	 const unsigned t = 10;
	 const unsigned q = 48;
	 const unsigned p = 12;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;
    const int b = NUM_BATCH;


    in_words_adj = round_up((p+q)*10+41*b+(p+q)*t*b, VALUES_PER_WORD);
    out_words_adj = round_up(b, VALUES_PER_WORD);
    in_size = in_words_adj * (1);
    // std::cout << in_size << std::endl;
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
    for(int u = 0; u < b; u++){
        int C_buff_idx = (p+q)*10 + u * round_up((41 + (p+q)*t), VALUES_PER_WORD);
        // std::cout << "buff = " << C_buff_idx << std::endl;
    // load index
    if(u == 0){
        #if IS_TYPE_FLOAT
        for(unsigned i = 0; i < 1; i++)
            for(unsigned j = 0; j < (p+q)*10 + 41; j++)
                inbuff[i * in_words_adj + j] = input_idx_word[j];
        #else
        for(unsigned i = 0; i < 1; i++)
            for(unsigned j = 0; j < (p+q)*10 + 41; j++){
                for(unsigned k = 0; k < 32; k++){
                    // std::cout << "k=" << k << std::endl;
                    inbuff[i * in_words_adj + j][k] = input_idx[j][k];
            // std::cout << "1_inbuff[" << i * in_words_adj + j << "][" << k << "] = " << inbuff[i * in_words_adj + j][k] << std::endl;
                }
            // std::cout << "1_inbuff[" << i * in_words_adj + j << "] = " << inbuff[i * in_words_adj + j] << std::endl;
            }
        #endif
    }
    else{
        #if IS_TYPE_FLOAT
        for(unsigned i = 0; i < 1; i++)
            for(unsigned j = 0; j < 41; j++)
                inbuff[i * in_words_adj + (p+q)*10 + (41 + (p+q)*t) * u + j] = input_idx_word[(p+q)*10 + 41 * u + j];
        #else
        for(unsigned i = 0; i < 1; i++)
            for(unsigned j = 0; j < 41; j++){
                for(unsigned k = 0; k < 32; k++){
                    // std::cout << "k=" << k << std::endl;
                    inbuff[i * in_words_adj + C_buff_idx + j][k] = input_idx[(p+q)*10 + 41 * u + j][k];
                    // std::cout << "1_inbuff_1[" << i * in_words_adj + C_buff_idx + j << "][" << k << "] = " << inbuff[i * in_words_adj + buff_idx + j][k] << std::endl;
                }
            // std::cout << "1_inbuff_1[" << i * in_words_adj + C_buff_idx + j << "] = " << inbuff[i * in_words_adj + C_buff_idx + j] << std::endl;
            // std::cout << "1_input_1[" << i * in_words_adj + (p+q)*10 + (41 + (p+q)*t) * u + j << "] = " << (p+q)*10 + 41 * u + j << std::endl;
            }
        #endif
    }

    // // load Ca index
    // #if IS_TYPE_FLOAT
    // for(unsigned i = 0; i < 1; i++)
    //     for(unsigned j = 0; j < 41; j++)
    //         inbuff[i * in_words_adj + (p+q)*10 + j] = input_idx_word[(p+q)*10 + b * 41 + j];
    // #else
    // for(unsigned i = 0; i < 1; i++)
    //     for(unsigned j = 0; j < 41; j++){
    //         for(unsigned k = 0; k < 32; k++){
    //             // std::cout << "k=" << k << std::endl;
    //             inbuff[i * in_words_adj + (p+q)*10 + b * 41 + j][k] = input_idx[(p+q)*10 + b * 41 + j][k];
    //         }
    //     }
    

    // load data
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < (p+q)*t; j++){
            word_t tmp = input[(p+q)*t*u + j] * 1000;
            int a = j/t;
            // inbuff[i * in_words_adj + (p+q)*10 + 41 * u + 41 + (p+q)*t*u + j] = tmp; //input[(p+q)*t*u + j];
            inbuff[i * in_words_adj + C_buff_idx + 41 + j] = tmp; 
            // std::cout << "D_inbuff[" << a << "][" << i * in_words_adj + (p+q)*10 + 41 * (u + 1) + (p+q)*t*u + j << "]=" << inbuff[i * in_words_adj + (p+q)*10 + 41 * (u + 1) + (p+q)*t*u + j] << std::endl;
            // std::cout << "input[" << i << "][" << j << "]=" << input[j] << std::endl;
        }
    }
    for(unsigned i = 0; i < dma_in_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++){
	    mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];
        // std::cout << "mem[" << i * VALUES_PER_WORD + k << "] = " << mem[i].word[k] << std::endl;
        // std::cout << "mem[" << i << "][" << k << "][" << i * VALUES_PER_WORD + k << "] = " << mem[i].word[k] << std::endl;
        // std::cout << i * VALUES_PER_WORD + k << std::endl;
    }

    // Set golden output
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < b; j++)
            outbuff_gold[i * out_words_adj + j] = (word_t) gold_out[j];

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
	for(unsigned k = 0; k < VALUES_PER_WORD; k++){
	    outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];
        // std::cout << "outbuff" << i * VALUES_PER_WORD + k << "=" << outbuff[i * VALUES_PER_WORD + k] << std::endl;
        // std::cout << "mem" << out_offset + i << "=" << mem[out_offset + i].word[k] << std::endl;
    }

    int errors = 0;
    word_t error = 0.01;
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < b; j++){
            // std::cout << "value = " << outbuff[i * out_words_adj + j]/1000 << ", gold_value = " << outbuff_gold[i * out_words_adj + j] << std::endl;

	        if ((outbuff[i * out_words_adj + j]/1000 < (outbuff_gold[i * out_words_adj + j] * (1 - error))) || (outbuff[i * out_words_adj + j]/1000 > (outbuff_gold[i * out_words_adj + j] * (1 + error)))){
                // std::cout << "aaa = " << outbuff[i * out_words_adj + j] << "," << outbuff_gold[i * out_words_adj + j] - error << "," << outbuff_gold[i * out_words_adj + j] + error << std::endl;
		        errors++;
            }
        }

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
