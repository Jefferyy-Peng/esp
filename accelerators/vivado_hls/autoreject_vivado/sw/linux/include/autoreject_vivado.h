// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _AUTOREJECT_VIVADO_H_
#define _AUTOREJECT_VIVADO_H_

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

struct autoreject_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned t;
	unsigned q;
	unsigned p;
	unsigned c;
	unsigned src_offset;
	unsigned dst_offset;
};

#define AUTOREJECT_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct autoreject_vivado_access)

#endif /* _AUTOREJECT_VIVADO_H_ */
