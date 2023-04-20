// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _LDA_VIVADO_H_
#define _LDA_VIVADO_H_

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

struct lda_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned Train_epochs;
	unsigned features;
	unsigned classes;
	unsigned windows;
	unsigned Test_epochs;
	unsigned Tol;
	unsigned src_offset;
	unsigned dst_offset;
};

#define LDA_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct lda_vivado_access)

#endif /* _LDA_VIVADO_H_ */
