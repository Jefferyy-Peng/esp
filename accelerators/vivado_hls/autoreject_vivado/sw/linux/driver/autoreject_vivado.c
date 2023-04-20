// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "autoreject_vivado.h"

#define DRV_NAME	"autoreject_vivado"

/* <<--regs-->> */
#define AUTOREJECT_T_REG 0x4c
#define AUTOREJECT_Q_REG 0x48
#define AUTOREJECT_P_REG 0x44
#define AUTOREJECT_C_REG 0x40

struct autoreject_vivado_device {
	struct esp_device esp;
};

static struct esp_driver autoreject_driver;

static struct of_device_id autoreject_device_ids[] = {
	{
		.name = "SLD_AUTOREJECT_VIVADO",
	},
	{
		.name = "eb_02a",
	},
	{
		.compatible = "sld,autoreject_vivado",
	},
	{ },
};

static int autoreject_devs;

static inline struct autoreject_vivado_device *to_autoreject(struct esp_device *esp)
{
	return container_of(esp, struct autoreject_vivado_device, esp);
}

static void autoreject_prep_xfer(struct esp_device *esp, void *arg)
{
	struct autoreject_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->t, esp->iomem + AUTOREJECT_T_REG);
	iowrite32be(a->q, esp->iomem + AUTOREJECT_Q_REG);
	iowrite32be(a->p, esp->iomem + AUTOREJECT_P_REG);
	iowrite32be(a->c, esp->iomem + AUTOREJECT_C_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool autoreject_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct autoreject_vivado_device *autoreject = to_autoreject(esp); */
	/* struct autoreject_vivado_access *a = arg; */

	return true;
}

static int autoreject_probe(struct platform_device *pdev)
{
	struct autoreject_vivado_device *autoreject;
	struct esp_device *esp;
	int rc;

	autoreject = kzalloc(sizeof(*autoreject), GFP_KERNEL);
	if (autoreject == NULL)
		return -ENOMEM;
	esp = &autoreject->esp;
	esp->module = THIS_MODULE;
	esp->number = autoreject_devs;
	esp->driver = &autoreject_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	autoreject_devs++;
	return 0;
 err:
	kfree(autoreject);
	return rc;
}

static int __exit autoreject_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct autoreject_vivado_device *autoreject = to_autoreject(esp);

	esp_device_unregister(esp);
	kfree(autoreject);
	return 0;
}

static struct esp_driver autoreject_driver = {
	.plat = {
		.probe		= autoreject_probe,
		.remove		= autoreject_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = autoreject_device_ids,
		},
	},
	.xfer_input_ok	= autoreject_xfer_input_ok,
	.prep_xfer	= autoreject_prep_xfer,
	.ioctl_cm	= AUTOREJECT_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct autoreject_vivado_access),
};

static int __init autoreject_init(void)
{
	return esp_driver_register(&autoreject_driver);
}

static void __exit autoreject_exit(void)
{
	esp_driver_unregister(&autoreject_driver);
}

module_init(autoreject_init)
module_exit(autoreject_exit)

MODULE_DEVICE_TABLE(of, autoreject_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("autoreject_vivado driver");
