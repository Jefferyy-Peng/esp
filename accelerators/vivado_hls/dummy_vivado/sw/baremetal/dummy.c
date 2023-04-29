/* Copyright (c) 2011-2023 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>
#include "input.h"

typedef int32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
        return (sizeof(void *) / _st);
}


#define SLD_DUMMY 0x09a
#define DEV_NAME "sld,dummy_vivado"

/* <<--params-->> */
const int32_t t = 1;
const int32_t q = 48;
const int32_t p = 12;
const int b = 1;


static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned mem_size;

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ?		\
			(_sz / CHUNK_SIZE) :		\
			(_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
#define DUMMY_T_REG 0x48
#define DUMMY_Q_REG 0x44
#define DUMMY_P_REG 0x40


static int validate_buf(token_t *out, float *gold)
{
	int i;
	int j;
	int x;
	unsigned errors = 0;
	float MAE;

	// for (i = 0; i < 1; i++)
	// 	for (j = 0; j < b; j++)
	// 		if (gold[i * out_words_adj + j] != out[i * out_words_adj + j])
	// 			errors++;
	for (i = 0; i < b; i++)
		for(x = 0; x < (p+q)*(10) + 41; x++){
			token_t val = out[i * out_words_adj + x];
			float gold_val = gold[i * out_words_adj + x];
			MAE = (val - gold_val) / gold_val;
			// printf("val[%d] = %.5f\n", j, val);
			if(x > (p+q)*10){
				uint32_t* tmp1 = (uint32_t*) &gold[i * out_words_adj + x];
				uint32_t* tmp2 = (uint32_t*) &out[i * out_words_adj + x];
				print_uart("gold [");print_uart_int(x);print_uart("] = ");print_uart_int(*tmp1);print_uart(" ");
				// print_uart("out_fixed = ");print_uart_int(*tmp3);print_uart(" ");
				print_uart("out [");print_uart_int(x);print_uart("] = ");print_uart_int(*tmp2);print_uart("\n");
			}
			// printf("out = %d", val);
			// if (MAE > 0.01)
			// 	errors++;
		}
		for (j = 0; j < (p+q)*t; j++){
			float val = fixed32_to_float(out[i * out_words_adj + x + j], 2);
			float gold_val = gold[i * out_words_adj + x + j];
			MAE = (val - gold_val) / gold_val;
			// printf("val[%d] = %.5f\n", j, val);
			uint32_t* tmp1 = (uint32_t*) &gold[i * out_words_adj + x + j];
			uint32_t* tmp2 = (uint32_t*) &val;
			print_uart("gold [");print_uart_int(j+x);print_uart("] = ");print_uart_int(*tmp1);print_uart(" ");
			// print_uart("out_fixed = ");print_uart_int(*tmp3);print_uart(" ");
			print_uart("out [");print_uart_int(j+x);print_uart("] = ");print_uart_int(*tmp2);print_uart("\n");
			// printf("out = %d", val);
			if (MAE > 0.01)
				errors++;
		}

	return errors;
}


static void init_buf (token_t *in, float * gold)
{
	int i;
	int j;
	int x;

	// for (i = 0; i < 1; i++)
	// 	for (j = 0; j < (p+q)*10+41+(p+q)*t; j++)
	// 		in[i * in_words_adj + j] = (token_t) j;

	// for (i = 0; i < 1; i++)
	// 	for (j = 0; j < p; j++)
	// 		gold[i * out_words_adj + j] = (token_t) j;
	printf("initialize buffer \n");
	// printf("Test");
	for (i = 0; i < b; i++){
		if(i == 0){
			for (j = 0; j < (p+q)*10 + 41; j++){
				in[j] = (token_t) input_idx[j];
				uint32_t* tmp1 = (uint32_t*) &in[j];
				if(j <= 10){
					// print_uart("input_index ");print_uart_int(j);print_uart(" = ");print_uart_int(*tmp1);print_uart(" \n");}
				}
			}
			printf("indexs loaded \n");
			for(int y = 0; y<(p+q); y++)
			for (x = 0; x < t; x++){
				in[x + y * t + j] = (token_t) float_to_fixed32(input[x + y * 462], 2);
				// printf("data[%d] = %d\n", x + j, in[x + j]);
				uint32_t* tmp5 = (uint32_t*) &in[x + y * t + j];
				uint32_t* tmp6 = (uint32_t*) &input[x + y * 462];
				print_uart("input_fixed = ");print_uart_int(*tmp5);print_uart(" \n");
				// print_uart("input_float = ");print_uart_int(*tmp6);print_uart("\n");
			}
			printf("data loaded \n");
		}
		else{
			int index_input_offset = (p+q)*10 + 41*i;
			int data_input_offset = (p+q)*t*i;
			int index_mem_offset = (p+q)*10 + i*round_up((41+(p+q)*t),DMA_WORD_PER_BEAT(sizeof(token_t)));
			int data_mem_offset = index_mem_offset + round_up(41,DMA_WORD_PER_BEAT(sizeof(token_t)));
			for (j = 0; j < 41; j++){
				in[index_mem_offset + j] = (token_t) input_idx[index_input_offset + j];
			}
			printf("indexs loaded \n");
			for (j = 0; j < (p+q)*t; j++){
				in[data_mem_offset + j] = (token_t) float_to_fixed32(input[data_input_offset + j],2);
			}
			printf("data loaded \n");
		}
	}
		

	for (i = 0; i < (q+p)*10 + 41; i++){
		gold[i] = input_idx[i];
		// if(i>(q+p)*10){
		// 	uint32_t* tmp4 = (uint32_t*) &gold[i];
		// 	// print_uart("input = ");print_uart_int(*tmp3);print_uart(" ");
		// 	print_uart("gold ");print_uart_int(i);print_uart("=");print_uart_int(*tmp4);print_uart("\n");
		// }
	}
	for(int y = 0; y < (p+q); y++)
	for(j = 0; j < t; j++){
		gold[i+j + y * t] = input[j + y * 462];
		uint32_t* tmp1 = (uint32_t*) &gold[i+j + y * t];
		// print_uart("gold ");print_uart_int(i+j + y * t);print_uart("=");print_uart_int(*tmp1);print_uart("\n");
	}
	printf("gold loaded \n");
}


int main(int argc, char * argv[])
{
	int i;
	int n;
	int ndev;
	struct esp_device *espdevs;
	struct esp_device *dev;
	unsigned done;
	unsigned **ptable;
	token_t *mem;
	float *gold;
	unsigned errors = 0;
	unsigned coherence;

	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = (p+q)*10+41+(p+q)*t;
		out_words_adj = (p+q)*10+41+(p+q)*t;
	} else {
		in_words_adj = round_up((p+q)*10+41+(p+q)*t, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up((p+q)*10+41+(p+q)*t, DMA_WORD_PER_BEAT(sizeof(token_t)));
		// printf("out_words_adj = %d", DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (b);
	out_len = out_words_adj * (b);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset  = in_len;
	mem_size = (out_offset * sizeof(token_t)) + out_size;


	// Search for the device
	printf("Scanning device tree... \n");

	ndev = probe(&espdevs, VENDOR_SLD, SLD_DUMMY, DEV_NAME);
	if (ndev == 0) {
		printf("dummy not found\n");
		return 0;
	}

	for (n = 0; n < ndev; n++) {

		printf("**************** %s.%d ****************\n", DEV_NAME, n);

		dev = &espdevs[n];

		// Check DMA capabilities
		if (ioread32(dev, PT_NCHUNK_MAX_REG) == 0) {
			printf("  -> scatter-gather DMA is disabled. Abort.\n");
			return 0;
		}

		if (ioread32(dev, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
			printf("  -> Not enough TLB entries available. Abort.\n");
			return 0;
		}

		// Allocate memory
		gold = aligned_malloc(out_size);
		mem = aligned_malloc(mem_size);
		printf("  memory buffer base-address = %p\n", mem);

		// Alocate and populate page table
		ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
		for (i = 0; i < NCHUNK(mem_size); i++)
			ptable[i] = (unsigned *) &mem[i * (CHUNK_SIZE / sizeof(token_t))];

		printf("  ptable = %p\n", ptable);
		printf("  nchunk = %lu\n", NCHUNK(mem_size));

#ifndef __riscv
		for (coherence = ACC_COH_NONE; coherence <= ACC_COH_RECALL; coherence++) {
#else
		{
			/* TODO: Restore full test once ESP caches are integrated */
			coherence = ACC_COH_NONE;
#endif
			printf("  --------------------\n");
			printf("  Generate input...\n");
			init_buf(mem, gold);

			// Pass common configuration parameters

			iowrite32(dev, SELECT_REG, ioread32(dev, DEVID_REG));
			iowrite32(dev, COHERENCE_REG, coherence);

#ifndef __sparc
			iowrite32(dev, PT_ADDRESS_REG, (unsigned long long) ptable);
#else
			iowrite32(dev, PT_ADDRESS_REG, (unsigned) ptable);
#endif
			iowrite32(dev, PT_NCHUNK_REG, NCHUNK(mem_size));
			iowrite32(dev, PT_SHIFT_REG, CHUNK_SHIFT);

			// Use the following if input and output data are not allocated at the default offsets
			iowrite32(dev, SRC_OFFSET_REG, 0x0);
			iowrite32(dev, DST_OFFSET_REG, 0x0);

			// Pass accelerator-specific configuration parameters
			/* <<--regs-config-->> */
		iowrite32(dev, DUMMY_T_REG, t);
		iowrite32(dev, DUMMY_Q_REG, q);
		iowrite32(dev, DUMMY_P_REG, p);

			// Flush (customize coherence model here)
			esp_flush(coherence);

			// Start accelerators
			printf("  Start...\n");
			iowrite32(dev, CMD_REG, CMD_MASK_START);

			// Wait for completion
			done = 0;
			while (!done) {
				done = ioread32(dev, STATUS_REG);
				done &= STATUS_MASK_DONE;
			}
			iowrite32(dev, CMD_REG, 0x0);

			printf("  Done\n");
			printf("  validating...\n");

			/* Validation */
			errors = validate_buf(&mem[out_offset], gold);
			if (errors)
				printf("  ... FAIL\n");
			else
				printf("  ... PASS\n");
		}
		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}
