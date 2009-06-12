/* adapted from Eric Youngdale's readelf program */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <link.h>
#include <unistd.h>
#include <sys/types.h>
#include <ld_elf.h>
#include "readsoname.h"

void warn(char *fmt, ...);
char *xstrdup(char *);

struct needed_tab
{
  char *soname;
  int type;
};

struct needed_tab needed_tab[] = {
  { "libc.so.0",    LIB_ELF_LIBC0 },
  { "libm.so.0",    LIB_ELF_LIBC0 },
  { "libdl.so.0",   LIB_ELF_LIBC0 },
  { "libc.so.5",    LIB_ELF_LIBC5 },
  { "libm.so.5",    LIB_ELF_LIBC5 },
  { "libdl.so.1",   LIB_ELF_LIBC5 },
  { "libc.so.6",    LIB_ELF_LIBC6 },
  { "libm.so.6",    LIB_ELF_LIBC6 },
  { "libdl.so.2",   LIB_ELF_LIBC6 },
  { NULL,           LIB_ELF }
};

char *readsoname(char *name, FILE *infile, int expected_type, 
		 int *type, int elfclass)
{
  char *res;

  if (elfclass == ELFCLASS32)
    res = readsoname32(name, infile, expected_type, type);
  else
  {
    res = readsoname64(name, infile, expected_type, type);
#if 0
    *type |= LIB_ELF64;
#endif
  }

  return res;
}

#undef __ELF_NATIVE_CLASS
#undef readsonameXX
#define readsonameXX readsoname32
#define __ELF_NATIVE_CLASS 32
#include "readsoname2.c"

#undef __ELF_NATIVE_CLASS
#undef readsonameXX
#define readsonameXX readsoname64
#define __ELF_NATIVE_CLASS 64
#include "readsoname2.c"
