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

    const unsigned length = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up((p+q)*10+41+(p+q)*t, VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);
    #ifndef __SYNTHESIS__
    std::cout << "length = " << length << std::endl;
    std::cout << "index = " << index << std::endl;
    #endif

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;
    #ifndef __SYNTHESIS__
    std::cout << "dma_length = " << dma_length << std::endl;
    #endif

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    store_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
        #ifndef __SYNTHESIS__
            std::cout << "out " << i * VALUES_PER_WORD + j << "=" << out[dma_index + i].word[j] << std::endl;
        #endif
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
    // const unsigned length = round_up((p+q)*t, VALUES_PER_WORD) / 1;
    // const unsigned index_offset = (p+q)*10+41+(p+q)*t - 1;

    const unsigned TrIdx_size = q * 10;
    const unsigned TeIdx_size = p * 10;
    const unsigned CaIdx_size = 41;
    // const unsigned comp_workers = 1;


    ap_uint<32> TrIdx[10][Q_MAX];
    ap_uint<32> TeIdx[10][P_MAX];
    ap_uint<32> CaIdx[41];
    word_t D[Q_MAX + P_MAX][T_MAX];

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < Q_MAX; j++){
            ap_uint<32> temp0;
            for(int k = 0; k < 32; k++){
                // std::cout << "Trk=" << k << std::endl;
                temp0(k,k) = _inbuff[i * q + j][k];
            }
            TrIdx[i][j] = temp0;
            #ifndef __SYNTHESIS__
            std::cout << "TrIdx[" << i << "][" << j << "] = " << TrIdx[i][j] << std::endl;
            #endif
        }
    }

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < P_MAX; j++){
            ap_uint<32> temp1;
            for(int k = 0; k < 32; k++){
                // std::cout << "Trk=" << k << std::endl;
                temp1(k,k) = _inbuff[TrIdx_size + i * p + j][k];
            }
            TeIdx[i][j] = temp1;
            #ifndef __SYNTHESIS__
            std::cout << "TeIdx[" << i << "][" << j << "] = " << TeIdx[i][j] << std::endl;
            #endif
        }
    }

    for(int i = 0; i < 41; i++){
        ap_uint<32> temp2;
        for(int k = 0; k < 32; k++){
                // std::cout << "Trk=" << k << std::endl;
                temp2(k,k) = _inbuff[TrIdx_size + TeIdx_size + i][k];
        }
        CaIdx[i] = temp2;
        #ifndef __SYNTHESIS__
        std::cout << "CaIdx[" << i << "] = " << CaIdx[i] << std::endl;
        #endif
    }


    const unsigned D_index = TrIdx_size + TeIdx_size + CaIdx_size;
    for(int i = 0; i < Q_MAX + P_MAX; i++){
        for(int j = 0; j < t; j++){
            D[i][j] = _inbuff[D_index + i * t + j];
            #ifndef __SYNTHESIS__
            std::cout << "D[" << i << "][" << j << "] = " << D[i][j] << std::endl;
            #endif
        }
    }
    for(int i = 0; i< 10; i++)
    for (int j = 0; j < Q_MAX; j++){
        ap_fixed<32, 2> temp3;
        for(int k = 0; k < 32; k++)
            temp3(k,k) = TrIdx[i][j][k];
        _outbuff[j + i * Q_MAX] = temp3;
        #ifndef __SYNTHESIS__
            std::cout << "outbuff[" << j + i * Q_MAX << "] = " << _outbuff[j + i * Q_MAX] << std::endl;
        #endif
    }
    for(int i = 0; i< 10; i++)
    for (int j = 0; j < P_MAX; j++){
        ap_fixed<32, 2> temp4;
        for(int k = 0; k < 32; k++)
            temp4(k,k) = TeIdx[i][j][k];
        _outbuff[q*10 + i * P_MAX + j] = temp4;
        #ifndef __SYNTHESIS__
            std::cout << "outbuff[" << q*10 + i * P_MAX + j << "] = " << _outbuff[q*10 + i * P_MAX + j] << std::endl;
        #endif
    }
    for (int i = 0; i < 41; i++){
        ap_fixed<32, 2> temp5;
        for(int k = 0; k < 32; k++)
            temp5(k,k) = CaIdx[i][k];
        _outbuff[(q+p)*10 + i] = temp5;
        #ifndef __SYNTHESIS__
            std::cout << "outbuff[" << (q+p)*10 + i << "] = " <<  _outbuff[(q+p)*10 + i] << std::endl;
        #endif
    }
    for (int i = 0; i < p+q; i++){
        for(int u = 0; u< t; u++){
            _outbuff[(q+p)*10 + 41 + i * t + u] = D[i][u];
            #ifndef __SYNTHESIS__
                std::cout << "outbuff[" << (q+p)*10 + 41 + i * t + u << "] = " << _outbuff[(q+p)*10 + 41 + i * t + u] << std::endl;
            #endif
        }
    }

    // word_t p2p[Q_MAX + P_MAX];
    // word_t high;
    // word_t low;
    // ap_uint<32> gl;
    // word_t mean[T_MAX];
    // word_t E[41][10];
    // word_t E_mean[41];
    // word_t low_E;
    // ap_uint<32> low_EIdx;
    // word_t T;
    // ap_uint<32> pos;
    // word_t pos_1 = 0;
    // word_t pos_2 = 0;
    // word_t median[10][T_MAX];
    // word_t min_tr_thre[10];
    // ap_uint<32> min_tr_idx[10];

    // ptp_loop0:for(int i = 0; i < q+p; i++){
    //     high = D[i][0];
    //     low = D[i][0];
    //     ptp_loop1:for(int j = 0; j < t; j++){
    //         if(D[i][j] > high){
    //             high = D[i][j];
    //         }
    //         else if(D[i][j] < low){
    //             low = D[i][j];
    //         }
    //     }
    //     p2p[i] = high - low;
    //     #ifndef __SYNTHESIS__
    //     std::cout << "p2p[" << i << "] =" << p2p[i] << std::endl; 
    //     #endif
    // }
    // cv_loop0:for(int u = 0; u< 10 ; u++){
    //     t_loop0:for(int i = 0; i < T_MAX ; i++){
    //         pos_1 = 0;
    //         pos_2 = 0;
    //         median[u][i] = 0;
    //         median_loop0:for(int j = 0; j < P_MAX ; j++){
    //             pos = 0;
    //             median_loop1:for(int v = 0; v < P_MAX; v++){
    //                 if(D[TeIdx[u][v]][i] < D[TeIdx[u][j]][i]){
    //                 // if(D[v][i] < D[v][i]){
    //                     pos += 1;
    //                 }
    //             }
    //             // std::cout << "D[" << u << "][" << i << "][" << j << "] = " << D[TeIdx[u][j]][i] << "," << TeIdx[u][j] << std::endl;
    //             // std::cout << "pos[" << u << "][" << i << "][" << j << "] = " << pos << std::endl;
    //             if(pos == 5){
    //                 // median[u][i] += D[1][i];
    //                 median[u][i] += D[TeIdx[u][j]][i];
    //                 pos_1 = 1;
    //             }
    //             else if(pos == 6){
    //                 // median[u][i] += D[1][i];
    //                 median[u][i] += D[TeIdx[u][j]][i];
    //                 pos_2 = 1;
    //             }
    //             if(pos_1 && pos_2){
    //                 median[u][i] /= 2;
    //                 #ifndef __SYNTHESIS__
    //                 std::cout << "Median[" << u << "][" << i << "] = " << median[u][i] << std::endl;
    //                 #endif
    //                 break;
    //             }
    //         }
    //     }
    // }

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
