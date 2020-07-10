#include <fs_util.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

path_char *fs_get_ex_path(void) {
#ifdef _WIN32
    DWORD chars = MAX_PATH;
    wchar_t *path = malloc(chars * sizeof(wchar_t));
    for (;;) {
        DWORD n = GetModuleFileNameW(NULL, path, chars);
        if (n == chars) {
            chars <<= 1u;
            path = realloc(path, chars * sizeof(wchar_t));
        } else {
            break;
        }
    }
    return path;
#else
    size_t bytes = 256;
    char *path = malloc(bytes);
    ssize_t n;
    for (;;) {
        n = readlink("/proc/self/exe", path, bytes);
        if (n == bytes) {
            bytes <<= 1u;
            path = realloc(path, bytes);
        } else {
            break;
        }
    }
    path[n] = 0;
    return path;
#endif
}

void fs_path_remove_file(path_char *path) {
#ifdef _WIN32
    path_char *sep = wcsrchr(path, PATH_SEP_CHAR);
    if (sep) {
        *sep = 0;
    }
#else
    path_char *sep = strrchr(path, PATH_SEP_CHAR);
    if (sep) {
        if (sep == path) {
            sep[1] = 0;
        } else {
            *sep = 0;
        }
    }
#endif
}

FS_DIR *fs_opendir(path_char *path) {
#ifdef _WIN32
    return _wopendir(path);
#else
    return opendir(path);
#endif
}

fs_dirent *fs_readdir(FS_DIR *dir) {
#ifdef _WIN32
    return _wreaddir(dir);
#else
    return readdir(dir);
#endif
}

int fs_closedir(FS_DIR *dir) {
#ifdef _WIN32
    return _wclosedir(dir);
#else
    return closedir(dir);
#endif
}
