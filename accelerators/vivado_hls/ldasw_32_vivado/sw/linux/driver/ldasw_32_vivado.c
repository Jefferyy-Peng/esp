// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "ldasw_32_vivado.h"

#define DRV_NAME	"ldasw_32_vivado"

/* <<--regs-->> */
#define LDASW_32_TRAIN_EPOCHS_REG 0x4c
#define LDASW_32_FEATURES_REG 0x48
#define LDASW_32_CLASSES_REG 0x44
#define LDASW_32_TEST_EPOCHS_REG 0x40

struct ldasw_32_vivado_device {
	struct esp_device esp;
};

static struct esp_driver ldasw_32_driver;

static struct of_device_id ldasw_32_device_ids[] = {
	{
		.name = "SLD_LDASW_32_VIVADO",
	},
	{
		.name = "eb_01b",
	},
	{
		.compatible = "sld,ldasw_32_vivado",
	},
	{ },
};

static int ldasw_32_devs;

static inline struct ldasw_32_vivado_device *to_ldasw_32(struct esp_device *esp)
{
	return container_of(esp, struct ldasw_32_vivado_device, esp);
}

static void ldasw_32_prep_xfer(struct esp_device *esp, void *arg)
{
	struct ldasw_32_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->Train_epochs, esp->iomem + LDASW_32_TRAIN_EPOCHS_REG);
	iowrite32be(a->features, esp->iomem + LDASW_32_FEATURES_REG);
	iowrite32be(a->classes, esp->iomem + LDASW_32_CLASSES_REG);
	iowrite32be(a->Test_epochs, esp->iomem + LDASW_32_TEST_EPOCHS_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool ldasw_32_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct ldasw_32_vivado_device *ldasw_32 = to_ldasw_32(esp); */
	/* struct ldasw_32_vivado_access *a = arg; */

	return true;
}

static int ldasw_32_probe(struct platform_device *pdev)
{
	struct ldasw_32_vivado_device *ldasw_32;
	struct esp_device *esp;
	int rc;

	ldasw_32 = kzalloc(sizeof(*ldasw_32), GFP_KERNEL);
	if (ldasw_32 == NULL)
		return -ENOMEM;
	esp = &ldasw_32->esp;
	esp->module = THIS_MODULE;
	esp->number = ldasw_32_devs;
	esp->driver = &ldasw_32_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	ldasw_32_devs++;
	return 0;
 err:
	kfree(ldasw_32);
	return rc;
}

static int __exit ldasw_32_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct ldasw_32_vivado_device *ldasw_32 = to_ldasw_32(esp);

	esp_device_unregister(esp);
	kfree(ldasw_32);
	return 0;
}

static struct esp_driver ldasw_32_driver = {
	.plat = {
		.probe		= ldasw_32_probe,
		.remove		= ldasw_32_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = ldasw_32_device_ids,
		},
	},
	.xfer_input_ok	= ldasw_32_xfer_input_ok,
	.prep_xfer	= ldasw_32_prep_xfer,
	.ioctl_cm	= LDASW_32_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct ldasw_32_vivado_access),
};

static int __init ldasw_32_init(void)
{
	return esp_driver_register(&ldasw_32_driver);
}

static void __exit ldasw_32_exit(void)
{
	esp_driver_unregister(&ldasw_32_driver);
}

module_init(ldasw_32_init)
module_exit(ldasw_32_exit)

MODULE_DEVICE_TABLE(of, ldasw_32_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ldasw_32_vivado driver");
