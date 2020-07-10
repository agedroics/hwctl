#pragma once
#include <libusb-1.0/libusb.h>

char *usb_create_id(libusb_device_handle*);

char *usb_create_desc(libusb_device_handle*);
