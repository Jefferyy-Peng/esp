// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "adder2_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define SIZE 64

/* <<--params-->> */
const int32_t size = SIZE;

#define NACC 1

struct adder2_vivado_access adder2_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.size = SIZE,
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
		.devname = "adder2_vivado.0",
		.ioctl_req = ADDER2_VIVADO_IOC_ACCESS,
		.esp_desc = &(adder2_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
