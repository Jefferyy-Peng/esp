// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t X_Train[Train_epochs][features],word_t Y_Train[Train_epochs][1], 
        word_t X_Test[Test_epochs][features], word_t &Tol, dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned Train_epochs,
	 const unsigned features,
	 const unsigned classes,
	 const unsigned windows,
	 const unsigned Test_epochs,
	 const unsigned Tol,//can I delete this?
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data://implement simply, use original one, Mid pre: algo, typo, milestones

    const unsigned length = round_up(windows * ((Test_epochs + Train_epochs) * (features + 1)) + 1, VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

	const unsigned index = 0; 
    const unsigned length_X_Train = Train_epochs * features;
    const unsigned length_Y_Train = Train_epochs;
    const unsigned length_X_Test = Test_epochs * features;
    const unsigned length_T = 1;
    // word_t tmp[2];
    // word_t dummy;
    // unsigned row = 0;

    unsigned Y_Train_index = length_X_Train;
    unsigned X_Test_index = Y_Train_index + length_Y_Train;
    unsigned T_index = X_Test_index + length_T;

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {

    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
        // tmp[j] = in1[dma_index + i].word[j]; //
        // unsigned col_index = 0;
        // unsigned total_index = (i * VALUES_PER_WORD) + j;
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];//can we use vector multiplication?
    	}
        // if (total_index < X_Train_index){
        //     col_index = total_index - row*features;
        //     X_Train[row][col_index] = tmp[j];
        //     if(col_index == features-1)
        //         row++;
        //     if(row == Train_epochs)
        //         row = 0;
        // }
        // else if(total_index < Y_Train_index){
        //     col_index = total_index - X_Train_index - row*features;//why total_index? The row is for one matrix?
        //     Y_Train[row][col_index] = tmp[j];
        //     if(col_index == features-1)
        //         row++;
        //     if(row == Train_epochs)
        //         row = 0;
        // }
        // else if(total_index < Y_Train_index){
        //     col_index = total_index - X_Train_index - row*features;//why total_index? The row is for one matrix?
        //     X_Train[row][col_index] = tmp[j];
        //     if(col_index == features-1)
        //         row++;
        //     if(row == Test_epochs)
        //         row = 0;
        // }
        // else if (total_index == T_index){
        //     Tol = tmp[j];
        //     //printf("index read by P is: %d, dma_length: %d \n", total_index, i+1);
        // }
        // else{
        //     dummy = tmp[j];//why dummy
        //     printf("index read by tmp is: %d, dma_length: %d \n", total_index, i+1);
        // }
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned Train_epochs,
	 const unsigned features,
	 const unsigned classes,
	 const unsigned windows,
	 const unsigned Test_epochs,
	 const unsigned Tol,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(windows * (Test_epochs + Train_epochs) + windows * features, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(windows * ((Test_epochs + Train_epochs) * (features + 1)) + 1, VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    store_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
	}
    }
}


void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
	 const unsigned Train_epochs,
	 const unsigned features,
	 const unsigned classes,
	 const unsigned windows,
	 const unsigned Test_epochs,
	 const unsigned Tol,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{//take data in plm and put into smaller areas, matrices
    //assume the class sorted sequencing is taken care by software
    const unsigned index = 0; 
    const unsigned length_X_Train = Train_epochs * features;
    const unsigned length_Y_Train = Train_epochs;
    const unsigned length_X_Test = Test_epochs * features;
    const unsigned length_T = 1;
    // word_t tmp[2];
    // word_t dummy;
    // unsigned row = 0;

    unsigned Y_Train_index = length_X_Train;
    unsigned X_Test_index = Y_Train_index + length_Y_Train;
    unsigned T_index = X_Test_index + length_T;

    word_t X_Train[Train_epochs][features];
    word_t Y_Train[Train_epochs][1];
    word_t X_Test[Test_epochs][features];
    word_t &Tol;
    unsigned row = 0;
    unsigned col_index = 0;
    for(int i = 0; i<Train_epochs; i++)
    for(int j = 0; j<features; j++)
        X_Train[i][j] = _inbuff[i * features + j];

    for(int i = 0; i<Train_epochs; i++)
        Y_Train[i][j] = _inbuff[X_Train_index + i];

    for(int i = 0; i<Train_epochs; i++)
    for(int j = 0; j<features; j++)
        X_Test[i][j] = _inbuff[Y_Train_index + i * features + j];

    Tol = _inbuff[X_Test_index];

    // TODO implement compute functionality
    const unsigned in_length = round_up(windows * ((Test_epochs + Train_epochs) * (features + 1)) + 1, VALUES_PER_WORD) / 1;
    const unsigned out_length = round_up(windows * ((Test_epochs + Train_epochs) * (features + 1)) + 1, VALUES_PER_WORD) / 1;

}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_Train_epochs,
	 const unsigned conf_info_features,
	 const unsigned conf_info_classes,
	 const unsigned conf_info_windows,
	 const unsigned conf_info_Test_epochs,
	 const unsigned conf_info_Tol,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned Train_epochs = conf_info_Train_epochs;
	 const unsigned features = conf_info_features;
	 const unsigned classes = conf_info_classes;
	 const unsigned windows = conf_info_windows;
	 const unsigned Test_epochs = conf_info_Test_epochs;
	 const unsigned Tol = conf_info_Tol;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 Train_epochs,
	 	 features,
	 	 classes,
	 	 windows,
	 	 Test_epochs,
	 	 Tol,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 Train_epochs,
	 	 features,
	 	 classes,
	 	 windows,
	 	 Test_epochs,
	 	 Tol,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 Train_epochs,
	 	 features,
	 	 classes,
	 	 windows,
	 	 Test_epochs,
	 	 Tol,
                  store_ctrl, c, b);
        }
    }
}
