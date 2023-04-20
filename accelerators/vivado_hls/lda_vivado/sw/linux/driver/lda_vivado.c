// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "lda_vivado.h"

#define DRV_NAME	"lda_vivado"

/* <<--regs-->> */
#define LDA_TRAIN_EPOCHS_REG 0x54
#define LDA_FEATURES_REG 0x50
#define LDA_CLASSES_REG 0x4c
#define LDA_WINDOWS_REG 0x48
#define LDA_TEST_EPOCHS_REG 0x44
#define LDA_TOL_REG 0x40

struct lda_vivado_device {
	struct esp_device esp;
};

static struct esp_driver lda_driver;

static struct of_device_id lda_device_ids[] = {
	{
		.name = "SLD_LDA_VIVADO",
	},
	{
		.name = "eb_011",
	},
	{
		.compatible = "sld,lda_vivado",
	},
	{ },
};

static int lda_devs;

static inline struct lda_vivado_device *to_lda(struct esp_device *esp)
{
	return container_of(esp, struct lda_vivado_device, esp);
}

static void lda_prep_xfer(struct esp_device *esp, void *arg)
{
	struct lda_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->Train_epochs, esp->iomem + LDA_TRAIN_EPOCHS_REG);
	iowrite32be(a->features, esp->iomem + LDA_FEATURES_REG);
	iowrite32be(a->classes, esp->iomem + LDA_CLASSES_REG);
	iowrite32be(a->windows, esp->iomem + LDA_WINDOWS_REG);
	iowrite32be(a->Test_epochs, esp->iomem + LDA_TEST_EPOCHS_REG);
	iowrite32be(a->Tol, esp->iomem + LDA_TOL_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool lda_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct lda_vivado_device *lda = to_lda(esp); */
	/* struct lda_vivado_access *a = arg; */

	return true;
}

static int lda_probe(struct platform_device *pdev)
{
	struct lda_vivado_device *lda;
	struct esp_device *esp;
	int rc;

	lda = kzalloc(sizeof(*lda), GFP_KERNEL);
	if (lda == NULL)
		return -ENOMEM;
	esp = &lda->esp;
	esp->module = THIS_MODULE;
	esp->number = lda_devs;
	esp->driver = &lda_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	lda_devs++;
	return 0;
 err:
	kfree(lda);
	return rc;
}

static int __exit lda_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct lda_vivado_device *lda = to_lda(esp);

	esp_device_unregister(esp);
	kfree(lda);
	return 0;
}

static struct esp_driver lda_driver = {
	.plat = {
		.probe		= lda_probe,
		.remove		= lda_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = lda_device_ids,
		},
	},
	.xfer_input_ok	= lda_xfer_input_ok,
	.prep_xfer	= lda_prep_xfer,
	.ioctl_cm	= LDA_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct lda_vivado_access),
};

static int __init lda_init(void)
{
	return esp_driver_register(&lda_driver);
}

static void __exit lda_exit(void)
{
	esp_driver_unregister(&lda_driver);
}

module_init(lda_init)
module_exit(lda_exit)

MODULE_DEVICE_TABLE(of, lda_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("lda_vivado driver");
