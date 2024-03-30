/* List definitions for Pound.
 * Copyright (C) 2022-2024 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _POUND_LIST_H
# define _POUND_LIST_H

/*
 * Singly-linked list macros.
 */

#define SLIST_HEAD(name, type)			\
  struct name					\
    {						\
      struct type *sl_first;			\
      struct type *sl_last;			\
    }

#define SLIST_HEAD_INITIALIZER(head)		\
  { NULL, NULL }

#define SLIST_ENTRY(type)			\
  struct type *

#define SLIST_INIT(head)				\
  do							\
    {							\
      (head)->sl_first = (head)->sl_last = NULL;	\
    }							\
  while (0)

/* Insert elt at the head of the list. */
#define SLIST_INSERT_HEAD(elt, head, field)			\
  do								\
    {								\
      if (((elt)->field = (head)->sl_first) == NULL)		\
	(head)->sl_last = (elt);				\
      (head)->sl_first = (elt);					\
    }								\
  while (0)

/* Append elt to the tail of the list. */
#define SLIST_INSERT_TAIL(head, elt, field)		\
  do							\
    {							\
      if ((head)->sl_last)				\
	(head)->sl_last->field = (elt);			\
      else						\
	(head)->sl_first = (elt);			\
      (head)->sl_last = (elt);				\
      (elt)->field = NULL;				\
    }							\
  while (0)

#define SLIST_PUSH   SLIST_INSERT_TAIL

/* Insert elt after given element (anchor). */
#define SLIST_INSERT_AFTER(head, anchor, elt, field)		\
  do								\
    {								\
      if (((elt)->field = (anchor)->field) == NULL)		\
	(head)->sl_last = (elt);				\
      (anchor)->field = (elt);					\
    }								\
  while (0)

/* Remove element from the head of the list. */
#define SLIST_REMOVE_HEAD(head, field)					\
  do									\
    {									\
      if ((head)->sl_first != NULL &&					\
	  ((head)->sl_first = (head)->sl_first->field) == NULL)		\
	(head)->sl_last = NULL;						\
    }									\
  while (0)

#define SLIST_SHIFT  SLIST_REMOVE_HEAD

#define SLIST_FOREACH(var, head, field)		\
  for ((var) = (head)->sl_first; (var); (var) = (var)->field)

#define SLIST_COPY(dst, src)			\
  *dst = *src

#define SLIST_CONCAT(a, b, field)				\
  do								\
    {								\
      if ((a)->sl_last)						\
	(a)->sl_last->field = (b)->sl_first;			\
      else							\
	(a)->sl_first = (b)->sl_first;				\
      (a)->sl_last = (b)->sl_last;				\
    }								\
  while (0)

#define SLIST_FIRST(head) ((head)->sl_first)
#define SLIST_LAST(head) ((head)->sl_last)
#define SLIST_EMPTY(head) (SLIST_FIRST (head) == NULL)
#define SLIST_NEXT(elt, field) ((elt)->field)

/*
 * Doubly-linked lists.
 */

#define DLIST_HEAD(name, type)			\
  struct name					\
    {						\
      struct type *dl_first;			\
      struct type *dl_last;			\
    }

#define DLIST_HEAD_INITIALIZER(head)		\
  { NULL, NULL }

#define DLIST_ENTRY(type)			\
  struct					\
    {						\
      struct type *dl_prev, *dl_next;		\
    }

#define DLIST_INIT(head)				\
  do							\
    {							\
      (head)->dl_first = (head)->dl_last = NULL;	\
    }							\
  while (0)

#define DLIST_INSERT_HEAD(head, elt, field)			\
  do								\
    {								\
      (elt)->field.dl_prev = NULL;				\
      if (((elt)->field.dl_next = (head)->dl_first) == NULL)	\
	(head)->dl_last = (elt);				\
      else							\
	(head)->dl_first->field.dl_prev = (elt);		\
      (head)->dl_first = (elt);					\
    }								\
  while (0)

#define DLIST_INSERT_TAIL(head, elt, field)			\
  do								\
    {								\
      (elt)->field.dl_next = NULL;				\
      if (((elt)->field.dl_prev = (head)->dl_last) == NULL)	\
	(head)->dl_first = (elt);				\
      else							\
	(head)->dl_last->field.dl_next = (elt);			\
      (head)->dl_last = (elt);					\
    }								\
  while (0)

#define DLIST_PUSH DLIST_INSERT_TAIL

#define DLIST_INSERT_AFTER(head, anchor, elt, field)			\
  do									\
    {									\
      if ((anchor) == NULL)						\
	DLIST_INSERT_TAIL (head, elt, field);				\
      else								\
	{								\
	  if (((elt)->field.dl_next = (anchor)->field.dl_next) == NULL)	\
	    (head)->dl_last = (elt);					\
	  else								\
	    (elt)->field.dl_next->field.dl_prev = (elt);		\
	  (anchor)->field.dl_next = (elt);				\
	  (elt)->field.dl_prev = (anchor);				\
	}								\
    }									\
  while (0)

#define DLIST_INSERT_BEFORE(head, anchor, elt, field)			\
  do									\
    {									\
      if ((anchor) == NULL)						\
	DLIST_INSERT_HEAD (head, elt, field);				\
      else								\
	{								\
	  if (((elt)->field.dl_prev = (anchor)->field.dl_prev) == NULL)	\
	    (head)->dl_first = (elt);					\
	  else								\
	    (elt)->field.dl_prev->field.dl_next = (elt);		\
	  (anchor)->field.dl_prev = (elt);				\
	  (elt)->field.dl_next = (anchor);				\
	}								\
    }									\
  while (0)

#define DLIST_REMOVE(head, elt, field)					\
  do									\
    {									\
      if ((elt)->field.dl_prev)						\
	(elt)->field.dl_prev->field.dl_next = (elt)->field.dl_next;	\
      else								\
	(head)->dl_first = (elt)->field.dl_next;			\
      if (elt->field.dl_next)						\
	(elt)->field.dl_next->field.dl_prev = (elt)->field.dl_prev;	\
      else								\
	(head)->dl_last = (elt)->field.dl_prev;				\
    }									\
  while (0)

#define DLIST_REMOVE_HEAD(head, field)					\
  do									\
    {									\
      if (((head)->dl_first = (head)->dl_first->field.dl_next) != NULL)	\
	(head)->dl_first->field.dl_prev = NULL;				\
      else								\
	(head)->dl_last = NULL;						\
    }									\
  while (0)

#define DLIST_SHIFT DLIST_REMOVE_HEAD

#define DLIST_FOREACH(var, head, field)		\
  for ((var) = (head)->dl_first; (var); (var) = (var)->field.dl_next)

#define DLIST_FOREACH_REVERSE(var, head, field)				\
  for ((var) = (head)->dl_last; (var); (var) = (var)->field.dl_prev)

#define DLIST_COPY(dst, src)			\
  *dst = *src

#define DLIST_FIRST(head) ((head)->dl_first)
#define DLIST_LAST(head) ((head)->dl_last)
#define DLIST_EMPTY(head) (DLIST_FIRST (head) == NULL)
#define DLIST_NEXT(elt, field) ((elt)->field.dl_next)
#define DLIST_PREV(elt, field) ((elt)->field.dl_prev)

#define DLIST_FOREACH_SAFE(var, tmp, head, field)			\
  for ((var) = (head)->dl_first,					\
	 ((tmp) = (var) ? DLIST_NEXT (var, field) : NULL);		\
       (var);								\
       (var) = (tmp),							\
	 ((tmp) = (var) ? DLIST_NEXT (var, field) : NULL))

#endif
