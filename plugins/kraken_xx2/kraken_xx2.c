#include <linux/usbdevice_fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <hid_util.h>
#include <hwctl/device.h>
#include <hwctl/plugin.h>
#include <str_util.h>

#define VENDOR_ID 0x1e71
#define PRODUCT_ID 0x170e

enum dev_type {
    DEV,
    SUBDEV
};

struct dev_data {
    enum dev_type type;
    char *id;
    char *desc;
    char *path;
    char *serial_number;
    hid_device *handle;
};

static void dev_data_init(struct dev_data *dev_data) {
    dev_data->type = DEV;
}

static void dev_data_destroy(void *data) {
    struct dev_data *dev_data = data;
    free(dev_data->id);
    free(dev_data->desc);
    free(dev_data->path);
    free(dev_data->serial_number);
    if (dev_data->handle) {
        hid_close(dev_data->handle);
    }
    free(dev_data);
}

struct subdev_data {
    enum dev_type type;
    char *id;
    char *desc;
    struct hwctl_dev *parent;
};

static void subdev_data_init(struct subdev_data *subdev_data) {
    subdev_data->type = SUBDEV;
}

static void subdev_data_destroy(void *data) {
    struct subdev_data *subdev_data = data;
    free(subdev_data->id);
    free(subdev_data->desc);
    free(subdev_data);
}

int hwctl_init_plugin(void) {
    return hid_init();
}

int hwctl_shutdown_plugin(void) {
    return hid_exit();
}

static const char *get_id(const struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->id;
}

static const char *get_desc(const struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->desc;
}

static enum dev_type get_type(const void *data) {
    return *((enum dev_type*) data);
}

static struct hwctl_dev *get_root(struct hwctl_dev *dev) {
    if (get_type(dev->data) == SUBDEV) {
        return get_root(((struct subdev_data*) dev->data)->parent);
    } else {
        return dev;
    }
}

static hid_device *get_handle(struct hwctl_dev *dev) {
    return ((struct dev_data*) get_root(dev)->data)->handle;
}

static int reopen_device(struct dev_data *dev_data) {
    if (dev_data->handle) {
        hid_close(dev_data->handle);
    }
    dev_data->handle = hid_open_path(dev_data->path);
    if (!dev_data->handle) {
        fprintf(stderr, "Failed to open HID device at path %s\n", dev_data->path);
        return 1;
    }
    return 0;
}

static int try_recover(struct dev_data *dev_data, unsigned fail_count) {
    if (fail_count == 1) {
        return reopen_device(dev_data);
    } else if (fail_count == 2) {
        int fd = hid_get_fd(dev_data->path);
        if (fd == -1) {
            if (reopen_device(dev_data)) {
                return 1;
            }
            fd = hid_get_fd(dev_data->path);
            if (fd == -1) {
                fprintf(stderr, "Failed to find file descriptor for HID device at path %s\n", dev_data->path);
                return 1;
            }
        }
        ioctl(fd, USBDEVFS_RESET);
        return reopen_device(dev_data);
    }
    return 1;
}

static int read_report(struct hwctl_dev *dev, unsigned char *buffer) {
    dev = get_root(dev);
    struct dev_data *dev_data = dev->data;

    if (!dev_data->handle) {
        int result = reopen_device(dev_data);
        if (result) {
            return 1;
        }
    }

    unsigned fail_count = 0;
    for (;;) {
        int result = hid_read_timeout(dev_data->handle, buffer, 64, 1000);

        if (result > 0) {
            return 0;
        } else {
            if (try_recover(dev_data, ++fail_count)) {
                return 1;
            }
        }
    }
}

static void base_dev_init(struct hwctl_dev *dev) {
    hwctl_dev_init(dev);
    dev->get_id = &get_id;
    dev->get_desc = &get_desc;
}

static void kraken_dev_init(struct hwctl_dev *dev, const struct hid_device_info *info) {
    base_dev_init(dev);
    dev->destroy_data = &dev_data_destroy;
    struct dev_data *dev_data = malloc(sizeof(struct dev_data));
    dev_data_init(dev_data);
    dev_data->id = hid_create_id(info);
    dev_data->desc = hid_create_desc(info);
    dev_data->path = str_make_copy(info->path);
    dev_data->serial_number = wstr_to_str(info->serial_number);
    dev_data->handle = NULL;
    dev->data = dev_data;
}

