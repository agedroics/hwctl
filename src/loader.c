#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dl_util.h>
#include <fs_util.h>
#include <str_util.h>
#include <hwctl/device.h>
#include <hwctl/loader.h>

static struct vec *hwctl_dev_dets;

static struct vec *plugins;

typedef int (*init_shutdown_t)(void);

typedef void (*init_dev_det_t)(struct hwctl_dev_det*);

const struct vec *get_hwctl_dev_dets(void) {
    return hwctl_dev_dets;
}

void hwctl_load_plugins(void) {
    vec_init(&hwctl_dev_dets, sizeof(struct hwctl_dev_det));

    vec_init(&plugins, sizeof(void*));

    path_char *plugin_path;
    {
        path_char *temp = fs_get_ex_path();
        fs_path_remove_file(temp);
        fs_path_remove_file(temp);
#ifdef WIN32
        plugin_path = wstr_concat(2, temp, L"\\lib\\hwctl");
#else
        plugin_path = str_concat(2, temp, "/lib/hwctl");
#endif
        free(temp);
    }

    FS_DIR *dir;
    fs_dirent *ent;
    if ((dir = fs_opendir(plugin_path)) != NULL) {
        while ((ent = fs_readdir(dir)) != NULL) {
#ifdef _WIN32
            wchar_t *dot = wcsrchr(ent->d_name, L'.');
            if (dot && !wcscmp(dot, DL_EXT)) {
                wchar_t *full_path = wstr_concat(3, plugin_path, PATH_SEP_STR, ent->d_name);
#else
            char *dot = strrchr(ent->d_name, '.');
            if (dot && !strcmp(dot, DL_EXT)) {
                char *full_path = str_concat(3, plugin_path, PATH_SEP_STR, ent->d_name);
#endif
                void *plugin = dl_open(full_path);
                free(full_path);
                if (plugin) {
                    init_shutdown_t init_plugin = dl_get_sym(plugin, "hwctl_init_plugin");
                    if (init_plugin) {
                        int result = init_plugin();
                        if (!result) {
#ifdef WIN32
                            printf("%ls init\n", ent->d_name);
#else
                            printf("%s init\n", ent->d_name);
#endif
                        } else {
#ifdef WIN32
                            fprintf(stderr, "%ls failed to init\n", ent->d_name);
#else
                            fprintf(stderr, "%s failed to init\n", ent->d_name);
#endif
                        }
                        void *ptr = vec_push_back(plugins);
                        memcpy(ptr, &plugin, sizeof(void*));
                        init_dev_det_t init_dev_det = dl_get_sym(plugin, "hwctl_init_dev_det");
                        if (init_dev_det != NULL) {
                            init_dev_det(vec_push_back(hwctl_dev_dets));
                        }
                    } else {
                        dl_close(plugin);
                    }
                }
            }
        }
        fs_closedir(dir);
    }

    free(plugin_path);
}

void hwctl_unload_plugins(void) {
    vec_destroy(hwctl_dev_dets);

    for (unsigned i = 0; i < vec_size(plugins); ++i) {
        void *plugin = ((void**) vec_data(plugins))[i];
        init_shutdown_t shutdown_plugin = dl_get_sym(plugin, "hwctl_shutdown_plugin");
        if (shutdown_plugin) {
            shutdown_plugin();
        }
        dl_close(plugin);
    }

    vec_destroy(plugins);
}
