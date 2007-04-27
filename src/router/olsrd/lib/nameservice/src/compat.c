#include "compat.h"

#ifndef linux
#include <stdlib.h>
#include <string.h>

/* strndup() is a GNU extention */
char *
strndup(const char *ptr, size_t size)
{
  size_t len = strlen(ptr);
  char *ret = NULL;

  if(!len)
    return NULL;

  if(len > size)
    len = size;

  ret = malloc(len + 1);
  
  if(!ret)
    return NULL;

  strncpy(ret, ptr, len);
  ret[len] = '\0';

  return ret;
}

#endif
