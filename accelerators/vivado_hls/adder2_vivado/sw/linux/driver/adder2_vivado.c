// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "adder2_vivado.h"

#define DRV_NAME	"adder2_vivado"

/* <<--regs-->> */
#define ADDER2_SIZE_REG 0x40

struct adder2_vivado_device {
	struct esp_device esp;
};

static struct esp_driver adder2_driver;

static struct of_device_id adder2_device_ids[] = {
	{
		.name = "SLD_ADDER2_VIVADO",
	},
	{
		.name = "eb_001",
	},
	{
		.compatible = "sld,adder2_vivado",
	},
	{ },
};

static int adder2_devs;

static inline struct adder2_vivado_device *to_adder2(struct esp_device *esp)
{
	return container_of(esp, struct adder2_vivado_device, esp);
}

static void adder2_prep_xfer(struct esp_device *esp, void *arg)
{
	struct adder2_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->size, esp->iomem + ADDER2_SIZE_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool adder2_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct adder2_vivado_device *adder2 = to_adder2(esp); */
	/* struct adder2_vivado_access *a = arg; */

	return true;
}

static int adder2_probe(struct platform_device *pdev)
{
	struct adder2_vivado_device *adder2;
	struct esp_device *esp;
	int rc;

	adder2 = kzalloc(sizeof(*adder2), GFP_KERNEL);
	if (adder2 == NULL)
		return -ENOMEM;
	esp = &adder2->esp;
	esp->module = THIS_MODULE;
	esp->number = adder2_devs;
	esp->driver = &adder2_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	adder2_devs++;
	return 0;
 err:
	kfree(adder2);
	return rc;
}

static int __exit adder2_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct adder2_vivado_device *adder2 = to_adder2(esp);

	esp_device_unregister(esp);
	kfree(adder2);
	return 0;
}

static struct esp_driver adder2_driver = {
	.plat = {
		.probe		= adder2_probe,
		.remove		= adder2_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = adder2_device_ids,
		},
	},
	.xfer_input_ok	= adder2_xfer_input_ok,
	.prep_xfer	= adder2_prep_xfer,
	.ioctl_cm	= ADDER2_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct adder2_vivado_access),
};

static int __init adder2_init(void)
{
	return esp_driver_register(&adder2_driver);
}

static void __exit adder2_exit(void)
{
	esp_driver_unregister(&adder2_driver);
}

module_init(adder2_init)
module_exit(adder2_exit)

MODULE_DEVICE_TABLE(of, adder2_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("adder2_vivado driver");
