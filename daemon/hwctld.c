#include <stdio.h>
#include <hwctl/device.h>
#include <hwctl/loader.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

static void print_spaces(unsigned n) {
    for (unsigned i = 0; i < n; ++i) {
        fputc(' ', stdout);
    }
}

static void print_dev(struct hwctl_dev *dev, unsigned depth) {
    fputc('\n', stdout);
    print_spaces(depth * 2);
    printf("ID: %s\n", dev->get_id(dev));
    print_spaces(depth * 2);
    printf("Description: %s\n", dev->get_desc(dev));
    if (dev->temp_sen) {
        print_spaces(depth * 2);
        printf("Temp: %.1fC\n", dev->temp_sen->read_temp(dev));
    }
    if (dev->speed_sen) {
        print_spaces(depth * 2);
        if (dev->speed_sen->read_duty) {
            printf("Speed: %d%%\n", dev->speed_sen->read_duty(dev));
        }
        if (dev->speed_sen->read_speed) {
            printf("Speed: %dRPM\n", dev->speed_sen->read_speed(dev));
        }
    }
    if (dev->speed_act) {
        dev->speed_act->write_duty(dev, 100);
    }
    for (unsigned i = 0; i < vec_size(dev->subdevs); ++i) {
        struct hwctl_dev *subdev = ((struct hwctl_dev*) vec_data(dev->subdevs)) + i;
        print_dev(subdev, depth + 1);
    }
}

int main(void) {
    hwctl_load_plugins();

    struct vec *devs;
    vec_init(&devs, sizeof(struct hwctl_dev));
    for (unsigned i = 0; i < vec_size(get_hwctl_dev_dets()); ++i) {
        struct hwctl_dev_det *dev_det = ((struct hwctl_dev_det*) vec_data(get_hwctl_dev_dets())) + i;
        dev_det->det_devs(devs);
    }

    for (unsigned i = 0; i < 100; ++i) {
        for (unsigned j = 0; j < vec_size(devs); ++j) {
            struct hwctl_dev *dev = ((struct hwctl_dev*) vec_data(devs)) + j;
            print_dev(dev, 0);
        }
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }

    for (unsigned i = 0; i < vec_size(devs); ++i) {
        struct hwctl_dev *dev = ((struct hwctl_dev*) vec_data(devs)) + i;
        hwctl_dev_destroy(dev);
    }

    vec_destroy(devs);
    hwctl_unload_plugins();
    return 0;
}
