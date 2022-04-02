/* charconv.c

   Copyright (C) 2010 Alexander Korolkov <alexander.korolkov@gmail.com>
   Copyright (C) 2018-2020 Pali Rohár <pali.rohar@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.

   The complete text of the GNU General Public License
   can be found in /usr/share/common-licenses/GPL-3 file.
*/

#include "charconv.h"
#include <langinfo.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

/* CP850 table for 0x80-0xFF range from:
 * http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP850.TXT
 */
static const wchar_t cp850_table[128] = {
    0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
    0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
    0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
    0x00ff, 0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x00d7, 0x0192,
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
    0x00bf, 0x00ae, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x00c0,
    0x00a9, 0x2563, 0x2551, 0x2557, 0x255d, 0x00a2, 0x00a5, 0x2510,
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x00e3, 0x00c3,
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4,
    0x00f0, 0x00d0, 0x00ca, 0x00cb, 0x00c8, 0x0131, 0x00cd, 0x00ce,
    0x00cf, 0x2518, 0x250c, 0x2588, 0x2584, 0x00a6, 0x00cc, 0x2580,
    0x00d3, 0x00df, 0x00d4, 0x00d2, 0x00f5, 0x00d5, 0x00b5, 0x00fe,
    0x00de, 0x00da, 0x00db, 0x00d9, 0x00fd, 0x00dd, 0x00af, 0x00b4,
    0x00ad, 0x00b1, 0x2017, 0x00be, 0x00b6, 0x00a7, 0x00f7, 0x00b8,
    0x00b0, 0x00a8, 0x00b7, 0x00b9, 0x00b3, 0x00b2, 0x25a0, 0x00a0,
};

/* CP850 translit table to 7bit ASCII for 0x80-0xFF range */
static const char *const cp850_translit_table[128] = {
    "C",   "u",   "e",  "a",     "a",     "a", "a",   "c",
    "e",   "e",   "e",  "i",     "i",     "i", "A",   "A",
    "E",   "ae",  "AE", "o",     "o",     "o", "u",   "u",
    "y",   "O",   "U",  "o",     "GBP",   "O", "x",   "f",
    "a",   "i",   "o",  "u",     "n",     "N", "a",   "o",
    "?",   "(R)", "!",  " 1/2 ", " 1/4 ", "!", "<<",  ">>",
    "?",   "?",   "?",  "|",     "+",     "A", "A",   "A",
    "(C)", "?",   "?",  "?",     "?",     "c", "JPY", "+",
    "+",   "+",   "+",  "+",     "-",     "+", "a",   "A",
    "?",   "?",   "?",  "?",     "?",     "?", "?",   "?",
    "d",   "D",   "E",  "E",     "E",     "i", "I",   "I",
    "I",   "+",   "+",  "?",     "?",     "|", "I",   "?",
    "O",   "ss",  "O",  "O",     "o",     "O", "u",   "th",
    "TH",  "U",   "U",  "U",     "y",     "Y", "?",   "'",
    "-",   "+-",  "?",  " 3/4 ", "?",     "?", "/",   ",",
    "?",   "?",   ".",  "1",     "3",     "2", "?",   " ",
};

static int wchar_string_to_cp850_string(char *out, const wchar_t *in, unsigned int out_size)
{
    unsigned i, j;
    for (i = 0; i < out_size-1 && in[i]; ++i) {
        if (in[i] > 0 && in[i] < 0x80) {
            out[i] = in[i];
            continue;
        }
        for (j = 0; j < 0x80; ++j) {
            if (in[i] == cp850_table[j]) {
                out[i] = (0x80 | j);
                break;
            }
        }
        if (j == 0x80) {
            fprintf(stderr, "Cannot convert input character 0x%04x to 'CP850': %s\n", (unsigned int)in[i], strerror(EILSEQ));
            return 0;
        }
    }
    if (in[i]) {
        fprintf(stderr, "Cannot convert input string to 'CP850': String is too long\n");
        return 0;
    }
    out[i] = 0;
    return 1;
}

static int cp850_string_to_wchar_string(wchar_t *out, const char *in, unsigned int out_size)
{
    unsigned i;
    for (i = 0; i < out_size-1 && i < 11 && in[i]; ++i) {
        out[i] = (in[i] & 0x80) ? cp850_table[in[i] & 0x7F] : in[i];
    }
    if (i < 11 && in[i]) {
        fprintf(stderr, "Cannot convert input string to 'CP850': String is too long\n");
        return 0;
    }
    out[i] = L'\0';
    return 1;
}

