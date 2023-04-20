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
    const unsigned length = round_up(1, VALUES_PER_WORD) / 1;

    const unsigned TrIdx_size = q * 10;
    const unsigned TeIdx_size = p * 10;
    const unsigned CaIdx_size = 41;
    // const unsigned comp_workers = 1;

    const unsigned D_index = TrIdx_size + TeIdx_size + CaIdx_size;

    ap_uint<32> TrIdx[10][Q_MAX];
    ap_uint<32> TeIdx[10][P_MAX];
    ap_uint<32> CaIdx[41];

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < Q_MAX; j++){
            for(int k = 0; k < sizeof(word_t)*8; k++){
                // std::cout << "Trk=" << k << std::endl;
                TrIdx[i][j][k] = _inbuff[i * q + j][k];
            }
            // TrIdx[i][j] = _inbuff[i * q + j];
            std::cout << "TrIdx[" << i << "][" << j << "] = " << TrIdx[i][j] << std::endl;
        }
    }

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < P_MAX; j++){
            for(int k = 0; k < sizeof(word_t)*8; k++){
                // std::cout << "Trk=" << k << std::endl;
                TeIdx[i][j][k] = _inbuff[TrIdx_size + i * p + j][k];
            }
            // TeIdx[i][j] = _inbuff[TrIdx_size + i * p + j];
            std::cout << "TeIdx[" << i << "][" << j << "] = " << TeIdx[i][j] << std::endl;
        }
    }

    for(int i = 0; i < 41; i++){
        // CaIdx[i] = _inbuff[TrIdx_size + TeIdx_size + i];
        for(int k = 0; k < sizeof(word_t)*8; k++){
                // std::cout << "Trk=" << k << std::endl;
                CaIdx[i][k] = _inbuff[TrIdx_size + TeIdx_size + i][k];
            }
        std::cout << "CaIdx[" << i << "] = " << CaIdx[i] << std::endl;
    }

    const unsigned D_size = (q+p) * t;
    word_t D[Q_MAX + P_MAX][T_MAX];

    for(int i = 0; i < Q_MAX + P_MAX; i++){
        for(int j = 0; j < T_MAX; j++){
            D[i][j] = _inbuff[D_index + i * t + j];
            std::cout << "D[" << i << "][" << j << "] = " << D[i][j] << std::endl;
        }
    }
    
    
    word_t p2p[Q_MAX + P_MAX];
    word_t high;
    word_t low;
    word_t gl;
    word_t mean[T_MAX];
    word_t E[41][10];
    word_t E_mean[41];
    word_t low_E;
    ap_uint<6> low_EIdx;
    word_t T;
    ap_uint<9> pos;
    word_t pos_1 = 0;
    word_t pos_2 = 0;
    word_t median[10][T_MAX];

    for(int i = 0; i < Q_MAX + P_MAX; i++){
        high = D[i][0];
        low = D[i][0];
        for(int j = 0; j < T_MAX; j++){
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
        for(int i = 0; i < T_MAX ; i++){
            pos_1 = 0;
            pos_2 = 0;
            median[u][i] = 0;
            for(int j = 0; j < P_MAX ; j++){
                pos = 0;
                for(int v = 0; v < P_MAX; v++){
                    if(D[TeIdx[u][v]][i] < D[TeIdx[u][j]][i]){
                    // if(D[v][i] < D[v][i]){
                        pos += 1;
                    }
                }
                // std::cout << "D[" << u << "][" << i << "][" << j << "] = " << D[TeIdx[u][j]][i] << "," << TeIdx[u][j] << std::endl;
                // std::cout << "pos[" << u << "][" << i << "][" << j << "] = " << pos << std::endl;
                if(pos == 5){
                    // median[u][i] += D[1][i];
                    median[u][i] += D[TeIdx[u][j]][i];
                    pos_1 = 1;
                }
                else if(pos == 6){
                    // median[u][i] += D[1][i];
                    median[u][i] += D[TeIdx[u][j]][i];
                    pos_2 = 1;
                }
                if(pos_1 && pos_2){
                    median[u][i] /= 2;
                    // std::cout << "Median[" << u << "][" << i << "] = " << median[u][i] << std::endl;
                    break;
                }
            }
        }
    }
    
    for(int u = 0; u < 10; u++){
        for(int i = 0; i < 41; i++){
            // for(int l = 0; l < t; l++)
            //     mean[l] = 0;
            gl = 0;
            for(int v = 0; v < Q_MAX; v++){
                // std::cout << "[u,i,v] = " << u << "," << i << "," << v << std::endl;
                // std::cout << "TrIdx=" << TrIdx[u][v] << ", CaIdx = " << CaIdx[i] << std::endl;
                // std::cout << "p2pTr=" << p2p[TrIdx[u][v]] << ", p2pCa = " << p2p[CaIdx[i]] << std::endl;
                if(p2p[TrIdx[u][v]] < p2p[CaIdx[i]]){
                // if(p2p[v] < p2p[u]){
                    // std::cout << "#gl=" << v << std::endl;
                    for(int o = 0; o < T_MAX; o++){
                        if(gl == 0)
                            mean[o] = D[TrIdx[u][v]][o];
                            // mean[o] = D[1][o];
                        else
                            mean[o] += D[TrIdx[u][v]][o];
                            // mean[o] += D[1][o];
                    }
                    gl += 1;
                }
            }
                
            for(int o = 0; o < T_MAX; o++){
                // std::cout << "mean[" << o << "] = " << mean[o]/gl << ", gl = " << gl << ", median = " << median[u][o] << std::endl;
                if(gl == 0){
                    E[i][u] = 0.1;
                }
                else{
                    mean[o] = (mean[o]/gl - median[u][o]) * (mean[o]/gl - median[u][o]);
                    E[i][u] += mean[o];
                }
            }
            E[i][u] = hls::sqrt(E[i][u]);
            // std::cout << "E[" << i << "][" << u << "] = " << E[i][u] << std::endl;
        }
    }
    // E = hls::sqrt(E);
    for(int i = 0; i < 41; i++){
        for(int u = 0; u < 10; u++){
            if(u == 0)
                E_mean[i] = E[i][0];
            else
                E_mean[i] += E[i][u];
        }
        E_mean[i] /= 10;
        // std::cout << "E_mean[" << i << "] = " << E_mean[i] << std::endl;
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
    // std::cout << "low_EIdx = " << low_EIdx << std::endl;
    T = p2p[CaIdx[low_EIdx]]; 
    // T = p2p[1]; 
    // std::cout << length << std::endl;
    // std::cout << "T = " << T << std::endl;
    #ifndef __SYNTHESIS__
        std::cout << T << std::endl;
    #endif
    for (int i = 0; i < length; i++)
        _outbuff[i] = T;
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
            static word_t _inbuff[SIZE_IN_CHUNK_DATA];
            static word_t _outbuff[SIZE_OUT_CHUNK_DATA];

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
