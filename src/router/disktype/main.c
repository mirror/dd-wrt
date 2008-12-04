/*
 * main.c
 * Main entry point and global utility functions.
 *
 * Copyright (c) 2003 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#include "global.h"

#ifdef USE_MACOS_TYPE
#include <CoreServices/CoreServices.h>
#endif

/*
 * local functions
 */

static void analyze_file(const char *filename);
static void print_kind(int filekind, u8 size, int size_known);

#ifdef USE_MACOS_TYPE
static void show_macos_type(const char *filename);
#endif

/*
 * entry point
 */

int main(int argc, char *argv[])
{
  int i;

  /* argument check */
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <device/file>...\n", PROGNAME);
    return 1;
  }

  /* loop over filenames */
  print_line(0, "");
  for (i = 1; i < argc; i++) {
    analyze_file(argv[i]);
    print_line(0, "");
  }

  return 0;
}

/*
 * Analyze one file
 */

static void analyze_file(const char *filename)
{
  int fd, filekind;
  u8 filesize;
  struct stat sb;
  char *reason;
  SOURCE *s;

  print_line(0, "--- %s", filename);

  /* stat check */
  if (stat(filename, &sb) < 0) {
    errore("Can't stat %.300s", filename);
    return;
  }

  filekind = 0;
  filesize = 0;
  reason = NULL;
  if (S_ISREG(sb.st_mode)) {
    filesize = sb.st_size;
    print_kind(filekind, filesize, 1);
  } else if (S_ISBLK(sb.st_mode))
    filekind = 1;
  else if (S_ISCHR(sb.st_mode))
    filekind = 2;
  else if (S_ISDIR(sb.st_mode))
    reason = "Is a directory";
  else if (S_ISFIFO(sb.st_mode))
    reason = "Is a FIFO";
#ifdef S_ISSOCK
  else if (S_ISSOCK(sb.st_mode))
    reason = "Is a socket";
#endif
  else
    reason = "Is an unknown kind of special file";

  if (reason != NULL) {
    error("%.300s: %s", filename, reason);
    return;
  }

  /* Mac OS type & creator code (if running on Mac OS X) */
#ifdef USE_MACOS_TYPE
  if (filekind == 0)
    show_macos_type(filename);
#endif

  /* empty regular files need no further analysis */
  if (filekind == 0 && filesize == 0)
    return;

  /* open for reading */
  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    errore("Can't open %.300s", filename);
    return;
  }

  /* (try to) guard against TTY character devices */
  if (filekind == 2) {
    if (isatty(fd)) {
      error("%.300s: Is a TTY device", filename);
      return;
    }
  }

  /* create a source */
  s = init_file_source(fd, filekind);

  /* tell the user what it is */
  if (filekind != 0)
    print_kind(filekind, s->size, s->size_known);

  /* now analyze it */
  analyze_source(s, 0);

  /* finish it up */
  close_source(s);
}

static void print_kind(int filekind, u8 size, int size_known)
{
  char buf[256], *kindname;

  if (filekind == 0)
    kindname = "Regular file";
  else if (filekind == 1)
    kindname = "Block device";
  else if (filekind == 2)
    kindname = "Character device";
  else
    kindname = "Unknown kind";

  if (size_known) {
    format_size_verbose(buf, size);
    print_line(0, "%s, size %s", kindname, buf);
  } else {
    print_line(0, "%s, unknown size", kindname);
  }
}

/*
 * Mac OS type & creator code
 */

#ifdef USE_MACOS_TYPE

static void show_macos_type(const char *filename)
{
  int err;
  FSRef ref;
  FSCatalogInfo info;
  FInfo *finfo;

  err = FSPathMakeRef(filename, &ref, NULL);
  if (err == 0) {
    err = FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo,
			   &info, NULL, NULL, NULL);
  }

  if (err == 0) {
    finfo = (FInfo *)(info.finderInfo);
    if (finfo->fdType != 0 || finfo->fdCreator != 0) {
      char typecode[5], creatorcode[5], s1[256], s2[256];

      memcpy(typecode, &finfo->fdType, 4);
      typecode[4] = 0;
      format_ascii(typecode, s1);

      memcpy(creatorcode, &finfo->fdCreator, 4);
      creatorcode[4] = 0;
      format_ascii(creatorcode, s2);

      print_line(0, "Type code \"%s\", creator code \"%s\"",
		 s1, s2);
    } else {
      print_line(0, "No type and creator code");
    }
  }
  if (err) {
    print_line(0, "Type and creator code unknown (error %d)", err);
  }
}

#endif

/* EOF */
