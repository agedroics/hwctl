#pragma once
#include <defs.h>
#include <dirent.h>
#ifdef _WIN32
#define FS_DIR _WDIR
#define fs_dirent struct _wdirent
#else
#define FS_DIR DIR
#define fs_dirent struct dirent
#endif

path_char *fs_get_ex_path(void);

void fs_path_remove_file(path_char*);

FS_DIR *fs_opendir(path_char*);

fs_dirent *fs_readdir(FS_DIR*);

int fs_closedir(FS_DIR*);
