/**
 * @file common.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "compat.h"
#include "tree_schema_internal.h"

void *
ly_realloc(void *ptr, size_t size)
{
    void *new_mem;

    new_mem = realloc(ptr, size);
    if (!new_mem) {
        free(ptr);
    }

    return new_mem;
}

char *
ly_strnchr(const char *s, int c, size_t len)
{
    for ( ; len && (*s != (char)c); ++s, --len) {}
    return len ? (char *)s : NULL;
}

int
ly_strncmp(const char *refstr, const char *str, size_t str_len)
{
    int rc = strncmp(refstr, str, str_len);

    if (!rc && (refstr[str_len] == '\0')) {
        return 0;
    } else {
        return rc ? rc : 1;
    }
}

#define LY_OVERFLOW_ADD(MAX, X, Y) ((X > MAX - Y) ? 1 : 0)

#define LY_OVERFLOW_MUL(MAX, X, Y) ((X > MAX / Y) ? 1 : 0)

LY_ERR
ly_strntou8(const char *nptr, size_t len, uint8_t *ret)
{
    uint8_t num = 0, dig, dec_pow;

    if (len > 3) {
        /* overflow for sure */
        return LY_EDENIED;
    }

    dec_pow = 1;
    for ( ; len && isdigit(nptr[len - 1]); --len) {
        dig = nptr[len - 1] - 48;

        if (LY_OVERFLOW_MUL(UINT8_MAX, dig, dec_pow)) {
            return LY_EDENIED;
        }
        dig *= dec_pow;

        if (LY_OVERFLOW_ADD(UINT8_MAX, num, dig)) {
            return LY_EDENIED;
        }
        num += dig;

        dec_pow *= 10;
    }

    if (len) {
        return LY_EVALID;
    }
    *ret = num;
    return LY_SUCCESS;
}

LY_ERR
ly_value_prefix_next(const char *str_begin, const char *str_end, uint32_t *len, ly_bool *is_prefix, const char **str_next)
{
    const char *stop, *prefix;
    size_t bytes_read;
    uint32_t c;
    ly_bool prefix_found;
    LY_ERR ret = LY_SUCCESS;

    assert(len && is_prefix && str_next);

#define IS_AT_END(PTR, STR_END) (STR_END ? PTR == STR_END : !(*PTR))

    *str_next = NULL;
    *is_prefix = 0;
    *len = 0;

    if (!str_begin || !(*str_begin) || (str_begin == str_end)) {
        return ret;
    }

    stop = str_begin;
    prefix = NULL;
    prefix_found = 0;

    do {
        /* look for the beginning of the YANG value */
        do {
            LY_CHECK_RET(ly_getutf8(&stop, &c, &bytes_read));
        } while (!is_xmlqnamestartchar(c) && !IS_AT_END(stop, str_end));

        if (IS_AT_END(stop, str_end)) {
            break;
        }

        /* maybe the prefix was found */
        prefix = stop - bytes_read;

        /* look for the the end of the prefix */
        do {
            LY_CHECK_RET(ly_getutf8(&stop, &c, &bytes_read));
        } while (is_xmlqnamechar(c) && !IS_AT_END(stop, str_end));

        prefix_found = c == ':' ? 1 : 0;

        /* if it wasn't the prefix, keep looking */
    } while (!IS_AT_END(stop, str_end) && !prefix_found);

    if ((str_begin == prefix) && prefix_found) {
        /* prefix found at the beginning of the input string */
        *is_prefix = 1;
        *str_next = IS_AT_END(stop, str_end) ? NULL : stop;
        *len = (stop - bytes_read) - str_begin;
    } else if ((str_begin != prefix) && (prefix_found)) {
        /* there is a some string before prefix */
        *str_next = prefix;
        *len = prefix - str_begin;
    } else {
        /* no prefix found */
        *len = stop - str_begin;
    }

#undef IS_AT_END

    return ret;
}

