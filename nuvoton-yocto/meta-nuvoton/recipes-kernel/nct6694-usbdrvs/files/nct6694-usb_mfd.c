// SPDX-License-Identifier: GPL-2.0+
/*
 * Nuvoton NCT6694 MFD driver based on USB interface.
 *
 * Copyright (C) 2024 Nuvoton Technology Corp.
 */

#include <linux/io.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mfd/core.h>
#include "nct6694-usb_mfd.h"

#define DRVNAME "nct6694-usb_mfd"

/* MFD device resources */
static const struct resource gpio_resources[] = {
	{
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_AUTO,
	}
};

static const struct mfd_cell nct6694_dev[] = {
	{
		.name = "nct6694-gpio",
		.num_resources = ARRAY_SIZE(gpio_resources),
		.resources = &gpio_resources[0],
	},
	{
		.name = "nct6694-i2c"
	},
	{
		.name = "nct6694-canfd"
	},
	{
		.name = "nct6694-wdt"
	},
};

/* 
 * Get usb command packet
 * - Read the data which firmware provides.
 *   - Packet format: 
 *
 * 	       OUT	|RSV|MOD|CMD|SEL|HCTL|RSV|LEN_L|LEN_H|			 
 * 		   OUT	|SEQ|STS|RSV|RSV|RSV|RSV||LEN_L|LEN_H|
 * 		   IN	|-------D------A------D------A-------|
 * 				|-------D------A------D------A-------|
 */
int nct6694_getusb(struct nct6694 *chip, u8 MOD, u16 OFFSET, u16 LEN, u8 rd_idx, u8 rd_len, unsigned char *buf)
{
	int ret;
	int tx_len, rx_len;
	struct usb_device *udev = chip->udev;
	int i = 0;
	mutex_lock(&chip->access_lock);

	chip->cmd_buffer[REQUEST_MOD_IDX] = MOD;
	chip->cmd_buffer[REQUEST_CMD_IDX] = OFFSET & 0xFF;
	chip->cmd_buffer[REQUEST_SEL_IDX] = (OFFSET >> 8) & 0xFF;
	chip->cmd_buffer[REQUEST_HCTRL_IDX] = HCTRL_GET;
	chip->cmd_buffer[REQUEST_LEN_L_IDX] = LEN & 0xFF;
	chip->cmd_buffer[REQUEST_LEN_H_IDX] = (LEN >> 8) & 0xFF;

	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, BULK_OUT_ENDPOINT), chip->cmd_buffer, CMD_PACKET_SZ, &tx_len, chip->timeout);
	if (ret)
		goto err;

	ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, CMD_PACKET_SZ, &rx_len, chip->timeout);
	if (ret)
		goto err;

	if (chip->rx_buffer[RESPONSE_STS_IDX]) {
		pr_debug("%s: MSG CH status = %2Xh\n", __func__, chip->rx_buffer[RESPONSE_STS_IDX]);
		ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, LEN, &rx_len, chip->timeout);
		if (ret)
			goto err;

		mutex_unlock(&chip->access_lock);
		return -1;
	}

	ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, LEN, &rx_len, chip->timeout);
	if (ret)
		goto err;

	for(i = 0; i < rd_len; i++)
		buf[i] = chip->rx_buffer[i + rd_idx];

err:
	mutex_unlock(&chip->access_lock);
	return ret;
}

EXPORT_SYMBOL(nct6694_getusb);

/* 
 * Set usb command packet
 * - Read the data which firmware provides.
 *   - Packet format: 
 *
 * 	       OUT	|RSV|MOD|CMD|SEL|HCTL|RSV|LEN_L|LEN_H|			 
 * 		   OUT	|-------D------A------D------A-------|
 * 		   IN	|SEQ|STS|RSV|RSV|RSV|RSV||LEN_L|LEN_H|
 * 		   IN   |-------D------A------D------A-------|
 * 				|-------D------A------D------A-------|
 */
