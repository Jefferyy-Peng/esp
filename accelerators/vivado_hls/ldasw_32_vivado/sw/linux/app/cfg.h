// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "ldasw_32_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define TRAIN_EPOCHS 352
#define FEATURES 240
#define CLASSES 2
#define TEST_EPOCHS 40

/* <<--params-->> */
const int32_t Train_epochs = TRAIN_EPOCHS;
const int32_t features = FEATURES;
const int32_t classes = CLASSES;
const int32_t Test_epochs = TEST_EPOCHS;

#define NACC 1

struct ldasw_32_vivado_access ldasw_32_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.Train_epochs = TRAIN_EPOCHS,
		.features = FEATURES,
		.classes = CLASSES,
		.Test_epochs = TEST_EPOCHS,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "ldasw_32_vivado.0",
		.ioctl_req = LDASW_32_VIVADO_IOC_ACCESS,
		.esp_desc = &(ldasw_32_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
