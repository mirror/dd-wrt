/*
 * Copyright 2021 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef MORSE_WIN_BUILD
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <dirent.h>

#include "morsectrl.h"
#include "portable_endian.h"

#define XSTR(s) #s
#define STR(s)  XSTR(s)

#define MAX(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#define MIN(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })

#define BIT(N) (1UL << (N))

#define DEVICE_NAME_LEN         (256)

#define MAC_ADDR_LEN    (6)
#define MACSTR          "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a)      (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_CMD_REGEX   "([a-f0-9]{2}:){5}([a-f0-9]{2})"

#define IPSTR          "%d.%d.%d.%d"
#define IP2STR(a)      (a)[0], (a)[1], (a)[2], (a)[3]

#define MORSE_FILENAME_LEN_MAX (sizeof(((struct dirent *)0)->d_name))

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define SECS_TO_MSECS(_secs) ((_secs) * 1000)

#define NSS_IDX_TO_NSS(x) ((x) + 1)
#define NSS_TO_NSS_IDX(x) ((x) - 1)

/* Bitmap utilities */
/* Extract the value of _fld from _var and return the result  */
#define BMGET(_var, _fld)       (((_var) & (_fld)) >> ctz(_fld))
/* Shift the _var bits to the _fld position and return the result */
#define BMSET(_var, _fld)       (((_var) << ctz(_fld)) & (_fld))
/* Replace the _fld bits of _var with _val */
#define BMUPD(_var, _fld, _val) (((_var) & (~_fld)) | ((_val) << ctz(_fld)))

#define MORSE_IS_BIT_SET(_field, _bit) (!!((_field) & BIT(_bit)))

/**
 * Validate the given condition -- if false, print a message and exit.
 *
 * @param _condition    The condition to check, should evaluate to @c true in the normal case.
 * @param _message      The message to display on error.
 */