int nct6694_setusb_rdata(struct nct6694 *chip, u8 MOD, u16 OFFSET, u16 LEN, u8 rd_idx, u8 rd_len, unsigned char *buf)
{
	int ret;
	int tx_len, rx_len;
	struct usb_device *udev = chip->udev;
	int i = 0;
	mutex_lock(&chip->access_lock);

	chip->cmd_buffer[REQUEST_MOD_IDX] = MOD;
	chip->cmd_buffer[REQUEST_CMD_IDX] = OFFSET & 0xFF;
	chip->cmd_buffer[REQUEST_SEL_IDX] = (OFFSET >> 8) & 0xFF;
	chip->cmd_buffer[REQUEST_HCTRL_IDX] = HCTRL_SET;
	chip->cmd_buffer[REQUEST_LEN_L_IDX] = LEN & 0xFF;
	chip->cmd_buffer[REQUEST_LEN_H_IDX] = (LEN >> 8) & 0xFF;

	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, BULK_OUT_ENDPOINT), chip->cmd_buffer, CMD_PACKET_SZ, &tx_len, chip->timeout);
	if (ret)
		goto err;

	memcpy(chip->tx_buffer, buf, LEN);

	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, BULK_OUT_ENDPOINT), chip->tx_buffer, LEN, &tx_len, chip->timeout);
	if (ret)
		goto err;

	ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, CMD_PACKET_SZ, &rx_len, chip->timeout);
	if (ret)
		goto err;

	if (chip->rx_buffer[RESPONSE_STS_IDX]) {
		pr_debug("%s: MSG CH status = %2Xh\n", __func__, chip->rx_buffer[RESPONSE_STS_IDX]);
		ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, LEN, &rx_len, chip->timeout);
		if (ret)
			goto err;

		mutex_unlock(&chip->access_lock);
		return -1;
	}

	ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, LEN, &rx_len, chip->timeout);
	if (ret)
		goto err;

	for(i = 0; i < rd_len; i++)
		buf[i] = chip->rx_buffer[i + rd_idx];

err:
	mutex_unlock(&chip->access_lock);
	return ret;
}

EXPORT_SYMBOL(nct6694_setusb_rdata);

/* 
 * Set usb command packet
 * - Write data to firmware.
 *   - Packet format: 
 *
 * 	       OUT	|RSV|MOD|CMD|SEL|HCTL|RSV|LEN_L|LEN_H|			 
 * 		   OUT	|-------D------A------D------A-------|
 * 		   IN	|SEQ|STS|RSV|RSV|RSV|RSV||LEN_L|LEN_H|
 * 		   IN   |-------D------A------D------A-------|
 * 				|-------D------A------D------A-------|
 */
int nct6694_setusb_wdata(struct nct6694 *chip, u8 MOD, u16 OFFSET, u16 LEN, unsigned char *buf)
{
	int ret;
	int tx_len, rx_len;
	struct usb_device *udev = chip->udev;

	mutex_lock(&chip->access_lock);

	chip->cmd_buffer[REQUEST_MOD_IDX] = MOD;
	chip->cmd_buffer[REQUEST_CMD_IDX] = OFFSET & 0xFF;
	chip->cmd_buffer[REQUEST_SEL_IDX] = (OFFSET >> 8) & 0xFF;
	chip->cmd_buffer[REQUEST_HCTRL_IDX] = HCTRL_SET;
	chip->cmd_buffer[REQUEST_LEN_L_IDX] = LEN & 0xFF;
	chip->cmd_buffer[REQUEST_LEN_H_IDX] = (LEN >> 8) & 0xFF;
	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, BULK_OUT_ENDPOINT), chip->cmd_buffer, CMD_PACKET_SZ, &tx_len, chip->timeout);
	if (ret)
		return ret;

	memcpy(chip->tx_buffer, buf, LEN);

	ret = usb_bulk_msg(udev, usb_sndbulkpipe(udev, BULK_OUT_ENDPOINT), chip->tx_buffer, LEN, &tx_len, chip->timeout);
	if (ret)
		goto err;

	ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, CMD_PACKET_SZ, &rx_len, chip->timeout);
	if (ret)
		goto err;

	if (chip->rx_buffer[RESPONSE_STS_IDX]) {
		pr_debug("%s: MSG CH status = %2Xh\n", __func__, chip->rx_buffer[RESPONSE_STS_IDX]);
		ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, LEN, &rx_len, chip->timeout);
		if (ret)
			goto err;

		mutex_unlock(&chip->access_lock);
		return -1;
	}

	ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, BULK_IN_ENDPOINT), chip->rx_buffer, LEN, &rx_len, chip->timeout);
	if (ret)
		goto err;

