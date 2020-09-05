#include <string.h>
#include <hwctl/device.h>

void hwctl_dev_init(struct hwctl_dev *dev) {
    memset(dev, 0, sizeof(struct hwctl_dev));
    vec_init(&dev->subdevs, sizeof(struct hwctl_dev));
}

void hwctl_dev_destroy(struct hwctl_dev *dev) {
    for (unsigned i = 0; i < vec_size(dev->subdevs); ++i) {
        hwctl_dev_destroy(((struct hwctl_dev*) vec_data(dev->subdevs)) + i);
    }
    dev->destroy_data(dev->data);
    vec_destroy(dev->subdevs, 0);
}
