#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <libusb-1.0/libusb.h>
#include <hwctl/device.h>
#include <hwctl/plugin.h>
#include <usb_util.h>
#include <str_util.h>

#define VENDOR_ID 0x1e71
#define PRODUCT_ID 0x170e

static libusb_context *ctx;

enum dev_type {
    DEV,
    SUBDEV
};

struct dev_data {
    enum dev_type type;
    char *id;
    char *desc;
    libusb_device_handle *handle;
    unsigned char report[64];
    time_t report_time;
};

static void dev_data_init(struct dev_data *dev_data) {
    dev_data->type = DEV;
    memset(dev_data->report, 0, sizeof(dev_data->report));
    dev_data->report_time = 0;
}

static void dev_data_destroy(void *data) {
    struct dev_data *dev_data = data;
    free(dev_data->id);
    free(dev_data->desc);
    libusb_close(dev_data->handle);
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

void hwctl_init_plugin(void) {
    int result = libusb_init(&ctx);
    if (result) {
        fprintf(stderr, "Failed to initialize libusb-1.0: %s\n", libusb_strerror(result));
        return;
    }

    libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_WARNING);
}

void hwctl_shutdown_plugin(void) {
    if (ctx) {
        libusb_exit(ctx);
    }
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

static libusb_device_handle *get_handle(struct hwctl_dev *dev) {
    return ((struct dev_data*) get_root(dev)->data)->handle;
}

static unsigned char *get_report(struct hwctl_dev *dev) {
    struct hwctl_dev *root = get_root(dev);
    struct dev_data *dev_data = root->data;
    time_t curtime = time(NULL);
    if (difftime(curtime, dev_data->report_time) >= 1) {
        int transferred;
        int result = libusb_interrupt_transfer(dev_data->handle, 0x81, dev_data->report, 64, &transferred, 1000);
        if (result) {
            fprintf(stderr, "Failed to read from USB device %s: %s\n", get_id(root), libusb_strerror(result));
        } else {
            dev_data->report_time = curtime;
        }
    }
    return dev_data->report;
}

static void base_dev_init(struct hwctl_dev *dev) {
    hwctl_dev_init(dev);
    dev->get_id = &get_id;
    dev->get_desc = &get_desc;
}

static void kraken_dev_init(struct hwctl_dev *dev, libusb_device_handle *handle) {
    base_dev_init(dev);
    dev->destroy_data = &dev_data_destroy;
    struct dev_data *dev_data = malloc(sizeof(struct dev_data));
    dev_data_init(dev_data);
    dev_data->id = usb_create_id(handle);
    dev_data->desc = usb_create_desc(handle);
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

static float read_liquid_temp(struct hwctl_dev *dev) {
    unsigned char *report = get_report(dev);
    return report[1] + report[2] / 10.f;
}

static void init_dev_liquid(struct hwctl_dev *liquid, struct hwctl_dev *parent) {
    subdev_init(liquid, "liquid", "Liquid temperature sensor", parent);
    liquid->temp_sen = malloc(sizeof(struct hwctl_temp_sen));
    liquid->temp_sen->read_temp = &read_liquid_temp;
}

static int32_t read_fan_speed(struct hwctl_dev *dev) {
    unsigned char *report = get_report(dev);
    return (uint32_t) (report[3] << 8u) | report[4];
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
    int transferred;
    int result = libusb_interrupt_transfer(dev_data->handle, 0x01, msg, sizeof(msg), &transferred, 1000);
    if (result) {
        fprintf(stderr, "Failed to write to USB device %s: %s\n", get_id(dev), libusb_strerror(result));
    }
}

static void write_fan_duty(struct hwctl_dev *dev, int32_t duty) {
    write_duty(dev, 0x00, (uint8_t) clamp(25, duty, 100));
}

static void init_dev_fan(struct hwctl_dev *fan, struct hwctl_dev *parent) {
    subdev_init(fan, "fan", "Fan", parent);
    
    fan->speed_sen = malloc(sizeof(struct hwctl_speed_sen));
    fan->speed_sen->read_duty = NULL;
    fan->speed_sen->read_speed = &read_fan_speed;

    fan->speed_act = malloc(sizeof(struct hwctl_speed_act));
    fan->speed_act->write_duty = &write_fan_duty;
}

static int32_t read_pump_speed(struct hwctl_dev *dev) {
    unsigned char *report = get_report(dev);
    return (uint32_t) (report[5] << 8u) | report[6];
}

static void write_pump_duty(struct hwctl_dev *dev, int32_t duty) {
    write_duty(dev, 0x40, (uint8_t) clamp(60, duty, 100));
}

static void init_dev_pump(struct hwctl_dev *pump, struct hwctl_dev *parent) {
    subdev_init(pump, "pump", "Liquid pump", parent);
    
    pump->speed_sen = malloc(sizeof(struct hwctl_speed_sen));
    pump->speed_sen->read_duty = NULL;
    pump->speed_sen->read_speed = &read_pump_speed;

    pump->speed_act = malloc(sizeof(struct hwctl_speed_act));
    pump->speed_act->write_duty = &write_pump_duty;
}

static void det_devs(struct vec *devs) {
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(ctx, &list);
    if (cnt < 0) {
        fprintf(stderr, "Failed to get list of USB devices: %s\n", libusb_strerror(cnt));
        return;
    }

    for (unsigned i = 0; i < cnt; ++i) {
        libusb_device *usb_dev = list[i];
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(usb_dev, &desc);
        
        if (desc.idVendor == VENDOR_ID && desc.idProduct == PRODUCT_ID) {
            libusb_device_handle *handle;
            int result = libusb_open(usb_dev, &handle);
            if (result) {
                fprintf(stderr, "Failed to get handle for USB device: %s\n", libusb_strerror(result));
                continue;
            }

            struct hwctl_dev *dev = vec_push_back(devs);
            kraken_dev_init(dev, handle);

            init_dev_liquid(vec_push_back(dev->subdevs), dev);
            init_dev_fan(vec_push_back(dev->subdevs), dev);
            init_dev_pump(vec_push_back(dev->subdevs), dev);
        }
    }

    libusb_free_device_list(list, 1);
}

void hwctl_init_dev_det(struct hwctl_dev_det* dev_det) {
    dev_det->det_devs = &det_devs;
}
