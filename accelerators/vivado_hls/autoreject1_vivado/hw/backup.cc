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
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD) / 1;
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
	 const unsigned t,
	 const unsigned q,
	 const unsigned p,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(1, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD) * 1;
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
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
    // const unsigned length = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD) / 1;
    const unsigned length = round_up(2, VALUES_PER_WORD) / 1;

    const unsigned TrIdx_size = q * 10;
    const unsigned TeIdx_size = p * 10;
    const unsigned CaIdx_size = 41;
    const unsigned comp_workers = 1;

    const unsigned D_index = TrIdx_size + TeIdx_size + CaIdx_size;

    ap_uint<6> TrIdx[q][10];
    ap_uint<6> TeIdx[p][10];
    ap_uint<6> CaIdx[41];

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

    const unsigned D_size = (q+p) * t;
    word_t D[p + q][t];

    for(int i = 0; i < p + q; i++){
        for(int j = 0; j < t; j++){
            D[i][j] = _inbuff[D_index + i * t + j];
            // std::cout << "D[" << i << "][" << j << "] = " << D[i][j] << std::endl;
        }
    }
    
    word_t p2p[p + q];
    word_t high;
    word_t low;
    ap_uint<6> gl;
    word_t mean[t];
    word_t E[41][10];
    word_t E_mean[41];
    word_t low_E;
    ap_uint<6> low_EIdx;
    word_t T;
    ap_uint<9> pos;
    ap_uint<6> pos_1 = 0;
    ap_uint<6> pos_2 = 0;
    word_t mid = 0;
    word_t median[t];

    for(int i = 0; i < p + q; i++){
        high = D[i][0];
        low = D[i][0];
        for(int j = 0; j < t; j++){
            if(D[i][j] > high){
                high = D[i][j];
            }
            else if(D[i][j] < low){
                low = D[i][j];
            }
        }
        p2p[i] = high - low;
        std::cout << "p2p[" << i << "] =" << p2p[i] << std::endl; 
    }
    
    for(int u = 0; u< 10 ; u++){
        for(int i = 0; i < p ; i++){
            mid = 0;
            pos_1 = 0;
            pos_2 = 0;
            pos = 0;
            for(int j = 0; j < t ; j++){
                for(int v = 0; v < t; v++){
                    if((v!= j) && (D[TrIdx[u][i]][v] < D[TrIdx[u][i]][j])){
                        pos += 1;
                    }
                }
                if(pos == 231){
                    mid += D[TrIdx[u][i]][j];
                    pos_1 = 1;
                }
                else if(pos == 232){
                    mid += D[TrIdx[u][i]][j];
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
    
    for(int i = 0; i < 41; i++){
        for(int u = 0; u < 10; u++){
            // for(int l = 0; l < t; l++)
            //     mean[l] = 0;
            gl = 0;
            for(int v = 0; v < 48; v++){
                std::cout << "[u,v] = " << u << "," << v << std::endl;
                    std::cout << "TrIdx=" << TrIdx[u][v] << ", CaIdx = " << CaIdx[i] << std::endl;
                    std::cout << "p2pTr=" << p2p[TrIdx[u][v]] << ", p2pCa = " << p2p[CaIdx[i]] << std::endl;
                if(p2p[TrIdx[u][v]] < p2p[CaIdx[i]]){
                    for(int o = 0; o < t; o++){
                        if(gl == 0)
                            mean[o] = D[TrIdx[u][v]][o];
                        else
                            mean[o] += D[TrIdx[u][v]][o];
                    }
                    gl += 1;
                }
            }
                
            for(int o = 0; o < t; o++){
                std::cout << "mean[" << o << "] = " << mean[o] << ", gl = " << gl << ", median = " << median[o] << std::endl;
                if(gl == 0){
                    E[i][u] = 0.1;
                }
                else{
                    mean[o] = (mean[o]/gl - median[o]) * (mean[o]/gl - median[o]);
                    E[i][u] += mean[o];
                }
            }
            E[i][u] /= t;
            E[i][u] = hls::sqrt(E[i][u]);
            if(u == 0)
                E_mean[i] = E[i][0];
            else
                E_mean[i] += E[i][u];
        }
        E_mean[i] /= 10;
        if(i == 0){
            low_E = E_mean[0];
            low_EIdx = 0;
        }
        else{
            if(E_mean[i] < low_E){
                low_E = E_mean[i];
                low_EIdx = i;
            }
        }
    }   
    T = p2p[CaIdx[low_EIdx]]; 
    std::cout << "T = " << T << std::endl;
    for (int i = 0; i < length; i++)
        _outbuff[i] = _inbuff[i];
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_t,
	 const unsigned conf_info_q,
	 const unsigned conf_info_p,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{
    /* <<--local-params-->> */
	 const unsigned t = conf_info_t;
	 const unsigned q = conf_info_q;
	 const unsigned p = conf_info_p;

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
	 	 t,
	 	 q,
	 	 p,
                 load_ctrl, c, b);

            compute(_inbuff,
                    /* <<--args-->> */
	 	 t,
	 	 q,
	 	 p,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 t,
	 	 q,
	 	 p,
                  store_ctrl, c, b);
        }
    }
}
