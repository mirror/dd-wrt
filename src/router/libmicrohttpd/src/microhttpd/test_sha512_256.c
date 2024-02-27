/*
  This file is part of libmicrohttpd
  Copyright (C) 2019-2022 Evgeny Grin (Karlson2k)

  This test tool is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2, or
  (at your option) any later version.

  This test tool is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/test_sha512_256.h
 * @brief  Unit tests for SHA-512/256 functions
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "sha512_256.h"
#include "test_helpers.h"
#include <stdio.h>
#include <stdlib.h>

static int verbose = 0; /* verbose level (0-1)*/


struct str_with_len
{
  const char *const str;
  const size_t len;
};

#define D_STR_W_LEN(s) {(s), (sizeof((s)) / sizeof(char)) - 1}

struct data_unit1
{
  const struct str_with_len str_l;
  const uint8_t digest[SHA512_256_DIGEST_SIZE];
};

static const struct data_unit1 data_units1[] = {
  {D_STR_W_LEN ("abc"),
   {0x53, 0x04, 0x8E, 0x26, 0x81, 0x94, 0x1E, 0xF9, 0x9B, 0x2E, 0x29, 0xB7,
    0x6B, 0x4C, 0x7D, 0xAB, 0xE4, 0xC2, 0xD0, 0xC6, 0x34, 0xFC, 0x6D, 0x46,
    0xE0, 0xE2, 0xF1, 0x31, 0x07, 0xE7, 0xAF, 0x23}},
  {D_STR_W_LEN ("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhi" \
                "jklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"),
   {0x39, 0x28, 0xE1, 0x84, 0xFB, 0x86, 0x90, 0xF8, 0x40, 0xDA, 0x39, 0x88,
    0x12, 0x1D, 0x31, 0xBE, 0x65, 0xCB, 0x9D, 0x3E, 0xF8, 0x3E, 0xE6, 0x14,
    0x6F, 0xEA, 0xC8, 0x61, 0xE1, 0x9B, 0x56, 0x3A}},
  {D_STR_W_LEN (""), /* The empty zero-size input */
   {0xc6, 0x72, 0xb8, 0xd1, 0xef, 0x56, 0xed, 0x28, 0xab, 0x87, 0xc3, 0x62,
    0x2c, 0x51, 0x14, 0x06, 0x9b, 0xdd, 0x3a, 0xd7, 0xb8, 0xf9, 0x73, 0x74,
    0x98, 0xd0, 0xc0, 0x1e, 0xce, 0xf0, 0x96, 0x7a}},
  {D_STR_W_LEN ("1234567890!@~%&$@#{}[]\\/!?`."),
   {0xc8, 0x7c, 0x5a, 0x55, 0x27, 0x77, 0x1b, 0xe7, 0x69, 0x3c, 0x50, 0x79,
    0x32, 0xad, 0x7c, 0x79, 0xe9, 0x60, 0xa0, 0x18, 0xb7, 0x78, 0x2b, 0x6f,
    0xa9, 0x7b, 0xa3, 0xa0, 0xb5, 0x18, 0x17, 0xa5}},
  {D_STR_W_LEN ("Simple string."),
   {0xde, 0xcb, 0x3c, 0x81, 0x65, 0x4b, 0xa0, 0xf5, 0xf0, 0x45, 0x6b, 0x7e,
    0x61, 0xf5, 0x0d, 0xf5, 0x38, 0xa4, 0xfc, 0xb1, 0x8a, 0x95, 0xff, 0x59,
    0xbc, 0x04, 0x82, 0xcf, 0x23, 0xb2, 0x32, 0x56}},
  {D_STR_W_LEN ("abcdefghijklmnopqrstuvwxyz"),
   {0xfc, 0x31, 0x89, 0x44, 0x3f, 0x9c, 0x26, 0x8f, 0x62, 0x6a, 0xea, 0x08,
    0xa7, 0x56, 0xab, 0xe7, 0xb7, 0x26, 0xb0, 0x5f, 0x70, 0x1c, 0xb0, 0x82,
    0x22, 0x31, 0x2c, 0xcf, 0xd6, 0x71, 0x0a, 0x26, }},
  {D_STR_W_LEN ("zyxwvutsrqponMLKJIHGFEDCBA"),
   {0xd2, 0x6d, 0x24, 0x81, 0xa4, 0xf9, 0x0a, 0x72, 0xd2, 0x7f, 0xc1, 0xac,
    0xac, 0xe1, 0xc0, 0x6b, 0x39, 0x94, 0xac, 0x73, 0x50, 0x2e, 0x27, 0x97,
    0xa3, 0x65, 0x37, 0x4e, 0xbb, 0x5c, 0x27, 0xe9}},
  {D_STR_W_LEN ("abcdefghijklmnopqrstuvwxyzzyxwvutsrqponMLKJIHGFEDCBA" \
                "abcdefghijklmnopqrstuvwxyzzyxwvutsrqponMLKJIHGFEDCBA"),
   {0xad, 0xe9, 0x5d, 0x55, 0x3b, 0x9e, 0x45, 0x69, 0xdb, 0x53, 0xa4, 0x04,
    0x92, 0xe7, 0x87, 0x94, 0xff, 0xc9, 0x98, 0x5f, 0x93, 0x03, 0x86, 0x45,
    0xe1, 0x97, 0x17, 0x72, 0x7c, 0xbc, 0x31, 0x15}},
  {D_STR_W_LEN ("/long/long/long/long/long/long/long/long/long/long/long" \
                "/long/long/long/long/long/long/long/long/long/long/long" \
                "/long/long/long/long/long/long/long/long/long/long/long" \
                "/long/long/long/long/long/long/long/long/long/long/long" \
                "/long/long/long/long/long/long/long/long/long/long/long" \
                "/long/long/long/long/long/long/long/long/long/long/long" \
                "/long/long/long/long/path?with%20some=parameters"),
   {0xbc, 0xab, 0xc6, 0x2c, 0x0a, 0x22, 0xd5, 0xcb, 0xac, 0xac, 0xe9, 0x25,
    0xcf, 0xce, 0xaa, 0xaf, 0x0e, 0xa1, 0xed, 0x42, 0x46, 0x8a, 0xe2, 0x01,
    0xee, 0x2f, 0xdb, 0x39, 0x75, 0x47, 0x73, 0xf1}}
};

