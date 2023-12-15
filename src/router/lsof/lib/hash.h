
/*
 * hash.sh - utilty functions for hash tables
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#if !defined(LSOF_HASH_H)
#    define LSOF_HASH_H 1

/* Find element in hash table by key, return NULL if not found
 *
 * table: hash table
 * hash: hash function/macro
 * type: struct type
 * member: member to match key
 * key: key
 *
 * the struct type should have a member called next
 **/
#    define HASH_FIND_ELEMENT(table, hash, type, member, key)                  \
        ({                                                                     \
            type *__value = NULL;                                              \
            int __h = hash(key);                                               \
            if ((table)) {                                                     \
                for (__value = (table)[__h]; __value;                          \
                     __value = __value->next) {                                \
                    if ((key) == __value->member)                              \
                        break;                                                 \
                };                                                             \
            };                                                                 \
            __value;                                                           \
        })

/* Insert element into hash table
 *
 * table: hash table
 * hash: hash function/macro
 * element: element
 * member: member name containing key
 *
 * the type of element should have a member called next
 **/
#    define HASH_INSERT_ELEMENT(table, hash, element, member)                  \
        ({                                                                     \
            int __h = hash((element)->member);                                 \
            (element)->next = (table)[__h];                                    \
            (table)[__h] = (element);                                          \
        })

#endif
