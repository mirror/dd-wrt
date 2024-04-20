/**
 * @file set.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic set routines implementations
 *
 * Copyright (c) 2015 - 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "set.h"

LIBYANG_API_DEF LY_ERR
ly_set_new(struct ly_set **set_p)
{
    LY_CHECK_ARG_RET(NULL, set_p, LY_EINVAL);

    *set_p = calloc(1, sizeof **set_p);
    LY_CHECK_ERR_RET(!(*set_p), LOGMEM(NULL), LY_EMEM);

    return LY_SUCCESS;
}

LIBYANG_API_DEF void
ly_set_clean(struct ly_set *set, void (*destructor)(void *obj))
{
    uint32_t u;

    if (!set) {
        return;
    }

    if (destructor) {
        for (u = 0; u < set->count; ++u) {
            destructor(set->objs[u]);
        }
    }
    set->count = 0;
}

LIBYANG_API_DEF void
ly_set_erase(struct ly_set *set, void (*destructor)(void *obj))
{
    if (!set) {
        return;
    }

    ly_set_clean(set, destructor);

    free(set->objs);
    set->size = 0;
    set->objs = NULL;
}

LIBYANG_API_DEF void
ly_set_free(struct ly_set *set, void (*destructor)(void *obj))
{
    if (!set) {
        return;
    }

    ly_set_erase(set, destructor);

    free(set);
}

LIBYANG_API_DEF ly_bool
ly_set_contains(const struct ly_set *set, const void *object, uint32_t *index_p)
{
    LY_CHECK_ARG_RET(NULL, set, 0);

    for (uint32_t i = 0; i < set->count; i++) {
        if (set->objs[i] == object) {
            /* object found */
            if (index_p) {
                *index_p = i;
            }
            return 1;
        }
    }

    /* object not found */
    return 0;
}

LIBYANG_API_DEF LY_ERR
ly_set_dup(const struct ly_set *set, void *(*duplicator)(const void *obj), struct ly_set **newset_p)
{
    struct ly_set *newset;
    uint32_t u;

    LY_CHECK_ARG_RET(NULL, set, newset_p, LY_EINVAL);

    newset = calloc(1, sizeof *newset);
    LY_CHECK_ERR_RET(!newset, LOGMEM(NULL), LY_EMEM);
    if (!set->count) {
        *newset_p = newset;
        return LY_SUCCESS;
    }

    newset->count = set->count;
    newset->size = set->count; /* optimize the size */
    newset->objs = malloc(newset->size * sizeof *(newset->objs));
    LY_CHECK_ERR_RET(!newset->objs, LOGMEM(NULL); free(newset), LY_EMEM);
    if (duplicator) {
        for (u = 0; u < set->count; ++u) {
            newset->objs[u] = duplicator(set->objs[u]);
        }
    } else {
        memcpy(newset->objs, set->objs, newset->size * sizeof *(newset->objs));
    }

    *newset_p = newset;
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_set_add(struct ly_set *set, const void *object, ly_bool list, uint32_t *index_p)
{
    void **new;

    LY_CHECK_ARG_RET(NULL, set, LY_EINVAL);

    if (!list) {
        /* search for duplication */
        for (uint32_t i = 0; i < set->count; i++) {
            if (set->objs[i] == object) {
                /* already in set */
                if (index_p) {
                    *index_p = i;
                }
                return LY_SUCCESS;
            }
        }
    }

    if (set->size == set->count) {
#define SET_SIZE_STEP 8
        new = realloc(set->objs, (set->size + SET_SIZE_STEP) * sizeof *(set->objs));
        LY_CHECK_ERR_RET(!new, LOGMEM(NULL), LY_EMEM);
        set->size += SET_SIZE_STEP;
        set->objs = new;
#undef SET_SIZE_STEP
    }

    if (index_p) {
        *index_p = set->count;
    }
    set->objs[set->count++] = (void *)object;

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_set_merge(struct ly_set *trg, const struct ly_set *src, ly_bool list, void *(*duplicator)(const void *obj))
{
    uint32_t u;
    void *obj;

    LY_CHECK_ARG_RET(NULL, trg, LY_EINVAL);

    if (!src) {
        /* nothing to do */
        return LY_SUCCESS;
    }

    for (u = 0; u < src->count; ++u) {
        if (duplicator) {
            obj = duplicator(src->objs[u]);
        } else {
            obj = src->objs[u];
        }
        LY_CHECK_RET(ly_set_add(trg, obj, list, NULL));
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_set_rm_index(struct ly_set *set, uint32_t index, void (*destructor)(void *obj))
{
    LY_CHECK_ARG_RET(NULL, set, LY_EINVAL);
    LY_CHECK_ERR_RET(index >= set->count, LOGARG(NULL, index), LY_EINVAL);

    if (destructor) {
        destructor(set->objs[index]);
    }
    if (index == set->count - 1) {
        /* removing last item in set */
        set->objs[index] = NULL;
    } else {
        /* removing item somewhere in a middle, so put there the last item */
        set->objs[index] = set->objs[set->count - 1];
        set->objs[set->count - 1] = NULL;
    }
    set->count--;

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
ly_set_rm(struct ly_set *set, void *object, void (*destructor)(void *obj))
{
    uint32_t i;

    LY_CHECK_ARG_RET(NULL, set, object, LY_EINVAL);

    /* get index */
    for (i = 0; i < set->count; i++) {
        if (set->objs[i] == object) {
            break;
        }
    }
    LY_CHECK_ERR_RET((i == set->count), LOGARG(NULL, object), LY_EINVAL); /* object is not in set */

    return ly_set_rm_index(set, i, destructor);
}

LIBYANG_API_DEF LY_ERR
ly_set_rm_index_ordered(struct ly_set *set, uint32_t index, void (*destructor)(void *obj))
{
    LY_CHECK_ARG_RET(NULL, set, set->count, LY_EINVAL);

    if (destructor) {
        destructor(set->objs[index]);
    }
    set->count--;
    if (index == set->count) {
        /* removing last item in set */
        set->objs[index] = NULL;
    } else {
        /* removing item somewhere in a middle, move following items */
        memmove(set->objs + index, set->objs + index + 1, (set->count - index) * sizeof *set->objs);
        set->objs[set->count] = NULL;
    }

    return LY_SUCCESS;
}