static const size_t units1_num = sizeof(data_units1) / sizeof(data_units1[0]);

struct bin_with_len
{
  const uint8_t bin[512];
  const size_t len;
};

struct data_unit2
{
  const struct bin_with_len bin_l;
  const uint8_t digest[SHA512_256_DIGEST_SIZE];
};

/* Size must be less than 512 bytes! */
static const struct data_unit2 data_units2[] = {
  { { {97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
       112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122}, 26}, /* a..z ASCII sequence */
    {0xfc, 0x31, 0x89, 0x44, 0x3f, 0x9c, 0x26, 0x8f, 0x62, 0x6a, 0xea, 0x08,
     0xa7, 0x56, 0xab, 0xe7, 0xb7, 0x26, 0xb0, 0x5f, 0x70, 0x1c, 0xb0, 0x82,
     0x22, 0x31, 0x2c, 0xcf, 0xd6, 0x71, 0x0a, 0x26}},
  { { {65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
       65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
       65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
       65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65},
      72 }, /* 'A' x 72 times */
    {0x36, 0x5d, 0x41, 0x0e, 0x55, 0xd1, 0xfd, 0xe6, 0xc3, 0xb8, 0x68, 0xcc,
     0xed, 0xeb, 0xcd, 0x0d, 0x2e, 0x34, 0xb2, 0x5c, 0xdf, 0xe7, 0x79, 0xe2,
     0xe9, 0x65, 0x07, 0x33, 0x78, 0x0d, 0x01, 0x89}},
  { { {19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
       37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
       55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
       73}, 55}, /* 19..73 sequence */
    {0xb9, 0xe5, 0x74, 0x11, 0xbf, 0xa2, 0x0e, 0x98, 0xbe, 0x08, 0x69, 0x2e,
     0x17, 0x9e, 0xc3, 0xfe, 0x61, 0xe3, 0x7a, 0x80, 0x2e, 0x25, 0x8c, 0xf3,
     0x76, 0xda, 0x9f, 0x5f, 0xcd, 0x87, 0x48, 0x0d}},
  { { {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
       26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
       44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
       62, 63, 64, 65, 66, 67, 68, 69}, 63}, /* 7..69 sequence */
    {0x80, 0x15, 0x83, 0xed, 0x7d, 0xef, 0x9f, 0xdf, 0xfb, 0x83, 0x1f, 0xc5,
     0x8b, 0x50, 0x37, 0x81, 0x00, 0xc3, 0x4f, 0xfd, 0xfe, 0xc2, 0x9b, 0xaf,
     0xfe, 0x15, 0x66, 0xe5, 0x08, 0x42, 0x5e, 0xae}},
  { { {38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
       56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
       74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
       92}, 55}, /* 38..92 sequence */
    {0x76, 0x2f, 0x27, 0x4d, 0xfa, 0xd5, 0xa9, 0x21, 0x4e, 0xe9, 0x56, 0x22,
     0x54, 0x38, 0x71, 0x3e, 0xef, 0x14, 0xa9, 0x22, 0x37, 0xf3, 0xb0, 0x50,
     0x3d, 0x95, 0x40, 0xb7, 0x08, 0x64, 0xa9, 0xfd}},
  { { {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
       21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
       39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
       57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72}, 72}, /* 1..72 sequence */
    {0x3f, 0x5c, 0xd3, 0xec, 0x40, 0xc4, 0xb9, 0x78, 0x35, 0x57, 0xc6, 0x4f,
     0x3e, 0x46, 0x82, 0xdc, 0xd4, 0x46, 0x11, 0xd0, 0xb3, 0x0a, 0xbb, 0x89,
     0xf1, 0x1d, 0x34, 0xb5, 0xf9, 0xd5, 0x10, 0x35}},
  { { {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
       21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
       39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
       57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
       75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
       93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
       109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
       123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
       137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
       151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
       165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
       179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
       193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206,
       207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
       221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234,
       235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
       249, 250, 251, 252, 253, 254, 255}, 256}, /* 0..255 sequence */
    {0x08, 0x37, 0xa1, 0x1d, 0x99, 0x4d, 0x5a, 0xa8, 0x60, 0xd0, 0x69, 0x17,
     0xa8, 0xa0, 0xf6, 0x3e, 0x31, 0x11, 0xb9, 0x56, 0x33, 0xde, 0xeb, 0x15,
     0xee, 0xd9, 0x94, 0x93, 0x76, 0xf3, 0x7d, 0x36, }},
  { { {199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186,
       185, 184, 183, 182, 181, 180,
       179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166,
       165, 164, 163, 162, 161, 160,
       159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146,
       145, 144, 143, 142, 141, 140,
       139}, 61},  /* 199..139 sequence */
    {0xcf, 0x21, 0x4b, 0xb2, 0xdd, 0x40, 0x98, 0xdf, 0x3a, 0xb7, 0x21, 0xb4,
     0x69, 0x0e, 0x19, 0x36, 0x24, 0xa9, 0xbe, 0x30, 0xf7, 0xd0, 0x75, 0xb0,
     0x39, 0x94, 0x82, 0xda, 0x55, 0x97, 0xe4, 0x79}},
  { { {255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242,
       241, 240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228,
       227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214,
       213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200,
       199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186,
       185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172,
       171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158,
       157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,
       143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130,
       129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116,
       115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102,
       101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85,
       84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67,
       66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,
       48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31,
       30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13,
       12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}, 255},  /* 255..1 sequence */
    {0x22, 0x31, 0xf2, 0xa1, 0xb4, 0x89, 0xb2, 0x44, 0xf7, 0x66, 0xa0, 0xb8,
     0x31, 0xed, 0xb7, 0x73, 0x8a, 0x34, 0xdc, 0x11, 0xc8, 0x2c, 0xf2, 0xb5,
     0x88, 0x60, 0x39, 0x6b, 0x5c, 0x06, 0x70, 0x37}},
  { { {41, 35, 190, 132, 225, 108, 214, 174, 82, 144, 73, 241, 241, 187, 233,
       235, 179, 166, 219, 60, 135, 12, 62, 153, 36, 94, 13, 28, 6, 183, 71,
       222, 179, 18, 77, 200, 67, 187, 139, 166, 31, 3, 90, 125, 9, 56, 37,
       31, 93, 212, 203, 252, 150, 245, 69, 59, 19, 13, 137, 10, 28, 219, 174,
       50, 32, 154, 80, 238, 64, 120, 54, 253, 18, 73, 50, 246, 158, 125, 73,
       220, 173, 79, 20, 242, 68, 64, 102, 208, 107, 196, 48, 183, 50, 59,
       161, 34, 246, 34, 145, 157, 225, 139, 31, 218, 176, 202, 153, 2, 185,
       114, 157, 73, 44, 128, 126, 197, 153, 213, 233, 128, 178, 234, 201,
       204, 83, 191, 103, 214, 191, 20, 214, 126, 45, 220, 142, 102, 131, 239,
       87, 73, 97, 255, 105, 143, 97, 205, 209, 30, 157, 156, 22, 114, 114,
       230, 29, 240, 132, 79, 74, 119, 2, 215, 232, 57, 44, 83, 203, 201, 18,
       30, 51, 116, 158, 12, 244, 213, 212, 159, 212, 164, 89, 126, 53, 207,
       50, 34, 244, 204, 207, 211, 144, 45, 72, 211, 143, 117, 230, 217, 29,
       42, 229, 192, 247, 43, 120, 129, 135, 68, 14, 95, 80, 0, 212, 97, 141,
       190, 123, 5, 21, 7, 59, 51, 130, 31, 24, 112, 146, 218, 100, 84, 206,
       177, 133, 62, 105, 21, 248, 70, 106, 4, 150, 115, 14, 217, 22, 47, 103,
       104, 212, 247, 74, 74, 208, 87, 104}, 255},  /* pseudo-random data */
    {0xb8, 0xdb, 0x2c, 0x2e, 0xf3, 0x12, 0x77, 0x14, 0xf9, 0x34, 0x2d, 0xfa,
     0xda, 0x42, 0xbe, 0xfe, 0x67, 0x3a, 0x8a, 0xf6, 0x71, 0x36, 0x00, 0xff,
     0x77, 0xa5, 0x83, 0x14, 0x55, 0x2a, 0x05, 0xaf}},
  { { {66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
       66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
       66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
       66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
       66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
       66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
       66, 66}, 110},  /* 'B' x 110 times */
    {0xc8, 0x9e, 0x0d, 0x8f, 0x7b, 0x35, 0xfd, 0x3e, 0xdc, 0x90, 0x87, 0x64,
     0x45, 0x94, 0x94, 0x21, 0xb3, 0x8e, 0xb5, 0xc7, 0x54, 0xc8, 0xee, 0xde,
     0xfc, 0x77, 0xd6, 0xe3, 0x9f, 0x81, 0x8e, 0x78}},
  { { {67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
       67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
       67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
       67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
       67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
       67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
       67, 67, 67}, 111},  /* 'C' x 111 times */
    {0x86, 0xca, 0x6d, 0x2a, 0x72, 0xe2, 0x8c, 0x17, 0x89, 0x86, 0x89, 0x1b,
     0x36, 0xf9, 0x6d, 0xda, 0x8c, 0xd6, 0x30, 0xb2, 0xd3, 0x60, 0x39, 0xfb,
     0xc9, 0x04, 0xc5, 0x11, 0xcd, 0x2d, 0xe3, 0x62}},
  { { {68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
       68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
       68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
       68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
       68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
       68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
       68, 68, 68, 68}, 112},  /* 'D' x 112 times */
    {0xdf, 0x9d, 0x4a, 0xcf, 0x81, 0x0d, 0x3a, 0xd4, 0x8e, 0xa4, 0x65, 0x9e,
     0x1e, 0x15, 0xe4, 0x15, 0x1b, 0x37, 0xb6, 0xeb, 0x17, 0xab, 0xf6, 0xb1,
     0xbc, 0x30, 0x46, 0x34, 0x24, 0x56, 0x1c, 0x06}},
  { { {69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
       69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
       69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
       69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
       69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
       69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
       69, 69, 69, 69, 69}, 113},  /* 'E' x 113 times */
    {0xa5, 0xf1, 0x47, 0x74, 0xf8, 0x2b, 0xed, 0x23, 0xe4, 0x10, 0x59, 0x8f,
     0x7e, 0xb1, 0x30, 0xe5, 0x7e, 0xd1, 0x4b, 0xbc, 0x72, 0x58, 0x58, 0x81,
     0xbb, 0xa0, 0xa5, 0xb6, 0x15, 0x39, 0x49, 0xa1}},
  { { {70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
       70, 70, 70, 70, 70, 70}, 114},  /* 'F' x 114 times */
    {0xe6, 0xa3, 0xc9, 0x63, 0xd5, 0x28, 0x6e, 0x2d, 0xfb, 0x71, 0xdf, 0xd4,
     0xff, 0xc2, 0xd4, 0x2b, 0x5d, 0x9b, 0x76, 0x28, 0xd2, 0xcb, 0x15, 0xc8,
     0x81, 0x57, 0x14, 0x09, 0xc3, 0x8e, 0x92, 0xce}},
  { { {76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
       76}, 127},  /* 'L' x 127 times */
    {0x5d, 0x18, 0xff, 0xd7, 0xbe, 0x23, 0xb2, 0xb2, 0xbd, 0xe3, 0x13, 0x12,
     0x1c, 0x16, 0x89, 0x14, 0x4a, 0x42, 0xb4, 0x3f, 0xab, 0xc8, 0x41, 0x14,
     0x62, 0x00, 0xb5, 0x53, 0xa7, 0xd6, 0xd5, 0x35}},
  { { {77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
       77, 77}, 128},  /* 'M' x 128 times */
    {0x6e, 0xf0, 0xda, 0x81, 0x3d, 0x50, 0x1d, 0x31, 0xf1, 0x4a, 0xf8, 0xd9,
     0x7d, 0xd2, 0x13, 0xdd, 0xa4, 0x46, 0x15, 0x0b, 0xb8, 0x5a, 0x8a, 0xc6,
     0x1e, 0x3a, 0x1f, 0x21, 0x35, 0xa2, 0xbb, 0x4f}},
  { { {78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
       78, 78, 78}, 129},  /* 'N' x 129 times */
    {0xee, 0xce, 0xd5, 0x34, 0xab, 0x14, 0x13, 0x9e, 0x8f, 0x5c, 0xb4, 0xef,
     0xac, 0xaf, 0xc5, 0xeb, 0x1d, 0x2f, 0xe3, 0xc5, 0xca, 0x09, 0x29, 0x96,
     0xfa, 0x84, 0xff, 0x12, 0x26, 0x6a, 0x50, 0x49}},
  { { {97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
       97, 97, 97, 97}, 238},  /* 'a' x 238 times */
    {0xb4, 0x24, 0xe5, 0x7b, 0xa7, 0x37, 0xe3, 0xc4, 0xac, 0x35, 0x21, 0x17,
     0x98, 0xec, 0xb9, 0xae, 0x45, 0x13, 0x24, 0xa4, 0x2c, 0x76, 0xae, 0x7d,
     0x17, 0x75, 0x27, 0x8a, 0xaa, 0x4a, 0x48, 0x60}},
  { { {98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
       98, 98, 98, 98, 98}, 239},  /* 'b' x 239 times */
    {0xcd, 0x93, 0xb8, 0xab, 0x6a, 0x74, 0xbd, 0x34, 0x8c, 0x43, 0x76, 0x0c,
     0x2a, 0xd0, 0x6e, 0xd8, 0x76, 0xcf, 0xdf, 0x2a, 0x21, 0x04, 0xfb, 0xf6,
     0x16, 0x53, 0x68, 0xf6, 0x10, 0xc3, 0xa1, 0xac}},
  { { {99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
       99, 99, 99, 99, 99, 99}, 240},  /* 'c' x 240 times */
    {0x5f, 0x60, 0xea, 0x44, 0xb6, 0xc6, 0x9e, 0xfe, 0xfc, 0x0e, 0x6a, 0x0a,
     0x99, 0x40, 0x1b, 0x61, 0x43, 0x58, 0xba, 0x4a, 0x0a, 0xee, 0x6b, 0x52,
     0x10, 0xdb, 0x32, 0xd9, 0x7f, 0x12, 0xba, 0x70}},
  { { {48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
       48, 48, 48, 48, 48, 48, 48}, 241}, /* '0' x 241 times */
    {0x3c, 0xcb, 0xcf, 0x50, 0x79, 0xd5, 0xb6, 0xf5, 0xbf, 0x25, 0x07, 0xfb,
     0x4d, 0x1f, 0xa3, 0x77, 0xc3, 0x6f, 0xe8, 0xe3, 0xc4, 0x4b, 0xf8, 0xcd,
     0x90, 0x93, 0xf1, 0x3e, 0x08, 0x09, 0xa7, 0x69}}
};

