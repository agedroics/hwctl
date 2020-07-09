#pragma once
#ifdef _WIN32
#include <stddef.h>
#define PATH_SEP_CHAR L'\\'
#define PATH_SEP_STR L"\\"
#define path_char wchar_t
#else
#define PATH_SEP_CHAR '/'
#define PATH_SEP_STR "/"
#define path_char char
#endif
