#include <stdlib.h>
#include <stdio.h>
#include <hid_util.h>
#include <str_util.h>

char *hid_create_id(struct hid_device_info *info) {
    return str_concat(2, "HID-", info->path);
}

char *hid_create_desc(struct hid_device_info *info) {
    char *manufacturer = NULL;
    if (info->manufacturer_string) {
        manufacturer = wstr_to_str(info->manufacturer_string);
    }

    char *product = NULL;
    if (info->product_string) {
        product = wstr_to_str(info->product_string);
    }

    int len = 0;
    if (manufacturer && manufacturer[0]) {
        len += snprintf(NULL, 0, "%s [%#04x] - ", manufacturer, info->vendor_id);
    } else {
        len += snprintf(NULL, 0, "[%#04x] - ", info->vendor_id);
    }
    if (product && product[0]) {
        len += snprintf(NULL, 0, "%s [%#04x]", product, info->product_id);
    } else {
        len += snprintf(NULL, 0, "[%#04x]", info->product_id);
    }

    char *str_desc = malloc(len + 1);
    len = 0;
    if (manufacturer && manufacturer[0]) {
        len += sprintf(str_desc, "%s [%#04x] - ", manufacturer, info->vendor_id);
    } else {
        len += sprintf(str_desc, "[%#04x] - ", info->vendor_id);
    }
    if (product && product[0]) {
        sprintf(str_desc + len, "%s [%#04x]", product, info->product_id);
    } else {
        sprintf(str_desc + len, "[%#04x]", info->product_id);
    }
    
    return str_desc;
}
