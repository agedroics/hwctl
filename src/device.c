#include <stdlib.h>
#include <string.h>
#include <hwctl/device.h>

void dev_init(struct dev *dev) {
    memset(dev, 0, sizeof(struct dev));
    vec_init(&dev->children, sizeof(struct dev));
}

void dev_destroy(struct dev *dev) {
    for (unsigned i = 0; i < vec_size(dev->children); ++i) {
        struct dev *child = ((struct dev*) vec_data(dev->children)) + i;

        dev_destroy(child);
    }

    dev->destroy_data(dev->data);
    if (dev->temp_sen) {
        free(dev->temp_sen);
    }
    if (dev->speed_sen) {
        free(dev->speed_sen);
    }
    if (dev->speed_act) {
        free(dev->speed_act);
    }
    vec_destroy(dev->children);
}
