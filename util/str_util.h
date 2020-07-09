#pragma once
#include <stddef.h>

char *str_make_copy(char*);

wchar_t *wstr_make_copy(wchar_t*);

char *str_concat(size_t n, ...);

wchar_t *wstr_concat(size_t n, ...);

wchar_t *str_to_wstr(char*);

char *wstr_to_str(wchar_t*);
