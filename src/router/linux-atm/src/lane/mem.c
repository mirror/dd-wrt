/*
 * Memory allocation unit
 *
 * $Id: mem.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <stdlib.h>
#include <string.h>

/* Local includes */
#include "mem.h"
#include "dump.h"
#include "load.h"
#include "units.h"

/* Type definitions */
typedef struct _MemList_t {
  void *mem;
  size_t memsize;
  const Unit_t *unit;
  struct _MemList_t *next;
} MemList_t;

/* Local function prototypes */
static void mem_init0(void);
static void mem_init1(void);
static void mem_dump_all(void);
static void mem_release(void);

/* Data */
static const char *rcsid = "$Id: mem.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $";

static MemList_t *memlist;

const Unit_t mem_unit = {
  "memory",
  &mem_init0,
  &mem_init1,
  &mem_dump_all,
  &mem_release
};

static unsigned int alloccount, freecount;

/* Functions */

/* Initialize local data */
static void
mem_init0(void)
{
  memlist = NULL;
  alloccount = 0;
  freecount = 0;
}

/* Initialization for data that needs other units */
static void
mem_init1(void)
{
  set_var_str(&mem_unit, "version", rcsid);
  Debug_unit(&mem_unit, "Initialized.");
}

/* Dump status, local data etc. */
static void
mem_dump_all(void)
{
  Debug_unit(&mem_unit, "Memory statistics:");
  mem_dump(NULL);
  Debug_unit(&mem_unit, "%d allocs, %d frees", alloccount, freecount);
}

/* Release allocated memory, close files etc. */
static void
mem_release(void)
{
  MemList_t *tmp;
  Debug_unit(&mem_unit, "Releasing unit");

  for (tmp = memlist; tmp != NULL; tmp = tmp->next) {
    Debug_unit(&mem_unit, "memory not released: unit %s size %u ptr 0x%x", tmp->unit->name, tmp->memsize, tmp->mem);
    mem_free(&mem_unit, tmp->mem);
  }
}

/* Allocate memory for unit */
void *
mem_alloc(const Unit_t *unit, size_t nbytes)
{
  MemList_t *tmp;

  alloccount++;
  tmp = (MemList_t *)malloc(sizeof(MemList_t));
  tmp->mem = malloc(nbytes);
  tmp->memsize = nbytes;
  tmp->unit = unit;
  tmp->next = memlist;
  memlist = tmp;
  if (get_var_bool(unit, "memdebug") == BL_TRUE) {
    Debug_unit(&mem_unit, "unit %s allocates size %u: ptr 0x%x", unit->name, nbytes, tmp->mem);
  }
  return tmp->mem;
}

/* Free memory block */
void
mem_free(const Unit_t *unit, const void *mem)
{
  MemList_t *tmp, *prev = NULL;
  Bool_t debug;

  freecount++;
  debug = get_var_bool(unit, "memdebug");
  if (debug == BL_TRUE) {
    Debug_unit(&mem_unit, "unit %s frees ptr 0x%x", unit->name, mem);
  }
  for (tmp = memlist; tmp != NULL; prev = tmp, tmp = tmp->next) {
    if (tmp->mem == mem) {
      break;
    }
  }
  /* Found a match? */
  if (tmp) {
    if (strcmp(tmp->unit->name, unit->name) != 0) {
      Debug_unit(&mem_unit, "unit %s frees ptr 0x%x size %d allocated by unit %s", unit->name, tmp->mem, tmp->memsize, tmp->unit->name);
    }
    if (memlist == tmp) {
      memlist = tmp->next;
    }
    if (prev != NULL) {
      prev->next = tmp->next;
    }
    if (debug == BL_TRUE) {
      Debug_unit(&mem_unit, "freeing %d bytes", tmp->memsize);
    }
    free(tmp->mem);
    free(tmp);
  }
  else {
    if (debug == BL_TRUE) {
      Debug_unit(&mem_unit, "could not find block 0x%x for freeing", mem);
    }
  }
}

/* Dump memory allocation information about unit, NULL == all units */
void
mem_dump(const Unit_t *unit)
{
  MemList_t *tmp;

  for (tmp = memlist; tmp != NULL; tmp = tmp->next) {
    if (unit == NULL || strcmp(tmp->unit->name, unit->name) == 0) {
      Debug_unit(&mem_unit, "unit %s size %u ptr 0x%x", tmp->unit->name, tmp->memsize, tmp->mem);
    }
  }
}

