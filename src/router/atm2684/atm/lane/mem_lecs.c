/*
 *
 * Memory handling funcs
 *
 * $Id$
 *
 */
/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* Local includes */
#include "mem_lecs.h"

typedef struct _Mem_t {
  size_t size;
  void *start;
  char *unit;
  struct _Mem_t *next;
} Mem_t;

static Mem_t *memlist = NULL;

void*
mem_alloc(const char *unit, size_t nbytes)
{
  Mem_t *entry;

  entry = (Mem_t *)malloc(sizeof(Mem_t));
  if (!entry) {
    perror("malloc");
    return NULL;
  }
  entry->size = nbytes;
  entry->unit = (char*)malloc(strlen(unit)+1);
  if (!entry->unit) {
    perror("malloc");
    free(entry);
    return NULL;
  }
  memcpy(entry->unit, unit, strlen(unit)+1);
  entry->start = malloc(nbytes);
  if (!entry->start) {
    perror("malloc");
    free(entry->unit);
    free(entry);
    return NULL;
  }
  entry->next = memlist;
  memlist = entry;
  return entry->start;
}

void 
mem_free(const char *unit, const void *mem)
{
  Mem_t *tmp, *prev = NULL;

  for (tmp=memlist;tmp;prev=tmp,tmp=tmp->next)
    if (tmp->start == mem)
      break;

  if (tmp) { /* Found match */
    if (memlist == tmp) {
      memlist = tmp->next;
    }
    if (prev != NULL)
      prev->next = tmp->next;
    free(tmp->unit);
    free(tmp->start);
    free(tmp);
  } else {
    printf("Trying to free memory by %s, allocated by %s, size %d\n",
	   unit, tmp->unit, tmp->size);
  }
}

void 
mem_dump(void)
{
  Mem_t *tmp;

  printf("Dumping memory allocation\n");
  for (tmp=memlist;tmp;tmp=tmp->next)
    printf("%s : %d bytes from %p\n", tmp->unit, tmp->size, tmp->start);
  printf("-------------------------\n");
}