static const size_t units2_num = sizeof(data_units2) / sizeof(data_units2[0]);


/*
 *  Helper functions
 */

/**
 * Print bin as hex
 *
 * @param bin binary data
 * @param len number of bytes in bin
 * @param hex pointer to len*2+1 bytes buffer
 */
static void
bin2hex (const uint8_t *bin,
         size_t len,
         char *hex)
{
  while (len-- > 0)
  {
    unsigned int b1, b2;
    b1 = (*bin >> 4) & 0xf;
    *hex++ = (char) ((b1 > 9) ? (b1 + 'A' - 10) : (b1 + '0'));
    b2 = *bin++ & 0xf;
    *hex++ = (char) ((b2 > 9) ? (b2 + 'A' - 10) : (b2 + '0'));
  }
  *hex = 0;
}


static int
check_result (const char *test_name,
              unsigned int check_num,
              const uint8_t calculated[SHA512_256_DIGEST_SIZE],
              const uint8_t expected[SHA512_256_DIGEST_SIZE])
{
  int failed = memcmp (calculated, expected, SHA512_256_DIGEST_SIZE);
  check_num++; /* Print 1-based numbers */
  if (failed)
  {
    char calc_str[SHA512_256_DIGEST_SIZE * 2 + 1];
    char expc_str[SHA512_256_DIGEST_SIZE * 2 + 1];
    bin2hex (calculated, SHA512_256_DIGEST_SIZE, calc_str);
    bin2hex (expected, SHA512_256_DIGEST_SIZE, expc_str);
    fprintf (stderr,
             "FAILED: %s check %u: calculated digest %s, expected digest %s.\n",
             test_name, check_num, calc_str, expc_str);
    fflush (stderr);
  }
  else if (verbose)
  {
    char calc_str[SHA512_256_DIGEST_SIZE * 2 + 1];
    bin2hex (calculated, SHA512_256_DIGEST_SIZE, calc_str);
    printf ("PASSED: %s check %u: calculated digest %s matches " \
            "expected digest.\n",
            test_name, check_num, calc_str);
    fflush (stdout);
  }
  return failed ? 1 : 0;
}


