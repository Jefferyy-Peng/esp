// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "test_vivado.h"

#define DRV_NAME	"test_vivado"

/* <<--regs-->> */
#define TEST_T_REG 0x4c
#define TEST_Q_REG 0x48
#define TEST_P_REG 0x44
#define TEST_C_REG 0x40

struct test_vivado_device {
	struct esp_device esp;
};

static struct esp_driver test_driver;

static struct of_device_id test_device_ids[] = {
	{
		.name = "SLD_TEST_VIVADO",
	},
	{
		.name = "eb_111",
	},
	{
		.compatible = "sld,test_vivado",
	},
	{ },
};

static int test_devs;

static inline struct test_vivado_device *to_test(struct esp_device *esp)
{
	return container_of(esp, struct test_vivado_device, esp);
}

static void test_prep_xfer(struct esp_device *esp, void *arg)
{
	struct test_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->t, esp->iomem + TEST_T_REG);
	iowrite32be(a->q, esp->iomem + TEST_Q_REG);
	iowrite32be(a->p, esp->iomem + TEST_P_REG);
	iowrite32be(a->c, esp->iomem + TEST_C_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool test_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct test_vivado_device *test = to_test(esp); */
	/* struct test_vivado_access *a = arg; */

	return true;
}

static int test_probe(struct platform_device *pdev)
{
	struct test_vivado_device *test;
	struct esp_device *esp;
	int rc;

	test = kzalloc(sizeof(*test), GFP_KERNEL);
	if (test == NULL)
		return -ENOMEM;
	esp = &test->esp;
	esp->module = THIS_MODULE;
	esp->number = test_devs;
	esp->driver = &test_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	test_devs++;
	return 0;
 err:
	kfree(test);
	return rc;
}

static int __exit test_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct test_vivado_device *test = to_test(esp);

	esp_device_unregister(esp);
	kfree(test);
	return 0;
}

static struct esp_driver test_driver = {
	.plat = {
		.probe		= test_probe,
		.remove		= test_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = test_device_ids,
		},
	},
	.xfer_input_ok	= test_xfer_input_ok,
	.prep_xfer	= test_prep_xfer,
	.ioctl_cm	= TEST_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct test_vivado_access),
};

static int __init test_init(void)
{
	return esp_driver_register(&test_driver);
}

static void __exit test_exit(void)
{
	esp_driver_unregister(&test_driver);
}

module_init(test_init)
module_exit(test_exit)

MODULE_DEVICE_TABLE(of, test_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("test_vivado driver");
