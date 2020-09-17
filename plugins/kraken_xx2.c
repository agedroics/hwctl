#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <hid_util.h>
#include <hwctl/device.h>
#include <hwctl/plugin.h>
#include <str_util.h>
#include <time_util.h>

#define VENDOR_ID 0x1e71
#define PRODUCT_ID 0x170e

// min time between reads/writes
static struct timespec min_access_delay = {0, 10 * 1000000};

enum dev_type {
    DEV,
    SUBDEV
};

struct dev_data {
    enum dev_type type;
    char *id;
    char *desc;
    hid_device *handle;
    pthread_mutex_t mutex;
    struct timespec access_time;
};

static void dev_data_init(struct dev_data *dev_data) {
    dev_data->type = DEV;
    pthread_mutex_init(&dev_data->mutex, NULL);

    dev_data->access_time = time_nanos();
    time_subtract(&dev_data->access_time, &min_access_delay);
}

static void dev_data_destroy(void *data) {
    struct dev_data *dev_data = data;
    free(dev_data->id);
    free(dev_data->desc);
    hid_close(dev_data->handle);
    pthread_mutex_destroy(&dev_data->mutex);
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

static char *get_id(struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->id;
}

static char *get_desc(struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->desc;
}

static enum dev_type get_type(void *data) {
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

static void wait_for_access(struct dev_data *dev_data) {
    struct timespec diff = time_nanos();
    time_subtract(&diff, &dev_data->access_time);
    struct timespec wait_time = time_diff(&min_access_delay, &diff);
    if (time_is_positive(&wait_time)) {
        nanosleep(&wait_time, NULL);
    }
}

static void acquire_io_lock(struct dev_data *dev_data) {
    pthread_mutex_lock(&dev_data->mutex);
    wait_for_access(dev_data);
}

static void release_io_lock(struct dev_data *dev_data) {
    clock_gettime(CLOCK_MONOTONIC, &dev_data->access_time);
    pthread_mutex_unlock(&dev_data->mutex);
}

static int read_report(struct hwctl_dev *dev, unsigned char *buffer) {
    struct hwctl_dev *root = get_root(dev);
    struct dev_data *dev_data = root->data;

    acquire_io_lock(dev_data);
    int result = hid_read_timeout(dev_data->handle, buffer, 64, 1000);
    release_io_lock(dev_data);

    if (result == -1) {
        fprintf(stderr, "Failed to read from HID device %s\n", get_id(root));
    } else if (result == 0) {
        fprintf(stderr, "Read from HID device %s timed out\n", get_id(root));
    } else {
        return 0;
    }

    return 1;
}

static void base_dev_init(struct hwctl_dev *dev) {
    hwctl_dev_init(dev);
    dev->get_id = &get_id;
    dev->get_desc = &get_desc;
}

static void kraken_dev_init(struct hwctl_dev *dev, struct hid_device_info *info, hid_device *handle) {
    base_dev_init(dev);
    dev->destroy_data = &dev_data_destroy;
    struct dev_data *dev_data = malloc(sizeof(struct dev_data));
    dev_data_init(dev_data);
    dev_data->id = hid_create_id(info);
    dev_data->desc = hid_create_desc(info);
    dev_data->handle = handle;
    dev->data = dev_data;
}

static void subdev_init(struct hwctl_dev *dev, char *id, char *desc, struct hwctl_dev *parent) {
    base_dev_init(dev);
    dev->destroy_data = &subdev_data_destroy;
    struct subdev_data *subdev_data = malloc(sizeof(struct subdev_data));
    subdev_data_init(subdev_data);
    subdev_data->id = str_make_copy(id);
    subdev_data->desc = str_make_copy(desc);
    subdev_data->parent = parent;
    dev->data = subdev_data;
}

static double read_liquid_temp(struct hwctl_dev *dev) {
    unsigned char report[64];
    if (read_report(dev, report)) {
        return 100;
    } else {
        return report[1] + report[2] / 10.;
    }
}

static void init_dev_liquid(struct hwctl_dev *liquid, struct hwctl_dev *parent) {
    subdev_init(liquid, "liquid", "Liquid temperature sensor (read degrees C)", parent);
    liquid->read_sen = &read_liquid_temp;
}

static double read_fan_speed(struct hwctl_dev *dev) {
    unsigned char report[64];
    if (read_report(dev, report)) {
        return 0;
    } else {
        return (uint32_t) (report[3] << 8u) | report[4];
    }
}

static int32_t clamp(int32_t min, int32_t val, int32_t max) {
    return val < min ? min : (val > max ? max : val);
}

static void write_duty(struct hwctl_dev *dev, uint8_t type, uint8_t duty) {
    unsigned char msg[65];
    memset(msg, 0, sizeof(msg));
    msg[0] = 0x02;
    msg[1] = 0x4d;
    msg[2] = type;
    msg[4] = duty;

    dev = get_root(dev);
    struct dev_data *dev_data = dev->data;
    acquire_io_lock(dev_data);
    int result = hid_write(dev_data->handle, msg, sizeof(msg));
    release_io_lock(dev_data);
    if (result == -1) {
        fprintf(stderr, "Failed to write to HID device %s\n", get_id(dev));
    }
}

static void write_fan_duty(struct hwctl_dev *dev, double duty) {
    write_duty(dev, 0x00, (uint8_t) clamp(25, duty, 100));
}

static void init_dev_fan(struct hwctl_dev *fan, struct hwctl_dev *parent) {
    subdev_init(fan, "fan", "Fan (read RPM, write %)", parent);
    fan->read_sen = &read_fan_speed;
    fan->write_act = &write_fan_duty;
}

static double read_pump_speed(struct hwctl_dev *dev) {
    unsigned char report[64];
    if (read_report(dev, report)) {
        return 0;
    } else {
        return (uint32_t) (report[5] << 8u) | report[6];
    }
}

static void write_pump_duty(struct hwctl_dev *dev, double duty) {
    if (read_liquid_temp(dev) >= 60) {
        duty = 100;
    }
    write_duty(dev, 0x40, (uint8_t) clamp(60, duty, 100));
}

static void init_dev_pump(struct hwctl_dev *pump, struct hwctl_dev *parent) {
    subdev_init(pump, "pump", "Liquid pump (read RPM, write %)", parent);
    pump->read_sen = &read_pump_speed;
    pump->write_act = &write_pump_duty;
}

static void det_devs(struct vec *devs) {
    struct hid_device_info *enumeration = hid_enumerate(VENDOR_ID, PRODUCT_ID);

    for (struct hid_device_info *info = enumeration; info; info = info->next) {
        hid_device *handle = hid_open_path(info->path);
        if (!handle) {
            fprintf(stderr, "Failed to open HID device at path %s\n", info->path);
            continue;
        }

        struct hwctl_dev *dev = vec_push_back(devs);
        kraken_dev_init(dev, info, handle);

        init_dev_liquid(vec_push_back(dev->subdevs), dev);
        init_dev_fan(vec_push_back(dev->subdevs), dev);
        init_dev_pump(vec_push_back(dev->subdevs), dev);
    }

    hid_free_enumeration(enumeration);
}

void hwctl_init_dev_det(struct hwctl_dev_det* dev_det) {
    dev_det->det_devs = &det_devs;
}
