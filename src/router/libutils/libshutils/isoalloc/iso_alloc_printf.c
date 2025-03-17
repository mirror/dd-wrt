/* iso_alloc_printf.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"
#include <stdarg.h>

#if __GNUC__ && !__clang__
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#define INTERNAL_HIDDEN __attribute__((visibility("hidden")))

/* This primitive printf implementation is only ever called
 * called from the LOG and LOG_AND_ABORT macros. We need to
 * be able to print basic log messages without invoking
 * malloc() or we run the risk of using a corrupted heap */
static int8_t fmt_buf[64];
static const int8_t asc_hex[] = "0123456789abcdef";

INTERNAL_HIDDEN int8_t *_fmt(uint64_t n, uint32_t base) {
    int8_t *ptr;
    uint32_t count = 0;

    __builtin_memset(fmt_buf, 0x0, sizeof(fmt_buf));
    ptr = &fmt_buf[63];

    while(n != 0) {
        *--ptr = asc_hex[n % base];
        n /= base;
        count++;
    };

    if(count == 0) {
        ptr = (int8_t *) "0";
    }

    return ptr;
}

INTERNAL_HIDDEN void _iso_alloc_printf(int32_t fd, const char *f, ...) {
    if(UNLIKELY(f == NULL)) {
        return;
    }

    uint64_t i;
    uint32_t j;
    char *s;
    va_list arg;
    va_start(arg, f);
    char out[65535];
    char *p = out;
    __builtin_memset(p, 0x0, sizeof(out));

    for(const char *idx = f; *idx != '\0'; idx++) {
        if(p >= (char *) (out + sizeof(out))) {
            break;
        }

        while(*idx != '%' && *idx != '\0') {
            *p = *idx;
            p++;

            if(*idx == '\n') {
                break;
            }

            idx++;
        }

        idx++;

        if(*idx == '\0') {
            break;
        }

        if(*idx == 'x' || *idx == 'p') {
            i = va_arg(arg, int64_t);
            s = (char *) _fmt(i, 16);
            __builtin_strncpy(p, s, strlen(s));
            p += strlen(s);
        } else if(*idx == 'd' || *idx == 'u') {
            j = va_arg(arg, int32_t);

            if(0 > j) {
                j = -j;
                *p = '-';
                p++;
            }

            s = (char *) _fmt(j, 10);

            __builtin_strncpy(p, s, strlen(s));
            p += strlen(s);
        } else if(*idx == 'l') {
            if(*(idx + 1) == 'd' || *(idx + 1) == 'u') {
                idx++;
            }

            i = va_arg(arg, int64_t);

            if(0 > i) {
                i = -i;
                *p = '-';
                p++;
            }

            s = (char *) _fmt(i, 10);

            __builtin_strncpy(p, s, strlen(s));
            p += strlen(s);
        } else if(*idx == 's') {
            s = va_arg(arg, char *);

            if(s == NULL) {
                break;
            }

            __builtin_strncpy(p, s, strlen(s));
            p += strlen(s);
        }
    }

    (void) !write(fd, out, strlen(out));
    va_end(arg);
}
