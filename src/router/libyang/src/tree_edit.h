/**
 * @file tree_edit.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang generic macros and functions to modify YANG schema or data trees. Intended for internal use and libyang
 * plugins.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_TREE_EDIT_H_
#define LY_TREE_EDIT_H_

#include <stdlib.h>

#ifndef LOGMEM
#define LOGMEM(CTX)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper for realloc() call. The only difference is that if it fails to
 * allocate the requested memory, the original memory is freed as well.
 *
 * @param[in] ptr Memory to reallocate.
 * @param[in] size New size of the memory block.
 *
 * @return Pointer to the new memory, NULL on error.
 */
void *ly_realloc(void *ptr, size_t size);

/**
 * @defgroup trees_edit Trees - modification
 * @ingroup trees
 *
 * Generic macros, functions, etc. to modify [schema](@ref schematree) and [data](@ref datatree) trees.
 * @{
 */

/**
 * @brief (Re-)Allocation of a ([sized array](@ref sizedarrays)).
 *
 * Increases the size information.
 *
 * This is a generic macro for ::LY_ARRAY_NEW_RET and ::LY_ARRAY_NEW_GOTO.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to allocate/resize. The size of the allocated
 * space is counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[in] EACTION Action to perform in case of error (memory allocation failure).
 */
#define LY_ARRAY_NEW(CTX, ARRAY, EACTION) \
    { \
        char *p__; \
        if (ARRAY) { \
            ++(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
            p__ = (char *)realloc(((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1), \
                    sizeof(LY_ARRAY_COUNT_TYPE) + (*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) * sizeof *(ARRAY))); \
            if (!p__) { \
                --(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
                LOGMEM(CTX); \
                EACTION; \
            } \
        } else { \
            p__ = (char *)malloc(sizeof(LY_ARRAY_COUNT_TYPE) + sizeof *(ARRAY)); \
            if (!p__) { \
                LOGMEM(CTX); \
                EACTION; \
            } \
            *((LY_ARRAY_COUNT_TYPE*)(p__)) = 1; \
        } \
        p__ = (char *)((LY_ARRAY_COUNT_TYPE*)(p__) + 1); \
        memcpy(&(ARRAY), &p__, sizeof p__); \
    }

/**
 * @brief (Re-)Allocation of a ([sized array](@ref sizedarrays)).
 *
 * Increases the size information.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to allocate/resize. The size of the allocated
 * space is counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[out] NEW_ITEM Returning pointer to the newly allocated record in the ARRAY.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_ARRAY_NEW_RET(CTX, ARRAY, NEW_ITEM, RETVAL) \
    LY_ARRAY_NEW(CTX, ARRAY, return RETVAL); \
    (NEW_ITEM) = &(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) - 1]; \
    memset(NEW_ITEM, 0, sizeof *(NEW_ITEM))

/**
 * @brief (Re-)Allocation of a ([sized array](@ref sizedarrays)).
 *
 * Increases the size information.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to allocate/resize. The size of the allocated
 * space is counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[out] NEW_ITEM Returning pointer to the newly allocated record in the ARRAY.
 * @param[out] RET Variable to store error code.
 * @param[in] GOTO Label to go in case of error (memory allocation failure).
 */
#define LY_ARRAY_NEW_GOTO(CTX, ARRAY, NEW_ITEM, RET, GOTO) \
    LY_ARRAY_NEW(CTX, ARRAY, RET = LY_EMEM; goto GOTO); \
    (NEW_ITEM) = &(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) - 1]; \
    memset(NEW_ITEM, 0, sizeof *(NEW_ITEM))

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added and zeroed).
 *
 * Does not set the size information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
 * when the items are filled.
 *
 * This is a generic macro for ::LY_ARRAY_CREATE_RET and ::LY_ARRAY_CREATE_GOTO.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to create.
 * @param[in] SIZE Number of the new items the array is supposed to hold. The size of the allocated
 * space is then counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[in] EACTION Action to perform in case of error (memory allocation failure).
 */
#define LY_ARRAY_CREATE(CTX, ARRAY, SIZE, EACTION) \
    { \
        char *p__; \
        if (ARRAY) { \
            p__ = (char *)realloc(((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1), \
                    sizeof(LY_ARRAY_COUNT_TYPE) + ((*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1) + (SIZE)) * sizeof *(ARRAY))); \
            if (!p__) { \
                LOGMEM(CTX); \
                EACTION; \
            } \
        } else { \
            p__ = (char *)calloc(1, sizeof(LY_ARRAY_COUNT_TYPE) + (SIZE) * sizeof *(ARRAY)); \
            if (!p__) { \
                LOGMEM(CTX); \
                EACTION; \
            } \
        } \
        p__ = (char *)((LY_ARRAY_COUNT_TYPE*)(p__) + 1); \
        memcpy(&(ARRAY), &p__, sizeof p__); \
        if (ARRAY) { \
            memset(&(ARRAY)[*((LY_ARRAY_COUNT_TYPE*)(p__) - 1)], 0, (SIZE) * sizeof *(ARRAY)); \
        } \
    }

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added and zeroed).
 *
 * Does not set the size information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
 * when the items are filled.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to create.
 * @param[in] SIZE Number of the new items the array is supposed to hold. The size of the allocated
 * space is then counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_ARRAY_CREATE_RET(CTX, ARRAY, SIZE, RETVAL) \
    LY_ARRAY_CREATE(CTX, ARRAY, SIZE, return RETVAL)

