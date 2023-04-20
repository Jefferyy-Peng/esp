// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "autoreject1_vivado.h"

#define DRV_NAME	"autoreject1_vivado"

/* <<--regs-->> */
#define AUTOREJECT1_T_REG 0x48
#define AUTOREJECT1_Q_REG 0x44
#define AUTOREJECT1_P_REG 0x40

struct autoreject1_vivado_device {
	struct esp_device esp;
};

static struct esp_driver autoreject1_driver;

static struct of_device_id autoreject1_device_ids[] = {
	{
		.name = "SLD_AUTOREJECT1_VIVADO",
	},
	{
		.name = "eb_02c",
	},
	{
		.compatible = "sld,autoreject1_vivado",
	},
	{ },
};

static int autoreject1_devs;

static inline struct autoreject1_vivado_device *to_autoreject1(struct esp_device *esp)
{
	return container_of(esp, struct autoreject1_vivado_device, esp);
}

static void autoreject1_prep_xfer(struct esp_device *esp, void *arg)
{
	struct autoreject1_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->t, esp->iomem + AUTOREJECT1_T_REG);
	iowrite32be(a->q, esp->iomem + AUTOREJECT1_Q_REG);
	iowrite32be(a->p, esp->iomem + AUTOREJECT1_P_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool autoreject1_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct autoreject1_vivado_device *autoreject1 = to_autoreject1(esp); */
	/* struct autoreject1_vivado_access *a = arg; */

	return true;
}

static int autoreject1_probe(struct platform_device *pdev)
{
	struct autoreject1_vivado_device *autoreject1;
	struct esp_device *esp;
	int rc;

	autoreject1 = kzalloc(sizeof(*autoreject1), GFP_KERNEL);
	if (autoreject1 == NULL)
		return -ENOMEM;
	esp = &autoreject1->esp;
	esp->module = THIS_MODULE;
	esp->number = autoreject1_devs;
	esp->driver = &autoreject1_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	autoreject1_devs++;
	return 0;
 err:
	kfree(autoreject1);
	return rc;
}

static int __exit autoreject1_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct autoreject1_vivado_device *autoreject1 = to_autoreject1(esp);

	esp_device_unregister(esp);
	kfree(autoreject1);
	return 0;
}

static struct esp_driver autoreject1_driver = {
	.plat = {
		.probe		= autoreject1_probe,
		.remove		= autoreject1_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = autoreject1_device_ids,
		},
	},
	.xfer_input_ok	= autoreject1_xfer_input_ok,
	.prep_xfer	= autoreject1_prep_xfer,
	.ioctl_cm	= AUTOREJECT1_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct autoreject1_vivado_access),
};

static int __init autoreject1_init(void)
{
	return esp_driver_register(&autoreject1_driver);
}

static void __exit autoreject1_exit(void)
{
	esp_driver_unregister(&autoreject1_driver);
}

module_init(autoreject1_init)
module_exit(autoreject1_exit)

MODULE_DEVICE_TABLE(of, autoreject1_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("autoreject1_vivado driver");
