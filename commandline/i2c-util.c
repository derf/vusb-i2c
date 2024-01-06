/*
 * Copyright (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 *       and (c) 2016 by Birte Friesel
 * License: GNU GPL v2
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>		/* this is libusb, see http://libusb.sourceforge.net/ */

#include "i2c-util.h"
#include "../global_config.h"

#define USBDEV_SHARED_VENDOR    0x16C0	/* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC	/* Obdev's free shared PID */
/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */

unsigned char bit_sda = 6;
unsigned char bit_scl = 7;

unsigned char val_sda = 1;
unsigned char val_scl = 1;

usb_dev_handle *handle = NULL;

static int usbGetStringAscii(usb_dev_handle * dev, int index, int langid, char
		*buf, int buflen)
{
	char buffer[256];
	int rval, i;

	if ((rval =
	     usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
			     (USB_DT_STRING << 8) + index, langid, buffer,
			     sizeof(buffer), 1000)) < 0)
		return rval;
	if (buffer[1] != USB_DT_STRING)
		return 0;
	if ((unsigned char)buffer[0] < rval)
		rval = (unsigned char)buffer[0];
	rval /= 2;
	/* lossy conversion to ISO Latin1 */
	for (i = 1; i < rval; i++) {
		if (i > buflen)	/* destination buffer overflow */
			break;
		buf[i - 1] = buffer[2 * i];
		if (buffer[2 * i + 1] != 0)	/* outside of ISO Latin1 range */
			buf[i - 1] = '?';
	}
	buf[i - 1] = 0;
	return i - 1;
}

/* PowerSwitch uses the free shared default VID/PID. If you want to see an
 * example device lookup where an individually reserved PID is used, see our
 * RemoteSensor reference implementation.
 */

#define USB_ERROR_NOTFOUND  1
#define USB_ERROR_ACCESS    2
#define USB_ERROR_IO        3

static int usbOpenDevice(usb_dev_handle ** device, int vendor, char *vendorName,
			 int product, char *productName, int upperversion,
			 int lowerversion)
{
	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *handle = NULL;
	int errorCode = USB_ERROR_NOTFOUND;
	int major, minor;
	static int didUsbInit = 0;

	if (!didUsbInit) {
		didUsbInit = 1;
		usb_init();
	}
	usb_find_busses();
	usb_find_devices();
	for (bus = usb_get_busses(); bus; bus = bus->next) {
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == vendor
			    && dev->descriptor.idProduct == product) {
				char string[256];
				int len;
				handle = usb_open(dev);	/* we need to open the device in order to query strings */
				if (!handle) {
					errorCode = USB_ERROR_ACCESS;
					fprintf(stderr,
						"Warning: cannot open USB device: %s\n",
						usb_strerror());
					continue;
				}
				if (vendorName == NULL && productName == NULL) {	/* name does not matter */
					break;
				}
				/* now check whether the names match: */
				len =
				    usbGetStringAscii(handle,
						      dev->descriptor.iManufacturer, 0x0409,
						      string, sizeof(string));
				if (len < 0) {
					errorCode = USB_ERROR_IO;
					fprintf(stderr,
						"Warning: cannot query manufacturer for device: %s\n",
						usb_strerror());
				} else {
					errorCode = USB_ERROR_NOTFOUND;
					/* fprintf(stderr, "seen device from vendor ->%s<-\n", string); */
					if (strcmp(string, vendorName) == 0) {
						len =
						    usbGetStringAscii(handle,
								      dev->descriptor.iProduct,
								      0x0409,
								      string, sizeof(string));
						if (len < 0) {
							errorCode = USB_ERROR_IO;
							fprintf(stderr,
								"Warning: cannot query product for device: %s\n",
								usb_strerror());
						} else {
							errorCode = USB_ERROR_NOTFOUND;
							/* fprintf(stderr, "seen product ->%s<-\n", string); */
							if (strcmp(string, productName) == 0)
								break;
						}
					}
				}
				usb_close(handle);
				handle = NULL;
			}
		}
		if (handle)
			break;
	}
	if (handle != NULL) {
		errorCode = 0;
		*device = handle;

		minor = dev->descriptor.bcdDevice & 0xff;
		major = dev->descriptor.bcdDevice >> 8;

		if (major != USBDEV_VERSION_MAJOR) {
			fprintf(stderr,
					"Error: Firmware and host software version are not compatible\n"
					"Firmware: %2d.%02d\n    Host: %2d.%02d\n"
					"Please upgrade the firmware or downgrade the CLI\n",
					major, minor, USBDEV_VERSION_MAJOR, USBDEV_VERSION_MINOR);
			errorCode = -1;
		}
		else if (minor != USBDEV_VERSION_MINOR) {
			fprintf(stderr,
					"Note: Firmware and host software version may be incompatible\n"
					"Firmware: %2d.%02d\n    Host: %2d.%02d\n"
					"Please upgrade the firmware or downgrade the CLI\n",
					major, minor, USBDEV_VERSION_MAJOR, USBDEV_VERSION_MINOR);
		}
	}
	return errorCode;
}

void i2c_getopt(int argc, char **argv)
{
	int opt;
	while ((opt = getopt(argc, argv, "c:d:")) != EOF) {
		switch (opt) {
		case 'c':
			bit_scl = atoi(optarg);
			break;
		case 'd':
			bit_sda = atoi(optarg);
			break;
		}
	}
}

