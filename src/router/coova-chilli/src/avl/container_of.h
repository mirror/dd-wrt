
/*
 * The olsr.org Optimized Link-State Routing daemon version 2 (olsrd2)
 * Copyright (c) 2004-2015, the olsr.org team - see HISTORY file
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/**
 * @file
 */

#ifndef CONTAINER_OF_H_
#define CONTAINER_OF_H_

#include <stddef.h>
#include "common_types.h"

#ifndef typeof
/*! map typeof macro to builtin typeof */
#define typeof(x) __typeof__(x)
#endif

/**
 * casts an embedded element of a struct into the surrounding struct
 * this macro returns bad results if ptr is NULL
 * @param ptr pointer to embedded element
 * @param type data type of surrounding struct
 * @param member name of node inside struct
 * @return pointer to surrounding struct
 */
#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member ) *__tempptr = (ptr); \
    (type *)((uint8_t *)__tempptr - offsetof(type,member)); \
  })

/**
 * Helper function for NULL safe container_of macro
 * @param ptr pointer to embedded element or NULL
 * @param offset offset to surrounding struct
 */
static INLINE void *
__container_of_if_notnull(void *ptr, size_t offset) {
  return ptr == NULL ? NULL : (((uint8_t *)ptr) - offset);
}

/**
 * casts an embedded node of a list/tree into the surrounding struct
 * this macro returns bad results if ptr is NULL
 * @param ptr pointer to node
 * @param type data type of surrounding struct
 * @param member name of node inside struct
 * @return pointer to surrounding struct
 */
#define container_of_if_notnull(ptr, type, member) ((type *)__container_of_if_notnull(ptr, offsetof(type, member)))

#endif /* CONTAINER_OF_H_ */
