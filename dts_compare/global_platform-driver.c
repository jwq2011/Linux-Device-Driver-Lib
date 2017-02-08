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

#define GLOBALMEM_MAJOR		231
#define GLOBALMEM_SIZE		1024
#define DEVICE_NUM			2

#if 0
#define CMD_CLEAR			0x81
#else
/* Read from Documentation/ioctl/ioctl-number.txt */
#define GLOBALMEM_MAGIC		'x'	//0x78 ASCII
#define CMD_CLEAR			_IO(GLOBALMEM_MAGIC, 0)
#endif

static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);

struct globalmem_dev {
	struct cdev cdev;
	dev_t devno;
	int major;
	unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;

static int globalmem_open(struct inode *inode, struct file *filp)
{
#if 0
	filp->private_data = globalmem_devp;
#else
	/* 支持N个globalmem设备的globalmem驱动 */
	struct globalmem_dev *dev = container_of(inode->i_cdev, struct globalmem_dev, cdev);
	filp->private_data = dev;
#endif
	return 0;
}

static int globalmem_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch(cmd)
	{
		case CMD_CLEAR:
			memset(dev->mem, 0, GLOBALMEM_SIZE);
			printk(KERN_INFO "globalmem is set to zero\n");
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static ssize_t globalmem_read (struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_to_user(buf, dev->mem + p, count))
		ret = -EFAULT;
	else
	{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %u bytes from %lu\n", count, p);
	}

	return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;

	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (copy_from_user(dev->mem + p, buf, count))
		ret = -EFAULT;
	else {
		*ppos += count;
		ret = count;

		printk(KERN_INFO "written %u bytes from %lu\n", count, p);
	}

	return ret;
}

static loff_t globalmem_llseek (struct file *filp, loff_t offset, int orig)
{
	return 0;
}

static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.unlocked_ioctl = globalmem_ioctl,
	.open = globalmem_open,
	.release = globalmem_close,
};

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int err, devno = MKDEV(globalmem_major, index);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding globalmem%d", err, index);
}

static int globalmem_probe(struct platform_device *pdev)
{
	int ret = -1, i;

	dev_t devno = MKDEV(globalmem_major, 0);

	if (globalmem_major)
	{
//		ret = register_chrdev_region(devno, 1, "globalmem");
		ret = register_chrdev_region(devno, DEVICE_NUM, "globalmem");
	}
	else
	{
//		ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		ret = alloc_chrdev_region(&devno, 0, DEVICE_NUM, "globalmem");
		globalmem_major = MAJOR(devno);
	}
	if (ret < 0)
		return ret;

//	globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	globalmem_devp = kzalloc(sizeof(struct globalmem_dev) * DEVICE_NUM, GFP_KERNEL);
	if(!globalmem_devp)
	{
		ret = -ENOMEM;
		goto fail_malloc;
	}

	for (i = 0; i < DEVICE_NUM; i++)
		globalmem_setup_cdev(globalmem_devp + i, i);

	return 0;

fail_malloc:
//	unregister_chrdev_region(devno, 1);
	unregister_chrdev_region(devno, DEVICE_NUM);
	return ret;
}


static int globalmem_remove(struct platform_device *pdev)
{
	int i;

//	cdev_del(&globalmem_devp->cdev);
	for(i = 0; i < DEVICE_NUM; i++)
		cdev_del(&(globalmem_devp + i)->cdev);

	kfree(globalmem_devp);
//	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), DEVICE_NUM);

	return 0;
}

static struct platform_driver globalmem_driver = {
	.probe = globalmem_probe,
	.remove = globalmem_remove,
	.driver = {
		.name = "globalmem_info",
		.owner = THIS_MODULE,
	},
};
module_platform_driver(globalmem_driver);

MODULE_AUTHOR("Kiwei <jiaweiqing.work@qq.com>");
MODULE_LICENSE("GPL v2");
