#include <stdio.h>
#include <hwctl/device.h>
#include <hwctl/loader.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

void print_spaces(unsigned n) {
    for (unsigned i = 0; i < n; ++i) {
        fputc(' ', stdout);
    }
}

void print_dev(struct dev *dev, unsigned depth) {
    fputc('\n', stdout);
    print_spaces(depth * 2);
    printf("Name: %s\n", dev->get_name(dev));
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
    for (unsigned i = 0; i < vec_size(dev->children); ++i) {
        struct dev *child = ((struct dev*) vec_data(dev->children)) + i;

        print_dev(child, depth + 1);
    }
}

int main(void) {
    hwctl_load_plugins();

    struct vec *devs;
    vec_init(&devs, sizeof(struct dev));
    for (unsigned i = 0; i < vec_size(dev_dets); ++i) {
        struct dev_det *dev_det = ((struct dev_det*) vec_data(dev_dets)) + i;

        dev_det->det_devs(devs);
    }

    for (unsigned i = 0; i < 100; ++i) {
        for (unsigned j = 0; j < vec_size(devs); ++j) {
            struct dev *dev = ((struct dev*) vec_data(devs)) + j;

            print_dev(dev, 0);
        }
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }

    for (unsigned i = 0; i < vec_size(devs); ++i) {
        struct dev *dev = ((struct dev*) vec_data(devs)) + i;
        dev_destroy(dev);
    }

    vec_destroy(devs);
    hwctl_unload_plugins();
    return 0;
}
