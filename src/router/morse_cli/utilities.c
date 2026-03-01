/*
 * Copyright 2021 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef MORSE_WIN_BUILD
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include "utilities.h"

int str_to_ip(const char* s, ipv4_addr_t* out)
{
    uint32_t addr;
    int ret = inet_pton(AF_INET, s, &addr);

    if (ret != 1)
    {
        return -1;
    }

    out->as_u32 = addr;
    return 0;
}

bool check_string_is_int(const char *str)
{
    if (*str == '-' && *(str+1) != '\0')
    {
        str++;
    }
    while (*str != '\0')
    {
        if (!isdigit(*str))
        {
            return false;
        }
        str++;
    }
    return true;
}

int str_to_uint16(const char *str, uint16_t *val)
{
    uint32_t local;
    int ret = str_to_uint32(str, &local);

    *val = (uint16_t) local;

    return ret;
}

int str_to_int8(const char *str, int8_t *val)
{
    int32_t local;
    int ret = str_to_int32(str, &local);

    *val = (int8_t) local;

    return ret;
}

int str_to_int8_range(const char *str, int8_t *val, const int8_t min, const int8_t max)
{
    int32_t local;
    int ret = str_to_int32_range(str, &local, min, max);

    *val = (int8_t) local;

    return ret;
}

int str_to_uint8(const char *str, uint8_t *val)
{
    uint32_t local;
    int ret = str_to_uint32(str, &local);

    *val = (uint8_t) local;

    return ret;
}

int str_to_uint8_range(const char *str, uint8_t *val, const uint8_t min, const uint8_t max)
{
    uint32_t local;
    int ret = str_to_uint32_range(str, &local, min, max);

    *val = (uint8_t) local;

    return ret;
}

int str_to_uint16_range(const char *str, uint16_t *val, const uint16_t min, const uint16_t max)
{
    uint32_t local;
    int ret = str_to_uint32_range(str, &local, min, max);

    *val = (uint16_t) local;

    return ret;
}

int str_to_int32(const char *str, int32_t *val)
{
    char *endptr = NULL;
    *val = strtol(str, &endptr, 0);

    if (endptr != str + strlen(str))
        return -1;
    return 0;
}

int str_to_uint32(const char *str, uint32_t *val)
{
    char *endptr = NULL;
    *val = strtoul(str, &endptr, 0);

    if (endptr != str + strlen(str))
        return -1;
    return 0;
}

int str_to_int32_range(const char *str, int32_t *val, const int32_t min, const int32_t max)
{
    char *endptr = NULL;
    *val = strtol(str, &endptr, 0);

    if ((endptr != str + strlen(str)) || (*val < min) || (*val > max))
        return -1;
    return 0;
}

int str_to_uint32_range(const char *str, uint32_t *val, const uint32_t min, const uint32_t max)
{
    char *endptr = NULL;
    *val = strtoul(str, &endptr, 0);

    if ((endptr != str + strlen(str)) || (*val < min) || (*val > max))
        return -1;
    return 0;
}

int str_to_uint64(const char *str, uint64_t *val)
{
    char *endptr = NULL;
    *val = strtoull(str, &endptr, 0);

    if (endptr != str + strlen(str))
        return -1;
    return 0;
}

static int hex2num(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return -1;
}

static int hex2byte(const char *hex)
{
    int a, b;

    a = hex2num(*hex++);
    if (a < 0)
        return -1;
    b = hex2num(*hex++);
    if (b < 0)
        return -1;

    return (a << 4) | b;
}

uint32_t popcount(uint32_t x)
{
    x  = x - ((x >> 1) & 0x55555555);
    x  = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x  = x + (x >> 4);
    x &= 0xF0F0F0F;
    return (x * 0x01010101) >> 24;
}

int32_t ctz(uint32_t x)
{
    uint32_t c = 32;

    if (!x)
    {
        return -1;
    }

    x &= ~x + 1;

    if (x) c--;
    if (x & 0x0000FFFF) c -= 16;
    if (x & 0x00FF00FF) c -= 8;
    if (x & 0x0F0F0F0F) c -= 4;
    if (x & 0x33333333) c -= 2;
    if (x & 0x55555555) c -= 1;

    return c;
}

int hexstr2bin(const char *hex, uint8_t *buf, size_t len)
{
    size_t i;
    int a;
    const char *ipos = hex;
    uint8_t *opos = buf;

    for (i = 0; i < len; i++)
    {
        a = hex2byte(ipos);
        if (a < 0)
            return -1;
        *opos++ = a;
        ipos += 2;
    }
    return 0;
}

int hexstr2uint32_arr(const char *hex, __le32 *buf, size_t len)
{
    size_t i, j;
    const char *ipos = hex;
    __le32 *opos = buf;
    int a = 0, tmp;

    if (strlen(hex) != 8 * len)
    {
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        a = 0;
        for (j = 0; j < 4; j++)
        {
            tmp = hex2byte(ipos);
            if (tmp < 0)
                return -1;
            a = a << 8 | tmp;
            ipos += 2;
        }
        *opos++ = htole32(a);
    }
    return 0;
}

void tolower_str(char *str)
{
    while (*str != '\0')
    {
        *str = tolower(*str);
        str++;
    }
}

#define MAX_EXPR_WIDTH  10
#define NUMBER_OF_EXPR  6

int expression_to_int(const char *str)
{
    char FALSE_EXPR[NUMBER_OF_EXPR][MAX_EXPR_WIDTH] = {
        "false",
        "disable",
        "no",
        "f",
        "n",
        "0"
    };

    char TRUE_EXPR[NUMBER_OF_EXPR][MAX_EXPR_WIDTH] = {
        "true",
        "enable",
        "yes",
        "t",
        "y",
        "1"
    };

    for (int i = 0; i < NUMBER_OF_EXPR; i++)
    {
        if (strncasecmp(TRUE_EXPR[i], str, MAX(strlen(str), strlen(TRUE_EXPR[i]))) == 0)
        {
            return 1;
        }
        else if (strncasecmp(FALSE_EXPR[i], str, MAX(strlen(str), strlen(FALSE_EXPR[i]))) == 0)
        {
            return 0;
        }
    }

    return -1;
}

char *strip(char *s)
{
    if (!s)
        return s;

    char *start = s;
    char *end = s + strlen(s) - 1;

    while (isspace(*start))
    {
        start++;
    }

    while ((end >= start) && isspace(*end))
    {
        *end-- = '\0';
    }
    return start;
}

uint8_t crc7_gen(uint64_t number, uint8_t bit_count)
{
    int ii;
    uint32_t reg_val = 0;

    for (ii = bit_count; ii > 0; ii -= 8)
    {
        uint32_t bit_idx;
        uint8_t shift_val = ii - 8;
        uint8_t octet_val = (number >> shift_val) & 0xFF;

        for (bit_idx = 0; bit_idx < 8; bit_idx++)
        {
            reg_val <<= 1;
            if (((octet_val ^ reg_val) & 0x80) > 0)
                reg_val ^= 9;

            octet_val = (octet_val << 1) & 0xFF;
        }

        reg_val &= 0x7F;
    }

    return (uint8_t)reg_val;
}

uint16_t crc16_gen(uint8_t *buff, size_t len)
{
    int ii, jj;
    uint32_t crc = 0;
    size_t bit_count = len * 8;

    for (ii = 0; ii < len; ii ++)
    {
        /* Let's go from MSb first. */
        for (jj = 7; jj >= 0; jj--)
        {
            uint32_t val = ((buff[ii] >> jj) ^ (crc >> 15)) & 0x1;

            crc ^= ((val << 4) | (val << 11));
            crc = (crc << 1) | val;
            bit_count -= 1;
            if (bit_count == 0)
                break;
        }
        if (bit_count == 0)
            break;
    }

    return (crc & 0xFFFF);
}

