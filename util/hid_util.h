#pragma once
#include <hidapi/hidapi.h>

char *hid_create_id(const struct hid_device_info*);

char *hid_create_desc(const struct hid_device_info*);

int hid_get_fd(const char *path);
