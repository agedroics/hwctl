#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <str_util.h>
#ifndef _WIN32
#include <wchar.h>
#endif

char *str_make_copy(char *str) {
    return str_concat(1, str);
}

wchar_t *wstr_make_copy(wchar_t *str) {
    return wstr_concat(1, str);
}

char *str_concat(size_t n, ...) {
    va_list args;
    va_start(args, n);
    size_t len = 0;
    for (unsigned i = 0; i < n; ++i) {
        len += strlen(va_arg(args, char*));
    }
    va_end(args);

    char *str = malloc(len + 1);
    va_start(args, n);
    len = 0;
    for (unsigned i = 0; i < n; ++i) {
        char *arg = va_arg(args, char*);
        size_t arg_len = strlen(arg);
        memcpy(str + len, arg, arg_len);
        len += arg_len;
    }
    str[len] = 0;
    va_end(args);

    return str;
}

wchar_t *wstr_concat(size_t n, ...) {
    va_list args;
    va_start(args, n);
    size_t len = 0;
    for (unsigned i = 0; i < n; ++i) {
        len += wcslen(va_arg(args, wchar_t*));
    }
    va_end(args);

    wchar_t *wstr = malloc((len + 1) * sizeof(wchar_t));
    va_start(args, n);
    len = 0;
    for (unsigned i = 0; i < n; ++i) {
        wchar_t *arg = va_arg(args, wchar_t*);
        size_t arg_len = wcslen(arg);
        memcpy(wstr + len, arg, arg_len * sizeof(wchar_t));
        len += arg_len;
    }
    wstr[len] = 0;
    va_end(args);

    return wstr;
}

wchar_t *str_to_wstr(char *str) {
    size_t wchars = strlen(str) + 1;
    wchar_t *wstr = malloc(wchars * sizeof(wchar_t));
    mbstowcs(wstr, str, wchars);
    return wstr;
}

char *wstr_to_str(wchar_t *wstr) {
    size_t bytes = wcslen(wstr) * sizeof(wchar_t) + 1;
    char *str = malloc(bytes);
    wcstombs(str, wstr, bytes);
    return str;
}
