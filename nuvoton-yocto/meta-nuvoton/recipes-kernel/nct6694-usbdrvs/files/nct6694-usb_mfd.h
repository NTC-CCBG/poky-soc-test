// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Nuvoton Technology Corp.
 */

#define NCT6694_VENDOR_ID		0x0416
#define NCT6694_PRODUCT_ID		0x200B
#define INT_IN_ENDPOINT			0x81
#define BULK_IN_ENDPOINT		0x82
#define BULK_OUT_ENDPOINT		0x03
#define MAX_PACKET_SZ			0x100

#define CMD_PACKET_SZ			0x8
#define HCTRL_SET				0x40
#define HCTRL_GET				0x80

#define REQUEST_MOD_IDX			0x01
#define REQUEST_CMD_IDX			0x02
#define REQUEST_SEL_IDX			0x03
#define REQUEST_HCTRL_IDX		0x04
#define REQUEST_LEN_L_IDX		0x06
#define REQUEST_LEN_H_IDX		0x07

#define RESPONSE_STS_IDX		0x01

#define MSG_PORT_IDX			0x00
#define MSG_BR_IDX				0x01
#define MSG_ADDR_IDX			0x02
#define MSG_W_CNT_IDX			0x03
#define MSG_R_CNT_IDX			0x04

#define MSG_I2C_RD_IDX			0x50
#define MSG_I2C_WR_IDX			0x10
#define MSG_CAN_RD_IDX			0x08
#define MSG_CAN_WR_IDX			0x08

#define GPIO_IRQ_STATUS			BIT(0)
#define CAN_IRQ_STATUS			BIT(2)

#define URB_TIMEOUT				0

struct nct6694
{
	struct usb_device *udev;
	struct urb *int_in_urb;
	struct mutex access_lock;
	struct workqueue_struct *async_workqueue;
	struct work_struct async_work;

	unsigned char *tx_buffer;
	unsigned char *rx_buffer;
    unsigned char *cmd_buffer;
    unsigned char *int_buffer;
	unsigned char err_status;

	u8 MOD;
	u16 OFFSET;
	u16 LEN;
	unsigned char *buf;
	int cmd_cnt;
	long timeout;

	void (*can_handler)(void);
	void (*gpio_hanlder)(void);
};

