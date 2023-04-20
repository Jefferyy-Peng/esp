// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned t,
	 const unsigned q,
	 const unsigned p,
	 const unsigned c,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(q * 10 + p * 10 + 41 + c * ( q + p ) * t, VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;
    std::cout << "Test1" << std::endl;
    for (unsigned i = 0; i < dma_length; i++) {
    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
    	}
    }
    std::cout << "Test2" << std::endl;
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned t,
	 const unsigned q,
	 const unsigned p,
	 const unsigned c,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(c, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(q * 10 + p * 10 + 41 + c * ( q + p ) * t, VALUES_PER_WORD) * 1;
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
	 const unsigned t,
	 const unsigned q,
	 const unsigned p,
	 const unsigned c,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
    const unsigned length = round_up(q * 10 + p * 10 + 41 + c * ( q + p ) * t, VALUES_PER_WORD) / 1;

    const unsigned TrIdx_size = q * 10;
    const unsigned TeIdx_size = p * 10;
    const unsigned CaIdx_size = 41;
    const unsigned comp_workers = 1;

    const unsigned D_index = TrIdx_size + TeIdx_size + CaIdx_size;

    word_t TrIdx[q][10];
    word_t TeIdx[p][10];
    word_t CaIdx[41];

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < q; j++){
            TrIdx[i][j] = _inbuff[i * q + j];
            std::cout << "TrIdx[" << i << "][" << j << "] = " << TrIdx[i][j] << std::endl;
        }
    }

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < p; j++){
            TeIdx[i][j] = _inbuff[TrIdx_size + i * p + j];
            std::cout << "TeIdx[" << i << "][" << j << "] = " << TeIdx[i][j] << std::endl;
        }
    }

    for(int i = 0; i < 41; i++){
        CaIdx[i] = _inbuff[TrIdx_size + TeIdx_size + i];
        std::cout << "CaIdx[" << i << "] = " << CaIdx[i] << std::endl;
    }

    for(int k = 0; k < c; k++){
        
        const unsigned D_size = (q+p) * t;

        word_t D[p + q][t];
        for(int i = 0; i < p + q; i++){
            for(int j = 0; j < t; j++){
                D[i][j] = _inbuff[D_index + k * ( q + p ) * t + i * ( q + p ) + j];
            }
        }
        word_t p2p_Ca[41];
        word_t high_Ca;
        word_t low_Ca;
        word_t p2p_Tr[48];
        word_t high_Tr;
        word_t low_Tr;
        word_t gl;
        word_t mean[t];
        for(int i = 0; i < 41; i++){
            high_Ca = D[CaIdx[i]][0];
            low_Ca = D[CaIdx[i]][0];
            for(int j = 0; j < t; j++){
                if(D[CaIdx[i]][j] > high_Ca)
                    high_Ca = D[CaIdx[i]][j];
                if(D[CaIdx[i]][j] < low_Ca)
                    low_Ca = D[CaIdx[i]][j];
            }
            p2p_Ca[i] = high_Ca - low_Ca;
            for(int u = 0; u < 10; u++){
                for(int l = 0; l < t; l++)
                    mean[l] = 0;
                gl = 0;
                for(int v = 0; v < 48; v++){
                    high_Tr = D[TrIdx[v][u]][0];
                    low_Tr = D[TrIdx[v][u]][0];
                    for(int j = 0; j < t; j++){
                        if(D[TrIdx[v][u]][j] > high_Tr)
                            high_Tr = D[TrIdx[v][u]][j];
                        if(D[TrIdx[v][u]][j] < low_Tr)
                            low_Tr = D[TrIdx[v][u]][j];
                    }
                    p2p_Tr[v] = high_Tr - low_Tr;
                    if(p2p_Tr[v] < p2p_Ca[i]){
                        for(int o = 0; o < t; o++){
                            mean[o] += D[TrIdx[v][u]][o];
                        }
                        gl += 1;
                    }
                }
                for(int o = 0; o < t; o++){
                    mean[o] /= gl;
                }
                word_t pos[t];
                word_t pos_1 = 0;
                word_t pos_2 = 0;
                word_t mid = 0;
                word_t median[q];
                for(int i = 0; i < t; i++){
                    pos[i] = 0;
                }
                for(int i = 0; i < q ; i++){
                    mid = 0;
                    for(int j = 0; j < t ; j++){
                        for(int v = 0; v < t; v++){
                            if((v!= j) && (D[i][v] < D[i][j])){
                                pos[j] += 1;
                            }
                        }
                        if(pos[j] == 231){
                            mid += pos[j];
                            pos_1 = 1;
                        }
                        else if(pos[j] == 232){
                            mid += pos[j];
                            pos_2 = 1;
                        }
                        if(pos_1 || pos_2){
                            mid /= 2;
                            break;
                        }
                    }
                    median[i] = mid;
                }
            }
        }
    }
    // for (int i = 0; i < length; i++)
    //     _outbuff[i] = _inbuff[i];
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_t,
	 const unsigned conf_info_q,
	 const unsigned conf_info_p,
	 const unsigned conf_info_c,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{
    std::cout << "Test2" << std::endl;
    /* <<--local-params-->> */
	 const unsigned t = conf_info_t;
	 const unsigned q = conf_info_q;
	 const unsigned p = conf_info_p;
	 const unsigned c = conf_info_c;
    
    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int a = 0; a < 1; a++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 t,
	 	 q,
	 	 p,
	 	 c,
                 load_ctrl, a, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 t,
	 	 q,
	 	 p,
	 	 c,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 t,
	 	 q,
	 	 p,
	 	 c,
                  store_ctrl, a, b);
        }
    }
}