/**
 * @brief Allocate a ([sized array](@ref sizedarrays)) for the specified number of items.
 * If the ARRAY already exists, it is resized (space for SIZE items is added).
 *
 * Does not set the count information, it is supposed to be incremented via ::LY_ARRAY_INCREMENT
 * when the items are filled.
 *
 * @param[in] CTX libyang context for logging.
 * @param[in,out] ARRAY Pointer to the array to create.
 * @param[in] SIZE Number of the new items the array is supposed to hold. The size of the allocated
 * space is then counted from the type of the ARRAY, so do not provide placeholder void pointers.
 * @param[out] RET Variable to store error code.
 * @param[in] GOTO Label to go in case of error (memory allocation failure).
 */
#define LY_ARRAY_CREATE_GOTO(CTX, ARRAY, SIZE, RET, GOTO) \
    LY_ARRAY_CREATE(CTX, ARRAY, SIZE, RET = LY_EMEM; goto GOTO)

/**
 * @brief Increment the items counter in a ([sized array](@ref sizedarrays)).
 *
 * Does not change the allocated memory used by the ARRAY. To do so, use LY_ARRAY_CREATE_RET,
 * LY_ARRAY_CREATE_GOTO or LY_ARRAY_RESIZE_ERR_RET.
 *
 * @param[in] ARRAY Pointer to the array to affect.
 */
#define LY_ARRAY_INCREMENT(ARRAY) \
        ++(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1))

/**
 * @brief Decrement the items counter in a ([sized array](@ref sizedarrays)).
 *
 * Does not change the allocated memory used by the ARRAY. To do so, use LY_ARRAY_CREATE_RET,
 * LY_ARRAY_CREATE_GOTO or LY_ARRAY_RESIZE_ERR_RET.
 *
 * @param[in] ARRAY Pointer to the array to affect.
 */
#define LY_ARRAY_DECREMENT(ARRAY) \
        --(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1))

/**
 * @brief Decrement the items counter in a ([sized array](@ref sizedarrays)) and free the whole array
 * in case it was decremented to 0.
 *
 * @param[in] ARRAY Pointer to the array to affect.
 */
#define LY_ARRAY_DECREMENT_FREE(ARRAY) \
        --(*((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1)); \
        if (!LY_ARRAY_COUNT(ARRAY)) { \
            LY_ARRAY_FREE(ARRAY); \
            (ARRAY) = NULL; \
        }

/**
 * @brief Free the space allocated for the ([sized array](@ref sizedarrays)).
 *
 * The items inside the array are not freed.
 *
 * @param[in] ARRAY A ([sized array](@ref sizedarrays)) to be freed.
 */
#define LY_ARRAY_FREE(ARRAY) \
        if (ARRAY){free((LY_ARRAY_COUNT_TYPE*)(ARRAY) - 1);}

/**
 * @brief Insert item into linked list.
 *
 * @param[in,out] LIST Linked list to add to.
 * @param[in] NEW_ITEM New item, that will be appended to the list, must be already allocated.
 * @param[in] LINKER name of structuring member that is used to connect items together.
 */
#define LY_LIST_INSERT(LIST, NEW_ITEM, LINKER)\
    if (!(*LIST)) { \
        memcpy(LIST, &(NEW_ITEM), sizeof NEW_ITEM); \
    } else { \
        size_t offset__ = (char *)&(*LIST)->LINKER - (char *)(*LIST); \
        char **iter__ = (char **)((size_t)(*LIST) + offset__); \
        while (*iter__) { \
            iter__ = (char **)((size_t)(*iter__) + offset__); \
        } \
        memcpy(iter__, &(NEW_ITEM), sizeof NEW_ITEM); \
    }

/**
 * @brief Allocate and insert new item into linked list, return in case of error.
 *
 * This is a generic macro for ::LY_LIST_NEW_RET and ::LY_LIST_NEW_GOTO.
 *
 * @param[in] CTX used for logging.
 * @param[in,out] LIST Linked list to add to.
 * @param[out] NEW_ITEM New item that is appended to the list.
 * @param[in] LINKER name of structure member that is used to connect items together.
 * @param[in] EACTION Action to perform in case of error (memory allocation failure).
 */
#define LY_LIST_NEW(CTX, LIST, NEW_ITEM, LINKER, EACTION) \
    { \
        char *p__ = (char *)calloc(1, sizeof *NEW_ITEM); \
        if (!p__) { \
            LOGMEM(CTX); \
            EACTION; \
        } \
        memcpy(&(NEW_ITEM), &p__, sizeof p__); \
        LY_LIST_INSERT(LIST, NEW_ITEM, LINKER); \
    }

/**
 * @brief Allocate and insert new item into linked list, return in case of error.
 *
 * @param[in] CTX used for logging.
 * @param[in,out] LIST Linked list to add to.
 * @param[out] NEW_ITEM New item that is appended to the list.
 * @param[in] LINKER name of structure member that is used to connect items together.
 * @param[in] RETVAL Return value for the case of error (memory allocation failure).
 */
#define LY_LIST_NEW_RET(CTX, LIST, NEW_ITEM, LINKER, RETVAL) \
    LY_LIST_NEW(CTX, LIST, NEW_ITEM, LINKER, return RETVAL)

/**
 * @brief Allocate and insert new item into linked list, goto specified label in case of error.
 *
 * @param[in] CTX used for logging.
 * @param[in,out] LIST Linked list to add to.
 * @param[out] NEW_ITEM New item that is appended to the list.
 * @param[in] LINKER name of structure member that is used to connect items together.
 * @param[in] RET variable to store returned error type.
 * @param[in] LABEL label to goto in case of error.
 */
#define LY_LIST_NEW_GOTO(CTX, LIST, NEW_ITEM, LINKER, RET, LABEL) \
    LY_LIST_NEW(CTX, LIST, NEW_ITEM, LINKER, RET = LY_EMEM; goto LABEL)

/** @} trees_edit */

#ifdef __cplusplus
}
#endif

#endif /* LY_TREE_EDIT_H_ */
