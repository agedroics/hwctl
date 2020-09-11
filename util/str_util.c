#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <str_util.h>

char *str_make_copy(char *str) {
    return str_concat(1, str);
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

char *wstr_to_str(wchar_t *wstr) {
    size_t bytes = wcslen(wstr) * sizeof(wchar_t) + 1;
    char *str = malloc(bytes);
    wcstombs(str, wstr, bytes);
    return str;
}
