#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hwctl/device.h>
#include <str_util.h>
#include <hwctl/loader.h>

static struct vec *hwctl_dev_dets;

static struct vec *plugins;

typedef int (*init_shutdown_t)(void);

typedef void (*init_dev_det_t)(struct hwctl_dev_det*);

struct vec *get_hwctl_dev_dets(void) {
    return hwctl_dev_dets;
}

static void remove_file(char *path) {
    char *sep = strrchr(path, '/');
    if (sep) {
        if (sep == path) {
            sep[1] = 0;
        } else {
            *sep = 0;
        }
    }
}

void hwctl_load_plugins(void) {
    vec_init(&hwctl_dev_dets, sizeof(struct hwctl_dev_det));

    vec_init(&plugins, sizeof(void*));

    char *plugin_path;
    {
        char *temp = realpath("/proc/self/exe", NULL);
        remove_file(temp);
        remove_file(temp);
        plugin_path = str_concat(2, temp, "/lib/hwctl");
        free(temp);
    }

    DIR *dir = opendir(plugin_path);
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            char *dot = strrchr(ent->d_name, '.');
            if (dot && !strcmp(dot, ".so")) {
                char *full_path = str_concat(3, plugin_path, "/", ent->d_name);
                void *plugin = dlopen(full_path, RTLD_LAZY | RTLD_LOCAL);
                free(full_path);
                if (plugin) {
                    init_shutdown_t init_plugin = dlsym(plugin, "hwctl_init_plugin");
                    if (init_plugin) {
                        int error = init_plugin();
                        if (error) {
                            fprintf(stderr, "%s failed to init\n", ent->d_name);
                        }
                        vec_push_back(plugins, &plugin);
                        init_dev_det_t init_dev_det = dlsym(plugin, "hwctl_init_dev_det");
                        if (init_dev_det != NULL) {
                            init_dev_det(vec_push_back(hwctl_dev_dets, NULL));
                        }
                    } else {
                        dlclose(plugin);
                    }
                }
            }
        }
        closedir(dir);
    }

    free(plugin_path);
}

void hwctl_unload_plugins(void) {
    vec_destroy(hwctl_dev_dets, 0);

    for (unsigned i = 0; i < vec_size(plugins); ++i) {
        void *plugin = *((void**) vec_at(plugins, i));
        init_shutdown_t shutdown_plugin = dlsym(plugin, "hwctl_shutdown_plugin");
        if (shutdown_plugin) {
            shutdown_plugin();
        }
        dlclose(plugin);
    }

    vec_destroy(plugins, 0);
}