err:
	mutex_unlock(&chip->access_lock);
	return ret;
}

EXPORT_SYMBOL(nct6694_setusb_wdata);

static void setusb_work(struct work_struct *async_work)
{
    struct nct6694 *chip = container_of(async_work, struct nct6694, async_work);
	int ret;
	ret = nct6694_setusb_wdata(chip, chip->MOD, chip->OFFSET, chip->LEN, chip->buf);
}

/*
 * Set usb command packet
 * - Write data to firmware.
 *   - Packet format: 
 *
 * 	       OUT	|RSV|MOD|CMD|SEL|HCTL|RSV|LEN_L|LEN_H|			 
 * 		   OUT	|-------D------A------D------A-------|
 * 		   IN	|SEQ|STS|RSV|RSV|RSV|RSV||LEN_L|LEN_H|
 * 		   IN   |-------D------A------D------A-------|
 * 				|-------D------A------D------A-------|
 */
void nct6694_setusb_async(struct nct6694 *chip, u8 MOD, u16 OFFSET, u16 LEN, unsigned char *buf)
{
	chip->MOD = MOD;
	chip->OFFSET = OFFSET;
	chip->LEN = LEN;
	chip->buf = buf;
    queue_work(chip->async_workqueue, &chip->async_work);
}

EXPORT_SYMBOL(nct6694_setusb_async);

static void usb_int_callback(struct urb *urb)
{
	unsigned char *int_status = urb->transfer_buffer;
    struct nct6694 *chip = urb->context;
	int ret;

	switch (urb->status) {
	case 0:
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	default:
		goto resubmit;
	}

	while (int_status[0]) {
		if (int_status[0] & 0x1) {
			if (chip->gpio_hanlder)
				chip->gpio_hanlder();
			else
				pr_err("The GPIO interrupt handler is unattached!");
			int_status[0] &= ~0x01;
		}
		else if (int_status[0] & 0x04) {
			if (chip->can_handler)
				chip->can_handler();
			else
				pr_err("The CAN interrupt handler is unattached!");
			int_status[0] &= ~0x04;
		} else {
			pr_err("Unsupported irq status! :%d", int_status[0]);
			break;
		}
	}

resubmit:
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret)
		pr_err("%s: Failed to resubmit urb, status %d", __func__, ret);
}

