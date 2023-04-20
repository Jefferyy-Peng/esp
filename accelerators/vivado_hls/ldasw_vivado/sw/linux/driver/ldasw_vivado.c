// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "ldasw_vivado.h"

#define DRV_NAME	"ldasw_vivado"

/* <<--regs-->> */
#define LDASW_TRAIN_EPOCHS_REG 0x4c
#define LDASW_FEATURES_REG 0x48
#define LDASW_CLASSES_REG 0x44
#define LDASW_TEST_EPOCHS_REG 0x40

struct ldasw_vivado_device {
	struct esp_device esp;
};

static struct esp_driver ldasw_driver;

static struct of_device_id ldasw_device_ids[] = {
	{
		.name = "SLD_LDASW_VIVADO",
	},
	{
		.name = "eb_01a",
	},
	{
		.compatible = "sld,ldasw_vivado",
	},
	{ },
};

static int ldasw_devs;

static inline struct ldasw_vivado_device *to_ldasw(struct esp_device *esp)
{
	return container_of(esp, struct ldasw_vivado_device, esp);
}

static void ldasw_prep_xfer(struct esp_device *esp, void *arg)
{
	struct ldasw_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->Train_epochs, esp->iomem + LDASW_TRAIN_EPOCHS_REG);
	iowrite32be(a->features, esp->iomem + LDASW_FEATURES_REG);
	iowrite32be(a->classes, esp->iomem + LDASW_CLASSES_REG);
	iowrite32be(a->Test_epochs, esp->iomem + LDASW_TEST_EPOCHS_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool ldasw_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct ldasw_vivado_device *ldasw = to_ldasw(esp); */
	/* struct ldasw_vivado_access *a = arg; */

	return true;
}

static int ldasw_probe(struct platform_device *pdev)
{
	struct ldasw_vivado_device *ldasw;
	struct esp_device *esp;
	int rc;

	ldasw = kzalloc(sizeof(*ldasw), GFP_KERNEL);
	if (ldasw == NULL)
		return -ENOMEM;
	esp = &ldasw->esp;
	esp->module = THIS_MODULE;
	esp->number = ldasw_devs;
	esp->driver = &ldasw_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	ldasw_devs++;
	return 0;
 err:
	kfree(ldasw);
	return rc;
}

static int __exit ldasw_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct ldasw_vivado_device *ldasw = to_ldasw(esp);

	esp_device_unregister(esp);
	kfree(ldasw);
	return 0;
}

static struct esp_driver ldasw_driver = {
	.plat = {
		.probe		= ldasw_probe,
		.remove		= ldasw_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = ldasw_device_ids,
		},
	},
	.xfer_input_ok	= ldasw_xfer_input_ok,
	.prep_xfer	= ldasw_prep_xfer,
	.ioctl_cm	= LDASW_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct ldasw_vivado_access),
};

static int __init ldasw_init(void)
{
	return esp_driver_register(&ldasw_driver);
}

static void __exit ldasw_exit(void)
{
	esp_driver_unregister(&ldasw_driver);
}

module_init(ldasw_init)
module_exit(ldasw_exit)

MODULE_DEVICE_TABLE(of, ldasw_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ldasw_vivado driver");