static int cp850_char_to_printable(char **p, unsigned char c, unsigned int out_size)
{
    size_t ret;
    wchar_t wcs[2];
    wcs[0] = (c & 0x80) ? cp850_table[c & 0x7F] : c;
    wcs[1] = 0;
    ret = wcstombs(*p, wcs, out_size);
    if (ret == 0)
        return 0;
    if (ret != (size_t)-1)
        *p += ret;
    else if (!(c & 0x80))
        *(*p++) = c;
    else {
        ret = strlen(cp850_translit_table[c & 0x7F]);
        if (ret > out_size)
            return 0;
        memcpy(*p, cp850_translit_table[c & 0x7F], ret);
        *p += ret;
    }
    return 1;
}

static int local_string_to_cp850_string(char *out, const char *in, unsigned int out_size)
{
    int ret;
    wchar_t *wcs;
    if (strlen(in) >= out_size) {
        fprintf(stderr, "Cannot convert input string '%s' to 'CP850': String is too long\n", in);
        return 0;
    }
    wcs = calloc(out_size, sizeof(wchar_t));
    if (!wcs) {
        fprintf(stderr, "Cannot convert input string '%s' to 'CP850': %s\n", in, strerror(ENOMEM));
        return 0;
    }
    if (mbstowcs(wcs, in, out_size) == (size_t)-1) {
        fprintf(stderr, "Cannot convert input string '%s' to 'CP850': %s\n", in, strerror(errno));
        free(wcs);
        return 0;
    }
    ret = wchar_string_to_cp850_string(out, wcs, out_size);
    free(wcs);
    return ret;
}

#ifdef HAVE_ICONV

static int iconv_init_codepage(int codepage, const char *local, iconv_t *to_local, iconv_t *from_local)
{
    char codepage_name[32];
    snprintf(codepage_name, sizeof(codepage_name), "CP%d//TRANSLIT", codepage);
    *to_local = iconv_open(local, codepage_name);
    if (*to_local == (iconv_t) - 1) {
        snprintf(codepage_name, sizeof(codepage_name), "CP%d", codepage);
        *to_local = iconv_open(local, codepage_name);
    }
    if (*to_local == (iconv_t) - 1)
        fprintf(stderr, "Cannot initialize conversion from codepage %d to %s: %s\n", codepage, local, strerror(errno));
    snprintf(codepage_name, sizeof(codepage_name), "CP%d", codepage);
    *from_local = iconv_open(codepage_name, local);
    if (*from_local == (iconv_t) - 1)
        fprintf(stderr, "Cannot initialize conversion from %s to codepage %d: %s\n", local, codepage, strerror(errno));
    return (*to_local != (iconv_t)-1 && *from_local != (iconv_t)-1) ? 1 : 0;
}

static iconv_t dos_to_local;
static iconv_t local_to_dos;
static iconv_t dos_to_wchar;
static iconv_t wchar_to_dos;
static int used_codepage;
static int internal_cp850;

/*
 * Initialize conversion from codepage.
 * codepage = -1 means default codepage.
 * Returns non-zero on success, 0 on failure
 */
static int init_conversion(int codepage)
{
    static int initialized = -1;
    if (initialized < 0) {
	initialized = 1;
	if (codepage < 0)
	    codepage = DEFAULT_DOS_CODEPAGE;
	setlocale(LC_CTYPE, "");	/* initialize locale for CODESET */
	if (!iconv_init_codepage(codepage, nl_langinfo(CODESET), &dos_to_local, &local_to_dos))
	    initialized = 0;
	if (initialized && !iconv_init_codepage(codepage, "WCHAR_T", &dos_to_wchar, &wchar_to_dos))
	    initialized = 0;
	if (!initialized && codepage == 850) {
	    fprintf(stderr, "Using internal CP850 conversion table\n");
	    internal_cp850 = 1;	/* use internal CP850 conversion table */
	    initialized = 1;
	}
	if (initialized)
	    used_codepage = codepage;
    }
    return initialized;
}

int set_dos_codepage(int codepage)
{
    return init_conversion(codepage);
}

int dos_char_to_printable(char **p, unsigned char c, unsigned int out_size)
{
    char in[1] = { c };
    ICONV_CONST char *pin = in;
    size_t bytes_in = 1;
    size_t bytes_out = out_size;
    if (!init_conversion(-1))
	return 0;
    if (internal_cp850)
        return cp850_char_to_printable(p, c, out_size);
    return iconv(dos_to_local, &pin, &bytes_in, p, &bytes_out) != (size_t)-1;
}

