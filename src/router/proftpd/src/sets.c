/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 2001, 2002, 2003 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/*
 * Generic set manipulation
 * $Id: sets.c,v 1.14 2004/12/17 01:58:47 castaglia Exp $
 *
 * TODO: Use a hash table to greatly speed up set manipulation on
 *       large sets.
 */

#include "conf.h"

#define POOL(x)		((x) ? (x) : permanent_pool)

/* Create a new set, compf is a pointer to the function used
 * to to compare members of the set ... it should return 1, 0, or
 * -1 after the fashion of strcmp.  Returns NULL if memory allocation
 * fails.
 */

xaset_t *xaset_create(pool *work_pool, XASET_COMPARE compf) {
  xaset_t *newset = palloc(POOL(work_pool), sizeof(xaset_t));

  if (!newset) return NULL;
  newset->xas_list = NULL;
  newset->pool = POOL(work_pool);
  newset->xas_compare = compf;
  return newset;
}

/* Inserts a new member into an existing set.  The member is inserted
 * at the beginning of the set.
 * Returns: 1 if successful
 *          0 if one or more arguments is invalid or something
 *            is wrong with the set
 *         -1 error (not used)
 */
int xaset_insert(xaset_t *set, xasetmember_t *member) {

  if (!set || !member) {
    errno = EINVAL;
    return 0;
  }

  member->next = set->xas_list;

  if (set->xas_list)
    set->xas_list->prev = member;

  set->xas_list = member;
  return 1;
}

/* Inserts a new member into an existing set at the end
 * of the list.
 */
int xaset_insert_end(xaset_t *set, xasetmember_t *member) {
  xasetmember_t **tmp, *prev = NULL;

  if (!set || !member) {
    errno = EINVAL;
    return 0;
  }

  for (tmp = &set->xas_list; *tmp; prev = *tmp, tmp = &(*tmp)->next)
    ;

  *tmp = member;
  member->prev = prev;
  member->next = NULL;

  if (prev)
    prev->next = member;

  return 1;
}

/* Inserts a new member into an existing set, sorted using the set's
   compare callback.  If dupes_allowed is non-0, returns 0 and the
   member is not added to the set.  Otherwise, it is added immediately
   before the first dupelicate.  If the set is not empty and not pre-
   sorted, results are undefined.
   Returns: 1 if successful
            0 if bad arguments or duplicate member
           -1 error (not used, applicable error would be in errno)
*/
int xaset_insert_sort(xaset_t *set, xasetmember_t *member, int dupes_allowed) {
  xasetmember_t **setp = NULL, *mprev = NULL;
  int c;

  if (!set || !member || !set->xas_compare) {
    errno = EINVAL;
    return 0;
  }

  for (setp = &set->xas_list; *setp; setp=&(*setp)->next) {
    if ((c = set->xas_compare(member,*setp)) <= 0) {
      if (c == 0 && !dupes_allowed)
        return 0;
      break;
    }
    mprev = *setp;
  }

  if (*setp)
    (*setp)->prev = member;

  member->prev = mprev;
  member->next = *setp;
  *setp = member;
  return 1;
}

/* Remove a member from a set.  The set need not be sorted.  Note that this
 * does NOT free the memory used by the member.
 * Returns: 1 if successful
 *          0 if invalid args
 *         -1 error (check errno)
 */
int xaset_remove(xaset_t *set, xasetmember_t *member) {
  xasetmember_t *m = NULL;

  if (!set || !member) {
    errno = EINVAL;
    return 0;
  }

  /* Check if member is actually a member of set. */
  for (m = set->xas_list; m; m = m->next)
    if (m == member)
      break;

  if (m == NULL) {
    errno = ENOENT;
    return -1;  
  }

  if (member->prev)
    member->prev->next = member->next;

  else /* assume that member is first in the list */
    set->xas_list = member->next;

  if (member->next)
    member->next->prev = member->prev;

  member->next = member->prev = NULL;
  return 1;
}

/* Perform a set union.  The two sets do not need to be sorted, however
   the returned new set will be.  Duplicate entries are, as per
   normal set logic, removed.  Return value is the new set, or
   NULL if out-of-memory or one or more arguments is invalid.
*/

