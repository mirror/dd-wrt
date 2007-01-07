/*
 * stringmap.h:
 * map of strings
 *
 * Copyright (c) 2001 Chris Lightfoot. All rights reserved.
 *
 * $Id: stringmap.h,v 1.1 2003/10/19 06:44:33 pdw Exp $
 *
 */

#ifndef __STRINGMAP_H_ /* include guard */
#define __STRINGMAP_H_

#include "vector.h"

typedef struct _stringmap {
    char *key;
    item d;
    struct _stringmap *l, *g;
} *stringmap;

stringmap stringmap_new(void);
void      stringmap_delete(stringmap);
void      stringmap_delete_free(stringmap);

/* Try to insert an item into a stringmap, returning 1 if the map already
 * contained an item with that key.
 */
item *stringmap_insert(stringmap, const char*, const item);
/* Find an item in a stringmap */
stringmap     stringmap_find(const stringmap, const char*);

#endif /* __STRINGMAP_H_ */
