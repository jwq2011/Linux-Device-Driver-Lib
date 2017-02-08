/*
 * a simple char device driver: globalmem
 * Copyright (C) 2017 Kiwei (jiaweiqing.work@qq.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_device.h>

struct platform_device *globalmem_pdev;
static int __init globalmem_init(void)
{
	int ret;
	globalmem_pdev = platform_device_alloc("globalmem_info", -1);
	if(!globalmem_pdev)
		return -ENOMEM;

	ret = platform_device_add(globalmem_pdev);
	if(ret)
	{
		platform_device_put(globalmem_pdev);
		return ret;
	}
//	platform_device_register(globalmem_pdev);
	return 0;
}
module_init(globalmem_init);

static void __exit globalmem_exit(void)
{
	platform_device_unregister(globalmem_pdev);
}
module_exit(globalmem_exit);

MODULE_AUTHOR("Kiwei <jiaweiqing.work@qq.com>");
MODULE_LICENSE("GPL v2");
