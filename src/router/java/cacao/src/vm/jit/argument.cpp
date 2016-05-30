/* src/vm/jit/argument.cpp - argument passing from and to JIT methods

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

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

#include <cassert>
#include <stdint.h>

#include "arch.hpp"
#include "md-abi.hpp"

#include "mm/dumpmemory.hpp"

#include "native/llni.hpp"

#include "vm/array.hpp"
#include "vm/descriptor.hpp"
#include "vm/global.hpp"
#include "vm/method.hpp"
#include "vm/primitive.hpp"
#include "vm/resolve.hpp"
#include "vm/vm.hpp"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/codegen-common.hpp" // for GET_LOW_REG, GET_HIGH_REG


/* argument_jitarray_load ******************************************************
 
   Returns the argument specified by index from one of the passed arrays
   and returns it.

*******************************************************************************/

imm_union argument_jitarray_load(methoddesc *md, int32_t index,
								 uint64_t *arg_regs, uint64_t *stack)
{
	imm_union  ret;
	paramdesc *pd;

	pd = &md->params[index];

	switch (md->paramtypes[index].type) {
		case TYPE_INT:
		case TYPE_ADR:
			if (pd->inmemory) {
#if (SIZEOF_VOID_P == 8)
				ret.l = (int64_t)stack[pd->index];
#else
				ret.l = *(int32_t *)(stack + pd->index);
#endif
			} else {
#if (SIZEOF_VOID_P == 8)
				ret.l = arg_regs[index];
#else
				ret.l = *(int32_t *)(arg_regs + index);
#endif
			}
			break;
		case TYPE_LNG:
			if (pd->inmemory) {
				ret.l = (int64_t)stack[pd->index];
			} else {
				ret.l = (int64_t)arg_regs[index];
			}
			break;
		case TYPE_FLT:
			if (pd->inmemory) {
				ret.l = (int64_t)stack[pd->index];
			} else {
				ret.l = (int64_t)arg_regs[index];
			}
			break;
		case TYPE_DBL:
			if (pd->inmemory) {
				ret.l = (int64_t)stack[pd->index];
			} else {
				ret.l = (int64_t)arg_regs[index];
			}
			break;
		default:
			ret.l = 0;
			os::shouldnotreach();
			break;
	}

	return ret;
}


/* argument_jitarray_store *****************************************************
 
   Stores the argument into one of the passed arrays at a slot specified
   by index.

*******************************************************************************/

void argument_jitarray_store(methoddesc *md, int32_t index,
							 uint64_t *arg_regs, uint64_t *stack,
							 imm_union param)
{
	paramdesc *pd;

	pd = &md->params[index];

	switch (md->paramtypes[index].type) {
		case TYPE_ADR:
			if (pd->inmemory) {
#if (SIZEOF_VOID_P == 8)
				stack[pd->index] = param.l;
#else
				assert(0);
#endif
			} else {
				arg_regs[index] = param.l;
			}
			break;
		default:
			os::unimplemented("argument_jitarray_store: type not implemented");
			break;
	}
}


/* argument_jitreturn_load *****************************************************

   Loads the proper return value form the return register and returns it.

*******************************************************************************/

imm_union argument_jitreturn_load(methoddesc *md, uint64_t *return_regs)
{
	imm_union ret;

	switch (md->returntype.type) {
		case TYPE_INT:
		case TYPE_ADR:
#if (SIZEOF_VOID_P == 8)
			ret.l = return_regs[0];
#else
			ret.l = *(int32_t *)return_regs;
#endif
			break;
		case TYPE_LNG:
			ret.l = *(int64_t *)return_regs;
			break;
		case TYPE_FLT:
			ret.l = *(int64_t *)return_regs;
			break;
		case TYPE_DBL:
			ret.l = *(int64_t *)return_regs;
			break;
		default:
			ret.l = 0;
			os::shouldnotreach();
			break;
	}

	return ret;
}


/* argument_jitreturn_store ****************************************************

   Stores the proper return value into the return registers.

*******************************************************************************/

void argument_jitreturn_store(methoddesc *md, uint64_t *return_regs, imm_union ret)
{
	switch (md->returntype.type) {
		case TYPE_ADR:
#if (SIZEOF_VOID_P == 8)
			return_regs[0] = ret.l;
#else
			assert(0);
#endif
			break;
		default:
			os::unimplemented("argument_jitreturn_store: type not implemented");
			break;
	}
}


/* argument_vmarray_store_int **************************************************

   Helper function to store an integer into the argument array, taking
   care of architecture specific issues.

*******************************************************************************/