LY_ERR
ly_getutf8(const char **input, uint32_t *utf8_char, size_t *bytes_read)
{
    uint32_t c, aux;
    size_t len;

    if (bytes_read) {
        (*bytes_read) = 0;
    }

    c = (*input)[0];
    LY_CHECK_RET(!c, LY_EINVAL);

    if (!(c & 0x80)) {
        /* one byte character */
        len = 1;

        if ((c < 0x20) && (c != 0x9) && (c != 0xa) && (c != 0xd)) {
            return LY_EINVAL;
        }
    } else if ((c & 0xe0) == 0xc0) {
        /* two bytes character */
        len = 2;

        aux = (*input)[1];
        if ((aux & 0xc0) != 0x80) {
            return LY_EINVAL;
        }
        c = ((c & 0x1f) << 6) | (aux & 0x3f);

        if (c < 0x80) {
            return LY_EINVAL;
        }
    } else if ((c & 0xf0) == 0xe0) {
        /* three bytes character */
        len = 3;

        c &= 0x0f;
        for (uint64_t i = 1; i <= 2; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                return LY_EINVAL;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if ((c < 0x800) || ((c > 0xd7ff) && (c < 0xe000)) || (c > 0xfffd)) {
            return LY_EINVAL;
        }
    } else if ((c & 0xf8) == 0xf0) {
        /* four bytes character */
        len = 4;

        c &= 0x07;
        for (uint64_t i = 1; i <= 3; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                return LY_EINVAL;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if ((c < 0x1000) || (c > 0x10ffff)) {
            return LY_EINVAL;
        }
    } else {
        return LY_EINVAL;
    }

    (*utf8_char) = c;
    (*input) += len;
    if (bytes_read) {
        (*bytes_read) = len;
    }
    return LY_SUCCESS;
}

LY_ERR
ly_pututf8(char *dst, uint32_t value, size_t *bytes_written)
{
    if (value < 0x80) {
        /* one byte character */
        if ((value < 0x20) &&
                (value != 0x09) &&
                (value != 0x0a) &&
                (value != 0x0d)) {
            return LY_EINVAL;
        }

        dst[0] = value;
        (*bytes_written) = 1;
    } else if (value < 0x800) {
        /* two bytes character */
        dst[0] = 0xc0 | (value >> 6);
        dst[1] = 0x80 | (value & 0x3f);
        (*bytes_written) = 2;
    } else if (value < 0xfffe) {
        /* three bytes character */
        if (((value & 0xf800) == 0xd800) ||
                ((value >= 0xfdd0) && (value <= 0xfdef))) {
            /* exclude surrogate blocks %xD800-DFFF */
            /* exclude noncharacters %xFDD0-FDEF */
            return LY_EINVAL;
        }

        dst[0] = 0xe0 | (value >> 12);
        dst[1] = 0x80 | ((value >> 6) & 0x3f);
        dst[2] = 0x80 | (value & 0x3f);

        (*bytes_written) = 3;
    } else if (value < 0x10fffe) {
        if ((value & 0xffe) == 0xffe) {
            /* exclude noncharacters %xFFFE-FFFF, %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
             * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
             * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
            return LY_EINVAL;
        }
        /* four bytes character */
        dst[0] = 0xf0 | (value >> 18);
        dst[1] = 0x80 | ((value >> 12) & 0x3f);
        dst[2] = 0x80 | ((value >> 6) & 0x3f);
        dst[3] = 0x80 | (value & 0x3f);

        (*bytes_written) = 4;
    } else {
        return LY_EINVAL;
    }
    return LY_SUCCESS;
}

/**
 * @brief Static table of the UTF8 characters lengths according to their first byte.
 */
static const unsigned char utf8_char_length_table[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

size_t
ly_utf8len(const char *str, size_t bytes)
{
    size_t len = 0;
    const char *ptr = str;

    while (((size_t)(ptr - str) < bytes) && *ptr) {
        ++len;
        ptr += utf8_char_length_table[((unsigned char)(*ptr))];
    }
    return len;
}

size_t
LY_VCODE_INSTREXP_len(const char *str)
{
    size_t len = 0;

    if (!str) {
        return len;
    } else if (!str[0]) {
        return 1;
    }
    for (len = 1; len < LY_VCODE_INSTREXP_MAXLEN && str[len]; ++len) {}
    return len;
}

LY_ERR
ly_mmap(struct ly_ctx *ctx, int fd, size_t *length, void **addr)
{
    struct stat sb;
    long pagesize;
    size_t m;

    assert(length);
    assert(addr);
    assert(fd >= 0);

    if (fstat(fd, &sb) == -1) {
        LOGERR(ctx, LY_ESYS, "Failed to stat the file descriptor (%s) for the mmap().", strerror(errno));
        return LY_ESYS;
    }
    if (!S_ISREG(sb.st_mode)) {
        LOGERR(ctx, LY_EINVAL, "File to mmap() is not a regular file.");
        return LY_ESYS;
    }
    if (!sb.st_size) {
        *addr = NULL;
        return LY_SUCCESS;
    }
    pagesize = sysconf(_SC_PAGESIZE);

    m = sb.st_size % pagesize;
    if (m && (pagesize - m >= 1)) {
        /* there will be enough space (at least 1 byte) after the file content mapping to provide zeroed NULL-termination byte */
        *length = sb.st_size + 1;
        *addr = mmap(NULL, *length, PROT_READ, MAP_PRIVATE, fd, 0);
    } else {
        /* there will not be enough bytes after the file content mapping for the additional bytes and some of them
         * would overflow into another page that would not be zerroed and any access into it would generate SIGBUS.
         * Therefore we have to do the following hack with double mapping. First, the required number of bytes
         * (including the additinal bytes) is required as anonymous and thus they will be really provided (actually more
         * because of using whole pages) and also initialized by zeros. Then, the file is mapped to the same address
         * where the anonymous mapping starts. */
        *length = sb.st_size + pagesize;
        *addr = mmap(NULL, *length, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *addr = mmap(*addr, sb.st_size, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);
    }
    if (*addr == MAP_FAILED) {
        LOGERR(ctx, LY_ESYS, "mmap() failed (%s).", strerror(errno));
        return LY_ESYS;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_munmap(void *addr, size_t length)
{
    if (munmap(addr, length)) {
        return LY_ESYS;
    }
    return LY_SUCCESS;
}

LY_ERR
ly_strcat(char **dest, const char *format, ...)
{
    va_list fp;
    char *addition = NULL;
    size_t len;

    va_start(fp, format);
    len = vasprintf(&addition, format, fp);
    len += (*dest ? strlen(*dest) : 0) + 1;

    if (*dest) {
        *dest = ly_realloc(*dest, len);
        if (!*dest) {
            va_end(fp);
            return LY_EMEM;
        }
        *dest = strcat(*dest, addition);
        free(addition);
    } else {
        *dest = addition;
    }

    va_end(fp);
    return LY_SUCCESS;
}

LY_ERR
ly_parse_int(const char *val_str, size_t val_len, int64_t min, int64_t max, int base, int64_t *ret)
{
    LY_ERR rc = LY_SUCCESS;
    char *ptr, *str;
    int64_t i;

    LY_CHECK_ARG_RET(NULL, val_str, val_str[0], val_len, LY_EINVAL);

    /* duplicate the value */
    str = strndup(val_str, val_len);
    LY_CHECK_RET(!str, LY_EMEM);

    /* parse the value to avoid accessing following bytes */
    errno = 0;
    i = strtoll(str, &ptr, base);
    if (errno || (ptr == str)) {
        /* invalid string */
        rc = LY_EVALID;
    } else if ((i < min) || (i > max)) {
        /* invalid number */
        rc = LY_EDENIED;
    } else if (*ptr) {
        while (isspace(*ptr)) {
            ++ptr;
        }
        if (*ptr) {
            /* invalid characters after some number */
            rc = LY_EVALID;
        }
    }

    /* cleanup */
    free(str);
    if (!rc) {
        *ret = i;
    }
    return rc;
}

LY_ERR
ly_parse_uint(const char *val_str, size_t val_len, uint64_t max, int base, uint64_t *ret)
{
    LY_ERR rc = LY_SUCCESS;
    char *ptr, *str;
    uint64_t u;

    LY_CHECK_ARG_RET(NULL, val_str, val_str[0], val_len, LY_EINVAL);

    /* duplicate the value to avoid accessing following bytes */
    str = strndup(val_str, val_len);
    LY_CHECK_RET(!str, LY_EMEM);

    /* parse the value */
    errno = 0;
    u = strtoull(str, &ptr, base);
    if (errno || (ptr == str)) {
        /* invalid string */
        rc = LY_EVALID;
    } else if ((u > max) || (u && (str[0] == '-'))) {
        /* invalid number */
        rc = LY_EDENIED;
    } else if (*ptr) {
        while (isspace(*ptr)) {
            ++ptr;
        }
        if (*ptr) {
            /* invalid characters after some number */
            rc = LY_EVALID;
        }
    }

    /* cleanup */
    free(str);
    if (!rc) {
        *ret = u;
    }
    return rc;
}

/**
 * @brief Parse an identifier.
 *
 * ;; An identifier MUST NOT start with (('X'|'x') ('M'|'m') ('L'|'l'))
 * identifier          = (ALPHA / "_")
 *                       *(ALPHA / DIGIT / "_" / "-" / ".")
 *
 * @param[in,out] id Identifier to parse. When returned, it points to the first character which is not part of the identifier.
 * @return LY_ERR value: LY_SUCCESS or LY_EINVAL in case of invalid starting character.
 */
static LY_ERR
lys_parse_id(const char **id)
{
    assert(id && *id);

    if (!is_yangidentstartchar(**id)) {
        return LY_EINVAL;
    }
    ++(*id);

    while (is_yangidentchar(**id)) {
        ++(*id);
    }
    return LY_SUCCESS;
}

LY_ERR
ly_parse_nodeid(const char **id, const char **prefix, size_t *prefix_len, const char **name, size_t *name_len)
{
    assert(id && *id);
    assert(prefix && prefix_len);
    assert(name && name_len);

    *prefix = *id;
    *prefix_len = 0;
    *name = NULL;
    *name_len = 0;

    LY_CHECK_RET(lys_parse_id(id));
    if (**id == ':') {
        /* there is prefix */
        *prefix_len = *id - *prefix;
        ++(*id);
        *name = *id;

        LY_CHECK_RET(lys_parse_id(id));
        *name_len = *id - *name;
    } else {
        /* there is no prefix, so what we have as prefix now is actually the name */
        *name = *prefix;
        *name_len = *id - *name;
        *prefix = NULL;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_parse_instance_predicate(const char **pred, size_t limit, LYD_FORMAT format,
        const char **prefix, size_t *prefix_len, const char **id, size_t *id_len, const char **value, size_t *value_len,
        const char **errmsg)
{
    LY_ERR ret = LY_EVALID;
    const char *in = *pred;
    size_t offset = 1;
    uint8_t expr = 0; /* 0 - position predicate; 1 - leaf-list-predicate; 2 - key-predicate */
    char quot;

    assert(in[0] == '[');

    *prefix = *id = *value = NULL;
    *prefix_len = *id_len = *value_len = 0;

    /* leading *WSP */
    for ( ; isspace(in[offset]); offset++) {}

    if (isdigit(in[offset])) {
        /* pos: "[" *WSP positive-integer-value *WSP "]" */
        if (in[offset] == '0') {
            /* zero */
            *errmsg = "The position predicate cannot be zero.";
            goto error;
        }

        /* positive-integer-value */
        *value = &in[offset++];
        for ( ; isdigit(in[offset]); offset++) {}
        *value_len = &in[offset] - *value;

    } else if (in[offset] == '.') {
        /* leaf-list-predicate: "[" *WSP "." *WSP "=" *WSP quoted-string *WSP "]" */
        *id = &in[offset];
        *id_len = 1;
        offset++;
        expr = 1;
    } else if (in[offset] == '-') {
        /* typically negative value */
        *errmsg = "Invalid instance predicate format (negative position or invalid node-identifier).";
        goto error;
    } else {
        /* key-predicate: "[" *WSP node-identifier *WSP "=" *WSP quoted-string *WSP "]" */
        in = &in[offset];
        if (ly_parse_nodeid(&in, prefix, prefix_len, id, id_len)) {
            *errmsg = "Invalid node-identifier.";
            goto error;
        }
        if ((format == LYD_XML) && !(*prefix)) {
            /* all node names MUST be qualified with explicit namespace prefix */
            *errmsg = "Missing prefix of a node name.";
            goto error;
        }
        offset = in - *pred;
        in = *pred;
        expr = 2;
    }

    if (expr) {
        /*  *WSP "=" *WSP quoted-string *WSP "]" */
        for ( ; isspace(in[offset]); offset++) {}

        if (in[offset] != '=') {
            if (expr == 1) {
                *errmsg = "Unexpected character instead of \'=\' in leaf-list-predicate.";
            } else { /* 2 */
                *errmsg = "Unexpected character instead of \'=\' in key-predicate.";
            }
            goto error;
        }
        offset++;
        for ( ; isspace(in[offset]); offset++) {}

        /* quoted-string */
        quot = in[offset++];
        if ((quot != '\'') && (quot != '\"')) {
            *errmsg = "String value is not quoted.";
            goto error;
        }
        *value = &in[offset];
        for ( ; offset < limit && (in[offset] != quot || (offset && in[offset - 1] == '\\')); offset++) {}
        if (in[offset] == quot) {
            *value_len = &in[offset] - *value;
            offset++;
        } else {
            *errmsg = "Value is not terminated quoted-string.";
            goto error;
        }
    }

    /* *WSP "]" */
    for ( ; isspace(in[offset]); offset++) {}
    if (in[offset] != ']') {
        if (expr == 0) {
            *errmsg = "Predicate (pos) is not terminated by \']\' character.";
        } else if (expr == 1) {
            *errmsg = "Predicate (leaf-list-predicate) is not terminated by \']\' character.";
        } else { /* 2 */
            *errmsg = "Predicate (key-predicate) is not terminated by \']\' character.";
        }
        goto error;
    }
    offset++;

    if (offset <= limit) {
        *pred = &in[offset];
        return LY_SUCCESS;
    }

    /* we read after the limit */
    *errmsg = "Predicate is incomplete.";
    *prefix = *id = *value = NULL;
    *prefix_len = *id_len = *value_len = 0;
    offset = limit;
    ret = LY_EINVAL;

error:
    *pred = &in[offset];
    return ret;
}
