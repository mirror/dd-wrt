/*
 * General-purpose hash table macros for pound.
 * Copyright (C) 2023-2024 Sergey Poznyakoff
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
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * SYNOPSIS
 *      typedef struct {
 *          char name;
 *          ...
 *      } X
 *      #define HT_TYPE X
 *      #include "ht.h"
 *
 * DESCRIPTION
 *   When used as shown above, this file will expand to definitions of
 *   the aggregate data type X_HASH, representing a hash table for objects
 *   of type X, and the following inline functions to manipulate it:
 *
 *     X_HASH X_HASH_NEW (void);
 *     void X_HASH_FREE (X_HASH *);
 *     X *X_INSERT(X_HASH *, X *);
 *     X *X_RETRIEVE(X_HASH *, X *);
 *     X *X_DELETE(X_HASH *, X *);
 *     void X_FOREACH(X_HASH *, void (*)(X *, void *), void *)
 *
 *   If the name of the key field is not "name", the HT_NAME_FIELD must
 *   be defined before including ht.h:
 *
 *      #define HT_NAME_FIELD key
 *
 *   If the structure does not contain "name" field, or its equivalent, the
 *   caller must provide functions for hashing and comparing structures,
 *   with the following prototypes:
 *
 *     unsigned long X_hash (const X *)
 *     int X_cmp (const X *, const X *)
 *
 *   and define the following macros:
 *
 *     #define HT_TYPE_HASH_FN_DEFINED 1
 *     #define HT_TYPE_CMP_FN_DEFINED 1
 *
 *   If some functions from the list above are not needed, define the
 *   following symbols to prevent them from being declared:
 *
 *     HT_NO_HASH_FREE   - omit X_HASH_FREE function;
 *     HT_NO_RETRIEVE    - omit X_RETRIEVE function;
 *     HT_NO_DELETE      - omit X_DELETE function;
 *     HT_NO_FOREACH     - omit X_FOREACH function;
 *
 *   (there is no way to omit the X_INSERT function - you need to populate
 *   the hash somehow, don't you?)
 *
 *   The file can be included multiple times, with different HT_TYPE
 *   definitions.
 *   HT_TYPE as well as all auxiliary macros are undefined at the end of the
 *   file.
 */
#define __cat2__(a,b) a ## b
#define cat2(a,b) __cat2__(a,b)
#define cat3(a,b,c) cat2(a, cat2(b,c))

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
# define HT_DECLARE(type) DEFINE_LHASH_OF (type)
#else
# define HT_DECLARE(type) DECLARE_LHASH_OF (type)
#endif
HT_DECLARE(HT_TYPE);
#undef HT_DECLARE

#define HT_TYPE_HASH_T cat2(HT_TYPE,_HASH)
#define HT_DEFTYPE(type, hashtype) typedef LHASH_OF(type) hashtype;
HT_DEFTYPE(HT_TYPE, HT_TYPE_HASH_T)
#undef HT_DEFTYPE

#define HT_TYPE_HASH_FN cat2(HT_TYPE,_hash)
#define HT_TYPE_CMP_FN cat2(HT_TYPE,_cmp)

#ifndef HT_NAME_FIELD
# define HT_NAME_FIELD name
#endif

#ifndef HT_TYPE_HASH_FN_DEFINED
static unsigned long
HT_TYPE_HASH_FN (const HT_TYPE *t)
{
  return lh_strhash (t->HT_NAME_FIELD);
}
#endif

#ifndef HT_TYPE_CMP_FN_DEFINED
static int
HT_TYPE_CMP_FN (const HT_TYPE *a, const HT_TYPE *b)
{
  return strcmp (a->HT_NAME_FIELD, b->HT_NAME_FIELD);
}
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define HT_IMPL_FN(type) \
  static IMPLEMENT_LHASH_HASH_FN (type, type) \
  static IMPLEMENT_LHASH_COMP_FN (type, type)
HT_IMPL_FN(HT_TYPE)
#undef HT_IMPL_FN
#endif

static inline HT_TYPE_HASH_T *
cat2(HT_TYPE,_HASH_NEW) (void)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  return cat3(lh_, HT_TYPE, _new) (HT_TYPE_HASH_FN, HT_TYPE_CMP_FN);
#else
  return LHM_lh_new (HT_TYPE, HT_TYPE);
#endif
}

#ifndef HT_NO_HASH_FREE
static inline void
cat2(HT_TYPE,_HASH_FREE) (HT_TYPE_HASH_T *tab)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  return cat3(lh_, HT_TYPE, _free) (tab);
#else
  return LHM_lh_free (HT_TYPE, tab);
#endif
}
#endif /* HT_NO_HASH_FREE */

static inline HT_TYPE *
cat2(HT_TYPE, _INSERT) (HT_TYPE_HASH_T *tab, HT_TYPE *node)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  return cat3(lh_, HT_TYPE, _insert) (tab, node);
#else
  return LHM_lh_insert (HT_TYPE, tab, node);
#endif
}

#ifndef HT_NO_RETRIEVE
static inline HT_TYPE *
cat2(HT_TYPE, _RETRIEVE) (HT_TYPE_HASH_T *tab, HT_TYPE *node)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  return cat3(lh_, HT_TYPE, _retrieve) (tab, node);
#else
  return LHM_lh_retrieve (HT_TYPE, tab, node);
#endif
}
#endif /* HT_NO_RETRIEVE */

#ifndef HT_NO_DELETE
static inline HT_TYPE *
cat2(HT_TYPE, _DELETE) (HT_TYPE_HASH_T *tab, HT_TYPE *node)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  return cat3(lh_, HT_TYPE, _delete) (tab, node);
#else
  return LHM_lh_delete (HT_TYPE, tab, node);
#endif
}
#endif /* HT_NO_DELETE */

#ifndef HT_NO_FOREACH
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
IMPLEMENT_LHASH_DOALL_ARG (HT_TYPE, void);
#endif

static inline void
cat2(HT_TYPE, _FOREACH) (HT_TYPE_HASH_T *tab, void (*fun) (HT_TYPE *, void *),
			 void *data)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  cat3 (lh_, HT_TYPE, _doall_void) (tab, fun, data);
#else
  LHM_lh_doall_arg (HT_TYPE, tab, (void (*)(void *, void *)) fun, void *, data);
#endif
}
#endif /* HT_NO_FOREACH */

#undef __cat2__
#undef cat2
#undef cat3
#undef HT_TYPE_HASH
#undef HT_TYPE_HASH_FN
#undef HT_TYPE_CMP_FN
#undef HT_TYPE_HASH_FN_DEFINED
#undef HT_TYPE_CMP_FN_DEFINED
#undef HT_NAME_FIELD
#undef HT_NO_FOREACH
#undef HT_NO_DELETE
#undef HT_NO_RETRIEVE
#undef HT_NO_HASH_FREE
#undef HT_TYPE

/* End of ht.h */