/*
 *  Tests
 */

/* Calculated SHA-512/256 as one pass for whole data */
static int
test1_str (void)
{
  int num_failed = 0;
  unsigned int i;
  struct Sha512_256Ctx ctx;

  for (i = 0; i < units1_num; i++)
  {
    uint8_t digest[SHA512_256_DIGEST_SIZE];

    MHD_SHA512_256_init (&ctx);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) data_units1[i].str_l.str,
                           data_units1[i].str_l.len);
    MHD_SHA512_256_finish (&ctx, digest);
    num_failed += check_result (__FUNCTION__, i, digest,
                                data_units1[i].digest);
  }
  return num_failed;
}


static int
test1_bin (void)
{
  int num_failed = 0;
  unsigned int i;
  struct Sha512_256Ctx ctx;

  for (i = 0; i < units2_num; i++)
  {
    uint8_t digest[SHA512_256_DIGEST_SIZE];

    MHD_SHA512_256_init (&ctx);
    MHD_SHA512_256_update (&ctx, data_units2[i].bin_l.bin,
                           data_units2[i].bin_l.len);
    MHD_SHA512_256_finish (&ctx, digest);
    num_failed += check_result (__FUNCTION__, i, digest,
                                data_units2[i].digest);
  }
  return num_failed;
}


