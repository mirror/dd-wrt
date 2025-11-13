/*  Functions from hack's utils library.
    Copyright (C) 1989-2022 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <https://www.gnu.org/licenses/>. */

#include <stdio.h>

#include "basicdefs.h"

enum exit_codes {
                      /* EXIT_SUCCESS is already defined as 0 */
  EXIT_BAD_USAGE = 1, /* bad program syntax, invalid command-line options */
  EXIT_BAD_INPUT = 2, /* failed to open some of the input files */
  EXIT_PANIC     = 4  /* PANIC during program execution */
};


_Noreturn void panic (const char *str, ...)
  _GL_ATTRIBUTE_FORMAT_PRINTF_STANDARD (1, 2);

FILE *ck_fopen (const char *name, const char *mode, int fail);
FILE *ck_fdopen (int fd, const char *name, const char *mode, int fail);
void ck_fwrite (const void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t ck_fread (void *ptr, size_t size, size_t nmemb, FILE *stream);
void ck_fflush (FILE *stream);
void ck_fclose (FILE *stream);
const char *follow_symlink (const char *path);
size_t ck_getdelim (char **text, size_t *buflen, char buffer_delimiter,
                    FILE *stream);
FILE * ck_mkstemp (char **p_filename, const char *tmpdir, const char *base,
                   const char *mode) _GL_ARG_NONNULL ((1, 2, 3, 4));
void ck_rename (const char *from, const char *to);

void *ck_malloc (size_t size);
void *ck_realloc (void *ptr, size_t size);

void cancel_cleanup (void);
void remove_cleanup_file (void);

struct buffer *init_buffer (void);
char *get_buffer (struct buffer const *b) _GL_ATTRIBUTE_PURE;
size_t size_buffer (struct buffer const *b) _GL_ATTRIBUTE_PURE;
char *add_buffer (struct buffer *b, const char *p, size_t n);
char *add1_buffer (struct buffer *b, int ch);
void free_buffer (struct buffer *b);