#define MCTRL_ASSERT(_condition, _message, ...)                         \
    do {                                                                \
        if (!(_condition)) {                                            \
            mctrl_err("Assertion failure at %s:%d: " _message "\n",     \
                      __FILE__, __LINE__, ##__VA_ARGS__);               \
            abort();                                                    \
        }                                                               \
    } while (0)

typedef union
{
    uint32_t as_u32;
    uint8_t octet[4];
} ipv4_addr_t;

/**
 * @brief Print a message to stdout
 *
 * @param format The format of the message
 * @param ... Variable length arguments
 */
void mctrl_print(const char* format, ...);

/**
 * @brief Print a message to stdout
 *
 * @param format The format of the message
 * @param args Variadic argument list
 */
void mctrl_vprint(const char* format, va_list args);

/**
 * @brief Print a message to stderr
 *
 * @param format The format of the message
 * @param ... Variable length arguments
 */
void mctrl_err(const char* format, ...);

/**
 * @brief Print a message to stderr
 *
 * @param format The format of the message
 * @param args Variadic argument list
 */
void mctrl_verr(const char* format, va_list args);

/**
 * @brief Convert str to ipv4_addr_t
 *
 * @param s String
 * @param out Object to store the IP address in.

 * @return 0 if success else -1
 */
int str_to_ip(const char* s, ipv4_addr_t* out);

/**
 *  Checks if a string is an integer (ie contains only numerals).
 *
 * @note Does not ignore whitespace.
 *
 * @param str A null terminated string
 *
 * @return `true` if string contains only digits, `false` otherwise
 */
bool check_string_is_int(const char *str);

/**
 *  Converts dec/hex int string to int32 variable
 *
 * @note Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_int32(const char str[], int32_t *val);

/**
 *  Converts dec/hex uint string to uint32 variable
 *
 * Note: Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint32(const char str[], uint32_t *val);

/**
 *  Converts dec/hex uint string to uint16 variable
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint16(const char *str, uint16_t *val);

/**
 *  Converts dec/hex int string to int8 variable
 *
 * @note Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_int8(const char *str, int8_t *val);

/**
 *  Converts dec/hex int string to uint8 variable
 *
 * @note Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint8(const char *str, uint8_t *val);

/**
 *  Converts dec/hex uint string to int32 variable within the min and max
 *
 * Note: Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 * @param min Acceptable min value of the converted value
 * @param max Acceptable max value of the converted value
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_int32_range(const char *str, int32_t *val, const int32_t min, const int32_t max);

/**
 *  Converts dec/hex uint string to uint32 variable within the min and max
 *
 * Note: Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 * @param min Acceptable min value of the converted value
 * @param max Acceptable max value of the converted value
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint32_range(const char *str, uint32_t *val, const uint32_t min, const uint32_t max);

/**
 *  Converts dec/hex uint string to int8 variable within the min and max
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 * @param min Acceptable min value of the converted value
 * @param max Acceptable max value of the converted value
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_int8_range(const char *str, int8_t *val, const int8_t min, const int8_t max);

/**
 *  Converts dec/hex uint string to uint8 variable within the min and max
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 * @param min Acceptable min value of the converted value
 * @param max Acceptable max value of the converted value
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint8_range(const char *str, uint8_t *val, const uint8_t min, const uint8_t max);

/**
 *  Converts dec/hex uint string to uint16 variable within the min and max
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 * @param min Acceptable min value of the converted value
 * @param max Acceptable max value of the converted value
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint16_range(const char *str, uint16_t *val, const uint16_t min, const uint16_t max);

/**
 *  Converts dec/hex uint string to uint64 variable
 *
 * Note: Does not ignore whitespace.
 *
 * @param str A null terminated string
 * @param val A pointer to store the converted value into
 *
 * @return 0 if successful conversion, -1 otherwise
 */
int str_to_uint64(const char *str, uint64_t *val);

/**
 * Convert an ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in bytes (of buf); hex will be double
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2bin(const char *hex, uint8_t *buf, size_t len);

/**
 * Convert an ASCII hex string into an array of uint32s.
 * Converts from host to little endian internally
 * @hex: ASCII hex string (e.g., "01abba10") (null terminated)
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in elements (of buf); hex will be 8 times
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2uint32_arr(const char *hex, __le32 *buf, size_t len);

/**
 *  Converts any uppercase characters in a given string to lowercases
 * @param str A null terminated string
 */
void tolower_str(char *str);

/**
 *  Checks if the given expression string is a valid true/false and
 *  returns that otherwise returns a fail (-1)
 * @param str A null terminated string
 *
 * @return `true/false` if string a recognised expression, `-1` otherwise
 */
int expression_to_int(const char *str);

/**
 *  Strips the spaces off the start and end of a string.
 *  Modifies the contents of the string.
 * @param str A null terminated string
 *
 * @return pointer to stripped string
 */
char *strip(char *s);

/**
 * @brief Simple inefficient calculation of CRC7
 *
 * @param number    value to run CRC on
 * @param bit_count number of bits of @c value to to run CRC on
 *
 * @return          CRC7 value
 */
uint8_t crc7_gen(uint64_t number, uint8_t bit_count);

/**
 * @brief Simple inefficient calculation of CRC16. This uses the xmodem crc16 model to calulate
 *        the CRC.
 *
 * @param buff  Buffer of octets to run CRC16 on
 * @param len   Number of octets to run CRC16 on
 * @return      CRC16 value
 */
uint16_t crc16_gen(uint8_t *buff, size_t len);

/**
 * @brief Calculate the CRC16 of a buffer and compare against a reference value. This uses the
 *        xmodem crc16 model to calulate the CRC.
 *
 * @param buff  Buffer to perform the CRC16 check on.
 * @param len   Length of the buffer.
 * @param crc16 Reference CRC16 value.
 * @return      true if calculated CRC16 matches the reference value, otherwise false.
 */
bool crc16_check(uint8_t *buff, size_t len, uint16_t crc16);

/**
 * @brief Get the file size of a file.
 *
 * @param infile    The file to get the size of.
 * @return          the size of the file.
 */
size_t get_file_size(FILE *infile);

/**
 * @brief Utility function to read a complete file into memory.
 *
 * @param infile    File to read into memory.
 * @param buf       Pointer to memory buffer to write the file into.
 */
void load_file(FILE *infile, uint8_t **buf);

/**
 * @brief Sleep for the given value in ms.
 *
 * @param ms    Time to sleep for in ms.
 */
static inline void sleep_ms(uint32_t ms)
{
#ifdef MORSE_WIN_BUILD
    Sleep(ms);
#else
    sleep(ms / 1000);
    /* usleep can only deal with upto 1s (1000000us). */
    usleep((ms % 1000) * 1000);
#endif
}

/**
 * Convert a MAC address string into a byte array.
 *
 * @param mac       Array to hold the output
 * @param str       MAC address string in the format xx:xx:xx:xx:xx:xx
 *
 * @return          0 for success or -1 for failure
 */
static inline int str_to_mac_addr(uint8_t *mac, const char *str)
{
        if (sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != MAC_ADDR_LEN)
        {
            return -1;
        }

        return 0;
}

/**
 * Create a directory, including parent directories as needed (equivalent to mkdir -p)
 *
 * @param dir       Directory name
 *
 * @return          0 for success or -1 for failure, in which case errno is set
 */
int mkdir_path(const char *dir);

/**
 * @brief Check if a filepath is a regular file
 *
 * @param path  path to file
 * @return      true if a regular file, else false
 */
bool is_file(const char *path);

/**
 * @brief Check if a filepath is a directory
 *
 * @param path  path to file
 * @return      true if a directory, else false
 */
bool is_dir(const char *path);

/**
 * @brief Counts the number of bits which are set
 * @param x an unsigned number
 * @return number of set bits
 */
uint32_t popcount(uint32_t x);

/**
 * @brief Counts the number of trailing zero bits on the right
 * @param x an unsigned number
 * @return number of consecutive zeros bits, or -1 for failure
 */
int32_t ctz(uint32_t x);

/**
 * @brief Calculate the aligned size of a memory region
 * @param unaligned_size original size of the memory region
 * @param alignment_octets desired alignment in octets
 * @return aligned memory size, adding padding if needed
 */
static inline size_t align_size(size_t unaligned_size, size_t alignment_octets)
{
    size_t remainder;

    if (alignment_octets > 0)
    {
        remainder = unaligned_size % alignment_octets;
        if (remainder > 0)
        {
            return unaligned_size + alignment_octets - remainder;
        }
    }

    return unaligned_size;
}
