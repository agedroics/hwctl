#ifndef HWCTL_DL_UTIL_H
#define HWCTL_DL_UTIL_H

#ifdef _WIN32
#include <windows.h>
#define DL_T HMODULE
#define PCHAR wchar_t
#define DL_EXT L".dll"
#else
#define DL_T void*
#define PCHAR char
#define DL_EXT ".so"
#endif

DL_T dl_open(PCHAR *filename);

void *dl_sym(DL_T, char *symbol);

void dl_close(DL_T);

#endif