/* Calculated SHA-512/256 as two iterations for whole data */
static int
test2_str (void)
{
  int num_failed = 0;
  unsigned int i;
  struct Sha512_256Ctx ctx;

  for (i = 0; i < units1_num; i++)
  {
    uint8_t digest[SHA512_256_DIGEST_SIZE];
    size_t part_s = data_units1[i].str_l.len / 4;

    MHD_SHA512_256_init (&ctx);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) "", 0);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) data_units1[i].str_l.str,
                           part_s);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) "", 0);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) data_units1[i].str_l.str
                           + part_s,
                           data_units1[i].str_l.len - part_s);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) "", 0);
    MHD_SHA512_256_finish (&ctx, digest);
    num_failed += check_result (__FUNCTION__, i, digest,
                                data_units1[i].digest);
  }
  return num_failed;
}


static int
test2_bin (void)
{
  int num_failed = 0;
  unsigned int i;
  struct Sha512_256Ctx ctx;

  for (i = 0; i < units2_num; i++)
  {
    uint8_t digest[SHA512_256_DIGEST_SIZE];
    size_t part_s = data_units2[i].bin_l.len * 2 / 3;

    MHD_SHA512_256_init (&ctx);
    MHD_SHA512_256_update (&ctx, data_units2[i].bin_l.bin, part_s);
    MHD_SHA512_256_update (&ctx, (const uint8_t *) "", 0);
    MHD_SHA512_256_update (&ctx, data_units2[i].bin_l.bin + part_s,
                           data_units2[i].bin_l.len - part_s);
    MHD_SHA512_256_finish (&ctx, digest);
    num_failed += check_result (__FUNCTION__, i, digest,
                                data_units2[i].digest);
  }
  return num_failed;
}


