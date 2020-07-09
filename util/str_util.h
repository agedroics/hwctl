#ifndef HWCTL_STR_UTIL_H
#define HWCTL_STR_UTIL_H

char *str_copy(char*);

wchar_t *wstr_copy(wchar_t*);

char *str_concat(size_t n, ...);

wchar_t *wstr_concat(size_t n, ...);

#endif
