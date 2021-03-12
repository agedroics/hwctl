#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <str_util.h>
#include <hid_util.h>

#define FD_DIR "/proc/self/fd"

char *hid_create_id(const struct hid_device_info *info) {
    char *serial_number = wstr_to_str(info->serial_number);
    char *id = malloc(15 + strlen(serial_number));
    sprintf(id, "HID-%04x:%04x-%s", info->vendor_id, info->product_id, serial_number);
    free(serial_number);
    return id;
}

char *hid_create_desc(const struct hid_device_info *info) {
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

int hid_get_fd(const char *path) {
    unsigned bus_number;
    unsigned dev_number;
    sscanf(path, "%04u:%04u", &bus_number, &dev_number);

    char fs_path[21];
    sprintf(fs_path, "/dev/bus/usb/%03u/%03u", bus_number, dev_number);

    int fd = -1;
    DIR *dir = opendir(FD_DIR);
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type != DT_LNK) {
                continue;
            }
            char *full_path = str_concat(3, FD_DIR, "/", ent->d_name);
            char *real_path = realpath(full_path, NULL);
            int found = 0;
            if (real_path) {
                found = !strcmp(fs_path, real_path);
                free(real_path);
            }
            free(full_path);
            if (found) {
                fd = atoi(ent->d_name);
                break;
            }
        }
        closedir(dir);
    }
    return fd;
}
