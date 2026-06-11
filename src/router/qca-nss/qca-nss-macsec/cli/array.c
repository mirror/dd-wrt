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

#include "cli.h"
#include "array.h"

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
ARRAY_T *array_new(sa_u32_t max)
{
	ARRAY_T *pArray;

	/* parater check */
	if (max > ARRAY_SLOT_MAX) {
		return NULL;
	} else if (0 == max) {
		max = ARRAY_SLOT_MIN;
	}

	pArray = osal_malloc(sizeof(ARRAY_T));
	if (NULL == pArray) {
		return NULL;
	}

	pArray->num = 0;
	pArray->index = 0;
	pArray->max = max;

	pArray->slot = osal_malloc(max * sizeof(void *));
	if (NULL == pArray->slot) {
		osal_free(pArray);
		return NULL;
	}

	osal_memset(pArray->slot, 0, max * sizeof(void *));

	return pArray;
}

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
void array_free(ARRAY_T *pArray)
{
	if (pArray != NULL) {
		if (pArray->slot != NULL) {
			osal_free(pArray->slot);
		}

		osal_free(pArray);
	}

	return;
}

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
ARRAY_T *array_copy(ARRAY_T *pArray)
{
	ARRAY_T *pNew = NULL;

	if ((NULL == pArray)
	    || (NULL == pArray->slot)) {
		return NULL;
	}

	pNew = array_new(pArray->max);
	if (NULL == pNew) {
		return NULL;
	}

	pNew->num = pArray->num;
	pNew->index = pArray->index;
	osal_memcpy(pNew->slot, pArray->slot, pArray->max * sizeof(void *));

	return pNew;
}

/******************************************************************************
 *
 * description:
 *   check whether the index is available in array, if not then extend array
 *   slots to cover the index
 *
 * input:
 *   pArray - pointer to srouce array
 *   i      - index of slot
 *
 * output:
 *
 * return:
 *   TRUE  - available
 *   FALSE - inavailable
 *
 ******************************************************************************/
static sa_bool_t _array_is_available(ARRAY_T *pArray, sa_u32_t i)
{
	void **newSlot;
	void **oldSlot;
	sa_u32_t max = pArray->max;

	if (i >= ARRAY_SLOT_MAX) {
		return SA_FALSE;
	}

	if (i < max) {
		return SA_TRUE;
	}

	while (max <= i) {
		max = max * 2;
	}

	newSlot = osal_malloc(max * sizeof(void *));
	if (NULL == newSlot) {
		return SA_FALSE;
	}

	osal_memset(newSlot, 0, max * sizeof(void *));
	oldSlot = pArray->slot;
	osal_memcpy(newSlot, oldSlot, pArray->max * sizeof(void *));
	pArray->slot = newSlot;
	pArray->max = max;
	osal_free(oldSlot);

	return SA_TRUE;
}

/******************************************************************************
 *
 * description:
 *   find first available slot index
 *
 * input:
 *   pArray - pointer to srouce array
 *
 * output:
 *
 * return:
 *   index of slot
 *
 ******************************************************************************/
static sa_u32_t _array_empty_slot_index(ARRAY_T *pArray)
{
	sa_u32_t i;

	if (0 == pArray->index) {
		return 0;
	}

	for (i = 1; i < pArray->index; i++) {
		if (NULL == pArray->slot[i]) {
			return i;
		}
	}

	return i;
}

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
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int array_set_slot(ARRAY_T *pArray, void *data, sa_u32_t i)
{
	if (NULL == pArray) {
		return CLI_FAIL;
	}

	if (!_array_is_available(pArray, i)) {
		return CLI_FAIL;
	}

	if (NULL == pArray->slot[i]) {
		pArray->num++;
	}

	pArray->slot[i] = data;

	if (pArray->index < (i + 1)) {
		pArray->index = i + 1;
	}

	return CLI_OK;
}

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
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int array_insert_slot(ARRAY_T *pArray, void *data)
{
	sa_u32_t i;

	if (NULL == pArray) {
		return CLI_FAIL;
	}

	i = _array_empty_slot_index(pArray);

	return array_set_slot(pArray, data, i);
}

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
 *   one of CLI_RET_T
 *
 ******************************************************************************/
int array_unset_slot(ARRAY_T *pArray, const sa_u32_t i)
{
	if (NULL == pArray) {
		return CLI_FAIL;
	}

	if (((i + 1) > pArray->max)
	    || ((i + 1) > pArray->index)) {
		return CLI_FAIL;
	}

	if (NULL != pArray->slot[i]) {
		pArray->num--;
	}

	pArray->slot[i] = NULL;

	/* if the slot is the top slot, need to re-calculate the index */
	if ((i + 1) == pArray->index) {
		sa_i32_t j = (sa_i32_t) i;

		while ((j >= 0) && (NULL == pArray->slot[j])) {
			j--;
		}

		pArray->index = j + 1;
	}

	return CLI_OK;
}
