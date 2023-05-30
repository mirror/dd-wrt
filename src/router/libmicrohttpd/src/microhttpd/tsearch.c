/*
 * Tree search generalized from Knuth (6.2.2) Algorithm T just like
 * the AT&T man page says.
 *
 * The node_t structure is for internal use only, lint doesn't grok it.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 */

#include "mhd_options.h"
#include "tsearch.h"
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */


typedef struct node
{
  const void *key;
  struct node  *llink, *rlink;
} node_t;


/*  $NetBSD: tsearch.c,v 1.7 2012/06/25 22:32:45 abs Exp $  */
/* find or insert datum into search tree */
void *
tsearch (const void *vkey, void **vrootp,
         int (*compar)(const void *, const void *))
{
  node_t *q;
  node_t **rootp = (node_t **) vrootp;

  if (rootp == NULL)
    return NULL;

  while (*rootp != NULL)        /* Knuth's T1: */
  {
    int r;

    if ((r = (*compar)(vkey, (*rootp)->key)) == 0) /* T2: */
      return *rootp;                               /* we found it! */

    rootp = (r < 0) ?
            &(*rootp)->llink :      /* T3: follow left branch */
            &(*rootp)->rlink;       /* T4: follow right branch */
  }

  q = malloc (sizeof(node_t)); /* T5: key not found */
  if (q != NULL)               /* make new node */
  {
    *rootp = q;                /* link new node to old */
    q->key = vkey; /* initialize new node */
    q->llink = q->rlink = NULL;
  }
  return q;
}


/*  $NetBSD: tfind.c,v 1.7 2012/06/25 22:32:45 abs Exp $    */
/* find a node by key "vkey" in tree "vrootp", or return 0 */
void *
tfind (const void *vkey, void * const *vrootp,
       int (*compar)(const void *, const void *))
{
  node_t * const *rootp = (node_t * const *) vrootp;

  if (rootp == NULL)
    return NULL;

  while (*rootp != NULL)            /* T1: */
  {
    int r;

    if ((r = (*compar)(vkey, (*rootp)->key)) == 0) /* T2: */
      return *rootp;                               /* key found */
    rootp = (r < 0) ?
            &(*rootp)->llink :                     /* T3: follow left branch */
            &(*rootp)->rlink;                      /* T4: follow right branch */
  }
  return NULL;
}


/*  $NetBSD: tdelete.c,v 1.8 2016/01/20 20:47:41 christos Exp $ */
/* find a node with key "vkey" in tree "vrootp" */
void *
tdelete (const void *vkey, void **vrootp,
         int (*compar)(const void *, const void *))
{
  node_t **rootp = (node_t **) vrootp;
  node_t *p, *q, *r;
  int cmp;

  if ((rootp == NULL) || ((p = *rootp) == NULL) )
    return NULL;

  while ((cmp = (*compar)(vkey, (*rootp)->key)) != 0)
  {
    p = *rootp;
    rootp = (cmp < 0) ?
            &(*rootp)->llink :       /* follow llink branch */
            &(*rootp)->rlink;        /* follow rlink branch */
    if (*rootp == NULL)
      return NULL;                   /* key not found */
  }
  r = (*rootp)->rlink;               /* D1: */
  if ((q = (*rootp)->llink) == NULL) /* Left NULL? */
    q = r;
  else if (r != NULL)                /* Right link is NULL? */
  {
    if (r->llink == NULL)            /* D2: Find successor */
    {
      r->llink = q;
      q = r;
    }
    else                    /* D3: Find NULL link */
    {
      for (q = r->llink; q->llink != NULL; q = r->llink)
        r = q;
      r->llink = q->rlink;
      q->llink = (*rootp)->llink;
      q->rlink = (*rootp)->rlink;
    }
  }
  free (*rootp);            /* D4: Free node */
  *rootp = q;               /* link parent to new node */
  return p;
}


/* end of tsearch.c */