unsigned char get_status()
{
	unsigned char buffer[8];
	int nBytes = usb_control_msg(handle,
				     USB_TYPE_VENDOR | USB_RECIP_DEVICE |
				     USB_ENDPOINT_IN, USBCMD_GETPORT, 0, 0, (char *)buffer,
				     sizeof(buffer), 5000);

	if (nBytes < 1) {
		fprintf(stderr, "ERR: read status: got %d bytes, expected 1\n",
				nBytes);
		exit(1);
	}
	return buffer[0];
}

static void verify_low(unsigned char bit, char *bitname)
{
	int i;
	for (i = 0; i < 10; i++) {
		if (~get_status() & (1 << bit))
			return;
		usleep(10);
	}
	fprintf(stderr, "%s stuck high for >1ms\n", bitname);
}

static void verify_high(unsigned char bit, char *bitname)
{
	int i;
	for (i = 0; i < 100; i++) {
		if (get_status() & (1 << bit))
			return;
		usleep(10);
	}
	fprintf(stderr, "%s stuck low for >1ms\n", bitname);
}

void verify_sda_low()
{
	verify_low(bit_sda, "SDA");
}

void verify_sda_high()
{
	verify_high(bit_sda, "SDA");
}

void verify_scl_low()
{
	verify_low(bit_scl, "SCL");
}

void verify_scl_high()
{
	verify_high(bit_scl, "SCL");
}

/*
 * Firmware: DDRB  = ~b  (with hardware pull-ups and PORTB = 0)
 * So to pull an output high (1), we turn it on, which sets it as input
 * -> pull-up works. for low (0): turn it off -> output -> pulled low
 */

void set_sda(char value)
{
	// discarded
	unsigned char buffer[8];

	val_sda = value;

	usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_SETPORT, 0, (val_sda << bit_sda) | (val_scl << bit_scl),
			(char *)buffer, sizeof(buffer), 5000);
}

void set_scl(char value)
{
	// discarded
	unsigned char buffer[8];

	val_scl = value;

	usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_SETPORT, 0, (val_sda << bit_sda) | (val_scl << bit_scl),
			(char *)buffer, sizeof(buffer), 5000);
}

unsigned char i2c_tx_byte(unsigned char byte)
{
	signed char i;
	unsigned char ack = 0;

	for (i = 7; i >= -1; i--) {
		if ((i < 0) || (byte & (1 << i))) {
			set_sda(1);
		}
		else {
			set_sda(0);
		}
		usleep(100);
		set_scl(1);
		usleep(100);
		if (i < 0) {
			if (get_status() & (1 << bit_sda))
				ack = 0;
			else
				ack = 1;
		}
		set_scl(0);
		usleep(100);
	}

	return ack;
}

unsigned char i2c_hw_tx_byte(unsigned char byte)
{
	unsigned char buffer[8];

	int nBytes = usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_TX, 0, byte,
			(char *)buffer, sizeof(buffer), 5000);

	if (nBytes < 1) {
		fprintf(stderr, "ERR: tx: got %d bytes, expected 1\n",
				nBytes);
		exit(1);
	}

	return buffer[0];
}

unsigned char i2c_rx_byte(unsigned char send_ack)
{
	signed char i;
	unsigned char ret = 0;

	set_sda(1);
	for (i = 7; i >= -1; i--) {
		if (( i < 0) && send_ack)
			set_sda(0);
		set_scl(1);
		usleep(100);
		if ((i >= 0) && ( get_status() & (1 << bit_sda)))
			ret |= (1 << i);
		if (( i < 0) && send_ack)
			set_sda(1);
		set_scl(0);
		usleep(100);
	}

	return ret;
}

unsigned char i2c_hw_rx_byte(unsigned char send_ack)
{
	unsigned char buffer[8];

	int nBytes = usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_RX, 0, send_ack,
			(char *)buffer, sizeof(buffer), 5000);

	if (nBytes < 1) {
		fprintf(stderr, "ERR: tx: got %d bytes, expected 1\n",
				nBytes);
		exit(1);
	}

	return buffer[0];
}

void i2c_start()
{
	set_sda(1);
	set_scl(1);
	usleep(1000);
	set_sda(0);
	usleep(1000);
	set_scl(0);
	verify_sda_low();
	verify_scl_low();
}

void i2c_hw_start()
{
	// discarded
	unsigned char buffer[8];

	usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_START, 0, 0,
			(char *)buffer, sizeof(buffer), 5000);
}

void i2c_stop()
{
	set_scl(1);
	usleep(100);
	verify_scl_high();
	set_sda(1);
	usleep(100);
	verify_sda_high();
}

void i2c_hw_stop()
{
	// discarded
	unsigned char buffer[8];

	usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_STOP, 0, 0,
			(char *)buffer, sizeof(buffer), 5000);
}

void i2c_init()
{
	usb_init();
	if (usbOpenDevice
	    (&handle, USBDEV_SHARED_VENDOR, "finalrewind.org",
	     USBDEV_SHARED_PRODUCT, "VUSB-I2C", 0, 1) != 0) {
		fprintf(stderr,
			"Could not find USB device \"VUSB-I2C\" with vid=0x%x pid=0x%x"
			" version~%d.%02d\n",
			USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT,
			USBDEV_VERSION_MAJOR, USBDEV_VERSION_MINOR);
		exit(1);
	}
	i2c_hw_setbits();
}

void i2c_hw_setbits()
{
	// discarded
	unsigned char buffer[8];

	usb_control_msg(handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
			USBCMD_SETBITS, 0, ((1 << bit_scl) << 8) | (1 << bit_sda),
			(char *)buffer, sizeof(buffer), 5000);
}

void i2c_deinit()
{
	usb_close(handle);
}