int local_string_to_dos_string(char *out, char *in, unsigned int out_size)
{
    ICONV_CONST char *pin = in;
    char *pout = out;
    size_t bytes_in = strlen(in);
    size_t bytes_out = out_size-1;
    size_t ret;
    if (!init_conversion(-1))
        return 0;
    if (internal_cp850)
        return local_string_to_cp850_string(out, in, out_size);
    ret = iconv(local_to_dos, &pin, &bytes_in, &pout, &bytes_out);
    if (ret == (size_t)-1) {
        if (errno == E2BIG)
            fprintf(stderr, "Cannot convert input string '%s' to 'CP%d': String is too long\n",
                    in, used_codepage);
        else
            fprintf(stderr, "Cannot convert input sequence '\\x%.02hhX' from codeset '%s' to 'CP%d': %s\n",
                    *pin, nl_langinfo(CODESET), used_codepage, strerror(errno));
        iconv(local_to_dos, NULL, NULL, &pout, &bytes_out);
        return 0;
    } else {
        ret = iconv(local_to_dos, NULL, NULL, &pout, &bytes_out);
        if (ret == (size_t)-1) {
            fprintf(stderr, "Cannot convert input string '%s' to 'CP%d': String is too long\n",
                    in, used_codepage);
            return 0;
        }
    }
    out[out_size-1-bytes_out] = 0;
    return 1;
}

int dos_string_to_wchar_string(wchar_t *out, char *in, unsigned int out_size)
{
    ICONV_CONST char *pin = in;
    char *pout = (char *)out;
    size_t bytes_in = strnlen(in, 11);
    size_t bytes_out = out_size-sizeof(wchar_t);
    size_t ret;
    if (!init_conversion(-1))
        return 0;
    if (internal_cp850)
        return cp850_string_to_wchar_string(out, in, out_size);
    ret = iconv(dos_to_wchar, &pin, &bytes_in, &pout, &bytes_out);
    if (ret == (size_t)-1) {
        if (errno == E2BIG)
            fprintf(stderr, "Cannot convert input string from 'CP%d': String is too long\n",
                    used_codepage);
        else
            fprintf(stderr, "Cannot convert input sequence '\\x%.02hhX' from 'CP%d': %s\n",
                    *pin, used_codepage, strerror(errno));
        iconv(dos_to_wchar, NULL, NULL, &pout, &bytes_out);
        return 0;
    } else {
        ret = iconv(dos_to_wchar, NULL, NULL, &pout, &bytes_out);
        if (ret == (size_t)-1) {
            fprintf(stderr, "Cannot convert input string from 'CP%d': String is too long\n",
                    used_codepage);
            return 0;
        }
    }
    out[(out_size-sizeof(wchar_t)-bytes_out)/sizeof(wchar_t)] = L'\0';
    return 1;
}

int wchar_string_to_dos_string(char *out, wchar_t *in, unsigned int out_size)
{
    ICONV_CONST char *pin = (char *)in;
    char *pout = out;
    size_t bytes_in = wcslen(in)*sizeof(wchar_t);
    size_t bytes_out = out_size-1;
    size_t ret;
    if (!init_conversion(-1))
        return 0;
    if (internal_cp850)
        return wchar_string_to_cp850_string(out, in, out_size);
    ret = iconv(wchar_to_dos, &pin, &bytes_in, &pout, &bytes_out);
    if (ret == (size_t)-1) {
        if (errno == E2BIG)
            fprintf(stderr, "Cannot convert input string '%ls' to 'CP%d': String is too long\n",
                    in, used_codepage);
        else
            fprintf(stderr, "Cannot convert input character '%lc' to 'CP%d': %s\n",
                    (wint_t)*(wchar_t *)pin, used_codepage, strerror(errno));
        iconv(wchar_to_dos, NULL, NULL, &pout, &bytes_out);
        return 0;
    } else {
        ret = iconv(wchar_to_dos, NULL, NULL, &pout, &bytes_out);
        if (ret == (size_t)-1) {
            fprintf(stderr, "Cannot convert input string '%ls' to 'CP%d': String is too long\n",
                    in, used_codepage);
            return 0;
        }
    }
    out[out_size-1-bytes_out] = 0;
    return 1;
}

#else

int set_dos_codepage(int codepage)
{
    static int initialized = -1;
    if (initialized < 0) {
        setlocale(LC_CTYPE, ""); /* initialize locale for wide character functions */
        if (codepage < 0)
            codepage = DEFAULT_DOS_CODEPAGE;
        initialized = (codepage == 850) ? 1 : 0;
        if (!initialized)
            fprintf(stderr, "Cannot initialize unsupported codepage %d, only codepage 850 is supported\n", codepage);
    }
    return initialized;
}

int dos_char_to_printable(char **p, unsigned char c, unsigned int out_size)
{
    return cp850_char_to_printable(p, c, out_size);
}

int local_string_to_dos_string(char *out, char *in, unsigned int out_size)
{
    return local_string_to_cp850_string(out, in, out_size);
}

int dos_string_to_wchar_string(wchar_t *out, char *in, unsigned int out_size)
{
    return cp850_string_to_wchar_string(out, in, out_size);
}

int wchar_string_to_dos_string(char *out, wchar_t *in, unsigned int out_size)
{
    return wchar_string_to_cp850_string(out, in, out_size);
}

#endif