static void argument_vmarray_store_int(uint64_t *array, paramdesc *pd, int32_t value)
{
	int32_t index;

	if (!pd->inmemory) {
		index        = pd->index;
		array[index] = (int64_t) value;
	}
	else {
		index        = ARG_CNT + pd->index;
#if SIZEOF_VOID_P == 8
		array[index] = (int64_t) value;
#else
# if WORDS_BIGENDIAN == 1
		array[index] = ((int64_t) value) << 32;
# else
		array[index] = (int64_t) value;
# endif
#endif
	}
}


/* argument_vmarray_store_lng **************************************************

   Helper function to store a long into the argument array, taking
   care of architecture specific issues.

*******************************************************************************/

static void argument_vmarray_store_lng(uint64_t *array, paramdesc *pd, int64_t value)
{
	int32_t index;

#if SIZEOF_VOID_P == 8
	if (!pd->inmemory)
		index = pd->index;
	else
		index = ARG_CNT + pd->index;

	array[index] = value;
#else
	if (!pd->inmemory) {
		/* move low and high 32-bits into it's own argument slot */

		index        = GET_LOW_REG(pd->index);
		array[index] = value & 0x00000000ffffffff;

		index        = GET_HIGH_REG(pd->index);
		array[index] = value >> 32;
	}
	else {
		index        = ARG_CNT + pd->index;
		array[index] = value;
	}
#endif
}


/* argument_vmarray_store_flt **************************************************

   Helper function to store a float into the argument array, taking
   care of architecture specific issues.

*******************************************************************************/

static void argument_vmarray_store_flt(uint64_t *array, paramdesc *pd, uint64_t value)
{
	int32_t index;

	if (!pd->inmemory) {
#if defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
		index        = pd->index;
#else
		index        = INT_ARG_CNT + pd->index;
#endif
#if WORDS_BIGENDIAN == 1 && !defined(__POWERPC__) && !defined(__POWERPC64__) && !defined(__S390__)
		array[index] = value >> 32;
#else
		array[index] = value;
#endif
	}
	else {
		index        = ARG_CNT + pd->index;
#if defined(__SPARC_64__)
		array[index] = value >> 32;
#else
		array[index] = value;
#endif
	}
}


/* argument_vmarray_store_dbl **************************************************

   Helper function to store a double into the argument array, taking
   care of architecture specific issues.

*******************************************************************************/

static void argument_vmarray_store_dbl(uint64_t *array, paramdesc *pd, uint64_t value)
{
	int32_t index;

	if (!pd->inmemory) {
#if SIZEOF_VOID_P != 8 && defined(SUPPORT_PASS_FLOATARGS_IN_INTREGS)
		index        = GET_LOW_REG(pd->index);
		array[index] = value & 0x00000000ffffffff;

		index        = GET_HIGH_REG(pd->index);
		array[index] = value >> 32;
#else
		index        = INT_ARG_CNT + pd->index;
		array[index] = value;
#endif
	}
	else {
		index        = ARG_CNT + pd->index;
		array[index] = value;
	}
}


/* argument_vmarray_store_adr **************************************************

   Helper function to store an address into the argument array, taking
   care of architecture specific issues.

   ATTENTION: This function has to be used outside the nativeworld.

*******************************************************************************/

static void argument_vmarray_store_adr(uint64_t *array, paramdesc *pd, java_handle_t *h)
{
	void    *value;
	int32_t  index;

	/* Take the reference value out of the handle. */

	value = LLNI_UNWRAP(h);

	if (!pd->inmemory) {
		index        = pd->index;
		array[index] = (uint64_t) (intptr_t) value;
	}
	else {
		index        = ARG_CNT + pd->index;
#if SIZEOF_VOID_P == 8
		array[index] = (uint64_t) (intptr_t) value;
#else
# if WORDS_BIGENDIAN == 1
		array[index] = ((uint64_t) (intptr_t) value) << 32;
# else
		array[index] = (uint64_t) (intptr_t) value;
# endif
#endif
	}
}


/* argument_vmarray_from_valist ************************************************

   Creates an argument array which can be passed to asm_vm_call_method.
   The array is created from the passed valist.

   ATTENTION: This function has to be used outside the native world.

*******************************************************************************/

uint64_t *argument_vmarray_from_valist(methodinfo *m, java_handle_t *o, va_list ap)
{
	methoddesc *md;
	paramdesc  *pd;
	typedesc   *td;
	uint64_t   *array;
	int32_t     i;
	imm_union   value;

	/* get the descriptors */

	md = m->parseddesc;
	pd = md->params;
	td = md->paramtypes;

	// Allocate argument array.
	array = (uint64_t*) DumpMemory::allocate(sizeof(uint64_t) * (INT_ARG_CNT + FLT_ARG_CNT + md->memuse));

	/* if method is non-static fill first block and skip `this' pointer */

	i = 0;

	if (o != NULL) {
		/* the `this' pointer */
		argument_vmarray_store_adr(array, pd, o);

		pd++;
		td++;
		i++;
	} 

	for (; i < md->paramcount; i++, pd++, td++) {
		switch (td->type) {
		case TYPE_INT:
			value.i = va_arg(ap, int32_t);
			argument_vmarray_store_int(array, pd, value.i);
			break;

		case TYPE_LNG:
			value.l = va_arg(ap, int64_t);
			argument_vmarray_store_lng(array, pd, value.l);
			break;

		case TYPE_FLT:
#if defined(__ALPHA__) || defined(__POWERPC__) || defined(__POWERPC64__)
			// The assembler code loads these directly and unconditionally into
			// the argument registers.

			if (!pd->inmemory)
				value.d = (double) va_arg(ap, double);
			else
				value.f = (float) va_arg(ap, double);
#else
			value.f = (float) va_arg(ap, double);
#endif
			argument_vmarray_store_flt(array, pd, value.l);
			break;

		case TYPE_DBL:
			value.d = va_arg(ap, double);
			argument_vmarray_store_dbl(array, pd, value.l);
			break;

		case TYPE_ADR: 
			value.a = va_arg(ap, void*);
			argument_vmarray_store_adr(array, pd, static_cast<java_handle_t*>(value.a));
			break;
		default:
			assert(false);
			break;
		}
	}

	return array;
}


/* argument_vmarray_from_jvalue ************************************************

   Creates an argument array which can be passed to asm_vm_call_method.
   The array is created from the passed jvalue array.

   ATTENTION: This function has to be used outside the native world.

*******************************************************************************/

uint64_t *argument_vmarray_from_jvalue(methodinfo *m, java_handle_t *o,
									   const jvalue *args)
{
	methoddesc *md;
	paramdesc  *pd;
	typedesc   *td;
	uint64_t   *array;
	int32_t     i;
	int32_t     j;

	/* get the descriptors */

	md = m->parseddesc;
	pd = md->params;
	td = md->paramtypes;

	/* allocate argument array */

	array = (uint64_t*) DumpMemory::allocate(sizeof(uint64_t) * (INT_ARG_CNT + FLT_ARG_CNT + md->memuse));

	/* if method is non-static fill first block and skip `this' pointer */

	i = 0;

	if (o != NULL) {
		/* the `this' pointer */
		argument_vmarray_store_adr(array, pd, o);

		pd++;
		td++;
		i++;
	} 

	for (j = 0; i < md->paramcount; i++, j++, pd++, td++) {
		switch (td->primitivetype) {
		case TYPE_INT:
			argument_vmarray_store_int(array, pd, args[j].i);
			break;

		case TYPE_LNG:
			argument_vmarray_store_lng(array, pd, args[j].j);
			break;

		case TYPE_FLT:
			argument_vmarray_store_flt(array, pd, args[j].j);
			break;

		case TYPE_DBL:
			argument_vmarray_store_dbl(array, pd, args[j].j);
			break;

		case TYPE_ADR: 
			argument_vmarray_store_adr(array, pd, (java_handle_t *) args[j].l);
			break;
		default:
			assert(false);
			break;
		}
	}

	return array;
}


/* argument_vmarray_from_objectarray *******************************************

   Creates an argument array which can be passed to asm_vm_call_method.
   The array is created from the passed objectarray of boxed values.

   ATTENTION: This function has to be used outside the native world.

   RETURN VALUE:
      NULL.........indicates an error while creating the array
      (-1).........no error, but an empty array
      otherwise....array containing the argument values

*******************************************************************************/

uint64_t *argument_vmarray_from_objectarray(methodinfo *m, java_handle_t *o,
											java_handle_objectarray_t *params)
{
	methoddesc    *md;
	paramdesc     *pd;
	typedesc      *td;
	uint64_t      *array;
	java_handle_t *param;
	classinfo     *c;
	int            type;
	int32_t        i;
	int32_t        j;
	imm_union      value;

	/* get the descriptors */

	md = m->parseddesc;
	pd = md->params;
	td = md->paramtypes;

	/* allocate argument array */

	array = (uint64_t*) DumpMemory::allocate(sizeof(uint64_t) * (INT_ARG_CNT + FLT_ARG_CNT + md->memuse));

	/* The array can be NULL if we don't have any arguments to pass
	   and the architecture does not have any argument registers
	   (e.g. i386).  In that case we return (-1) to indicate
	   that no exception should be thrown */

	if (array == NULL)
		array = (uint64_t *)(-1);

	/* if method is non-static fill first block and skip `this' pointer */

	i = 0;

	if (o != NULL) {
		/* this pointer */
		argument_vmarray_store_adr(array, pd, o);

		pd++;
		td++;
		i++;
	}

	ObjectArray oa(params);

	for (j = 0; i < md->paramcount; i++, j++, pd++, td++) {
		/* XXX This function can throw an exception, which should not happend
		   here, since we are outside the nativeworld. */
		param = oa.get_element(j);

		switch (td->type) {
		case TYPE_INT:
			if (param == NULL)
				return NULL;

			/* convert the value according to its declared type */

			LLNI_class_get(param, c);
			type = Primitive::get_type_by_wrapperclass(c);

			switch (td->primitivetype) {
			case PRIMITIVETYPE_BOOLEAN:
				switch (type) {
				case PRIMITIVETYPE_BOOLEAN:
					/* This type is OK. */
					break;
				default:
					return NULL;
				}
				break;

			case PRIMITIVETYPE_BYTE:
				switch (type) {
				case PRIMITIVETYPE_BYTE:
					/* This type is OK. */
					break;
				default:
					return NULL;
				}
				break;

			case PRIMITIVETYPE_CHAR:
				switch (type) {
				case PRIMITIVETYPE_CHAR:
					/* This type is OK. */
					break;
				default:
					return NULL;
				}
				break;

			case PRIMITIVETYPE_SHORT:
				switch (type) {
				case PRIMITIVETYPE_BYTE:
				case PRIMITIVETYPE_SHORT:
					/* These types are OK. */
					break;
				default:
					return NULL;
				}
				break;

			case PRIMITIVETYPE_INT:
				switch (type) {
				case PRIMITIVETYPE_BYTE:
				case PRIMITIVETYPE_SHORT:
				case PRIMITIVETYPE_INT:
					/* These types are OK. */
					break;
				default:
					return NULL;
				}
				break;

			default:
				os::abort("argument_vmarray_from_objectarray: invalid type %d",
						 td->primitivetype);
			}

			value = Primitive::unbox(param);
			argument_vmarray_store_int(array, pd, value.i);
			break;

		case TYPE_LNG:
			if (param == NULL)
				return NULL;

			LLNI_class_get(param, c);
			type = Primitive::get_type_by_wrapperclass(c);

			assert(td->primitivetype == PRIMITIVETYPE_LONG);

			switch (type) {
			case PRIMITIVETYPE_BYTE:
			case PRIMITIVETYPE_SHORT:
			case PRIMITIVETYPE_INT:
			case PRIMITIVETYPE_LONG:
				/* These types are OK. */
				break;
			default:
				return NULL;
			}

			value = Primitive::unbox(param);
			argument_vmarray_store_lng(array, pd, value.l);
			break;

		case TYPE_FLT:
			if (param == NULL)
				return NULL;

			LLNI_class_get(param, c);
			type = Primitive::get_type_by_wrapperclass(c);

			assert(td->primitivetype == PRIMITIVETYPE_FLOAT);

			switch (type) {
			case PRIMITIVETYPE_FLOAT:
				/* This type is OK. */
				break;
			default:
				return NULL;
			}

			value = Primitive::unbox(param);
			argument_vmarray_store_flt(array, pd, value.l);
			break;

		case TYPE_DBL:
			if (param == NULL)
				return NULL;

			LLNI_class_get(param, c);
			type = Primitive::get_type_by_wrapperclass(c);

			assert(td->primitivetype == PRIMITIVETYPE_DOUBLE);

			switch (type) {
			case PRIMITIVETYPE_FLOAT:
			case PRIMITIVETYPE_DOUBLE:
				/* These types are OK. */
				break;
			default:
				return NULL;
			}

			value = Primitive::unbox(param);
			argument_vmarray_store_dbl(array, pd, value.l);
			break;
		
		case TYPE_ADR:
			if (!resolve_class_from_typedesc(td, true, true, &c))
				return NULL;

			if (param != NULL) {
				if (td->arraydim > 0) {
					if (!builtin_arrayinstanceof(param, c))
						return NULL;
				}
				else {
					if (!builtin_instanceof(param, c))
						return NULL;
				}
			}

			argument_vmarray_store_adr(array, pd, param);
			break;

		default:
			os::abort("argument_vmarray_from_objectarray: invalid type %d", td->type);
		}
	}

	return array;
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
