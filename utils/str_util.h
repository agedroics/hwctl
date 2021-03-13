#pragma once
#include <stddef.h>

char *str_make_copy(const char*);

wchar_t *wstr_make_copy(const wchar_t*);

char *str_concat(size_t n, ...);

wchar_t *wstr_concat(size_t n, ...);

wchar_t *str_to_wstr(const char*);

char *wstr_to_str(const wchar_t*);
