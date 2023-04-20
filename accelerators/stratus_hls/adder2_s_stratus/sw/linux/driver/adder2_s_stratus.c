// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "adder2_s_stratus.h"

#define DRV_NAME	"adder2_s_stratus"

/* <<--regs-->> */
#define ADDER2_S_SIZE_REG 0x40

struct adder2_s_stratus_device {
	struct esp_device esp;
};

static struct esp_driver adder2_s_driver;

static struct of_device_id adder2_s_device_ids[] = {
	{
		.name = "SLD_ADDER2_S_STRATUS",
	},
	{
		.name = "eb_002",
	},
	{
		.compatible = "sld,adder2_s_stratus",
	},
	{ },
};

static int adder2_s_devs;

static inline struct adder2_s_stratus_device *to_adder2_s(struct esp_device *esp)
{
	return container_of(esp, struct adder2_s_stratus_device, esp);
}

static void adder2_s_prep_xfer(struct esp_device *esp, void *arg)
{
	struct adder2_s_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->size, esp->iomem + ADDER2_S_SIZE_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool adder2_s_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct adder2_s_stratus_device *adder2_s = to_adder2_s(esp); */
	/* struct adder2_s_stratus_access *a = arg; */

	return true;
}

static int adder2_s_probe(struct platform_device *pdev)
{
	struct adder2_s_stratus_device *adder2_s;
	struct esp_device *esp;
	int rc;

	adder2_s = kzalloc(sizeof(*adder2_s), GFP_KERNEL);
	if (adder2_s == NULL)
		return -ENOMEM;
	esp = &adder2_s->esp;
	esp->module = THIS_MODULE;
	esp->number = adder2_s_devs;
	esp->driver = &adder2_s_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	adder2_s_devs++;
	return 0;
 err:
	kfree(adder2_s);
	return rc;
}

static int __exit adder2_s_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct adder2_s_stratus_device *adder2_s = to_adder2_s(esp);

	esp_device_unregister(esp);
	kfree(adder2_s);
	return 0;
}

static struct esp_driver adder2_s_driver = {
	.plat = {
		.probe		= adder2_s_probe,
		.remove		= adder2_s_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = adder2_s_device_ids,
		},
	},
	.xfer_input_ok	= adder2_s_xfer_input_ok,
	.prep_xfer	= adder2_s_prep_xfer,
	.ioctl_cm	= ADDER2_S_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct adder2_s_stratus_access),
};

static int __init adder2_s_init(void)
{
	return esp_driver_register(&adder2_s_driver);
}

static void __exit adder2_s_exit(void)
{
	esp_driver_unregister(&adder2_s_driver);
}

module_init(adder2_s_init)
module_exit(adder2_s_exit)

MODULE_DEVICE_TABLE(of, adder2_s_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("adder2_s_stratus driver");
