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


#define SLD_AUTOREJECT1 0x02c
#define DEV_NAME "sld,autoreject1_vivado"

/* <<--params-->> */
const int32_t t = 10;
const int32_t q = 48;
const int32_t p = 12;
// const int32_t b = 1;

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
#define AUTOREJECT1_T_REG 0x48
#define AUTOREJECT1_Q_REG 0x44
#define AUTOREJECT1_P_REG 0x40


static int validate_buf(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;
	float MAE;
	int b = 1;

	for (i = 0; i < b; i++)
		for (j = 0; j < 1; j++){
			float val = fixed32_to_float(out[i * out_words_adj + j], 2);
			float gold_val = gold_out[i * out_words_adj + j];
			MAE = (val - gold_val) / gold_val;
			printf("val[%d] = %f", i, val);
			if (gold[i * out_words_adj + j] != out[i * out_words_adj + j])
				errors++;
		}

	return errors;
}


static void init_buf (token_t *in, token_t * gold)
{
	int i;
	int j;
	int b = 1;

	printf("initialize buffer \n");
	
	for (i = 0; i < b; i++)
		if(i == 0){
			int data_offset = (p+q)*10 + 41;
			for (j = 0; j < (p+q)*10 + 41; j++){
				in[j] = (token_t) input_idx[j];
			}
			printf("indexs loaded \n");
			for (j = 0; j < (p+q)*t; j++){
				in[data_offset + j] = (token_t) input[j];
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
				in[data_mem_offset + j] = (token_t) input[data_input_offset + j];
			}
			printf("data loaded \n");
		}
		

	for (i = 0; i < b; i++)
		gold[i] = (token_t) gold_out[i];
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
	token_t *gold;
	unsigned errors = 0;
	unsigned coherence;
	int b = 1;

	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = (p+q)*10+41+(p+q)*t;
		out_words_adj = 1;
	} else {
		in_words_adj = round_up((p+q)*10+41+(p+q)*t, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(1, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = (p+q)*10 + b*round_up(41+(p+q)*t, DMA_WORD_PER_BEAT(sizeof(token_t)));
	// out_len = out_words_adj * (1);
	out_len = b;
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset  = in_len;
	mem_size = (out_offset * sizeof(token_t)) + out_size;


	// Search for the device
	printf("Scanning device tree... \n");

	ndev = probe(&espdevs, VENDOR_SLD, SLD_AUTOREJECT1, DEV_NAME);
	if (ndev == 0) {
		printf("autoreject1 not found\n");
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
		iowrite32(dev, AUTOREJECT1_T_REG, t);
		iowrite32(dev, AUTOREJECT1_Q_REG, q);
		iowrite32(dev, AUTOREJECT1_P_REG, p);

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