xaset_t *xaset_union(pool *work_pool, xaset_t *set1, xaset_t *set2,
    size_t msize, XASET_MCOPY copyf) {
  xaset_t *newset;
  xasetmember_t *setp,*n;
  xaset_t *setv[3];
  xaset_t **setcp;

  setv[0] = set1; setv[1] = set2; setv[2] = NULL;

  if (!set1 || !set2 || (!msize && !copyf)) {
    errno = EINVAL;
    return NULL;
  }

  work_pool = (work_pool ? work_pool :
    (set1->pool ? set1->pool : set2->pool));

  if (!(newset = xaset_create(work_pool, set1->xas_compare)))
    return NULL;

  for (setcp = setv; *setcp; setcp++)
    for (setp = (*setcp)->xas_list; setp; setp=setp->next) {
      n = copyf ? copyf(setp) : (xasetmember_t*) palloc(work_pool, msize);
      if (!n)
        return NULL;			/* Could cleanup here */

      if (!copyf)
        memcpy(n, setp, msize);

      if (xaset_insert_sort(newset, n, 0) == -1)
        return NULL;
    }

  return newset;
}

/* Perform set subtraction: set1 - set2, creating a new set composed of
   all the elements in set1 that are not in set2.  NULL is returned if
   a new set cannot be created (out-of-memory), or either set1 or set2
   are NULL.  If the copyf argument is non-NULL it is used to copy each
   applicable element to the new set.  If either of the two sets is
   unsorted, the result will be undefined.  set1's comparison callback
   function is used when comparing members of each set.
*/

xaset_t *xaset_subtract(pool *work_pool, xaset_t *set1, xaset_t *set2,
    size_t msize, XASET_MCOPY copyf) {
  xaset_t *newset;
  xasetmember_t *set1p,*set2p,*n,**pos;
  int c;

  if (!set1 || !set2 || (!msize && !copyf)) {
    errno = EINVAL;
    return NULL;
  }

  work_pool = (work_pool ? work_pool :
    (set1->pool ? set1->pool : set2->pool));

  if (!(newset = xaset_create(work_pool, set1->xas_compare)))
    return NULL;

  pos = &newset->xas_list;

  /* NOTE: xaset_insert_sort is not used here for performance
     reasons. */

  for (set1p = set1->xas_list, set2p = set2->xas_list; set1p;
      set1p = set1p->next) {
    if (!set2p || (c = set1->xas_compare(set1p, set2p)) < 0) {
      /* Copy if set2 is exhausted or set1p's "value" is less than
         set2p's. */
      n = copyf ? copyf(set1p) : (xasetmember_t *) palloc(work_pool, msize);
      if (!n)
        return NULL;			/* Could cleanup here */

      if (!copyf)
        memcpy(n, set1p, msize);

      /* Create links */
      n->prev = *pos;
      n->next = NULL;
      if (*pos)
        pos = &(*pos)->next;
      *pos = n;

    } else if (c >= 0) {

      /* Traverse set2 until we reach a point where set2 is
         exhausted or set2p is "greater" than set1p */
      while ((set2p = set2p->next) != NULL &&
            set1->xas_compare(set1p, set2p) > 0) ;

      /* In case there are dupes in set1, examine the next (if any)
         value(s) and skip if necessary */
      while (set1p->next && set1->xas_compare(set1p, set1p->next) == 0)
        set1p = set1p->next;
    }
  }

  return newset;
}


/* Perform an exact copy of the entire set, returning the new set.
   msize specifies the size of each member.  If copyf is non-NULL, it
   is called instead to copy each member.
   Returns NULL if out of memory condition occurs
*/

xaset_t *xaset_copy(pool *work_pool, xaset_t *set, size_t msize,
    XASET_MCOPY copyf) {
  xaset_t *newset;
  xasetmember_t *n,*p,**pos;

  if (!copyf && !msize) {
    errno = EINVAL;
    return NULL;
  }

  work_pool = (work_pool ? work_pool : set->pool);

  if (!(newset = xaset_create(work_pool, set->xas_compare)))
    return NULL;

  pos = &newset->xas_list;

  /* NOTE: xaset_insert_sort is not used here for performance
     reasons. */

  for (p = set->xas_list; p; p=p->next) {
    n = copyf ? copyf(p) : (xasetmember_t *) palloc(work_pool, msize);
    if (!n)
      return NULL;			/* Could clean up here */

    if (!copyf)
      memcpy(n, p, msize);

    /* Create links */
    n->prev = *pos;
    n->next = NULL;
    if (*pos)
      pos = &(*pos)->next;
    *pos = n;
  }

  return newset;
}
