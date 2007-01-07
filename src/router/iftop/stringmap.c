/*
 * stringmap.c: sucky implementation of binary trees
 *
 * This makes no attempt to balance the tree, so has bad worst-case behaviour.
 * Also, I haven't implemented removal of items from the tree. So sue me.
 *
 * Copyright (c) 2001 Chris Lightfoot. All rights reserved.
 *
 */

static const char rcsid[] = "$Id: stringmap.c,v 1.3 2003/11/06 23:37:20 chris Exp $";


#include <stdlib.h>
#include <string.h>

#include "stringmap.h"
#include "vector.h"
#include "iftop.h"

/* stringmap_new:
 * Allocate memory for a new stringmap. */
stringmap stringmap_new() {
    stringmap S;
    
    S = xcalloc(sizeof *S, 1);

    return S;
}

/* stringmap_delete:
 * Free memory for a stringmap. */
void stringmap_delete(stringmap S) {
    if (!S) return;
    if (S->l) stringmap_delete(S->l);
    if (S->g) stringmap_delete(S->g);

    xfree(S->key);
    xfree(S);
}

/* stringmap_delete_free:
 * Free memory for a stringmap, and the objects contained in it, assuming that
 * they are pointers to memory allocated by xmalloc(3). */
void stringmap_delete_free(stringmap S) {
    if (!S) return;
    if (S->l) stringmap_delete_free(S->l);
    if (S->g) stringmap_delete_free(S->g);

    xfree(S->key);
    xfree(S->d.v);
    xfree(S);
}

/* stringmap_insert:
 * Insert into S an item having key k and value d. Returns an existing key
 * or NULL if it was inserted. 
 */
item *stringmap_insert(stringmap S, const char *k, const item d) {
    if (!S) return 0;
    if (S->key == NULL) {
        S->key = xstrdup(k);
        S->d   = d;
        return NULL;
    } else {
        stringmap S2;
        for (S2 = S;;) {
            int i = strcmp(k, S2->key);
            if (i == 0) {
                return &(S2->d);
            }
            else if (i < 0) {
                if (S2->l) S2 = S2->l;
                else {
                    if (!(S2->l = stringmap_new())) return NULL;
                    S2->l->key = xstrdup(k);
                    S2->l->d   = d;
                    return NULL;
                }
            } else if (i > 0) {
                if (S2->g) S2 = S2->g;
                else {
                    if (!(S2->g = stringmap_new())) return NULL;
                    S2->g->key = xstrdup(k);
                    S2->g->d   = d;
                    return NULL;
                }
            }
        }
    }
}

/* stringmap_find:
 * Find in d an item having key k in the stringmap S, returning the item found
 * on success NULL if no key was found. */
stringmap stringmap_find(const stringmap S, const char *k) {
    stringmap S2;
    int i;
    if (!S || S->key == NULL) return 0;
    for (S2 = S;;) {
        i = strcmp(k, S2->key);
        if (i == 0) return S2;
        else if (i < 0) {
            if (S2->l) S2 = S2->l;
            else return NULL;
        } else if (i > 0) {
            if (S2->g) S2 = S2->g;
            else return NULL;
        }
    }
}
