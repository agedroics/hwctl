#pragma once
#include <hidapi/hidapi.h>

char *hid_create_id(struct hid_device_info *info);

char *hid_create_desc(struct hid_device_info *info);