bool crc16_check(uint8_t *buff, size_t len, uint16_t crc16)
{
    uint16_t buff_crc16;

    if (!buff)
        return false;

    buff_crc16 = crc16_gen(buff, len);

    return (crc16 == buff_crc16);
}

size_t get_file_size(FILE *infile)
{
    struct stat file_stats;

    if (!fstat(fileno(infile), &file_stats))
        return file_stats.st_size;

    return -1;
}

void load_file(FILE *infile, uint8_t **buf)
{
    struct stat file_stats;
    uint8_t *ptr = NULL;
    size_t bytes_read = 0;

    *buf = NULL;

    errno = 0;
    if (fstat(fileno(infile), &file_stats) != 0)
    {
        mctrl_err("File stat failed - errno %d\n", errno);
        return;
    }

    if (fseek(infile, 0, SEEK_SET))
    {
        mctrl_err("Seek failed - errno %d\n", errno);
        return;
    }

    ptr = malloc(file_stats.st_size);
    if (ptr == NULL)
    {
        return;
    }

    while (bytes_read < file_stats.st_size)
    {
        bytes_read += fread(ptr + bytes_read, 1, file_stats.st_size - bytes_read, infile);
        if (ferror(infile))
        {
            mctrl_err("error reading file\n");
            free(ptr);
            return;
        }
    }

    *buf = ptr;
}

static inline int _mkdir(const char *dir)
{
#ifdef __WINDOWS__
    return mkdir(dir);
#else
    return mkdir(dir, S_IRWXU | S_IRWXG);
#endif
}

int mkdir_path(const char *dir)
{
    char tmp[MORSE_FILENAME_LEN_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if ((_mkdir(tmp) != 0) && (errno != EEXIST))
                return errno;
            *p = '/';
        }
    }
    if ((_mkdir(tmp) != 0) && (errno != EEXIST))
        return -1;

    return 0;
}

bool is_file(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1)
    {
        return false; /* does not exist */
    }

    if (statbuf.st_mode & S_IFREG)
    {
        return true;
    }
    return false;
}

bool is_dir(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1)
    {
        return false; /* does not exist */
    }

    if (statbuf.st_mode & S_IFDIR)
    {
        return true;
    }
    return false;
}

void mctrl_vprint(const char* format, va_list args)
{
    vfprintf(stdout, format, args);
}

void mctrl_print(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    mctrl_vprint(format, args);
    va_end(args);
}

void mctrl_verr(const char* format, va_list args)
{
    vfprintf(stderr, format, args);
}

void mctrl_err(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    mctrl_verr(format, args);
    va_end(args);
}
