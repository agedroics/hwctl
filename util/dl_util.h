#pragma once
#include <defs.h>
#ifdef _WIN32
#define DL_EXT L".dll"
#else
#define DL_EXT ".so"
#endif

void *dl_open(path_char*);

void *dl_get_sym(void*, char*);

void dl_close(void*);
