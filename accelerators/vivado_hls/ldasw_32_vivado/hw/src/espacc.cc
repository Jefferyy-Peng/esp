// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned Train_epochs,
	 const unsigned features,
	 const unsigned classes,
	 const unsigned Test_epochs,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(Train_epochs*features+Train_epochs+Test_epochs*features, VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
    	}
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned Train_epochs,
	 const unsigned features,
	 const unsigned classes,
	 const unsigned Test_epochs,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(Test_epochs+Train_epochs+features, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(Train_epochs*features+Train_epochs+Test_epochs*features, VALUES_PER_WORD) * 1;
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
	 const unsigned Test_epochs,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    const unsigned XTrain_size = Train_epochs*features;
    const unsigned YTrain_size = Train_epochs;
    const unsigned XTest_size = Test_epochs*features;

    word_t XTrain[Train_epochs][features];
    word_t YTrain[Train_epochs];
    word_t XTest[Test_epochs][features];
    word_t XTrain_class[classes][Train_epochs/2][features];

    for(int i = 0; i < Train_epochs; i++)
        for(int j = 0; j < features; j++){
            XTrain[i][j] = _inbuff[i*features+j];
            //std::std::cout << XTrain[i][j] << std::std::endl;
        }

    for(int j = 0; j < Train_epochs; j++){
        YTrain[j] = _inbuff[XTrain_size+j];
    }

    for(int i = 0; i < Test_epochs; i++)
        for(int j = 0; j < features; j++){
            XTest[i][j] = _inbuff[XTrain_size+YTrain_size+i*features+j];
        }
    //split by class
    int class_idx1 = 0;
    int class_idx2 = 0;
    for(int i = 0; i < Train_epochs; i++){
        if(YTrain[i] == 0){
            for(int j = 0; j < features; j++){
                XTrain_class[0][class_idx1][j] = XTrain[i][j];
                //std::cout << "class: 0," << XTrain[i][j] << ",i=" << i << ",class_idx=" << class_idx1 << "," << j << std::endl; 
            }
            class_idx1 += 1;
        }
        else{
            for(int j = 0; j < features; j++){
                XTrain_class[1][class_idx2][j] = XTrain[i][j];
                //std::cout << "class: 1," << XTrain[i][j] << ",i=" << i << ",class_idx=" << class_idx2 << "," << j << std::endl; 
            }
            class_idx2 += 1;
        }
    }

    word_t M[classes][features];
    // Mean
    for(int i = 0; i < features ; i++){
        M[0][i] = 0;
        M[1][i] = 0;
        for(int j = 0; j < class_idx1 ; j++){
            M[0][i] += XTrain_class[0][j][i];
        }
        M[0][i] /= class_idx1;
        // std::cout << "Means0: " << M[0][i] << std::endl;
        for(int j = 0; j < class_idx2 ; j++){
            M[1][i] += XTrain_class[1][j][i];
        }
        M[1][i] /= class_idx2;
        // std::cout << "Means1: " << M[1][i] << std::endl;
    }

    word_t Xc[TEpochs][N_FEATURES];
    for(int i = 0; i < class_idx1 ; i++)
        for(int j = 0; j < features; j++){
            Xc[i][j] = XTrain_class[0][i][j] - M[0][j];
            //std::cout << "class: 0," << Xc[i][j] << ",i=" << i << "," << j << std::endl; 
        }
    for(int i = 0; i < class_idx2 ; i++)
        for(int j = 0; j < features; j++){
            Xc[class_idx1 + i][j] = XTrain_class[1][i][j] - M[1][j];
            //std::cout << "class: 1," << Xc[class_idx1 + i][j] << ",i=" << i << "," << j << std::endl; 
        }
    
    //standard deviation
    word_t std[features];
    for(int i = 0; i < features ; i++){
        std[i] = 0;
        for(int j = 0; j < class_idx1 ; j++){
            std[i] += ((Xc[j][i] - M[0][i]) * (Xc[j][i] - M[0][i]) + (Xc[j+class_idx1][i] - M[1][i]) * (Xc[j+class_idx1][i] - M[1][i]));
        }
        std[i] = sqrtf(std[i]/Train_epochs);
    }

    //divide
    word_t mat_divide[N_FEATURES][N_FEATURES];
    for(int i = 0; i < features; i++)
        for(int j = 0 ; j < Train_epochs; j++){
            mat_divide[j][i] = 0.16222142113076254 * Xc[j][i] / std[i];
            std::cout << j << "," << i << ":" << mat_divide[j][i] << std::endl;
        }
    for(int i = 0; i < features ; i++)
        for(int j = Train_epochs; j < features; j++){
            mat_divide[j][i] = 0;
            std::cout << j << "," << i << ":" << mat_divide[j][i] << std::endl;
        }
    //hls::print_matrix<TEpochs, N_FEATURES, word_t, hls::NoTranspose>(mat_divide, "   ");
    word_t S[N_FEATURES][N_FEATURES];
    word_t U[N_FEATURES][N_FEATURES];
    word_t V[N_FEATURES][N_FEATURES];

    hls::svd_top<N_FEATURES/*A_ROWS*/, N_FEATURES/*A_COLS*/, TRAITS_SVD, word_t, word_t>(mat_divide, S, U, V);
    std::cout << "Matrix S" << std::endl;
    for(int i = 0; i < features ; i++)
        for(int j = 0; j < features; j++){
            std::cout << j << "," << i << ":" << S[j][i] << std::endl;
        }

    std::cout << "Matrix U" << std::endl;
    for(int i = 0; i < features ; i++)
        for(int j = 0; j < features; j++){
            std::cout << j << "," << i << ":" << U[j][i] << std::endl;
        }

    std::cout << "Matrix V" << std::endl;
    for(int i = 0; i < features ; i++)
        for(int j = 0; j < features; j++){
            std::cout << j << "," << i << ":" << V[j][i] << std::endl;
        }

    // TODO implement compute functionality
    const unsigned length = round_up(Train_epochs*features+Train_epochs+Test_epochs*features, VALUES_PER_WORD) / 1;

    for (int i = 0; i < Train_epochs + Test_epochs + features; i++)
        _outbuff[i] = _inbuff[i];
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_Train_epochs,
	 const unsigned conf_info_features,
	 const unsigned conf_info_classes,
	 const unsigned conf_info_Test_epochs,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned Train_epochs = conf_info_Train_epochs;
	 const unsigned features = conf_info_features;
	 const unsigned classes = conf_info_classes;
	 const unsigned Test_epochs = conf_info_Test_epochs;

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
	 	 Test_epochs,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 Train_epochs,
	 	 features,
	 	 classes,
	 	 Test_epochs,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 Train_epochs,
	 	 features,
	 	 classes,
	 	 Test_epochs,
                  store_ctrl, c, b);
        }
    }
}
