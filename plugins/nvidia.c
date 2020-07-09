#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <nvml.h>
#include <hwctl/plugin.h>
#include <str_util.h>

struct dev_data {
    char *name;
    char *desc;
    char *uuid;
};

static char *get_name(struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->name;
}

static char *get_desc(struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->desc;
}

static char *get_uuid(struct hwctl_dev *dev) {
    return ((struct dev_data*) dev->data)->uuid;
}

static void destroy_data(void *data) {
    struct dev_data *dev_data = data;
    free(dev_data->name);
    free(dev_data->desc);
    free(dev_data->uuid);
    free(dev_data);
}

static void nvidia_dev_init(struct hwctl_dev *dev) {
    dev->destroy_data = &destroy_data;
    dev->get_name = &get_name;
    dev->get_desc = &get_desc;
}

static float read_temp(struct hwctl_dev *dev) {
    nvmlDevice_t nvml_dev;
    nvmlReturn_t result = nvmlDeviceGetHandleByUUID(get_uuid(dev), &nvml_dev);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to get handle for %s: %s\n", get_uuid(dev), nvmlErrorString(result));
        return 0;
    }

    unsigned temp;
    result = nvmlDeviceGetTemperature(nvml_dev, NVML_TEMPERATURE_GPU, &temp);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to get temperature of %s: %s\n", get_uuid(dev), nvmlErrorString(result));
        return 0;
    }
    return (float) temp;
}

static void init_dev_core(struct hwctl_dev *core, struct hwctl_dev *parent) {
    hwctl_dev_init(core);
    nvidia_dev_init(core);
    
    struct dev_data *dev_data = malloc(sizeof(struct dev_data));

    dev_data->name = malloc(5);
    memcpy(dev_data->name, "core", 5);

    dev_data->desc = malloc(28);
    memcpy(dev_data->desc, "GPU core temperature sensor", 28);

    dev_data->uuid = str_make_copy(get_uuid(parent));

    core->data = dev_data;

    core->temp_sen = malloc(sizeof(struct hwctl_temp_sen));
    core->temp_sen->read_temp = &read_temp;
}

static int32_t read_fan_duty(struct hwctl_dev *dev) {
    nvmlDevice_t nvml_dev;
    nvmlReturn_t result = nvmlDeviceGetHandleByUUID(get_uuid(dev), &nvml_dev);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to get handle for %s: %s\n", get_uuid(dev), nvmlErrorString(result));
        return 0;
    }

    unsigned duty;
    result = nvmlDeviceGetFanSpeed(nvml_dev, &duty);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to get speed of %s: %s\n", get_uuid(dev), nvmlErrorString(result));
        return 0;
    }
    return duty;
}

static void init_dev_fan(struct hwctl_dev *fan, struct hwctl_dev *parent) {
    hwctl_dev_init(fan);
    nvidia_dev_init(fan);
    
    struct dev_data *dev_data = malloc(sizeof(struct dev_data));

    dev_data->name = malloc(4);
    memcpy(dev_data->name, "fan", 4);

    dev_data->desc = malloc(4);
    memcpy(dev_data->desc, "Fan", 4);

    dev_data->uuid = str_make_copy(get_uuid(parent));

    fan->data = dev_data;

    fan->speed_sen = malloc(sizeof(struct hwctl_speed_sen));
    fan->speed_sen->read_duty = &read_fan_duty;
    fan->speed_sen->read_speed = NULL;
}

static void det_devs(struct vec *devs) {
    unsigned dev_count;
    nvmlReturn_t result = nvmlDeviceGetCount(&dev_count);
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to query nVidia device count: %s\n", nvmlErrorString(result));
        return;
    }

    for (unsigned i = 0; i < dev_count; ++i) {
        nvmlDevice_t nvml_dev;
        result = nvmlDeviceGetHandleByIndex(i, &nvml_dev);
        if (result != NVML_SUCCESS) { 
            fprintf(stderr, "Failed to get handle for nVidia device %u: %s\n", i, nvmlErrorString(result));
            continue;
        }

        char uuid[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
        result = nvmlDeviceGetUUID(nvml_dev, uuid, NVML_DEVICE_UUID_V2_BUFFER_SIZE);
        if (result != NVML_SUCCESS) { 
            fprintf(stderr, "Failed to get UUID of nVidia device %u: %s\n", i, nvmlErrorString(result));
            continue;
        }

        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        result = nvmlDeviceGetName(nvml_dev, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (result != NVML_SUCCESS) { 
            fprintf(stderr, "Failed to get name of nVidia device %u: %s\n", i, nvmlErrorString(result));
            continue;
        }

        struct dev_data *dev_data = malloc(sizeof(struct dev_data));

        size_t uuid_len = strlen(uuid);
        dev_data->uuid = malloc(uuid_len + 1);
        memcpy(dev_data->uuid, uuid, uuid_len + 1);

        dev_data->name = str_make_copy(dev_data->uuid);

        size_t name_len = strlen(name);
        dev_data->desc = malloc(name_len + 1);
        memcpy(dev_data->desc, name, name_len + 1);

        struct hwctl_dev *dev = vec_push_back(devs);
        hwctl_dev_init(dev);
        nvidia_dev_init(dev);
        dev->data = dev_data;

        init_dev_core(vec_push_back(dev->subdevs), dev);
        init_dev_fan(vec_push_back(dev->subdevs), dev);
    }
}

void hwctl_init_plugin(void) {
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to initialize NVML: %s\n", nvmlErrorString(result));
    }
}

void hwctl_shutdown_plugin(void) {
    nvmlReturn_t result = nvmlShutdown();
    if (result != NVML_SUCCESS) {
        fprintf(stderr, "Failed to shutdown NVML: %s\n", nvmlErrorString(result));
    }
}

void hwctl_init_dev_det(struct hwctl_dev_det *dev_det) {
    dev_det->det_devs = &det_devs;
}
