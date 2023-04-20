// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _LDASW_VIVADO_H_
#define _LDASW_VIVADO_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#ifndef __user
#define __user
#endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

struct ldasw_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned Train_epochs;
	unsigned features;
	unsigned classes;
	unsigned Test_epochs;
	unsigned src_offset;
	unsigned dst_offset;
};

#define LDASW_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct ldasw_vivado_access)

#endif /* _LDASW_VIVADO_H_ */
