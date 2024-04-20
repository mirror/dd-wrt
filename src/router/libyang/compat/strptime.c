/*
 * This comes from the musl C library which has been licensed under the permissive MIT license.
 *
 * Downloaded from https://git.musl-libc.org/cgit/musl/plain/src/time/strptime.c,
 * commit 98e688a9da5e7b2925dda17a2d6820dddf1fb287.
 *
 * Lobotomized to remove references to nl_langinfo().
 * Adjusted coding style to fit libyang's uncrustify rules.
 * */

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *
strptime(const char * restrict s, const char * restrict f, struct tm * restrict tm)
{
    int i, w, neg, adj, min, range, *dest, dummy;
    int want_century = 0, century = 0, relyear = 0;

    while (*f) {
        if (*f != '%') {
            if (isspace(*f)) {
                for ( ; *s && isspace(*s); s++) {}
            } else if (*s != *f) {
                return 0;
            } else {
                s++;
            }
            f++;
            continue;
        }
        f++;
        if (*f == '+') {
            f++;
        }
        if (isdigit(*f)) {
            char *new_f;

            w = strtoul(f, &new_f, 10);
            f = new_f;
        } else {
            w = -1;
        }
        adj = 0;
        switch (*f++) {
        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'h':
        case 'c':
            goto fail_nl_langinfo;
        case 'C':
            dest = &century;
            if (w < 0) {
                w = 2;
            }
            want_century |= 2;
            goto numeric_digits;
        case 'd':
        case 'e':
            dest = &tm->tm_mday;
            min = 1;
            range = 31;
            goto numeric_range;
        case 'D':
            s = strptime(s, "%m/%d/%y", tm);
            if (!s) {
                return 0;
            }
            break;
        case 'H':
            dest = &tm->tm_hour;
            min = 0;
            range = 24;
            goto numeric_range;
        case 'I':
            dest = &tm->tm_hour;
            min = 1;
            range = 12;
            goto numeric_range;
        case 'j':
            dest = &tm->tm_yday;
            min = 1;
            range = 366;
            adj = 1;
            goto numeric_range;
        case 'm':
            dest = &tm->tm_mon;
            min = 1;
            range = 12;
            adj = 1;
            goto numeric_range;
        case 'M':
            dest = &tm->tm_min;
            min = 0;
            range = 60;
            goto numeric_range;
        case 'n':
        case 't':
            for ( ; *s && isspace(*s); s++) {}
            break;
        case 'p':
        case 'r':
            goto fail_nl_langinfo;
        case 'R':
            s = strptime(s, "%H:%M", tm);
            if (!s) {
                return 0;
            }
            break;
        case 'S':
            dest = &tm->tm_sec;
            min = 0;
            range = 61;
            goto numeric_range;
        case 'T':
            s = strptime(s, "%H:%M:%S", tm);
            if (!s) {
                return 0;
            }
            break;
        case 'U':
        case 'W':
            /* Throw away result, for now. (FIXME?) */
            dest = &dummy;
            min = 0;
            range = 54;
            goto numeric_range;
        case 'w':
            dest = &tm->tm_wday;
            min = 0;
            range = 7;
            goto numeric_range;
        case 'x':
        case 'X':
            goto fail_nl_langinfo;
        case 'y':
            dest = &relyear;
            w = 2;
            want_century |= 1;
            goto numeric_digits;
        case 'Y':
            dest = &tm->tm_year;
            if (w < 0) {
                w = 4;
            }
            adj = 1900;
            want_century = 0;
            goto numeric_digits;
        case '%':
            if (*s++ != '%') {
                return 0;
            }
            break;
        default:
            return 0;
numeric_range:
            if (!isdigit(*s)) {
                return 0;
            }
            *dest = 0;
            for (i = 1; i <= min + range && isdigit(*s); i *= 10) {
                *dest = *dest * 10 + *s++ - '0';
            }
            if (*dest - min >= range) {
                return 0;
            }
            *dest -= adj;
            switch ((char *)dest - (char *)tm) {
            case offsetof(struct tm, tm_yday):
                ;
            }
            goto update;
numeric_digits:
            neg = 0;
            if (*s == '+') {
                s++;
            } else if (*s == '-') {
                neg = 1, s++;
            }
            if (!isdigit(*s)) {
                return 0;
            }
            for (*dest = i = 0; i < w && isdigit(*s); i++) {
                *dest = *dest * 10 + *s++ - '0';
            }
            if (neg) {
                *dest = -*dest;
            }
            *dest -= adj;
            goto update;
update:
            // FIXME
            ;
        }
    }
    if (want_century) {
        tm->tm_year = relyear;
        if (want_century & 2) {
            tm->tm_year += century * 100 - 1900;
        } else if (tm->tm_year <= 68) {
            tm->tm_year += 100;
        }
    }
    return (char *)s;
fail_nl_langinfo:
    fprintf(stderr, "strptime: nl_langinfo not available");
    return NULL;
}
