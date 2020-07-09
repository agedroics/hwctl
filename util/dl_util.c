#include <dl_util.h>
#ifdef _WIN32
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

void *dl_open(path_char *path) {
#ifdef _WIN32
    return LoadLibraryW(path);
#else
    return dlopen(path, RTLD_LOCAL);
#endif
}

void *dl_get_sym(void *dl, char *name) {
#ifdef _WIN32
    return GetProcAddress(dl, name);
#else
    return dlsym(dl, name);
#endif
}

void dl_close(void *dl) {
#ifdef _WIN32
    FreeLibrary(dl);
#else
    dlclose(dl);
#endif
}