/* Use data set number 7 as it has the longest sequence */
#define DATA_POS 6
#define MAX_OFFSET 63

static int
test_unaligned (void)
{
  int num_failed = 0;
  unsigned int offset;
  uint8_t *buf;
  uint8_t *digest_buf;
  struct Sha512_256Ctx ctx;

  const struct data_unit2 *const tdata = data_units2 + DATA_POS;

  buf = malloc (tdata->bin_l.len + MAX_OFFSET);
  digest_buf = malloc (SHA512_256_DIGEST_SIZE + MAX_OFFSET);
  if ((NULL == buf) || (NULL == digest_buf))
    exit (99);

  for (offset = MAX_OFFSET; offset >= 1; --offset)
  {
    uint8_t *unaligned_digest;
    uint8_t *unaligned_buf;

    unaligned_buf = buf + offset;
    memcpy (unaligned_buf, tdata->bin_l.bin, tdata->bin_l.len);
    unaligned_digest = digest_buf + MAX_OFFSET - offset;
    memset (unaligned_digest, 0, SHA512_256_DIGEST_SIZE);

    MHD_SHA512_256_init (&ctx);
    MHD_SHA512_256_update (&ctx, unaligned_buf, tdata->bin_l.len);
    MHD_SHA512_256_finish (&ctx, unaligned_digest);
    num_failed += check_result (__FUNCTION__, MAX_OFFSET - offset,
                                unaligned_digest, tdata->digest);
  }
  free (digest_buf);
  free (buf);
  return num_failed;
}


int
main (int argc, char *argv[])
{
  int num_failed = 0;
  (void) has_in_name; /* Mute compiler warning. */
  if (has_param (argc, argv, "-v") || has_param (argc, argv, "--verbose"))
    verbose = 1;

  num_failed += test1_str ();
  num_failed += test1_bin ();

  num_failed += test2_str ();
  num_failed += test2_bin ();

  num_failed += test_unaligned ();

  return num_failed ? 1 : 0;
}
