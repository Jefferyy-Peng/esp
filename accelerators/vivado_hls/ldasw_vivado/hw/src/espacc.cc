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