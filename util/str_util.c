#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <str_util.h>

char *str_copy(char *src) {
    return str_concat(1, src);
}

wchar_t *wstr_copy(wchar_t *src) {
    return wstr_concat(1, src);
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
        memcpy(str + len, arg, arg_len + 1);
        len += arg_len;
    }
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

    wchar_t *str = malloc((len + 1) * sizeof(wchar_t));
    va_start(args, n);
    len = 0;
    for (unsigned i = 0; i < n; ++i) {
        wchar_t *arg = va_arg(args, wchar_t*);
        size_t arg_len = wcslen(arg);
        memcpy(str + len, arg, (arg_len + 1) * sizeof(wchar_t));
        len += arg_len;
    }
    va_end(args);

    return str;
}
