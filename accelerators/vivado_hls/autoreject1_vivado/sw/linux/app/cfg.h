// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "autoreject1_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define T 462
#define Q 48
#define P 12

/* <<--params-->> */
const int32_t t = T;
const int32_t q = Q;
const int32_t p = P;

#define NACC 1

struct autoreject1_vivado_access autoreject1_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.t = T,
		.q = Q,
		.p = P,
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
		.devname = "autoreject1_vivado.0",
		.ioctl_req = AUTOREJECT1_VIVADO_IOC_ACCESS,
		.esp_desc = &(autoreject1_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */