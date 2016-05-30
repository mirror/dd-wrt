/* src/vm/optimizing/bytecode_escape.c

   Copyright (C) 1996-2013
   CACAOVM - Verein zu Foerderung der freien virtuellen Machine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#include <stdint.h>

#include "mm/dumpmemory.hpp"
#include "mm/memory.hpp"

#include "toolbox/bitvector.hpp"

#include "vm/class.hpp"
#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/references.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/ir/bytecode.hpp"
#include "vm/jit/optimizing/escape.hpp"

#include <assert.h>
#include <stdarg.h>

#define BC_ESCAPE_VERBOSE !defined(NDEBUG)

/*** dprintf *****************************************************************/

#if BC_ESCAPE_VERBOSE && 0
void dprintf(int depth, const char *fmt, ...) {
	va_list ap;

	while (depth-- > 0) {
		printf("    ");
	}
	printf("| ");

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}
#endif

#define dprintf(x, ...) printf(__VA_ARGS__)

/*** op_stack_slot  **********************************************************/

typedef enum {
	OP_STACK_SLOT_TYPE_PARAM = 0,
	OP_STACK_SLOT_TYPE_UNKNOWN = 1
} op_stack_slot_type_t;

typedef struct {
	unsigned type:1;
	unsigned index:31;
} op_stack_slot_t;

const op_stack_slot_t OP_STACK_SLOT_UNKNOWN = {
	OP_STACK_SLOT_TYPE_UNKNOWN,
	0
};

static inline op_stack_slot_t op_stack_slot_create_param(s4 index) {
	op_stack_slot_t res;
	res.type = OP_STACK_SLOT_TYPE_PARAM;
	res.index = index;
	return res;
}

static inline bool op_stack_slot_is_unknown(const op_stack_slot_t slot) {
	return slot.type == OP_STACK_SLOT_TYPE_UNKNOWN;
}

static inline bool op_stack_slot_is_param(const op_stack_slot_t slot) {
	return slot.type == OP_STACK_SLOT_TYPE_PARAM;
}

/*** op_stack *****************************************************************/

/*

+---+---+---+---+  push 1
        ^

+---+---+-1-+---+  push 2
            ^

+---+---+-1-+-2-+  pop 2
                ^

+---+---+-1-+---+  pop 1
            ^

+---+---+---+---+
        ^
*/

typedef struct {
	op_stack_slot_t *elements;
	op_stack_slot_t *start;
	op_stack_slot_t *end;
	op_stack_slot_t *ptr;
	op_stack_slot_t *bottom;
	unsigned max;
	bool *perror_flag;
} op_stack_t;

static bool op_stack_test_position(op_stack_t *stack, op_stack_slot_t *pos);

static void op_stack_init(op_stack_t *stack, unsigned max, bool *perror_flag) {
	op_stack_slot_t *it;

	stack->elements = DMNEW(op_stack_slot_t, max * 2);
	stack->max = max;
	stack->start = stack->elements + max;
	stack->end = stack->elements + max + max;

	for (it = stack->elements; it != stack->start; ++it) {
		*it = OP_STACK_SLOT_UNKNOWN;
	}

	stack->ptr = stack->start;
	stack->bottom = stack->start;

	stack->perror_flag = perror_flag;
}

static void op_stack_set_error(op_stack_t *stack) {
	*(stack->perror_flag) = true;
#if BC_ESCAPE_VERBOSE
	printf("%s: error.\n", __FUNCTION__);
#endif
}

static bool op_stack_test_position(op_stack_t *stack, op_stack_slot_t *pos) {
	if (!(stack->elements <= pos)) {
		op_stack_set_error(stack);
		return false;
	} else if (!(pos < stack->end)) {
		op_stack_set_error(stack);
		return false;
	} else {
		return true;
	}
}

static void op_stack_reset(op_stack_t *stack) {
	op_stack_slot_t *it;

	/* Clear bottom half. */

	for (it = stack->bottom; it != stack->elements + stack->max; ++it) {
		*it = OP_STACK_SLOT_UNKNOWN;
	}

	/* Reset pointers. */

	stack->ptr = stack->start;
	stack->bottom = stack->start;
}

static op_stack_slot_t op_stack_pop(op_stack_t *stack) {
	op_stack_slot_t ret;
	stack->ptr -= 1;
	if (! op_stack_test_position(stack, stack->ptr)) {
		return OP_STACK_SLOT_UNKNOWN;
	}
	ret = *(stack->ptr);
	if (stack->ptr < stack->bottom) {
		stack->bottom = stack->ptr;
	}
	return ret;
}

static void op_stack_push(op_stack_t *stack, op_stack_slot_t element) {
	if (op_stack_test_position(stack, stack->ptr)) {
		*(stack->ptr) = element;
		stack->ptr += 1;
	}
}

static op_stack_slot_t op_stack_get(op_stack_t *stack, int offset) {
	if (op_stack_test_position(stack, stack->ptr - offset)) {
		return *(stack->ptr - offset);
	} else {
		return OP_STACK_SLOT_UNKNOWN;
	}
}

static void op_stack_set(op_stack_t *stack, int offset, op_stack_slot_t value) {
	if (op_stack_test_position(stack, stack->ptr - offset)) {
		*(stack->ptr - offset) = value;
	}
}

static inline void op_stack_push_unknown(op_stack_t *stack) {
	op_stack_push(stack, OP_STACK_SLOT_UNKNOWN);
}

static void op_stack_print(const op_stack_t *stack, FILE *f) {
	op_stack_slot_t *it;
	char sep;

	for (it = stack->bottom; it < stack->ptr; ++it) {
		if (it == stack->start) {
			sep = '!';
		} else {
			sep = '|';
		}
		if (op_stack_slot_is_unknown(*it)) {
			fprintf(f, "%c----", sep);
		} else {
			fprintf(f, "%cP%3d", sep, it->index);
		}
	}

	fprintf(f, "|");
}

static bool op_stack_is_empty(const op_stack_t *stack) {
	return !(stack->bottom < stack->ptr);
}

static s4 op_stack_element_count(const op_stack_t *stack) {
	return (stack->ptr - stack->bottom);
}

/*** bit_vector **************************************************************/

typedef struct {
	bitvector bv;
	s4 size;
} bit_vector_t;

static void bit_vector_init(bit_vector_t *bv, s4 size) {
	bv->bv = bv_new(size);
	bv->size = size;
}

static s4 bit_vector_size(const bit_vector_t *bv) {
	return bv->size;
}

static void bit_vector_set(bit_vector_t *bv, s4 bit) {
	assert(0 <= bit && bit < bv->size);
	bv_set_bit(bv->bv, bit);
}

static bool bit_vector_get(const bit_vector_t *bv, s4 bit) {
	assert(0 <= bit && bit < bv->size);
	return bv_get_bit(bv->bv, bit);
}

/*** basicblock_work_list ***********************************************/

typedef struct basicblock_work_item {
	s4 bytecode_index;
	struct basicblock_work_item *next;
} basicblock_work_item_t;

typedef struct {
	basicblock_work_item_t *first;
	basicblock_work_item_t *last;
} basicblock_work_list_t;

void basicblock_work_list_init(basicblock_work_list_t *lst) {
	lst->first = NULL;
	lst->last = NULL;
}

#define FOR_EACH_BASICBLOCK_WORK_LIST(lst, it) \
	for ((it) = (lst)->first; (it); (it) = (it)->next)

void basicblock_work_list_insert(basicblock_work_list_t *lst, s4 bytecode_index) {
	basicblock_work_item_t *it, *item;

	/* If the destination is already present in the list, do nothing. */

	FOR_EACH_BASICBLOCK_WORK_LIST(lst, it) {
		if (it->bytecode_index == bytecode_index) {
			return;
		}
	}

	item = DNEW(basicblock_work_item_t);
	item->bytecode_index = bytecode_index;
	item->next = NULL;

	if (lst->first == NULL) {
		lst->first = item;
		lst->last = item;
	} else {
		lst->last->next = item;
		lst->last = item;
	}
}

/*** value_category *****************************************************/

typedef enum {
	VALUE_CATEGORY_1,
	VALUE_CATEGORY_2
} value_category_t;

/*** jcode **************************************************************/

typedef struct {
	u1 *start;
	u1 *end;
	u1 *pos;
	u1 *instruction_start;
	s4 offset;
	bool *perror_flag;
} jcode_t;

static void jcode_init(jcode_t *jc, u1 *start, s4 length, s4 offset, bool *perror_flag) {
	jc->start = start;
	jc->end = jc->start + length;
	jc->pos = jc->start;
	jc->offset = offset;
	jc->perror_flag = perror_flag;
}

static void jcode_set_error(jcode_t *jc) {
	*(jc->perror_flag) = true;
#if BC_ESCAPE_VERBOSE
	printf("%s: error.\n", __FUNCTION__);
#endif
}

static void jcode_move_to_index(jcode_t *jc, s4 index) {
	jc->pos = jc->start + (index - jc->offset);
}

static bool jcode_end(const jcode_t *jc) {
	return (jc->pos >= jc->end);
}

static void jcode_record_instruction_start(jcode_t *jc) {
	jc->instruction_start = jc->pos;
}

static void jcode_rewind_instruction(jcode_t *jc) {
	jc->pos = jc->instruction_start;
}

static s4 jcode_get_instruction_length(const jcode_t *jc) {
	return (jc->pos - jc->instruction_start);
}

static void jcode_align_bytecode_index(jcode_t *jc, s4 align) {
	s4 idx, aidx;

	idx = jc->offset + (jc->pos - jc->start);
	aidx = MEMORY_ALIGN(idx, align);

	jc->pos += (aidx - idx);
}

static void jcode_forward_instruction_relative(jcode_t *jc, s4 n) {
	jc->pos = jc->instruction_start + n;
}

static s4 jcode_get_index(const jcode_t *jc) {
	return jc->offset + (jc->pos - jc->start);
}

bool jcode_test_has_bytes(jcode_t *jc, s4 n) {
	if ((jc->pos + n) <= jc->end) {
		return true;
	} else {
		jcode_set_error(jc);
		return false;
	}
}

static u1 jcode_get_u1(jcode_t *jc) {
	u1 ret;
	if (jcode_test_has_bytes(jc, 1)) {
		ret = jc->pos[0];
		jc->pos += 1;
	} else {
		ret = 0;
	}
	return ret;
}

static ByteCode jcode_get_bytecode(jcode_t *jc) {
	return (ByteCode) jcode_get_u1(jc);
}

static s2 jcode_get_s2(jcode_t *jc) {
	s2 ret;
	if (jcode_test_has_bytes(jc, 2)) {
		ret = (jc->pos[0] << 8) | (jc->pos[1]);
		jc->pos += 2;
	} else {
		ret = 0;
	}
	return ret;
}

static u2 jcode_get_u2(jcode_t *jc) {
	u2 ret;
	if (jcode_test_has_bytes(jc, 2)) {
		ret = (jc->pos[0] << 8) | (jc->pos[1]);
		jc->pos += 2;
	} else {
		ret = 0;
	}
	return ret;
}

static s4 jcode_get_s4(jcode_t *jc) {
	s4 ret;
	if (jcode_test_has_bytes(jc, 4)) {
		ret = (jc->pos[0] << 24) | (jc->pos[1] << 16) | (jc->pos[2] << 8) | (jc->pos[3]);
		jc->pos += 4;
	} else {
		ret = 0;
	}
	return ret;
}

static s4 jcode_get_branch_target(jcode_t *jc) {
	s2 off = jcode_get_s2(jc);
	return jc->offset + (jc->instruction_start - jc->start) + off;
}

static s4 jcode_get_branch_target_wide(jcode_t *jc) {
	s4 off = jcode_get_s4(jc);
	return jc->offset + (jc->instruction_start - jc->start) + off;
}

static s4 jcode_get_fall_through_target(jcode_t *jc) {
	int length = bytecode[*jc->instruction_start].length;
	if (length <= 0) {
		jcode_set_error(jc);
	}
	return jc->offset + (jc->instruction_start - jc->start) + length;
}

/*** bc_escape_analysis *************************************************/

typedef struct {
	methodinfo *method;
	op_stack_t *stack;
	basicblock_work_list_t *basicblocks;

	op_stack_slot_t *local_to_adr_param;
	s4 local_to_adr_param_size;

	u1 *param_escape;
	s4 param_escape_size;

	bit_vector_t *adr_param_dirty;
	bit_vector_t *adr_param_returned;

	s4 non_escaping_adr_params;

#if BC_ESCAPE_VERBOSE
	bool verbose;
#endif
	int depth;

	bool fatal_error;
} bc_escape_analysis_t;

static void bc_escape_analysis_perform_intern(methodinfo *m, int depth);

static void bc_escape_analysis_init(bc_escape_analysis_t *be, methodinfo *m, bool verbose, int depth) {
	u2 p;
	int l;
	int a;
	u1 *ite;
	u1 t;
	unsigned n;
	int ret_val_is_adr;

	be->method = m;

	be->stack = DNEW(op_stack_t);
	op_stack_init(be->stack, m->maxstack, &(be->fatal_error));

	be->basicblocks = DNEW(basicblock_work_list_t);
	basicblock_work_list_init(be->basicblocks);

	be->local_to_adr_param_size = m->parseddesc->paramslots;
	be->local_to_adr_param = DMNEW(op_stack_slot_t, m->parseddesc->paramslots);

	/* Count number of address parameters a. */

	for (p = 0, l = 0, a = 0; p < m->parseddesc->paramcount; ++p) {
		t = m->parseddesc->paramtypes[p].type;
		if (t == TYPE_ADR) {
			be->local_to_adr_param[l] = op_stack_slot_create_param(a);
			a += 1;
			l += 1;
		} else if (IS_2_WORD_TYPE(t)) {
			be->local_to_adr_param[l] = OP_STACK_SLOT_UNKNOWN;
			be->local_to_adr_param[l + 1] = OP_STACK_SLOT_UNKNOWN;
			l += 2;
		} else {
			be->local_to_adr_param[l] = OP_STACK_SLOT_UNKNOWN;
			l += 1;
		}
	}

	assert(l == be->local_to_adr_param_size);

	ret_val_is_adr = m->parseddesc->returntype.type == TYPE_ADR ? 1 : 0;

	/* Allocate param_escape on heap. */

	be->param_escape_size = a;
	n = a + ret_val_is_adr;

	if (n == 0) {
		/* Use some non-NULL value. */
		be->param_escape = (u1 *)1;
	} else {
		be->param_escape = MNEW(u1, n);
		be->param_escape += ret_val_is_adr;
	}

	for (ite = be->param_escape; ite != be->param_escape + n; ++ite) {
		*ite = escape_state_to_u1(ESCAPE_NONE);
	}

	if (ret_val_is_adr) {
		be->param_escape[-1] = escape_state_to_u1(ESCAPE_NONE);
	}

	be->adr_param_dirty = DNEW(bit_vector_t);
	bit_vector_init(be->adr_param_dirty, a);

	be->adr_param_returned= DNEW(bit_vector_t);
	bit_vector_init(be->adr_param_returned, a);

	be->non_escaping_adr_params = be->param_escape_size;

#if BC_ESCAPE_VERBOSE
	be->verbose = verbose;
#endif

	be->depth = depth;

	be->fatal_error = false;
}

static void bc_escape_analysis_branch_target(bc_escape_analysis_t *be, s4 branch_target) {
	basicblock_work_list_insert(be->basicblocks, branch_target);
}

static void bc_escape_analysis_adjust_state(
	bc_escape_analysis_t *be,
	op_stack_slot_t adr_param,
	escape_state_t escape_state
) {
	escape_state_t old;

	if (op_stack_slot_is_param(adr_param)) {
		if (0 <= adr_param.index && adr_param.index < be->param_escape_size) {
			old = (escape_state_t)be->param_escape[adr_param.index];
			if (old < escape_state) {

				/* Adjust escape state. */
				be->param_escape[adr_param.index] = (u1)escape_state;

				/* If the parameter has just escaped, decrement number of non-escaping
				   parameters. */

				if (
					old < ESCAPE_GLOBAL &&
					escape_state >= ESCAPE_GLOBAL
				) {
					be->non_escaping_adr_params -= 1;
				}
			}
		}
	}
}

static void bc_escape_analysis_dirty(bc_escape_analysis_t *be, s4 local) {
	op_stack_slot_t adr_param;

	if (0 <= local && local < be->local_to_adr_param_size) {
		adr_param = be->local_to_adr_param[local];
		if (op_stack_slot_is_param(adr_param)) {
			bit_vector_set(be->adr_param_dirty, adr_param.index);
		}
	}
}

static void bc_escape_analysis_dirty_2(bc_escape_analysis_t *be, s4 local) {
	bc_escape_analysis_dirty(be, local);
	bc_escape_analysis_dirty(be, local + 1);
}

static void bc_escape_analysis_returned(bc_escape_analysis_t *be, op_stack_slot_t value) {
	if (op_stack_slot_is_param(value)) {
		/* A parameter is returned, mark it as being returned. */
		bit_vector_set(be->adr_param_returned, value.index);
		/* The escape state of the return value will be adjusted later. */
	} else {
		/* Adjust escape state of return value. */
		if (be->method->parseddesc->returntype.type == TYPE_ADR) {
			be->param_escape[-1] = escape_state_to_u1(ESCAPE_GLOBAL);
		}
		bc_escape_analysis_adjust_state(be, value, ESCAPE_GLOBAL);
	}
}

op_stack_slot_t bc_escape_analysis_address_local(bc_escape_analysis_t *be, s4 local) {
	if (0 <= local && local < be->local_to_adr_param_size) {
		return be->local_to_adr_param[local];
	} else {
		return OP_STACK_SLOT_UNKNOWN;
	}
}

value_category_t bc_escape_analysis_value_category(bc_escape_analysis_t *be, s4 index) {
	constant_FMIref *fmi;
	fmi = (constant_FMIref *) class_getconstant(be->method->clazz, index, CONSTANT_Fieldref);

	if (fmi == NULL) {
		/* TODO */
		assert(0);
	}

	switch (fmi->parseddesc.fd->type) {
		case TYPE_LNG:
		case TYPE_DBL:
			return VALUE_CATEGORY_2;
		default:
			return VALUE_CATEGORY_1;
	}
}

static void bc_escape_analysis_push_return_value(
	bc_escape_analysis_t *be,
	methoddesc *md
) {
	switch (md->returntype.type) {
		case TYPE_LNG:
		case TYPE_DBL:
			op_stack_push_unknown(be->stack);
			op_stack_push_unknown(be->stack);
			break;
		case TYPE_VOID:
			/* Do nothing */
			break;
		default:
			op_stack_push_unknown(be->stack);
			break;
	}
}

static void bc_escape_analysis_adjust_invoke_parameters(
	bc_escape_analysis_t *be,
	methodinfo *mi
) {
	int i;
	methoddesc *md = mi->parseddesc;
	u1 *paramescape = mi->paramescape;
	s4 stack_depth = md->paramslots;
	unsigned num_params_returned = 0;
	op_stack_slot_t param_returned;

	/* Process parameters.
	 * The first parameter is at the highest depth on the stack.
	 */

	for (i = 0; i < md->paramcount; ++i) {
		switch (md->paramtypes[i].type) {
			case TYPE_ADR:
				if (*paramescape & 0x80) {
					num_params_returned += 1;
					param_returned = op_stack_get(be->stack, stack_depth);
				}
				bc_escape_analysis_adjust_state(
					be,
					op_stack_get(be->stack, stack_depth),
					escape_state_from_u1(*paramescape++)
				);
				stack_depth -= 1;
				break;
			case TYPE_LNG:
			case TYPE_DBL:
				stack_depth -= 2;
				break;
			default:
				stack_depth -= 1;
				break;
		}
	}

	/* Pop all parameters. */

	for (i = 0; i < md->paramslots; ++i) {
		op_stack_pop(be->stack);
	}

	/* Push return value. */

	if (md->returntype.type == TYPE_ADR) {
		if ((num_params_returned == 1) && (mi->paramescape[-1] < ESCAPE_GLOBAL)) {
			/* Only a single argument can be returned by the method,
			   and the retun value does not escape otherwise. */
			op_stack_push(be->stack, param_returned);
		} else {
			op_stack_push_unknown(be->stack);
		}
	} else {
		bc_escape_analysis_push_return_value(be, md);
	}
}

static void bc_escape_analysis_escape_invoke_parameters(
	bc_escape_analysis_t *be,
	methoddesc *md
) {
	s4 i;
	for (i = 0; i < md->paramslots; ++i) {
		bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
	}

	bc_escape_analysis_push_return_value(be, md);
}

static void bc_escape_analysis_parse_invoke(bc_escape_analysis_t *be, jcode_t *jc) {
	constant_FMIref *fmi;
	methoddesc *md;
	resolve_result_t result;
	methodinfo *mi;

	ByteCode opc      = jcode_get_bytecode(jc);
	u2       cp_index = jcode_get_u2(jc);

	/* Get method reference */

	if (opc == BC_invokeinterface) {
		fmi = (constant_FMIref *) class_getconstant(be->method->clazz, cp_index, CONSTANT_InterfaceMethodref);
	} else {
		fmi = (constant_FMIref *) class_getconstant(be->method->clazz, cp_index, CONSTANT_Methodref);
	}

	if (fmi == NULL) {
		/* TODO */
		assert(0);
	}

	md = fmi->parseddesc.md;

	assert(md != NULL);

	/* Parse parameters if not done yet. */

	descriptor_params_from_paramtypes(md, opc == BC_invokestatic ? ACC_STATIC : 0);

	/* Try to lazyly resolve method. */

	result = resolve_method_lazy(be->method, fmi, opc == BC_invokespecial);

	if (result == resolveSucceeded) {
		mi = fmi->p.method;

#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			dprintf(
				be->depth,
				"Succefully resolved callee %s/%s. Recursing.\n",
				mi->clazz->name.begin(),
				mi->name.begin()
			);
		}
#endif
	} else {
		mi = NULL;
#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			dprintf(
				be->depth,
				"Failed to resolve callee %s/%s.\n",
				(fmi->is_resolved() ? "ERR" : fmi->p.classref->name.begin()),
				fmi->name.begin()
			);
		}
#endif
	}

	/* If we could resolve the method, either reuse available escape inormation
	   or recurse into callee.
	   Otherwise we must assume, that all parameters escape. */

	if (mi != NULL && escape_is_monomorphic(be->method, mi)) {

		if (mi->paramescape == NULL) {
			bc_escape_analysis_perform_intern(mi, be->depth + 1);
		}

		if (mi->paramescape == NULL) {
			/* No escape information. */
			bc_escape_analysis_escape_invoke_parameters(be, md);
		} else {
			/* Adjust escape state of parameters. */
			bc_escape_analysis_adjust_invoke_parameters(be, mi);
		}

	} else {
		bc_escape_analysis_escape_invoke_parameters(be, md);
	}
}

static void bc_escape_analysis_parse_tableswitch(
	bc_escape_analysis_t *be,
	jcode_t *jc
) {
	s4 high, low, def;
	s4 i;

	jcode_get_bytecode(jc); /* opcode */

	jcode_align_bytecode_index(jc, 4);

	def = jcode_get_s4(jc);
	low = jcode_get_s4(jc);
	high = jcode_get_s4(jc);

	if (low <= high) {
		for (i = 0; i < (high - low + 1); ++i) {
			bc_escape_analysis_branch_target(
				be,
				jcode_get_branch_target_wide(jc)
			);
		}
	}

}

static void bc_escape_analysis_parse_lookupswitch(
	bc_escape_analysis_t *be,
	jcode_t *jc
) {
	s4 npairs;
	s4 i;

	jcode_get_bytecode(jc); /* opcode */

	jcode_align_bytecode_index(jc, 4);

	/* default */

	bc_escape_analysis_branch_target(
		be,
		jcode_get_branch_target_wide(jc)
	);

	/* npairs */

	npairs = jcode_get_s4(jc);

	for (i = 0; i < npairs; ++i) {
		/* Match */
		jcode_get_s4(jc);

		/* Offset */
		bc_escape_analysis_branch_target(
			be,
			jcode_get_branch_target_wide(jc)
		);
	}

}

static void bc_escape_analysis_process_basicblock(bc_escape_analysis_t *be, jcode_t *jc) {
	op_stack_slot_t value1, value2, value3, value4;
	u1 dim;
	int length;
	bool bb_end = false;

#if BC_ESCAPE_VERBOSE
	if (be->verbose) {
		dprintf(be->depth, "Processing basicblock at offset %d.\n", jcode_get_index(jc));
	}
#endif

	op_stack_reset(be->stack);

	/* TODO end if all parameters escape */
	/* TODO move code into process_instruction or the like */

	while ((! jcode_end(jc)) && (! bb_end) && (! be->fatal_error)) {

		jcode_record_instruction_start(jc);

		ByteCode opc = jcode_get_bytecode(jc);

		length = bytecode[opc].length;

#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			dprintf(be->depth, "* %s, ", bytecode[opc].mnemonic);
			op_stack_print(be->stack, stdout);
			printf(" => ");
		}
#endif

		switch (opc) {
			case BC_nop:
				break;

			case BC_aconst_null:
			case BC_iconst_m1:
			case BC_iconst_0:
			case BC_iconst_1:
			case BC_iconst_2:
			case BC_iconst_3:
			case BC_iconst_4:
			case BC_iconst_5:
			case BC_fconst_0:
			case BC_fconst_1:
			case BC_fconst_2:
				op_stack_push_unknown(be->stack);
				break;

			case BC_dconst_0:
			case BC_dconst_1:
			case BC_lconst_0:
			case BC_lconst_1:
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_bipush:
			case BC_sipush:
				op_stack_push_unknown(be->stack);
				break;

			case BC_ldc1:
			case BC_ldc2:
				op_stack_push_unknown(be->stack);
				break;

			case BC_ldc2w:
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_iload:
			case BC_fload:
			case BC_iload_0:
			case BC_iload_1:
			case BC_iload_2:
			case BC_iload_3:
			case BC_fload_0:
			case BC_fload_1:
			case BC_fload_2:
			case BC_fload_3:
				op_stack_push_unknown(be->stack);
				break;

			case BC_dload:
			case BC_lload:
			case BC_lload_0:
			case BC_lload_1:
			case BC_lload_2:
			case BC_lload_3:
			case BC_dload_0:
			case BC_dload_1:
			case BC_dload_2:
			case BC_dload_3:
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_aload:
				op_stack_push(be->stack, bc_escape_analysis_address_local(be, jcode_get_u1(jc)));
				break;

			case BC_aload_0:
				op_stack_push(be->stack, bc_escape_analysis_address_local(be, 0));
				break;

			case BC_aload_1:
				op_stack_push(be->stack, bc_escape_analysis_address_local(be, 1));
				break;

			case BC_aload_2:
				op_stack_push(be->stack, bc_escape_analysis_address_local(be, 2));
				break;

			case BC_aload_3:
				op_stack_push(be->stack, bc_escape_analysis_address_local(be, 3));
				break;

			case BC_iaload:
			case BC_faload:
			case BC_baload:
			case BC_caload:
			case BC_saload:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_laload:
			case BC_daload:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_aaload:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_istore:
			case BC_fstore:
				bc_escape_analysis_dirty(be, jcode_get_u1(jc));
				op_stack_pop(be->stack);
				break;

			case BC_istore_0:
			case BC_fstore_0:
				bc_escape_analysis_dirty(be, 0);
				op_stack_pop(be->stack);
				break;

			case BC_istore_1:
			case BC_fstore_1:
				bc_escape_analysis_dirty(be, 1);
				op_stack_pop(be->stack);
				break;

			case BC_istore_2:
			case BC_fstore_2:
				bc_escape_analysis_dirty(be, 2);
				op_stack_pop(be->stack);
				break;

			case BC_istore_3:
			case BC_fstore_3:
				bc_escape_analysis_dirty(be, 3);
				op_stack_pop(be->stack);
				break;

			case BC_lstore:
			case BC_dstore:
				bc_escape_analysis_dirty_2(be, jcode_get_u1(jc));
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_lstore_0:
			case BC_dstore_0:
				bc_escape_analysis_dirty_2(be, 0);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_lstore_1:
			case BC_dstore_1:
				bc_escape_analysis_dirty_2(be, 1);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_lstore_2:
			case BC_dstore_2:
				bc_escape_analysis_dirty_2(be, 2);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_lstore_3:
			case BC_dstore_3:
				bc_escape_analysis_dirty_2(be, 3);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_astore:
				bc_escape_analysis_dirty(be, jcode_get_u1(jc));
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				break;

			case BC_astore_0:
				bc_escape_analysis_dirty(be, 0);
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				break;

			case BC_astore_1:
				bc_escape_analysis_dirty(be, 1);
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				break;

			case BC_astore_2:
				bc_escape_analysis_dirty(be, 2);
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				break;

			case BC_astore_3:
				bc_escape_analysis_dirty(be, 3);
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				break;

			case BC_iastore:
			case BC_fastore:
			case BC_bastore:
			case BC_castore:
			case BC_sastore:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_lastore:
			case BC_dastore:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_aastore:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				break;

			case BC_pop:
				op_stack_pop(be->stack);
				break;

			case BC_pop2:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				break;

			case BC_dup:
				value1 = op_stack_get(be->stack, 1);
				op_stack_push(be->stack, value1);
				break;

			case BC_dup_x1:
				value1 = op_stack_pop(be->stack);
				value2 = op_stack_pop(be->stack);
				op_stack_push(be->stack, value1);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				break;

			case BC_dup_x2:
				value1 = op_stack_pop(be->stack);
				value2 = op_stack_pop(be->stack);
				value3 = op_stack_pop(be->stack);
				op_stack_push(be->stack, value1);
				op_stack_push(be->stack, value3);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				break;

			case BC_dup2:
				value1 = op_stack_get(be->stack, 1);
				value2 = op_stack_get(be->stack, 2);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				break;

			case BC_dup2_x1:
				value1 = op_stack_pop(be->stack);
				value2 = op_stack_pop(be->stack);
				value3 = op_stack_pop(be->stack);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				op_stack_push(be->stack, value3);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				break;

			case BC_dup2_x2:
				value1 = op_stack_pop(be->stack);
				value2 = op_stack_pop(be->stack);
				value3 = op_stack_pop(be->stack);
				value4 = op_stack_pop(be->stack);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				op_stack_push(be->stack, value4);
				op_stack_push(be->stack, value3);
				op_stack_push(be->stack, value2);
				op_stack_push(be->stack, value1);
				break;

			case BC_swap:
				value1 = op_stack_get(be->stack, 1);
				value2 = op_stack_get(be->stack, 2);
				op_stack_set(be->stack, 1, value2);
				op_stack_set(be->stack, 2, value1);
				break;

			case BC_iadd:
			case BC_fadd:

			case BC_isub:
			case BC_fsub:

			case BC_imul:
			case BC_fmul:

			case BC_idiv:
			case BC_fdiv:

			case BC_irem:
			case BC_frem:

				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_ladd:
			case BC_dadd:

			case BC_lsub:
			case BC_dsub:

			case BC_ldiv:
			case BC_ddiv:

			case BC_lmul:
			case BC_dmul:

			case BC_lrem:
			case BC_drem:

				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_ineg:
			case BC_lneg:
			case BC_fneg:
			case BC_dneg:

				/* Nothing */
				break;

			case BC_ishl:
			case BC_ishr:
			case BC_iushr:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_lshl:
			case BC_lshr:
			case BC_lushr:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				/* Second operand is int. */
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_iand:
			case BC_ior:
			case BC_ixor:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_land:
			case BC_lor:
			case BC_lxor:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_iinc:
				/* Not stack operation. */
				bc_escape_analysis_dirty(be, jcode_get_u1(jc));
				break;

			case BC_i2l:
			case BC_i2d:
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_i2f:
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_l2i:
			case BC_l2f:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_l2d:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_f2i:
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_f2l:
			case BC_f2d:
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_d2i:
			case BC_d2f:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_d2l:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_int2byte:
			case BC_int2char:
			case BC_int2short:
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_fcmpl:
			case BC_fcmpg:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_lcmp:
			case BC_dcmpl:
			case BC_dcmpg:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_ifeq:
			case BC_ifne:
			case BC_iflt:
			case BC_ifge:
			case BC_ifgt:
			case BC_ifle:
				op_stack_pop(be->stack);
				bc_escape_analysis_branch_target(be, jcode_get_branch_target(jc));
				bc_escape_analysis_branch_target(be, jcode_get_fall_through_target(jc));
				bb_end = true;
				break;

			case BC_if_icmpeq:
			case BC_if_icmpne:
			case BC_if_icmplt:
			case BC_if_icmpge:
			case BC_if_icmpgt:
			case BC_if_icmple:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				bc_escape_analysis_branch_target(be, jcode_get_branch_target(jc));
				bc_escape_analysis_branch_target(be, jcode_get_fall_through_target(jc));
				bb_end = true;
				break;

			case BC_if_acmpeq:
			case BC_if_acmpne:
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_METHOD);
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_METHOD);
				bc_escape_analysis_branch_target(be, jcode_get_branch_target(jc));
				bc_escape_analysis_branch_target(be, jcode_get_fall_through_target(jc));
				bb_end = true;
				break;

			case BC_goto:
				bc_escape_analysis_branch_target(be, jcode_get_branch_target(jc));
				bb_end = true;
				break;

			case BC_jsr:
				op_stack_push_unknown(be->stack);
				bc_escape_analysis_branch_target(be, jcode_get_branch_target(jc));
				bb_end = true;
				break;

			case BC_ret:
				break;

			case BC_tableswitch:
				op_stack_pop(be->stack);
				jcode_rewind_instruction(jc);
				bc_escape_analysis_parse_tableswitch(be, jc);
				length = jcode_get_instruction_length(jc);
				bb_end = 1;
				break;

			case BC_lookupswitch:
				op_stack_pop(be->stack);
				jcode_rewind_instruction(jc);
				bc_escape_analysis_parse_lookupswitch(be, jc);
				length = jcode_get_instruction_length(jc);
				bb_end = 1;
				break;

			case BC_return:
				bb_end = true;
				break;

			case BC_ireturn:
			case BC_freturn:
				op_stack_pop(be->stack);
				bb_end = true;
				break;

			case BC_lreturn:
			case BC_dreturn:
				op_stack_pop(be->stack);
				op_stack_pop(be->stack);
				bb_end = true;
				break;

			case BC_areturn:
				/* FIXME */
				bc_escape_analysis_returned(be, op_stack_pop(be->stack));
				bb_end = true;
				break;

			case BC_getfield:
				op_stack_pop(be->stack);
				/* Fall through. */

			case BC_getstatic:
				if (
					bc_escape_analysis_value_category(be, jcode_get_s2(jc)) ==
					VALUE_CATEGORY_2
				) {
					op_stack_push_unknown(be->stack);
				}
				op_stack_push_unknown(be->stack);
				break;


			case BC_putfield:
				if (
					bc_escape_analysis_value_category(be, jcode_get_u2(jc)) ==
					VALUE_CATEGORY_2
				) {
					op_stack_pop(be->stack);

					op_stack_pop(be->stack);
					op_stack_pop(be->stack);
				} else {
					value2 = op_stack_pop(be->stack);
					value1 = op_stack_pop(be->stack);
					bc_escape_analysis_adjust_state(be, value2, ESCAPE_GLOBAL);
				}
				break;

			case BC_putstatic:
				if (
					bc_escape_analysis_value_category(be, jcode_get_u2(jc)) ==
					VALUE_CATEGORY_2
				) {
					op_stack_pop(be->stack);
					op_stack_pop(be->stack);
				} else {
					value1 = op_stack_pop(be->stack);
					bc_escape_analysis_adjust_state(be, value1, ESCAPE_GLOBAL);
				}
				break;

			case BC_invokevirtual:
			case BC_invokespecial:
			case BC_invokestatic:
			case BC_invokeinterface:
				jcode_rewind_instruction(jc);
				bc_escape_analysis_parse_invoke(be, jc);
				break;

			case BC_new:
				op_stack_push_unknown(be->stack);
				break;

			case BC_newarray:
			case BC_anewarray:
				op_stack_pop(be->stack);
				op_stack_push_unknown(be->stack);
				break;

			case BC_arraylength:
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_METHOD);
				op_stack_push_unknown(be->stack);
				break;

			case BC_athrow:
				bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
				bb_end = true;
				break;

			case BC_checkcast:
				value1 = op_stack_get(be->stack, 1);
				bc_escape_analysis_adjust_state(be, value1, ESCAPE_METHOD);
				break;

			case BC_instanceof:
				value1 = op_stack_pop(be->stack);
				bc_escape_analysis_adjust_state(be, value1, ESCAPE_METHOD);
				op_stack_push_unknown(be->stack);
				break;

			case BC_monitorenter:
			case BC_monitorexit:
				value1 = op_stack_pop(be->stack);
				bc_escape_analysis_adjust_state(be, value1, ESCAPE_METHOD);
				break;

			case BC_wide:

				/* All except iinc have a length of 4. */
				length = 4;

				switch (jcode_get_bytecode(jc)) {
					case BC_iload:
					case BC_fload:
						op_stack_push_unknown(be->stack);
						break;

					case BC_lload:
					case BC_dload:
						op_stack_push_unknown(be->stack);
						op_stack_push_unknown(be->stack);
						break;

					case BC_aload:
						op_stack_push(
							be->stack,
							bc_escape_analysis_address_local(
								be,
								jcode_get_u1(jc)
							)
						);
						break;

					case BC_istore:
					case BC_fstore:
						bc_escape_analysis_dirty(be, jcode_get_u2(jc));
						op_stack_pop(be->stack);
						break;

					case BC_lstore:
					case BC_dstore:
						bc_escape_analysis_dirty_2(be, jcode_get_u2(jc));
						op_stack_pop(be->stack);
						op_stack_pop(be->stack);
						break;

					case BC_astore:
						bc_escape_analysis_dirty(be, jcode_get_u2(jc));
						bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
						break;

					case BC_ret:
						/* Do nothing. */
						break;

					case BC_iinc:
						bc_escape_analysis_dirty(be, jcode_get_u2(jc));
						length = 6;
						/* Do nothing. */
						break;
					default:
						assert(false);
						break;
				}
				break;

			case BC_multianewarray:
				jcode_get_u2(jc);
				dim = jcode_get_u1(jc);
				while (dim-- > 0) {
					op_stack_pop(be->stack);
				}
				op_stack_push_unknown(be->stack);
				break;

			case BC_ifnull:
			case BC_ifnonnull:
				value1 = op_stack_pop(be->stack);
				bc_escape_analysis_adjust_state(be, value1, ESCAPE_METHOD);
				bc_escape_analysis_branch_target(be, jcode_get_branch_target(jc));
				bc_escape_analysis_branch_target(be, jcode_get_fall_through_target(jc));
				bb_end = true;
				break;

			case BC_goto_w:
				bc_escape_analysis_branch_target(be, jcode_get_branch_target_wide(jc));
				bb_end = true;
				break;

			case BC_jsr_w:
				bc_escape_analysis_branch_target(be, jcode_get_branch_target_wide(jc));
				bb_end = true;
				break;

			case BC_breakpoint:
			case BC_impdep1:
			case BC_impdep2:
				break;

			default:
				break;
		}
		assert(length > 0);
		jcode_forward_instruction_relative(jc, length);

#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			op_stack_print(be->stack, stdout);
			printf("\n");
		}
#endif
	}

#if BC_ESCAPE_VERBOSE
	if (be->verbose) {
		dprintf(be->depth, "Elements left on stack: %d\n", op_stack_element_count(be->stack));
	}
#endif

	while ((! op_stack_is_empty(be->stack)) && (! be->fatal_error)) {
#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			dprintf(be->depth, "Stack element: ");
			op_stack_print(be->stack, stdout);
			printf(" => ");
		}
#endif
		bc_escape_analysis_adjust_state(be, op_stack_pop(be->stack), ESCAPE_GLOBAL);
#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			op_stack_print(be->stack, stdout);
			printf("\n");
		}
#endif
	}

	if (be->fatal_error) {
#if BC_ESCAPE_VERBOSE
		if (be->verbose) {
			printf("Fatal error while processing basic block. Aborting.\n");
		}
#endif
		assert(0);
	}
}

static void	bc_escape_analysis_adjust_return_value(bc_escape_analysis_t *be) {
	escape_state_t re, pe;
	int i;

	/* Only calculate, if return value is of type address. */

	if (be->method->parseddesc->returntype.type != TYPE_ADR) {
		return ;
	}

	/* If a parameter can be returned, adjust to its escape state. */

	for (i = 0; i < be->param_escape_size; ++i) {
		if (bit_vector_get(be->adr_param_returned, i)) {
			be->param_escape[i] |= 0x80;

			pe = escape_state_from_u1(be->param_escape[i]);
			re = escape_state_from_u1(be->param_escape[-1]);

			if (pe > re) {
				be->param_escape[-1] = escape_state_to_u1(pe);
			}
		}
	}
}

