#ifndef HWCTL_USB_UTIL_H
#define HWCTL_USB_UTIL_H

#include <libusb-1.0/libusb.h>

char *usb_create_name(libusb_device_handle*);

char *usb_create_desc(libusb_device_handle*);

#endif
