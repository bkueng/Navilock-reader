/*
 * Navilock Serial USB driver
 *
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version
 *	2 as published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/uaccess.h>

#define DRIVER_AUTHOR "Beat Kueng"
#define DRIVER_DESC "navilock serial over usb driver"

#define NAVILOCK_VENDOR_ID				0x1ee4
#define NAVILOCK_PRODUCT_ID				0x0001

#define DRIVER_NAME						"navilock"


static int debug;

static struct usb_device_id id_table [] = {
	{ USB_DEVICE(NAVILOCK_VENDOR_ID, NAVILOCK_PRODUCT_ID) },
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver navilock_driver = {
	.name =		DRIVER_NAME,
	.probe =	usb_serial_probe,
	.disconnect =	usb_serial_disconnect,
	.id_table =	id_table,
	.no_dynamic_id = 	1,
};

static struct usb_serial_driver navilock_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		DRIVER_NAME,
	},
	.id_table =			id_table,
	.usb_driver = 		&navilock_driver,
	.num_ports =		1,
};

static int __init my_driver_init(void) {
	int retval;

	retval = usb_serial_register(&navilock_device);
	if (retval)
		return retval;
	retval = usb_register(&navilock_driver);
	if (retval)
		usb_serial_deregister(&navilock_device);
	
	return retval;
}

static void __exit my_driver_exit(void) {
	usb_deregister(&navilock_driver);
	usb_serial_deregister(&navilock_device);
}

module_init(my_driver_init);
module_exit(my_driver_exit);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");
