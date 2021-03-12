#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nvml.h>
#include <hwctl/plugin.h>
#include <str_util.h>

enum dev_type {
    DEV,
    SUBDEV
};

struct dev_data {
    enum dev_type type;
    char *uuid;
    char *desc;
    nvmlDevice_t handle;
};

static void dev_data_init(struct dev_data *dev_data) {
    dev_data->type = DEV;
}

static void dev_data_destroy(void *data) {
    struct dev_data* dev_data = data;
    free(dev_data->uuid);
    free(dev_data->desc);
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
    struct subdev_data* subdev_data = data;
    free(subdev_data->id);
    free(subdev_data->desc);
    free(subdev_data);
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

static char *get_uuid(struct hwctl_dev *dev) {
    return ((struct dev_data*) get_root(dev)->data)->uuid;
}

static const char *get_desc(const struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->desc;
}

static const char *get_id(const struct hwctl_dev* dev) {
    if (get_type(dev->data) == DEV) {
        return ((struct dev_data*) dev->data)->uuid;
    } else {
        return ((struct subdev_data*) dev->data)->id;
    }
}

static nvmlDevice_t get_handle(struct hwctl_dev *dev) {
    return ((struct dev_data *) get_root(dev)->data)->handle;
}

static void base_dev_init(struct hwctl_dev *dev) {
    hwctl_dev_init(dev);
    dev->get_id = &get_id;
    dev->get_desc = &get_desc;
}

static void nvidia_dev_init(struct hwctl_dev *dev) {
    base_dev_init(dev);
    dev->destroy_data = &dev_data_destroy;
}

static int read_temp(struct hwctl_dev *dev, double *temp) {
    unsigned temp1;
    nvmlReturn_t result = nvmlDeviceGetTemperature(get_handle(dev), NVML_TEMPERATURE_GPU, &temp1);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to get temperature of %s: %s\n", get_uuid(dev), nvmlErrorString(result));
        return 1;
    }
    *temp = temp1;
    return 0;
}

static void subdev_init(struct hwctl_dev *subdev, char *id, char *desc, struct hwctl_dev *parent) {
    base_dev_init(subdev);
    subdev->destroy_data = &subdev_data_destroy;

    struct subdev_data *subdev_data = malloc(sizeof(struct subdev_data));
    subdev_data_init(subdev_data);

    subdev_data->id = str_make_copy(id);
    subdev_data->desc = str_make_copy(desc);
    subdev_data->parent = parent;

    subdev->data = subdev_data;
}

static void core_dev_init(struct hwctl_dev *core, struct hwctl_dev *parent) {
    subdev_init(core, "core", "GPU core temperature sensor (read degrees C)", parent);
    core->read_sen = &read_temp;
}

static int read_fan_duty(struct hwctl_dev *dev, double *duty) {
    unsigned duty1;
    nvmlReturn_t result = nvmlDeviceGetFanSpeed(get_handle(dev), &duty1);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to get speed of %s: %s\n", get_uuid(dev), nvmlErrorString(result));
        return 1;
    }
    *duty = duty1;
    return 0;
}

static void fan_dev_init(struct hwctl_dev *fan, struct hwctl_dev *parent) {
    subdev_init(fan, "fan", "Fan (read %)", parent);
    fan->read_sen = &read_fan_duty;
}

static void det_devs(struct vec *devs) {
    unsigned dev_count;
    nvmlReturn_t result = nvmlDeviceGetCount(&dev_count);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to query nVidia device count: %s\n", nvmlErrorString(result));
        return;
    }

    for (unsigned i = 0; i < dev_count; ++i) {
        nvmlDevice_t handle;
        result = nvmlDeviceGetHandleByIndex(i, &handle);
        if (result != NVML_SUCCESS) { 
            fprintf(stderr, "Failed to get handle for nVidia device %u: %s\n", i, nvmlErrorString(result));
            continue;
        }

        char uuid[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
        result = nvmlDeviceGetUUID(handle, uuid, NVML_DEVICE_UUID_V2_BUFFER_SIZE);
        if (result != NVML_SUCCESS) { 
            fprintf(stderr, "Failed to get UUID of nVidia device %u: %s\n", i, nvmlErrorString(result));
            continue;
        }

        char name[NVML_DEVICE_NAME_BUFFER_SIZE + 7];
        result = nvmlDeviceGetName(handle, name + 7, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (result != NVML_SUCCESS) { 
            fprintf(stderr, "Failed to get name of nVidia device %u: %s\n", i, nvmlErrorString(result));
            continue;
        }
        memcpy(name, "Nvidia ", 7);

        struct dev_data *dev_data = malloc(sizeof(struct dev_data));
        dev_data_init(dev_data);

        dev_data->uuid = str_make_copy(uuid);
        dev_data->desc = str_make_copy(name);
        dev_data->handle = handle;

        struct hwctl_dev *dev = vec_push_back(devs);
        nvidia_dev_init(dev);
        dev->data = dev_data;

        core_dev_init(vec_push_back(dev->subdevs), dev);
        fan_dev_init(vec_push_back(dev->subdevs), dev);
    }
}

int hwctl_init_plugin(void) {
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to initialize NVML: %s\n", nvmlErrorString(result));
    }
    return result != NVML_SUCCESS;
}

int hwctl_shutdown_plugin(void) {
    nvmlReturn_t result = nvmlShutdown();
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to shutdown NVML: %s\n", nvmlErrorString(result));
    }
    return result != NVML_SUCCESS;
}

void hwctl_init_dev_det(struct hwctl_dev_det *dev_det) {
    dev_det->det_devs = &det_devs;
}
