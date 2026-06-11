/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _ARRAY_H
#define _ARRAY_H

#include <sys_base.h>

/* definition of array
 * e.g. "aa bb cc": num = 3 index = 3
 * e.g. "aa bb NULL cc": num = 3 index = 4
 * e.g. "NULL aa bb NULL cc": num = 3 index = 5
 */
typedef struct {
	sa_u32_t num;
	sa_u32_t index;		/* max index + 1 */
	sa_u32_t max;
	void **slot;
} ARRAY_T;

//#define ARRAY_MIN_NUM 1
#define ARRAY_SLOT_MIN  1
#define ARRAY_SLOT_MAX  4096

#define array_num(pArray)       (pArray->num)
#define array_index(pArray)     (pArray->index)
#define array_slot(pArray, i)   ((i >= pArray->index)?NULL:(pArray->slot[i]))

/******************************************************************************
 *
 * description:
 *   create new array
 *
 * input:
 *   max - max number of slots
 *
 * output:
 *
 * return:
 *   NULL     - create fail
 *   not NULL - pointer to new array
 *
 ******************************************************************************/
ARRAY_T *array_new(sa_u32_t max);

/******************************************************************************
 *
 * description:
 *   free array
 *
 * input:
 *   pArray - pointer to array
 *
 * output:
 *
 * return:
 *
 ******************************************************************************/
void array_free(ARRAY_T *pArray);

/******************************************************************************
 *
 * description:
 *   duplicate array
 *
 * input:
 *   pArray - pointer to srouce array
 *
 * output:
 *
 * return:
 *   NULL     - fail
 *   not NULL - pointer to new array
 *
 * note:
 *   only copy slots, does not copy content of slot
 *
 ******************************************************************************/
ARRAY_T *array_copy(ARRAY_T *pArray);

/******************************************************************************
 *
 * description:
 *   set value to slot i of array
 *
 * input:
 *   pArray - pointer to srouce array
 *   data   - pointer to data
 *   i      - slot index
 *
 * output:
 *   pArray - pointer to array, the slot of array may be modified
 *
 * return:
 *   one of SHELL_ERROR_T
 *
 ******************************************************************************/
int array_set_slot(ARRAY_T *pArray, void *data, sa_u32_t i);

/******************************************************************************
 *
 * description:
 *   insert value to array in first available position
 *
 * input:
 *   pArray - pointer to srouce array
 *   data   - pointer to data
 *
 * output:
 *   pArray - pointer to array, the slot of array may be modified
 *
 * return:
 *   one of SHELL_ERROR_T
 *
 ******************************************************************************/
int array_insert_slot(ARRAY_T *pArray, void *data);

/******************************************************************************
 *
 * description:
 *   set slot i of array to empty
 *
 * input:
 *   pArray - pointer to srouce array
 *   i      - slot index
 *
 * output:
 *   pArray - pointer to array, the slot of array may be modified
 *
 * return:
 *   one of SHELL_ERROR_T
 *
 ******************************************************************************/
int array_unset_slot(ARRAY_T *pArray, const sa_u32_t i);

#endif /* _ARRAY_H */
