/*
 * Copyright 2022-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/* Bitmap utilities */
#define BMGET(_v, _f)     (((_v) & (_f)) >> __builtin_ctz(_f))
#define BMSET(_v, _f)     (((_v) << __builtin_ctz(_f)) & (_f))

#define ROUND_BYTES_TO_WORD(_nbytes) (((_nbytes) + 3) & ~((typeof(_nbytes))0x03))

/* Returns number of bytes needed to word align */
#define BYTES_NEEDED_TO_WORD_ALIGN(bytes) ((bytes) & 0x3 ? (4 - ((bytes) & 0x3)) : 0)

/* Rounds down to the nearest word boundary */
#define ROUND_DOWN_TO_WORD(bytes) (BYTES_NEEDED_TO_WORD_ALIGN(bytes) ? \
					bytes - (4 - BYTES_NEEDED_TO_WORD_ALIGN(bytes)) : bytes)