static void subdev_init(struct hwctl_dev *dev, const char *id, const char *desc, struct hwctl_dev *parent) {
    base_dev_init(dev);
    dev->destroy_data = &subdev_data_destroy;
    struct subdev_data *subdev_data = malloc(sizeof(struct subdev_data));
    subdev_data_init(subdev_data);
    subdev_data->id = str_make_copy(id);
    subdev_data->desc = str_make_copy(desc);
    subdev_data->parent = parent;
    dev->data = subdev_data;
}

static int read_liquid_temp(struct hwctl_dev *dev, double *temp) {
    unsigned char report[64];
    if (read_report(dev, report)) {
        return 1;
    } else {
        *temp = report[1] + report[2] / 10.;
        return 0;
    }
}

static void init_dev_liquid(struct hwctl_dev *liquid, struct hwctl_dev *parent) {
    subdev_init(liquid, "liquid", "Liquid temperature sensor (read degrees C)", parent);
    liquid->read_sen = &read_liquid_temp;
}

static int read_fan_speed(struct hwctl_dev *dev, double *speed) {
    unsigned char report[64];
    if (read_report(dev, report)) {
        return 1;
    } else {
        *speed = (uint32_t) (report[3] << 8u) | report[4];
        return 0;
    }
}

static int32_t clamp(int32_t min, int32_t val, int32_t max) {
    return val < min ? min : (val > max ? max : val);
}

static int write_duty(struct hwctl_dev *dev, uint8_t type, uint8_t duty) {
    unsigned char msg[65];
    memset(msg, 0, sizeof(msg));
    msg[0] = 0x02;
    msg[1] = 0x4d;
    msg[2] = type;
    msg[4] = duty;

    dev = get_root(dev);
    struct dev_data *dev_data = dev->data;

    if (!dev_data->handle) {
        int result = reopen_device(dev_data);
        if (result) {
            return 1;
        }
    }

    unsigned fail_count = 0;
    for (;;) {
        int result = hid_write(dev_data->handle, msg, sizeof(msg));

        if (result == -1) {
            if (try_recover(dev_data, ++fail_count)) {
                return 1;
            }
        } else {
            return 0;
        }
    }
}

static int write_fan_duty(struct hwctl_dev *dev, double duty) {
    return write_duty(dev, 0x00, (uint8_t) clamp(25, duty, 100));
}

static void init_dev_fan(struct hwctl_dev *fan, struct hwctl_dev *parent) {
    subdev_init(fan, "fan", "Fan (read RPM, write %)", parent);
    fan->read_sen = &read_fan_speed;
    fan->write_act = &write_fan_duty;
}

static int read_pump_speed(struct hwctl_dev *dev, double *speed) {
    unsigned char report[64];
    if (read_report(dev, report)) {
        return 1;
    } else {
        *speed = (uint32_t) (report[5] << 8u) | report[6];
        return 0;
    }
}

static int write_pump_duty(struct hwctl_dev *dev, double duty) {
    double temp;
    if (read_liquid_temp(dev, &temp)) {
        return 1;
    } else if (temp >= 60) {
        duty = 100;
    }
    return write_duty(dev, 0x40, (uint8_t) clamp(60, duty, 100));
}

static void init_dev_pump(struct hwctl_dev *pump, struct hwctl_dev *parent) {
    subdev_init(pump, "pump", "Liquid pump (read RPM, write %)", parent);
    pump->read_sen = &read_pump_speed;
    pump->write_act = &write_pump_duty;
}

static void det_devs(struct vec *devs) {
    struct hid_device_info *enumeration = hid_enumerate(VENDOR_ID, PRODUCT_ID);

    for (struct hid_device_info *info = enumeration; info; info = info->next) {
        struct hwctl_dev *dev = vec_push_back(devs);
        kraken_dev_init(dev, info);

        init_dev_liquid(vec_push_back(dev->subdevs), dev);
        init_dev_fan(vec_push_back(dev->subdevs), dev);
        init_dev_pump(vec_push_back(dev->subdevs), dev);
    }

    hid_free_enumeration(enumeration);
}

void hwctl_init_dev_det(struct hwctl_dev_det* dev_det) {
    dev_det->det_devs = &det_devs;
}
