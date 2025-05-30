// SPDX-License-Identifier: GPL-2.0
/*
 * Author(s)......: Holger Smolinski <Holger.Smolinski@de.ibm.com>
 *		    Horst Hummel <Horst.Hummel@de.ibm.com>
 *		    Carsten Otte <Cotte@de.ibm.com>
 *		    Martin Schwidefsky <schwidefsky@de.ibm.com>
 * Bugreports.to..: <Linux390@de.ibm.com>
 * Copyright IBM Corp. 1999, 2001
 *
 * gendisk related functions for the dasd driver.
 *
 */

#define KMSG_COMPONENT "dasd"

#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/blkpg.h>

#include <linux/uaccess.h>

/* This is ugly... */
#define PRINTK_HEADER "dasd_gendisk:"

#include "dasd_int.h"

static struct lock_class_key dasd_bio_compl_lkclass;

/*
 * Allocate and register gendisk structure for device.
 */
int dasd_gendisk_alloc(struct dasd_block *block)
{
	struct gendisk *gdp;
	struct dasd_device *base;
	int len, rc;

	/* Make sure the minor for this device exists. */
	base = block->base;
	if (base->devindex >= DASD_PER_MAJOR)
		return -EBUSY;

	gdp = blk_mq_alloc_disk_for_queue(block->request_queue,
					  &dasd_bio_compl_lkclass);
	if (!gdp)
		return -ENOMEM;

	/* Initialize gendisk structure. */
	gdp->major = DASD_MAJOR;
	gdp->first_minor = base->devindex << DASD_PARTN_BITS;
	gdp->minors = 1 << DASD_PARTN_BITS;
	gdp->fops = &dasd_device_operations;

	/*
	 * Set device name.
	 *   dasda - dasdz : 26 devices
	 *   dasdaa - dasdzz : 676 devices, added up = 702
	 *   dasdaaa - dasdzzz : 17576 devices, added up = 18278
	 *   dasdaaaa - dasdzzzz : 456976 devices, added up = 475252
	 */
	len = sprintf(gdp->disk_name, "dasd");
	if (base->devindex > 25) {
		if (base->devindex > 701) {
			if (base->devindex > 18277)
			        len += sprintf(gdp->disk_name + len, "%c",
					       'a'+(((base->devindex-18278)
						     /17576)%26));
			len += sprintf(gdp->disk_name + len, "%c",
				       'a'+(((base->devindex-702)/676)%26));
		}
		len += sprintf(gdp->disk_name + len, "%c",
			       'a'+(((base->devindex-26)/26)%26));
	}
	len += sprintf(gdp->disk_name + len, "%c", 'a'+(base->devindex%26));

	if (base->features & DASD_FEATURE_READONLY ||
	    test_bit(DASD_FLAG_DEVICE_RO, &base->flags))
		set_disk_ro(gdp, 1);
	dasd_add_link_to_gendisk(gdp, base);
	block->gdp = gdp;
	set_capacity(block->gdp, 0);

	rc = device_add_disk(&base->cdev->dev, block->gdp, NULL);
	if (rc) {
		dasd_gendisk_free(block);
		return rc;
	}

	return 0;
}

/*
 * Unregister and free gendisk structure for device.
 */
void dasd_gendisk_free(struct dasd_block *block)
{
	if (block->gdp) {
		del_gendisk(block->gdp);
		block->gdp->private_data = NULL;
		put_disk(block->gdp);
		block->gdp = NULL;
	}
}

/*
 * Trigger a partition detection.
 */
int dasd_scan_partitions(struct dasd_block *block)
{
	struct file *bdev_file;
	int rc;

	bdev_file = bdev_file_open_by_dev(disk_devt(block->gdp), BLK_OPEN_READ,
				       NULL, NULL);
	if (IS_ERR(bdev_file)) {
		DBF_DEV_EVENT(DBF_ERR, block->base,
			      "scan partitions error, blkdev_get returned %ld",
			      PTR_ERR(bdev_file));
		return -ENODEV;
	}

	mutex_lock(&block->gdp->open_mutex);
	rc = bdev_disk_changed(block->gdp, false);
	mutex_unlock(&block->gdp->open_mutex);
	if (rc)
		DBF_DEV_EVENT(DBF_ERR, block->base,
				"scan partitions error, rc %d", rc);

	/*
	 * Since the matching fput() call to the
	 * bdev_file_open_by_path() in this function is not called before
	 * dasd_destroy_partitions the offline open_count limit needs to be
	 * increased from 0 to 1. This is done by setting device->bdev_file
	 * (see dasd_generic_set_offline). As long as the partition detection
	 * is running no offline should be allowed. That is why the assignment
	 * to block->bdev_file is done AFTER the BLKRRPART ioctl.
	 */
	block->bdev_file = bdev_file;
	return 0;
}

/*
 * Remove all inodes in the system for a device, delete the
 * partitions and make device unusable by setting its size to zero.
 */
void dasd_destroy_partitions(struct dasd_block *block)
{
	struct file *bdev_file;

	/*
	 * Get the bdev_file pointer from the device structure and clear
	 * device->bdev_file to lower the offline open_count limit again.
	 */
	bdev_file = block->bdev_file;
	block->bdev_file = NULL;

	mutex_lock(&file_bdev(bdev_file)->bd_disk->open_mutex);
	bdev_disk_changed(file_bdev(bdev_file)->bd_disk, true);
	mutex_unlock(&file_bdev(bdev_file)->bd_disk->open_mutex);

	/* Matching blkdev_put to the blkdev_get in dasd_scan_partitions. */
	fput(bdev_file);
}

int dasd_gendisk_init(void)
{
	int rc;

	/* Register to static dasd major 94 */
	rc = register_blkdev(DASD_MAJOR, "dasd");
	if (rc != 0) {
		pr_warn("Registering the device driver with major number %d failed\n",
			DASD_MAJOR);
		return rc;
	}
	return 0;
}

void dasd_gendisk_exit(void)
{
	unregister_blkdev(DASD_MAJOR, "dasd");
}
