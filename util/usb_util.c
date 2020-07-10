#include <stdlib.h>
#include <stdio.h>
#include <usb_util.h>

char *usb_create_id(libusb_device_handle *handle) {
    libusb_device *dev = libusb_get_device(handle);
    
    uint8_t bus_num = libusb_get_bus_number(dev);
    uint8_t port_nums[7];
    int depth = libusb_get_port_numbers(dev, port_nums, 7);
    
    size_t len = 0;
    len += snprintf(NULL, 0, "USB%u", bus_num);
    for (int i = 0; i < depth; ++i) {
        len += snprintf(NULL, 0, ".%u", port_nums[i]);
    }
    
    char *id = malloc(len + 1);
    len = 0;
    len += sprintf(id, "USB%u", bus_num);
    for (int i = 0; i < depth; ++i) {
        len += sprintf(id + len, ".%u", port_nums[i]);
    }
    
    return id;
}

char *usb_create_desc(libusb_device_handle *handle) {
    libusb_device *dev = libusb_get_device(handle);

    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(dev, &desc);

    unsigned char vendor[256];
    if (desc.iManufacturer) {
        libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, vendor, sizeof(vendor));
    }

    unsigned char product[256];
    if (desc.iProduct) {
        libusb_get_string_descriptor_ascii(handle, desc.iProduct, product, sizeof(product));
    }

    size_t len = 0;
    if (desc.iManufacturer && vendor[0]) {
        len += snprintf(NULL, 0, "%s [%#04x] - ", vendor, desc.idVendor);
    } else {
        len += snprintf(NULL, 0, "[%#04x] - ", desc.idVendor);
    }
    if (desc.iProduct && product[0]) {
        len += snprintf(NULL, 0, "%s [%#04x]", product, desc.idProduct);
    } else {
        len += snprintf(NULL, 0, "[%#04x]", desc.idProduct);
    }

    char *str_desc = malloc(len + 1);
    len = 0;
    if (desc.iManufacturer && vendor[0]) {
        len += sprintf(str_desc, "%s [%#04x] - ", vendor, desc.idVendor);
    } else {
        len += sprintf(str_desc, "[%#04x] - ", desc.idVendor);
    }
    if (desc.iProduct && product[0]) {
        sprintf(str_desc + len, "%s [%#04x]", product, desc.idProduct);
    } else {
        sprintf(str_desc + len, "[%#04x]", desc.idProduct);
    }
    
    return str_desc;
}