static void bc_escape_analysis_analyze(bc_escape_analysis_t *be) {
	raw_exception_entry *itee;
	basicblock_work_item_t *bb;
	jcode_t jc;

	/* Add root as basic block. */

	bc_escape_analysis_branch_target(be, 0);

	/* Add each exception handler as basic block. */

	for (
		itee = be->method->rawexceptiontable;
		itee != be->method->rawexceptiontable + be->method->rawexceptiontablelength;
		++itee
	) {
		bc_escape_analysis_branch_target(be, itee->handlerpc);
	}

	/* Init jcode parser. */

	jcode_init(
		&jc,
		be->method->jcode,
		be->method->jcodelength,
		0,
		&(be->fatal_error)
	);

	/* Process basicblock by basicblock. */

	for (bb = be->basicblocks->first; bb; bb = bb->next) {
		jcode_move_to_index(&jc, bb->bytecode_index);
		bc_escape_analysis_process_basicblock(
			be,
			&jc
		);
	}

	/* Calculate escape of return value. */

	bc_escape_analysis_adjust_return_value(be);

	/* Export escape of parameters. */

	be->method->paramescape = be->param_escape;
}

static void bc_escape_analysis_perform_intern(methodinfo *m, int depth) {
	bc_escape_analysis_t *be;
	bool verbose = false;

#if BC_ESCAPE_VERBOSE
	if (verbose) {
		dprintf(
			depth,
			"=== BC escape analysis of %s/%s at depth %d ===\n",
			m->clazz->name.begin(),
			m->name.begin(),
			depth
		);
	}
#endif

	if (depth >= 3) {
		return;
	}

	if (m->paramescape != NULL) {
#if BC_ESCAPE_VERBOSE
		if (verbose) {
			dprintf(depth, "Escape info for method already available.\n");
		}
#endif
		return;
	}

	if ((m->flags & ACC_METHOD_EA) != 0) {
#if BC_ESCAPE_VERBOSE
		if (verbose) {
			dprintf(depth, "Detected recursion, aborting.\n");
		}
#endif
		return;
	}

	if (m->jcode == NULL) {
#if BC_ESCAPE_VERBOSE
		if (verbose) {
			dprintf(depth, "No bytecode for callee.\n");
		}
#endif
		return;
	}

	if (m->jcodelength > 250) {
#if BC_ESCAPE_VERBOSE
		if (verbose) {
			dprintf(depth, "Bytecode too long: %d.\n", m->jcodelength);
		}
#endif
		return;
	}

	be = DNEW(bc_escape_analysis_t);
	bc_escape_analysis_init(be, m, verbose, depth);

	m->flags |= ACC_METHOD_EA;

	bc_escape_analysis_analyze(be);

#if BC_ESCAPE_VERBOSE
	if (be->verbose) {
		dprintf(
			depth,
			"%s/%s: Non-escaping params: %d\n",
			m->clazz->name.begin(),
			m->name.begin(),
			be->non_escaping_adr_params
		);
	}
#endif

	m->flags &= ~ACC_METHOD_EA;
}

void bc_escape_analysis_perform(methodinfo *m) {
	bc_escape_analysis_perform_intern(m, 0);
}



/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */

