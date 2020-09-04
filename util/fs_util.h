#pragma once
#include <defs.h>
#ifdef _WIN32
#include <dirent_win32.h>
#define FS_DIR _WDIR
#define fs_dirent struct _wdirent
#else
#include <dirent.h>
#define FS_DIR DIR
#define fs_dirent struct dirent
#endif

path_char *fs_get_ex_path(void);

void fs_path_remove_file(path_char*);

FS_DIR *fs_opendir(path_char*);

fs_dirent *fs_readdir(FS_DIR*);

int fs_closedir(FS_DIR*);