int nct6694_usb_probe(struct usb_interface *iface,
						 const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(iface);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *int_endpoint;
	struct nct6694 *chip;
	int ret = EINVAL;
	int pipe, maxp;

	interface = iface->cur_altsetting;
	/* Binding interface class : 0xFF */
	if (interface->desc.bInterfaceClass != USB_CLASS_VENDOR_SPEC ||
		interface->desc.bInterfaceSubClass != 0x00 ||
		interface->desc.bInterfaceProtocol != 0x00)
		return -ENODEV;

	int_endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(int_endpoint))
		return -ENODEV;

	chip = kzalloc(sizeof(struct nct6694), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	pipe = usb_rcvintpipe(udev, INT_IN_ENDPOINT);
	maxp = 8;

	if (!(chip->cmd_buffer = kzalloc(sizeof(unsigned char) * CMD_PACKET_SZ, GFP_KERNEL)))
		goto fail1;
	if (!(chip->rx_buffer = kzalloc(sizeof(unsigned char) * MAX_PACKET_SZ, GFP_KERNEL)))
		goto fail2;
	if (!(chip->tx_buffer = kzalloc(sizeof(unsigned char) * MAX_PACKET_SZ, GFP_KERNEL)))
		goto fail3;
	if (!(chip->int_buffer = kzalloc(sizeof(unsigned char) * maxp, GFP_KERNEL)))
		goto fail4;

	if (!(chip->int_in_urb = usb_alloc_urb(0, GFP_KERNEL)))
		goto fail5;

	mutex_init(&chip->access_lock);
	chip->udev = udev;
	chip->timeout = URB_TIMEOUT;	/* Wait until urb complete */

	usb_fill_int_urb(chip->int_in_urb, udev, pipe,
					 chip->int_buffer, maxp, usb_int_callback, chip, int_endpoint->bInterval);
	ret = usb_submit_urb(chip->int_in_urb, GFP_KERNEL);
	if (ret)
		goto fail6;

	dev_set_drvdata(&udev->dev, chip);
	usb_set_intfdata(iface, chip);

	ret = mfd_add_devices(&udev->dev, -1, nct6694_dev, ARRAY_SIZE(nct6694_dev), NULL, 0, NULL);
	if (ret) {
		dev_err(&udev->dev, "Failed to add gpio child\n");
		goto fail7;
	}

	INIT_WORK(&chip->async_work, setusb_work);

	chip->async_workqueue = alloc_ordered_workqueue("asyn_workqueue", 0);

	dev_info(&udev->dev, "Probe device: (%04X:%04X)\n", id->idVendor, id->idProduct);
	return 0;

fail7:
	usb_kill_urb(chip->int_in_urb);
fail6:
	usb_free_urb(chip->int_in_urb);
fail5:
	kfree(chip->int_buffer);
fail4:
	kfree(chip->tx_buffer);
fail3:
	kfree(chip->rx_buffer);
fail2:
	kfree(chip->cmd_buffer);
fail1:
	kfree(chip);
	
	return ret;
}

static void nct6694_usb_disconnect(struct usb_interface *iface)
{
	struct usb_device *udev = interface_to_usbdev(iface);
	struct nct6694 *chip = usb_get_intfdata(iface);
	int ret;
	dev_info(&udev->dev, "%s\n", __FUNCTION__);
	mfd_remove_devices(&udev->dev);
	ret = cancel_work_sync(&chip->async_work);
	flush_workqueue(chip->async_workqueue);
	destroy_workqueue(chip->async_workqueue);
	usb_set_intfdata(iface, NULL);
	usb_kill_urb(chip->int_in_urb);
	usb_free_urb(chip->int_in_urb);
	kfree(chip->int_buffer);
	kfree(chip->tx_buffer);
	kfree(chip->rx_buffer);
	kfree(chip->cmd_buffer);
	kfree(chip);
}

static const struct usb_device_id nct6694_ids[] = {
	{ USB_DEVICE(NCT6694_VENDOR_ID, NCT6694_PRODUCT_ID)},
	{},
};
MODULE_DEVICE_TABLE(usb, nct6694_ids);

static struct usb_driver nct6694_usb_driver = {
	.name	= DRVNAME,
	.id_table = nct6694_ids,
	.probe = nct6694_usb_probe,
	.disconnect = nct6694_usb_disconnect,
};

module_usb_driver(nct6694_usb_driver);

MODULE_DESCRIPTION("USB-MFD driver for NCT6694");
MODULE_AUTHOR("Tzu-Ming Yu <tmyu0@nuvoton.com>");
MODULE_LICENSE("GPL");
