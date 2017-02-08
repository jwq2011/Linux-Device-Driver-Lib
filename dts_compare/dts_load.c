/*
 * a simple char device driver: globalmem
 * Copyright (C) 2017 Kiwei (jiaweiqing.work@qq.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>

struct dts_compare_data {
	bool status1;
	char *buf;
};


static const struct dts_compare_data dts_data1 = {
	.status1 = 0x13,
	.buf = "I am here!"
};

static int dts_compare_probe(struct platform_device *pdev)
{
	return;
}

static int dts_compare_remove(struct platform_device *pdev)
{
	return;
}

static struct of_device_id dts_compare_of_match[] = {
	{.compatible = "vexpress,dts_compare", .data = &dts_data1},
	{},
};
MODULE_DEVICE_TABLE(of, dts_compare_of_match);

static struct platform_driver dts_compare_platform_driver = {
	.probe = dts_compare_probe,
	.remove = dts_compare_remove,
	.driver = {
		.name = "dts_compare",
		.owner = THIS_MODULE,
		.of_match_table = dts_compare_of_match,
	},
};
module_platform_driver(dts_compare_platform_driver);

static int __init dts_compare_init(void)
{
	platform_driver_register(&dts_compare_platform_driver);
	return 0;
}
module_init(dts_compare_init);

static void __exit dts_compare_exit(void)
{
	platform_driver_unregister(&dts_compare_platform_driver);
	return;
}
module_exit(dts_compare_exit);

