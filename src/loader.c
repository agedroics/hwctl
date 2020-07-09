#include <stdio.h>
#include <dirent.h>
#include <dl_util.h>
#include <str_util.h>
#include <hwctl/device.h>
#include <hwctl/loader.h>

#ifdef _WIN32
#define PATH_SEP L"\\"
#else
#define PATH_SEP "/"
#endif

struct vec *dev_dets;

static struct vec *plugins;

typedef void (*init_shutdown_t)(void);

typedef void (*init_dev_det_t)(struct dev_det*);

void hwctl_load_plugins(void) {
    vec_init(&dev_dets, sizeof(struct dev_det));

    vec_init(&plugins, sizeof(DL_T));

    PCHAR *plugin_path;

    #ifdef _WIN32
    {
        wchar_t *temp = malloc((MAX_PATH) * sizeof(wchar_t));
        GetModuleFileNameW(NULL, temp, MAX_PATH);
        wchar_t *slash = wcsrchr(temp, L'\\');
        if (slash) {
            *slash = 0;
        }
        slash = wcsrchr(temp, L'\\');
        if (slash) {
            *slash = 0;
        }
        plugin_path = wstr_concat(2, temp, L"\\lib\\hwctl");
        free(temp);
    }
    #else
    plugin_path = "/usr/local/lib/hwctl";
    #endif

    #ifdef WIN32
    _WDIR *dir;
    struct _wdirent *ent;
    if ((dir = _wopendir(plugin_path)) != NULL) {
        while ((ent = _wreaddir(dir)) != NULL) {
            wchar_t *dot = wcsrchr(ent->d_name, L'.');
            if (dot && !wcscmp(dot, DL_EXT)) {
                wchar_t *full_path = wstr_concat(3, plugin_path, PATH_SEP, ent->d_name);
    #else
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(plugin_path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char *dot = strrchr(ent->d_name, '.');
            if (dot && !strcmp(dot, DL_EXT)) {
                char *full_path = str_concat(3, plugin_path, PATH_SEP, ent->d_name);
    #endif
                DL_T plugin = dl_open(full_path);
                free(full_path);
                if (plugin) {
                    init_shutdown_t init_plugin = (init_shutdown_t) dl_sym(plugin, "init_plugin");
                    if (init_plugin) {
                        init_plugin();
                        DL_T *ptr = vec_push_back(plugins);
                        *ptr = plugin;
                        init_dev_det_t init_dev_det = (init_dev_det_t) dl_sym(plugin, "init_dev_det");
                        if (init_dev_det != NULL) {
                            init_dev_det(vec_push_back(dev_dets));
                            #ifdef WIN32
                            printf("%ls loaded\n", ent->d_name);
                            #else
                            printf("%s loaded\n", ent->d_name);
                            #endif
                        }
                    } else {
                        dl_close(plugin);
                    }
                }
            }
        }
        #ifdef WIN32
        _wclosedir(dir);
        #else
        closedir(dir);
        #endif
    }

    #ifdef _WIN32
    free(plugin_path);
    #endif
}

void hwctl_unload_plugins(void) {
    vec_destroy(dev_dets);

    for (unsigned i = 0; i < vec_size(plugins); ++i) {
        DL_T plugin = ((DL_T*) vec_data(plugins))[i];
        init_shutdown_t shutdown_plugin = (init_shutdown_t) dl_sym(plugin, "shutdown_plugin");
        if (shutdown_plugin != NULL) {
            shutdown_plugin();
        }
        dl_close(plugin);
    }

    vec_destroy(plugins);
}
