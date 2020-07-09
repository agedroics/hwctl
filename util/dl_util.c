#include <dl_util.h>

DL_T dl_open(PCHAR *filename) {
    #ifdef _WIN32
    return LoadLibraryW(filename);
    #else
    return dlopen(filename, RTLD_LOCAL);
    #endif
}

void *dl_sym(DL_T dl, char *symbol) {
    #ifdef _WIN32
    return (void*) GetProcAddress(dl, symbol);
    #else
    return dlsym(dl, symbol);
    #endif
}

void dl_close(DL_T dl) {
    #ifdef _WIN32
    FreeLibrary(dl);
    #else
    dlclose(dl);
    #endif
}
