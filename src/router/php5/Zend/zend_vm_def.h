/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2014 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Dmitry Stogov <dmitry@zend.com>                             |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* If you change this file, please regenerate the zend_vm_execute.h and
 * zend_vm_opcodes.h files by running:
 * php zend_vm_gen.php
 */

ZEND_VM_HANDLER(1, ZEND_ADD, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	fast_add_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(2, ZEND_SUB, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	fast_sub_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(3, ZEND_MUL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	fast_mul_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(4, ZEND_DIV, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	fast_div_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(5, ZEND_MOD, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	fast_mod_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(6, ZEND_SL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	shift_left_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(7, ZEND_SR, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	shift_right_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(8, ZEND_CONCAT, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	concat_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(15, ZEND_IS_IDENTICAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	is_identical_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(16, ZEND_IS_NOT_IDENTICAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *result = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	is_identical_function(result,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	Z_LVAL_P(result) = !Z_LVAL_P(result);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(17, ZEND_IS_EQUAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *result = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	ZVAL_BOOL(result, fast_equal_function(result,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC));
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(18, ZEND_IS_NOT_EQUAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *result = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	ZVAL_BOOL(result, fast_not_equal_function(result,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC));
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(19, ZEND_IS_SMALLER, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *result = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	ZVAL_BOOL(result, fast_is_smaller_function(result,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC));
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(20, ZEND_IS_SMALLER_OR_EQUAL, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *result = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	ZVAL_BOOL(result, fast_is_smaller_or_equal_function(result,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC));
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(9, ZEND_BW_OR, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	bitwise_or_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(10, ZEND_BW_AND, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	bitwise_and_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(11, ZEND_BW_XOR, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	bitwise_xor_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(14, ZEND_BOOL_XOR, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	boolean_xor_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R),
		GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(12, ZEND_BW_NOT, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;

	SAVE_OPLINE();
	bitwise_not_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(13, ZEND_BOOL_NOT, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;

	SAVE_OPLINE();
	boolean_not_function(&EX_T(opline->result.var).tmp_var,
		GET_OP1_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);
	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HELPER_EX(zend_binary_assign_op_obj_helper, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV, int (*binary_op)(zval *result, zval *op1, zval *op2 TSRMLS_DC))
{
	USE_OPLINE
	zend_free_op free_op1, free_op2, free_op_data1;
	zval **object_ptr = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_RW);
	zval *object;
	zval *property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	zval *value = get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data, &free_op_data1, BP_VAR_R);
	int have_get_ptr = 0;

	if (OP1_TYPE == IS_VAR && UNEXPECTED(object_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an object");
	}

	make_real_object(object_ptr TSRMLS_CC);
	object = *object_ptr;

	if (UNEXPECTED(Z_TYPE_P(object) != IS_OBJECT)) {
		zend_error(E_WARNING, "Attempt to assign property of non-object");
		FREE_OP2();
		FREE_OP(free_op_data1);

		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			EX_T(opline->result.var).var.ptr = &EG(uninitialized_zval);
			EX_T(opline->result.var).var.ptr_ptr = NULL;
		}
	} else {
		/* here we are sure we are dealing with an object */
		if (IS_OP2_TMP_FREE()) {
			MAKE_REAL_ZVAL_PTR(property);
		}

		/* here property is a string */
		if (opline->extended_value == ZEND_ASSIGN_OBJ
			&& Z_OBJ_HT_P(object)->get_property_ptr_ptr) {
			zval **zptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property, BP_VAR_RW, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
			if (zptr != NULL) { 			/* NULL means no success in getting PTR */
				SEPARATE_ZVAL_IF_NOT_REF(zptr);

				have_get_ptr = 1;
				binary_op(*zptr, *zptr, value TSRMLS_CC);
				if (RETURN_VALUE_USED(opline)) {
					PZVAL_LOCK(*zptr);
					EX_T(opline->result.var).var.ptr = *zptr;
					EX_T(opline->result.var).var.ptr_ptr = NULL;
				}
			}
		}

		if (!have_get_ptr) {
			zval *z = NULL;

			if (opline->extended_value == ZEND_ASSIGN_OBJ) {
				if (Z_OBJ_HT_P(object)->read_property) {
					z = Z_OBJ_HT_P(object)->read_property(object, property, BP_VAR_R, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
				}
			} else /* if (opline->extended_value == ZEND_ASSIGN_DIM) */ {
				if (Z_OBJ_HT_P(object)->read_dimension) {
					z = Z_OBJ_HT_P(object)->read_dimension(object, property, BP_VAR_R TSRMLS_CC);
				}
			}
			if (z) {
				if (Z_TYPE_P(z) == IS_OBJECT && Z_OBJ_HT_P(z)->get) {
					zval *value = Z_OBJ_HT_P(z)->get(z TSRMLS_CC);

					if (Z_REFCOUNT_P(z) == 0) {
						GC_REMOVE_ZVAL_FROM_BUFFER(z);
						zval_dtor(z);
						FREE_ZVAL(z);
					}
					z = value;
				}
				Z_ADDREF_P(z);
				SEPARATE_ZVAL_IF_NOT_REF(&z);
				binary_op(z, z, value TSRMLS_CC);
				if (opline->extended_value == ZEND_ASSIGN_OBJ) {
					Z_OBJ_HT_P(object)->write_property(object, property, z, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
				} else /* if (opline->extended_value == ZEND_ASSIGN_DIM) */ {
					Z_OBJ_HT_P(object)->write_dimension(object, property, z TSRMLS_CC);
				}
				if (RETURN_VALUE_USED(opline)) {
					PZVAL_LOCK(z);
					EX_T(opline->result.var).var.ptr = z;
					EX_T(opline->result.var).var.ptr_ptr = NULL;
				}
				zval_ptr_dtor(&z);
			} else {
				zend_error(E_WARNING, "Attempt to assign property of non-object");
				if (RETURN_VALUE_USED(opline)) {
					PZVAL_LOCK(&EG(uninitialized_zval));
					EX_T(opline->result.var).var.ptr = &EG(uninitialized_zval);
					EX_T(opline->result.var).var.ptr_ptr = NULL;
				}
			}
		}

		if (IS_OP2_TMP_FREE()) {
			zval_ptr_dtor(&property);
		} else {
			FREE_OP2();
		}
		FREE_OP(free_op_data1);
	}

	FREE_OP1_VAR_PTR();
	/* assign_obj has two opcodes! */
	CHECK_EXCEPTION();
	ZEND_VM_INC_OPCODE();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HELPER_EX(zend_binary_assign_op_helper, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV, int (*binary_op)(zval *result, zval *op1, zval *op2 TSRMLS_DC))
{
	USE_OPLINE
	zend_free_op free_op1, free_op2, free_op_data2, free_op_data1;
	zval **var_ptr;
	zval *value;

	SAVE_OPLINE();
	switch (opline->extended_value) {
		case ZEND_ASSIGN_OBJ:
			ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_obj_helper, binary_op, binary_op);
			break;
		case ZEND_ASSIGN_DIM: {
				zval **container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_RW);

				if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
					zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
				} else if (UNEXPECTED(Z_TYPE_PP(container) == IS_OBJECT)) {
					if (OP1_TYPE == IS_VAR && !OP1_FREE) {
						Z_ADDREF_PP(container);  /* undo the effect of get_obj_zval_ptr_ptr() */
					}
					ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_obj_helper, binary_op, binary_op);
				} else {
					zval *dim = GET_OP2_ZVAL_PTR(BP_VAR_R);

					zend_fetch_dimension_address(&EX_T((opline+1)->op2.var), container, dim, OP2_TYPE, BP_VAR_RW TSRMLS_CC);
					value = get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data, &free_op_data1, BP_VAR_R);
					var_ptr = _get_zval_ptr_ptr_var((opline+1)->op2.var, execute_data, &free_op_data2 TSRMLS_CC);
				}
			}
			break;
		default:
			value = GET_OP2_ZVAL_PTR(BP_VAR_R);
			var_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);
			/* do nothing */
			break;
	}

	if (UNEXPECTED(var_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use assign-op operators with overloaded objects nor string offsets");
	}

	if (UNEXPECTED(*var_ptr == &EG(error_zval))) {
		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		}
		FREE_OP2();
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		if (opline->extended_value == ZEND_ASSIGN_DIM) {
			ZEND_VM_INC_OPCODE();
		}
		ZEND_VM_NEXT_OPCODE();
	}

	SEPARATE_ZVAL_IF_NOT_REF(var_ptr);

	if (UNEXPECTED(Z_TYPE_PP(var_ptr) == IS_OBJECT)
	   && Z_OBJ_HANDLER_PP(var_ptr, get)
	   && Z_OBJ_HANDLER_PP(var_ptr, set)) {
		/* proxy object */
		zval *objval = Z_OBJ_HANDLER_PP(var_ptr, get)(*var_ptr TSRMLS_CC);
		Z_ADDREF_P(objval);
		binary_op(objval, objval, value TSRMLS_CC);
		Z_OBJ_HANDLER_PP(var_ptr, set)(var_ptr, objval TSRMLS_CC);
		zval_ptr_dtor(&objval);
	} else {
		binary_op(*var_ptr, *var_ptr, value TSRMLS_CC);
	}

	if (RETURN_VALUE_USED(opline)) {
		PZVAL_LOCK(*var_ptr);
		AI_SET_PTR(&EX_T(opline->result.var), *var_ptr);
	}
	FREE_OP2();

	if (opline->extended_value == ZEND_ASSIGN_DIM) {
		FREE_OP(free_op_data1);
		FREE_OP_VAR_PTR(free_op_data2);
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_INC_OPCODE();
	} else {
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
	}
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(23, ZEND_ASSIGN_ADD, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, add_function);
}

ZEND_VM_HANDLER(24, ZEND_ASSIGN_SUB, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, sub_function);
}

ZEND_VM_HANDLER(25, ZEND_ASSIGN_MUL, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, mul_function);
}

ZEND_VM_HANDLER(26, ZEND_ASSIGN_DIV, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, div_function);
}

ZEND_VM_HANDLER(27, ZEND_ASSIGN_MOD, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, mod_function);
}

ZEND_VM_HANDLER(28, ZEND_ASSIGN_SL, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, shift_left_function);
}

ZEND_VM_HANDLER(29, ZEND_ASSIGN_SR, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, shift_right_function);
}

ZEND_VM_HANDLER(30, ZEND_ASSIGN_CONCAT, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, concat_function);
}

ZEND_VM_HANDLER(31, ZEND_ASSIGN_BW_OR, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, bitwise_or_function);
}

ZEND_VM_HANDLER(32, ZEND_ASSIGN_BW_AND, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, bitwise_and_function);
}

ZEND_VM_HANDLER(33, ZEND_ASSIGN_BW_XOR, VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_binary_assign_op_helper, binary_op, bitwise_xor_function);
}

ZEND_VM_HELPER_EX(zend_pre_incdec_property_helper, VAR|UNUSED|CV, CONST|TMP|VAR|CV, incdec_t incdec_op)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **object_ptr;
	zval *object;
	zval *property;
	zval **retval;
	int have_get_ptr = 0;

	SAVE_OPLINE();
	object_ptr = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_RW);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	retval = &EX_T(opline->result.var).var.ptr;

	if (OP1_TYPE == IS_VAR && UNEXPECTED(object_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot increment/decrement overloaded objects nor string offsets");
	}

	make_real_object(object_ptr TSRMLS_CC); /* this should modify object only if it's empty */
	object = *object_ptr;

	if (UNEXPECTED(Z_TYPE_P(object) != IS_OBJECT)) {
		zend_error(E_WARNING, "Attempt to increment/decrement property of non-object");
		FREE_OP2();
		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			*retval = &EG(uninitialized_zval);
		}
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	/* here we are sure we are dealing with an object */

	if (IS_OP2_TMP_FREE()) {
		MAKE_REAL_ZVAL_PTR(property);
	}

	if (Z_OBJ_HT_P(object)->get_property_ptr_ptr) {
		zval **zptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property, BP_VAR_RW, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
		if (zptr != NULL) { 			/* NULL means no success in getting PTR */
			SEPARATE_ZVAL_IF_NOT_REF(zptr);

			have_get_ptr = 1;
			incdec_op(*zptr);
			if (RETURN_VALUE_USED(opline)) {
				*retval = *zptr;
				PZVAL_LOCK(*retval);
			}
		}
	}

	if (!have_get_ptr) {
		if (Z_OBJ_HT_P(object)->read_property && Z_OBJ_HT_P(object)->write_property) {
			zval *z = Z_OBJ_HT_P(object)->read_property(object, property, BP_VAR_R, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);

			if (UNEXPECTED(Z_TYPE_P(z) == IS_OBJECT) && Z_OBJ_HT_P(z)->get) {
				zval *value = Z_OBJ_HT_P(z)->get(z TSRMLS_CC);

				if (Z_REFCOUNT_P(z) == 0) {
					GC_REMOVE_ZVAL_FROM_BUFFER(z);
					zval_dtor(z);
					FREE_ZVAL(z);
				}
				z = value;
			}
			Z_ADDREF_P(z);
			SEPARATE_ZVAL_IF_NOT_REF(&z);
			incdec_op(z);
			*retval = z;
			Z_OBJ_HT_P(object)->write_property(object, property, z, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
			SELECTIVE_PZVAL_LOCK(*retval, opline);
			zval_ptr_dtor(&z);
		} else {
			zend_error(E_WARNING, "Attempt to increment/decrement property of non-object");
			if (RETURN_VALUE_USED(opline)) {
				PZVAL_LOCK(&EG(uninitialized_zval));
				*retval = &EG(uninitialized_zval);
			}
		}
	}

	if (IS_OP2_TMP_FREE()) {
		zval_ptr_dtor(&property);
	} else {
		FREE_OP2();
	}
	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(132, ZEND_PRE_INC_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_pre_incdec_property_helper, incdec_op, increment_function);
}

ZEND_VM_HANDLER(133, ZEND_PRE_DEC_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_pre_incdec_property_helper, incdec_op, decrement_function);
}

ZEND_VM_HELPER_EX(zend_post_incdec_property_helper, VAR|UNUSED|CV, CONST|TMP|VAR|CV, incdec_t incdec_op)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **object_ptr;
	zval *object;
	zval *property;
	zval *retval;
	int have_get_ptr = 0;

	SAVE_OPLINE();
	object_ptr = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_RW);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	retval = &EX_T(opline->result.var).tmp_var;

	if (OP1_TYPE == IS_VAR && UNEXPECTED(object_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot increment/decrement overloaded objects nor string offsets");
	}

	make_real_object(object_ptr TSRMLS_CC); /* this should modify object only if it's empty */
	object = *object_ptr;

	if (UNEXPECTED(Z_TYPE_P(object) != IS_OBJECT)) {
		zend_error(E_WARNING, "Attempt to increment/decrement property of non-object");
		FREE_OP2();
		ZVAL_NULL(retval);
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	/* here we are sure we are dealing with an object */

	if (IS_OP2_TMP_FREE()) {
		MAKE_REAL_ZVAL_PTR(property);
	}

	if (Z_OBJ_HT_P(object)->get_property_ptr_ptr) {
		zval **zptr = Z_OBJ_HT_P(object)->get_property_ptr_ptr(object, property, BP_VAR_RW, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
		if (zptr != NULL) { 			/* NULL means no success in getting PTR */
			have_get_ptr = 1;
			SEPARATE_ZVAL_IF_NOT_REF(zptr);

			ZVAL_COPY_VALUE(retval, *zptr);
			zendi_zval_copy_ctor(*retval);

			incdec_op(*zptr);

		}
	}

	if (!have_get_ptr) {
		if (Z_OBJ_HT_P(object)->read_property && Z_OBJ_HT_P(object)->write_property) {
			zval *z = Z_OBJ_HT_P(object)->read_property(object, property, BP_VAR_R, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
			zval *z_copy;

			if (UNEXPECTED(Z_TYPE_P(z) == IS_OBJECT) && Z_OBJ_HT_P(z)->get) {
				zval *value = Z_OBJ_HT_P(z)->get(z TSRMLS_CC);

				if (Z_REFCOUNT_P(z) == 0) {
					GC_REMOVE_ZVAL_FROM_BUFFER(z);
					zval_dtor(z);
					FREE_ZVAL(z);
				}
				z = value;
			}
			ZVAL_COPY_VALUE(retval, z);
			zendi_zval_copy_ctor(*retval);
			ALLOC_ZVAL(z_copy);
			INIT_PZVAL_COPY(z_copy, z);
			zendi_zval_copy_ctor(*z_copy);
			incdec_op(z_copy);
			Z_ADDREF_P(z);
			Z_OBJ_HT_P(object)->write_property(object, property, z_copy, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
			zval_ptr_dtor(&z_copy);
			zval_ptr_dtor(&z);
		} else {
			zend_error(E_WARNING, "Attempt to increment/decrement property of non-object");
			ZVAL_NULL(retval);
		}
	}

	if (IS_OP2_TMP_FREE()) {
		zval_ptr_dtor(&property);
	} else {
		FREE_OP2();
	}
	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(134, ZEND_POST_INC_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_post_incdec_property_helper, incdec_op, increment_function);
}

ZEND_VM_HANDLER(135, ZEND_POST_DEC_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_post_incdec_property_helper, incdec_op, decrement_function);
}

ZEND_VM_HANDLER(34, ZEND_PRE_INC, VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval **var_ptr;

	SAVE_OPLINE();
	var_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(var_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot increment/decrement overloaded objects nor string offsets");
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(*var_ptr == &EG(error_zval))) {
		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		}
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	SEPARATE_ZVAL_IF_NOT_REF(var_ptr);

	if (UNEXPECTED(Z_TYPE_PP(var_ptr) == IS_OBJECT)
	   && Z_OBJ_HANDLER_PP(var_ptr, get)
	   && Z_OBJ_HANDLER_PP(var_ptr, set)) {
		/* proxy object */
		zval *val = Z_OBJ_HANDLER_PP(var_ptr, get)(*var_ptr TSRMLS_CC);
		Z_ADDREF_P(val);
		fast_increment_function(val);
		Z_OBJ_HANDLER_PP(var_ptr, set)(var_ptr, val TSRMLS_CC);
		zval_ptr_dtor(&val);
	} else {
		fast_increment_function(*var_ptr);
	}

	if (RETURN_VALUE_USED(opline)) {
		PZVAL_LOCK(*var_ptr);
		AI_SET_PTR(&EX_T(opline->result.var), *var_ptr);
	}

	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(35, ZEND_PRE_DEC, VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval **var_ptr;

	SAVE_OPLINE();
	var_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(var_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot increment/decrement overloaded objects nor string offsets");
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(*var_ptr == &EG(error_zval))) {
		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		}
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	SEPARATE_ZVAL_IF_NOT_REF(var_ptr);

	if (UNEXPECTED(Z_TYPE_PP(var_ptr) == IS_OBJECT)
	   && Z_OBJ_HANDLER_PP(var_ptr, get)
	   && Z_OBJ_HANDLER_PP(var_ptr, set)) {
		/* proxy object */
		zval *val = Z_OBJ_HANDLER_PP(var_ptr, get)(*var_ptr TSRMLS_CC);
		Z_ADDREF_P(val);
		fast_decrement_function(val);
		Z_OBJ_HANDLER_PP(var_ptr, set)(var_ptr, val TSRMLS_CC);
		zval_ptr_dtor(&val);
	} else {
		fast_decrement_function(*var_ptr);
	}

	if (RETURN_VALUE_USED(opline)) {
		PZVAL_LOCK(*var_ptr);
		AI_SET_PTR(&EX_T(opline->result.var), *var_ptr);
	}

	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(36, ZEND_POST_INC, VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval **var_ptr, *retval;

	SAVE_OPLINE();
	var_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(var_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot increment/decrement overloaded objects nor string offsets");
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(*var_ptr == &EG(error_zval))) {
		ZVAL_NULL(&EX_T(opline->result.var).tmp_var);
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	retval = &EX_T(opline->result.var).tmp_var;
	ZVAL_COPY_VALUE(retval, *var_ptr);
	zendi_zval_copy_ctor(*retval);

	SEPARATE_ZVAL_IF_NOT_REF(var_ptr);

	if (UNEXPECTED(Z_TYPE_PP(var_ptr) == IS_OBJECT)
	   && Z_OBJ_HANDLER_PP(var_ptr, get)
	   && Z_OBJ_HANDLER_PP(var_ptr, set)) {
		/* proxy object */
		zval *val = Z_OBJ_HANDLER_PP(var_ptr, get)(*var_ptr TSRMLS_CC);
		Z_ADDREF_P(val);
		fast_increment_function(val);
		Z_OBJ_HANDLER_PP(var_ptr, set)(var_ptr, val TSRMLS_CC);
		zval_ptr_dtor(&val);
	} else {
		fast_increment_function(*var_ptr);
	}

	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(37, ZEND_POST_DEC, VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval **var_ptr, *retval;

	SAVE_OPLINE();
	var_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(var_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot increment/decrement overloaded objects nor string offsets");
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(*var_ptr == &EG(error_zval))) {
		ZVAL_NULL(&EX_T(opline->result.var).tmp_var);
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	retval = &EX_T(opline->result.var).tmp_var;
	ZVAL_COPY_VALUE(retval, *var_ptr);
	zendi_zval_copy_ctor(*retval);

	SEPARATE_ZVAL_IF_NOT_REF(var_ptr);

	if (UNEXPECTED(Z_TYPE_PP(var_ptr) == IS_OBJECT)
	   && Z_OBJ_HANDLER_PP(var_ptr, get)
	   && Z_OBJ_HANDLER_PP(var_ptr, set)) {
		/* proxy object */
		zval *val = Z_OBJ_HANDLER_PP(var_ptr, get)(*var_ptr TSRMLS_CC);
		Z_ADDREF_P(val);
		fast_decrement_function(val);
		Z_OBJ_HANDLER_PP(var_ptr, set)(var_ptr, val TSRMLS_CC);
		zval_ptr_dtor(&val);
	} else {
		fast_decrement_function(*var_ptr);
	}

	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(40, ZEND_ECHO, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *z;

	SAVE_OPLINE();
	z = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_TMP_VAR && Z_TYPE_P(z) == IS_OBJECT) {
		INIT_PZVAL(z);
	}
	zend_print_variable(z);

	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(41, ZEND_PRINT, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE

	ZVAL_LONG(&EX_T(opline->result.var).tmp_var, 1);
	ZEND_VM_DISPATCH_TO_HANDLER(ZEND_ECHO);
}

ZEND_VM_HELPER_EX(zend_fetch_var_address_helper, CONST|TMP|VAR|CV, UNUSED|CONST|VAR, int type)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *varname;
	zval **retval;
	zval tmp_varname;
	HashTable *target_symbol_table;
	ulong hash_value;

	SAVE_OPLINE();
	varname = GET_OP1_ZVAL_PTR(BP_VAR_R);

 	if (OP1_TYPE != IS_CONST && UNEXPECTED(Z_TYPE_P(varname) != IS_STRING)) {
		ZVAL_COPY_VALUE(&tmp_varname, varname);
		zval_copy_ctor(&tmp_varname);
		Z_SET_REFCOUNT(tmp_varname, 1);
		Z_UNSET_ISREF(tmp_varname);
		convert_to_string(&tmp_varname);
		varname = &tmp_varname;
	}

	if (OP2_TYPE != IS_UNUSED) {
		zend_class_entry *ce;

		if (OP2_TYPE == IS_CONST) {
			if (CACHED_PTR(opline->op2.literal->cache_slot)) {
				ce = CACHED_PTR(opline->op2.literal->cache_slot);
			} else {
				ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv), opline->op2.literal + 1, 0 TSRMLS_CC);
				if (UNEXPECTED(ce == NULL)) {
					if (OP1_TYPE != IS_CONST && varname == &tmp_varname) {
						zval_dtor(&tmp_varname);
					}
					FREE_OP1();
					CHECK_EXCEPTION();
					ZEND_VM_NEXT_OPCODE();
				}
				CACHE_PTR(opline->op2.literal->cache_slot, ce);
			}
		} else {
			ce = EX_T(opline->op2.var).class_entry;
		}
		retval = zend_std_get_static_property(ce, Z_STRVAL_P(varname), Z_STRLEN_P(varname), 0, ((OP1_TYPE == IS_CONST) ? opline->op1.literal : NULL) TSRMLS_CC);
		FREE_OP1();
	} else {
		target_symbol_table = zend_get_target_symbol_table(opline->extended_value & ZEND_FETCH_TYPE_MASK TSRMLS_CC);
/*
		if (!target_symbol_table) {
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE();
		}
*/
		if (OP1_TYPE == IS_CONST) {
			hash_value = Z_HASH_P(varname);
		} else if (IS_INTERNED(Z_STRVAL_P(varname))) {
			hash_value = INTERNED_HASH(Z_STRVAL_P(varname));
		} else {
			hash_value = zend_hash_func(Z_STRVAL_P(varname), Z_STRLEN_P(varname)+1);
		}

		if (zend_hash_quick_find(target_symbol_table, Z_STRVAL_P(varname), Z_STRLEN_P(varname)+1, hash_value, (void **) &retval) == FAILURE) {
			switch (type) {
				case BP_VAR_R:
				case BP_VAR_UNSET:
					zend_error(E_NOTICE,"Undefined variable: %s", Z_STRVAL_P(varname));
					/* break missing intentionally */
				case BP_VAR_IS:
					retval = &EG(uninitialized_zval_ptr);
					break;
				case BP_VAR_RW:
					zend_error(E_NOTICE,"Undefined variable: %s", Z_STRVAL_P(varname));
					/* break missing intentionally */
				case BP_VAR_W:
					Z_ADDREF_P(&EG(uninitialized_zval));
					zend_hash_quick_update(target_symbol_table, Z_STRVAL_P(varname), Z_STRLEN_P(varname)+1, hash_value, &EG(uninitialized_zval_ptr), sizeof(zval *), (void **) &retval);
					break;
				EMPTY_SWITCH_DEFAULT_CASE()
			}
		}
		switch (opline->extended_value & ZEND_FETCH_TYPE_MASK) {
			case ZEND_FETCH_GLOBAL:
				if (OP1_TYPE != IS_TMP_VAR) {
					FREE_OP1();
				}
				break;
			case ZEND_FETCH_LOCAL:
				FREE_OP1();
				break;
			case ZEND_FETCH_STATIC:
				zval_update_constant(retval, (void*) 1 TSRMLS_CC);
				break;
			case ZEND_FETCH_GLOBAL_LOCK:
				if (OP1_TYPE == IS_VAR && !free_op1.var) {
					PZVAL_LOCK(*EX_T(opline->op1.var).var.ptr_ptr);
				}
				break;
		}
	}


	if (OP1_TYPE != IS_CONST && varname == &tmp_varname) {
		zval_dtor(&tmp_varname);
	}
	if (opline->extended_value & ZEND_FETCH_MAKE_REF) {
		SEPARATE_ZVAL_TO_MAKE_IS_REF(retval);
	}
	PZVAL_LOCK(*retval);
	switch (type) {
		case BP_VAR_R:
		case BP_VAR_IS:
			AI_SET_PTR(&EX_T(opline->result.var), *retval);
			break;
		case BP_VAR_UNSET: {
			zend_free_op free_res;

			PZVAL_UNLOCK(*retval, &free_res);
			if (retval != &EG(uninitialized_zval_ptr)) {
				SEPARATE_ZVAL_IF_NOT_REF(retval);
			}
			PZVAL_LOCK(*retval);
			FREE_OP_VAR_PTR(free_res);
		}
		/* break missing intentionally */
		default:
			EX_T(opline->result.var).var.ptr_ptr = retval;
			break;
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(80, ZEND_FETCH_R, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_fetch_var_address_helper, type, BP_VAR_R);
}

ZEND_VM_HANDLER(83, ZEND_FETCH_W, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_fetch_var_address_helper, type, BP_VAR_W);
}

ZEND_VM_HANDLER(86, ZEND_FETCH_RW, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_fetch_var_address_helper, type, BP_VAR_RW);
}

ZEND_VM_HANDLER(92, ZEND_FETCH_FUNC_ARG, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	USE_OPLINE

	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_fetch_var_address_helper, type,
		ARG_SHOULD_BE_SENT_BY_REF(EX(call)->fbc, (opline->extended_value & ZEND_FETCH_ARG_MASK))?BP_VAR_W:BP_VAR_R);
}

ZEND_VM_HANDLER(95, ZEND_FETCH_UNSET, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_fetch_var_address_helper, type, BP_VAR_UNSET);
}

ZEND_VM_HANDLER(89, ZEND_FETCH_IS, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_fetch_var_address_helper, type, BP_VAR_IS);
}

ZEND_VM_HANDLER(81, ZEND_FETCH_DIM_R, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *container;

	SAVE_OPLINE();

	if (OP1_TYPE == IS_VAR && (opline->extended_value & ZEND_FETCH_ADD_LOCK)) {
		PZVAL_LOCK(EX_T(opline->op1.var).var.ptr);
	}
	container = GET_OP1_ZVAL_PTR(BP_VAR_R);
	zend_fetch_dimension_address_read(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_R TSRMLS_CC);
	FREE_OP2();
	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(84, ZEND_FETCH_DIM_W, VAR|CV, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
	}
	zend_fetch_dimension_address(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_W TSRMLS_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
		EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
	}
	FREE_OP1_VAR_PTR();

	/* We are going to assign the result by reference */
	if (UNEXPECTED(opline->extended_value != 0)) {
		zval **retval_ptr = EX_T(opline->result.var).var.ptr_ptr;

		if (retval_ptr) {
			Z_DELREF_PP(retval_ptr);
			SEPARATE_ZVAL_TO_MAKE_IS_REF(retval_ptr);
			Z_ADDREF_PP(retval_ptr);
		}
	}

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(87, ZEND_FETCH_DIM_RW, VAR|CV, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR(BP_VAR_RW);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
	}
	zend_fetch_dimension_address(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_RW TSRMLS_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
		EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
	}
	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(90, ZEND_FETCH_DIM_IS, VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR(BP_VAR_IS);
	zend_fetch_dimension_address_read(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_IS TSRMLS_CC);
	FREE_OP2();
	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(93, ZEND_FETCH_DIM_FUNC_ARG, VAR|CV, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();

	if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->fbc, (opline->extended_value & ZEND_FETCH_ARG_MASK))) {
		zval **container = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

		if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
			zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
		}
		zend_fetch_dimension_address(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_W TSRMLS_CC);
		if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
			EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
		}
		FREE_OP2();
		FREE_OP1_VAR_PTR();
	} else {
		zval *container;

		if (OP2_TYPE == IS_UNUSED) {
			zend_error_noreturn(E_ERROR, "Cannot use [] for reading");
		}
		container = GET_OP1_ZVAL_PTR(BP_VAR_R);
		zend_fetch_dimension_address_read(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_R TSRMLS_CC);
		FREE_OP2();
		FREE_OP1();
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(96, ZEND_FETCH_DIM_UNSET, VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR_PTR(BP_VAR_UNSET);

	if (OP1_TYPE == IS_CV) {
		if (container != &EG(uninitialized_zval_ptr)) {
			SEPARATE_ZVAL_IF_NOT_REF(container);
		}
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
	}
	zend_fetch_dimension_address(&EX_T(opline->result.var), container, GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_UNSET TSRMLS_CC);
	FREE_OP2();
	if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
		EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
	}
	FREE_OP1_VAR_PTR();
	if (UNEXPECTED(EX_T(opline->result.var).var.ptr_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot unset string offsets");
		ZEND_VM_NEXT_OPCODE();
	} else {
		zend_free_op free_res;
		zval **retval_ptr = EX_T(opline->result.var).var.ptr_ptr;

		PZVAL_UNLOCK(*retval_ptr, &free_res);
		if (retval_ptr != &EG(uninitialized_zval_ptr)) {
			SEPARATE_ZVAL_IF_NOT_REF(retval_ptr);
		}
		PZVAL_LOCK(*retval_ptr);
		FREE_OP_VAR_PTR(free_res);
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}
}

ZEND_VM_HELPER(zend_fetch_property_address_read_helper, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *container;
	zend_free_op free_op2;
	zval *offset;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_R);
	offset  = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(Z_TYPE_P(container) != IS_OBJECT) ||
	    UNEXPECTED(Z_OBJ_HT_P(container)->read_property == NULL)) {
		zend_error(E_NOTICE, "Trying to get property of non-object");
		PZVAL_LOCK(&EG(uninitialized_zval));
		AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		FREE_OP2();
	} else {
		zval *retval;

		if (IS_OP2_TMP_FREE()) {
			MAKE_REAL_ZVAL_PTR(offset);
		}

		/* here we are sure we are dealing with an object */
		retval = Z_OBJ_HT_P(container)->read_property(container, offset, BP_VAR_R, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);

		PZVAL_LOCK(retval);
		AI_SET_PTR(&EX_T(opline->result.var), retval);

		if (IS_OP2_TMP_FREE()) {
			zval_ptr_dtor(&offset);
		} else {
			FREE_OP2();
		}
	}

	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(82, ZEND_FETCH_OBJ_R, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER(zend_fetch_property_address_read_helper);
}

ZEND_VM_HANDLER(85, ZEND_FETCH_OBJ_W, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *property;
	zval **container;

	SAVE_OPLINE();
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (IS_OP2_TMP_FREE()) {
		MAKE_REAL_ZVAL_PTR(property);
	}
	container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_W);
	if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an object");
	}

	zend_fetch_property_address(&EX_T(opline->result.var), container, property, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL), BP_VAR_W TSRMLS_CC);
	if (IS_OP2_TMP_FREE()) {
		zval_ptr_dtor(&property);
	} else {
		FREE_OP2();
	}
	if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
		EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
	}
	FREE_OP1_VAR_PTR();

	/* We are going to assign the result by reference */
	if (opline->extended_value & ZEND_FETCH_MAKE_REF) {
		zval **retval_ptr = EX_T(opline->result.var).var.ptr_ptr;

		Z_DELREF_PP(retval_ptr);
		SEPARATE_ZVAL_TO_MAKE_IS_REF(retval_ptr);
		Z_ADDREF_PP(retval_ptr);
		EX_T(opline->result.var).var.ptr = *EX_T(opline->result.var).var.ptr_ptr;
		EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
	}

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(88, ZEND_FETCH_OBJ_RW, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *property;
	zval **container;

	SAVE_OPLINE();
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);
	container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_RW);

	if (IS_OP2_TMP_FREE()) {
		MAKE_REAL_ZVAL_PTR(property);
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an object");
	}
	zend_fetch_property_address(&EX_T(opline->result.var), container, property, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL), BP_VAR_RW TSRMLS_CC);
	if (IS_OP2_TMP_FREE()) {
		zval_ptr_dtor(&property);
	} else {
		FREE_OP2();
	}
	if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
		EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
	}
	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(91, ZEND_FETCH_OBJ_IS, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *container;
	zend_free_op free_op2;
	zval *offset;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_IS);
	offset  = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(Z_TYPE_P(container) != IS_OBJECT) ||
	    UNEXPECTED(Z_OBJ_HT_P(container)->read_property == NULL)) {
		PZVAL_LOCK(&EG(uninitialized_zval));
		AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		FREE_OP2();
	} else {
		zval *retval;

		if (IS_OP2_TMP_FREE()) {
			MAKE_REAL_ZVAL_PTR(offset);
		}

		/* here we are sure we are dealing with an object */
		retval = Z_OBJ_HT_P(container)->read_property(container, offset, BP_VAR_IS, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);

		PZVAL_LOCK(retval);
		AI_SET_PTR(&EX_T(opline->result.var), retval);

		if (IS_OP2_TMP_FREE()) {
			zval_ptr_dtor(&offset);
		} else {
			FREE_OP2();
		}
	}

	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(94, ZEND_FETCH_OBJ_FUNC_ARG, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE

	if (ARG_SHOULD_BE_SENT_BY_REF(EX(call)->fbc, (opline->extended_value & ZEND_FETCH_ARG_MASK))) {
		/* Behave like FETCH_OBJ_W */
		zend_free_op free_op1, free_op2;
		zval *property;
		zval **container;

		SAVE_OPLINE();
		property = GET_OP2_ZVAL_PTR(BP_VAR_R);
		container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_W);

		if (IS_OP2_TMP_FREE()) {
			MAKE_REAL_ZVAL_PTR(property);
		}
		if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
			zend_error_noreturn(E_ERROR, "Cannot use string offset as an object");
		}
		zend_fetch_property_address(&EX_T(opline->result.var), container, property, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL), BP_VAR_W TSRMLS_CC);
		if (IS_OP2_TMP_FREE()) {
			zval_ptr_dtor(&property);
		} else {
			FREE_OP2();
		}
		if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
			EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
		}
		FREE_OP1_VAR_PTR();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	} else {
		ZEND_VM_DISPATCH_TO_HELPER(zend_fetch_property_address_read_helper);
	}
}

ZEND_VM_HANDLER(97, ZEND_FETCH_OBJ_UNSET, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2, free_res;
	zval **container;
	zval *property;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_UNSET);
	property = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_CV) {
		if (container != &EG(uninitialized_zval_ptr)) {
			SEPARATE_ZVAL_IF_NOT_REF(container);
		}
	}
	if (IS_OP2_TMP_FREE()) {
		MAKE_REAL_ZVAL_PTR(property);
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(container == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an object");
	}
	zend_fetch_property_address(&EX_T(opline->result.var), container, property, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL), BP_VAR_UNSET TSRMLS_CC);
	if (IS_OP2_TMP_FREE()) {
		zval_ptr_dtor(&property);
	} else {
		FREE_OP2();
	}
	if (OP1_TYPE == IS_VAR && OP1_FREE && READY_TO_DESTROY(free_op1.var)) {
		EXTRACT_ZVAL_PTR(&EX_T(opline->result.var));
	}
	FREE_OP1_VAR_PTR();

	PZVAL_UNLOCK(*EX_T(opline->result.var).var.ptr_ptr, &free_res);
	if (EX_T(opline->result.var).var.ptr_ptr != &EG(uninitialized_zval_ptr)) {
		SEPARATE_ZVAL_IF_NOT_REF(EX_T(opline->result.var).var.ptr_ptr);
	}
	PZVAL_LOCK(*EX_T(opline->result.var).var.ptr_ptr);
	FREE_OP_VAR_PTR(free_res);
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(98, ZEND_FETCH_DIM_TMP_VAR, CONST|TMP, CONST)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *container;

	SAVE_OPLINE();
	container = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(Z_TYPE_P(container) != IS_ARRAY)) {
		PZVAL_LOCK(&EG(uninitialized_zval));
		AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
	} else {
		zend_free_op free_op2;
		zval *value = *zend_fetch_dimension_address_inner(Z_ARRVAL_P(container), GET_OP2_ZVAL_PTR(BP_VAR_R), OP2_TYPE, BP_VAR_R TSRMLS_CC);

		PZVAL_LOCK(value);
		AI_SET_PTR(&EX_T(opline->result.var), value);
		FREE_OP2();
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(136, ZEND_ASSIGN_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **object_ptr;
	zval *property_name;

	SAVE_OPLINE();
	object_ptr = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_W);
	property_name = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (IS_OP2_TMP_FREE()) {
		MAKE_REAL_ZVAL_PTR(property_name);
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(object_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
	}
	zend_assign_to_object(RETURN_VALUE_USED(opline)?&EX_T(opline->result.var).var.ptr:NULL, object_ptr, property_name, (opline+1)->op1_type, &(opline+1)->op1, execute_data, ZEND_ASSIGN_OBJ, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
	if (IS_OP2_TMP_FREE()) {
		zval_ptr_dtor(&property_name);
	} else {
		FREE_OP2();
	}
	FREE_OP1_VAR_PTR();
	/* assign_obj has two opcodes! */
	CHECK_EXCEPTION();
	ZEND_VM_INC_OPCODE();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(147, ZEND_ASSIGN_DIM, VAR|CV, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval **object_ptr;

	SAVE_OPLINE();
	object_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(object_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Cannot use string offset as an array");
	}
	if (Z_TYPE_PP(object_ptr) == IS_OBJECT) {
		zend_free_op free_op2;
		zval *property_name = GET_OP2_ZVAL_PTR(BP_VAR_R);

		if (IS_OP2_TMP_FREE()) {
			MAKE_REAL_ZVAL_PTR(property_name);
		}
		zend_assign_to_object(RETURN_VALUE_USED(opline)?&EX_T(opline->result.var).var.ptr:NULL, object_ptr, property_name, (opline+1)->op1_type, &(opline+1)->op1, execute_data, ZEND_ASSIGN_DIM, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
		if (IS_OP2_TMP_FREE()) {
			zval_ptr_dtor(&property_name);
		} else {
			FREE_OP2();
		}
	} else {
		zend_free_op free_op2, free_op_data1, free_op_data2;
		zval *value;
		zval *dim = GET_OP2_ZVAL_PTR(BP_VAR_R);
		zval **variable_ptr_ptr;

		zend_fetch_dimension_address(&EX_T((opline+1)->op2.var), object_ptr, dim, OP2_TYPE, BP_VAR_W TSRMLS_CC);
		FREE_OP2();

		value = get_zval_ptr((opline+1)->op1_type, &(opline+1)->op1, execute_data, &free_op_data1, BP_VAR_R);
		variable_ptr_ptr = _get_zval_ptr_ptr_var((opline+1)->op2.var, execute_data, &free_op_data2 TSRMLS_CC);
		if (UNEXPECTED(variable_ptr_ptr == NULL)) {
			if (zend_assign_to_string_offset(&EX_T((opline+1)->op2.var), value, (opline+1)->op1_type TSRMLS_CC)) {
				if (RETURN_VALUE_USED(opline)) {
					zval *retval;

					ALLOC_ZVAL(retval);
					ZVAL_STRINGL(retval, Z_STRVAL_P(EX_T((opline+1)->op2.var).str_offset.str)+EX_T((opline+1)->op2.var).str_offset.offset, 1, 1);
					INIT_PZVAL(retval);
					AI_SET_PTR(&EX_T(opline->result.var), retval);
				}
			} else if (RETURN_VALUE_USED(opline)) {
				PZVAL_LOCK(&EG(uninitialized_zval));
				AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
			}
		} else if (UNEXPECTED(*variable_ptr_ptr == &EG(error_zval))) {
			if (IS_TMP_FREE(free_op_data1)) {
				zval_dtor(value);
			}
			if (RETURN_VALUE_USED(opline)) {
				PZVAL_LOCK(&EG(uninitialized_zval));
				AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
			}
		} else {
			if ((opline+1)->op1_type == IS_TMP_VAR) {
			 	value = zend_assign_tmp_to_variable(variable_ptr_ptr, value TSRMLS_CC);
			} else if ((opline+1)->op1_type == IS_CONST) {
			 	value = zend_assign_const_to_variable(variable_ptr_ptr, value TSRMLS_CC);
			} else {
			 	value = zend_assign_to_variable(variable_ptr_ptr, value TSRMLS_CC);
			}
			if (RETURN_VALUE_USED(opline)) {
				PZVAL_LOCK(value);
				AI_SET_PTR(&EX_T(opline->result.var), value);
			}
		}
		FREE_OP_VAR_PTR(free_op_data2);
	 	FREE_OP_IF_VAR(free_op_data1);
	}
 	FREE_OP1_VAR_PTR();
	/* assign_dim has two opcodes! */
	CHECK_EXCEPTION();
	ZEND_VM_INC_OPCODE();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(38, ZEND_ASSIGN, VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *value;
	zval **variable_ptr_ptr;

	SAVE_OPLINE();
	value = GET_OP2_ZVAL_PTR(BP_VAR_R);
	variable_ptr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(variable_ptr_ptr == NULL)) {
		if (zend_assign_to_string_offset(&EX_T(opline->op1.var), value, OP2_TYPE TSRMLS_CC)) {
			if (RETURN_VALUE_USED(opline)) {
				zval *retval;

				ALLOC_ZVAL(retval);
				ZVAL_STRINGL(retval, Z_STRVAL_P(EX_T(opline->op1.var).str_offset.str)+EX_T(opline->op1.var).str_offset.offset, 1, 1);
				INIT_PZVAL(retval);
				AI_SET_PTR(&EX_T(opline->result.var), retval);
			}
		} else if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		}
	} else if (OP1_TYPE == IS_VAR && UNEXPECTED(*variable_ptr_ptr == &EG(error_zval))) {
		if (IS_OP2_TMP_FREE()) {
			zval_dtor(value);
		}
		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(&EG(uninitialized_zval));
			AI_SET_PTR(&EX_T(opline->result.var), &EG(uninitialized_zval));
		}
	} else {
		if (OP2_TYPE == IS_TMP_VAR) {
		 	value = zend_assign_tmp_to_variable(variable_ptr_ptr, value TSRMLS_CC);
		} else if (OP2_TYPE == IS_CONST) {
		 	value = zend_assign_const_to_variable(variable_ptr_ptr, value TSRMLS_CC);
		} else {
		 	value = zend_assign_to_variable(variable_ptr_ptr, value TSRMLS_CC);
		}
		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(value);
			AI_SET_PTR(&EX_T(opline->result.var), value);
		}
	}

	FREE_OP1_VAR_PTR();

	/* zend_assign_to_variable() always takes care of op2, never free it! */
 	FREE_OP2_IF_VAR();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(39, ZEND_ASSIGN_REF, VAR|CV, VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **variable_ptr_ptr;
	zval **value_ptr_ptr;

	SAVE_OPLINE();
	value_ptr_ptr = GET_OP2_ZVAL_PTR_PTR(BP_VAR_W);

	if (OP2_TYPE == IS_VAR &&
	    value_ptr_ptr &&
	    !Z_ISREF_PP(value_ptr_ptr) &&
	    opline->extended_value == ZEND_RETURNS_FUNCTION &&
	    !EX_T(opline->op2.var).var.fcall_returned_reference) {
		if (free_op2.var == NULL) {
			PZVAL_LOCK(*value_ptr_ptr); /* undo the effect of get_zval_ptr_ptr() */
		}
		zend_error(E_STRICT, "Only variables should be assigned by reference");
		if (UNEXPECTED(EG(exception) != NULL)) {
			FREE_OP2_VAR_PTR();
			HANDLE_EXCEPTION();
		}
		ZEND_VM_DISPATCH_TO_HANDLER(ZEND_ASSIGN);
	} else if (OP2_TYPE == IS_VAR && opline->extended_value == ZEND_RETURNS_NEW) {
		PZVAL_LOCK(*value_ptr_ptr);
	}
	if (OP1_TYPE == IS_VAR && UNEXPECTED(EX_T(opline->op1.var).var.ptr_ptr == &EX_T(opline->op1.var).var.ptr)) {
		zend_error_noreturn(E_ERROR, "Cannot assign by reference to overloaded object");
	}

	variable_ptr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);
	if ((OP2_TYPE == IS_VAR && UNEXPECTED(value_ptr_ptr == NULL)) ||
	    (OP1_TYPE == IS_VAR && UNEXPECTED(variable_ptr_ptr == NULL))) {
		zend_error_noreturn(E_ERROR, "Cannot create references to/from string offsets nor overloaded objects");
	}
	zend_assign_to_variable_reference(variable_ptr_ptr, value_ptr_ptr TSRMLS_CC);

	if (OP2_TYPE == IS_VAR && opline->extended_value == ZEND_RETURNS_NEW) {
		Z_DELREF_PP(variable_ptr_ptr);
	}

	if (RETURN_VALUE_USED(opline)) {
		PZVAL_LOCK(*variable_ptr_ptr);
		AI_SET_PTR(&EX_T(opline->result.var), *variable_ptr_ptr);
	}

	FREE_OP1_VAR_PTR();
	FREE_OP2_VAR_PTR();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HELPER(zend_leave_helper, ANY, ANY)
{
	zend_bool nested = EX(nested);
	zend_op_array *op_array = EX(op_array);

	EG(current_execute_data) = EX(prev_execute_data);
	EG(opline_ptr) = NULL;
	if (!EG(active_symbol_table)) {
		i_free_compiled_variables(execute_data);
	}

	zend_vm_stack_free((char*)execute_data - (ZEND_MM_ALIGNED_SIZE(sizeof(temp_variable)) * op_array->T) TSRMLS_CC);

	if ((op_array->fn_flags & ZEND_ACC_CLOSURE) && op_array->prototype) {
		zval_ptr_dtor((zval**)&op_array->prototype);
	}

	if (nested) {
		execute_data = EG(current_execute_data);
	}
	if (nested) {
		USE_OPLINE

		LOAD_REGS();
		LOAD_OPLINE();
		if (UNEXPECTED(opline->opcode == ZEND_INCLUDE_OR_EVAL)) {

			EX(function_state).function = (zend_function *) EX(op_array);
			EX(function_state).arguments = NULL;

			EG(opline_ptr) = &EX(opline);
			EG(active_op_array) = EX(op_array);
			EG(return_value_ptr_ptr) = EX(original_return_value);
			destroy_op_array(op_array TSRMLS_CC);
			efree(op_array);
			if (UNEXPECTED(EG(exception) != NULL)) {
				zend_throw_exception_internal(NULL TSRMLS_CC);
				HANDLE_EXCEPTION_LEAVE();
			}

			ZEND_VM_INC_OPCODE();
			ZEND_VM_LEAVE();
		} else {
			EG(opline_ptr) = &EX(opline);
			EG(active_op_array) = EX(op_array);
			EG(return_value_ptr_ptr) = EX(original_return_value);
			if (EG(active_symbol_table)) {
				zend_clean_and_cache_symbol_table(EG(active_symbol_table) TSRMLS_CC);
			}
			EG(active_symbol_table) = EX(symbol_table);

			EX(function_state).function = (zend_function *) EX(op_array);
			EX(function_state).arguments = NULL;

			if (EG(This)) {
				if (UNEXPECTED(EG(exception) != NULL) && EX(call)->is_ctor_call) {
					if (EX(call)->is_ctor_result_used) {
						Z_DELREF_P(EG(This));
					}
					if (Z_REFCOUNT_P(EG(This)) == 1) {
						zend_object_store_ctor_failed(EG(This) TSRMLS_CC);
					}
				}
				zval_ptr_dtor(&EG(This));
			}
			EG(This) = EX(current_this);
			EG(scope) = EX(current_scope);
			EG(called_scope) = EX(current_called_scope);

			EX(call)--;

			zend_vm_stack_clear_multiple(1 TSRMLS_CC);

			if (UNEXPECTED(EG(exception) != NULL)) {
				zend_throw_exception_internal(NULL TSRMLS_CC);
				if (RETURN_VALUE_USED(opline) && EX_T(opline->result.var).var.ptr) {
					zval_ptr_dtor(&EX_T(opline->result.var).var.ptr);
				}
				HANDLE_EXCEPTION_LEAVE();
			}

			ZEND_VM_INC_OPCODE();
			ZEND_VM_LEAVE();
		}
	}
	ZEND_VM_RETURN();
}

ZEND_VM_HELPER(zend_do_fcall_common_helper, ANY, ANY)
{
	USE_OPLINE
	zend_bool should_change_scope = 0;
	zend_function *fbc = EX(function_state).function;

	SAVE_OPLINE();
	EX(object) = EX(call)->object;
	if (UNEXPECTED((fbc->common.fn_flags & (ZEND_ACC_ABSTRACT|ZEND_ACC_DEPRECATED)) != 0)) {
		if (UNEXPECTED((fbc->common.fn_flags & ZEND_ACC_ABSTRACT) != 0)) {
			zend_error_noreturn(E_ERROR, "Cannot call abstract method %s::%s()", fbc->common.scope->name, fbc->common.function_name);
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE(); /* Never reached */
		}
		if (UNEXPECTED((fbc->common.fn_flags & ZEND_ACC_DEPRECATED) != 0)) {
			zend_error(E_DEPRECATED, "Function %s%s%s() is deprecated",
				fbc->common.scope ? fbc->common.scope->name : "",
				fbc->common.scope ? "::" : "",
				fbc->common.function_name);
		}
	}
	if (fbc->common.scope &&
		!(fbc->common.fn_flags & ZEND_ACC_STATIC) &&
		!EX(object)) {

		if (fbc->common.fn_flags & ZEND_ACC_ALLOW_STATIC) {
			/* FIXME: output identifiers properly */
			zend_error(E_STRICT, "Non-static method %s::%s() should not be called statically", fbc->common.scope->name, fbc->common.function_name);
		} else {
			/* FIXME: output identifiers properly */
			/* An internal function assumes $this is present and won't check that. So PHP would crash by allowing the call. */
			zend_error_noreturn(E_ERROR, "Non-static method %s::%s() cannot be called statically", fbc->common.scope->name, fbc->common.function_name);
		}
	}

	if (fbc->type == ZEND_USER_FUNCTION || fbc->common.scope) {
		should_change_scope = 1;
		EX(current_this) = EG(This);
		EX(current_scope) = EG(scope);
		EX(current_called_scope) = EG(called_scope);
		EG(This) = EX(object);
		EG(scope) = (fbc->type == ZEND_USER_FUNCTION || !EX(object)) ? fbc->common.scope : NULL;
		EG(called_scope) = EX(call)->called_scope;
	}

	EX(function_state).arguments = zend_vm_stack_top(TSRMLS_C);
	zend_vm_stack_push((void*)(zend_uintptr_t)opline->extended_value TSRMLS_CC);
	LOAD_OPLINE();

	if (fbc->type == ZEND_INTERNAL_FUNCTION) {
		if (fbc->common.arg_info) {
			zend_uint i=0;
			zval **p = (zval**)EX(function_state).arguments;
			ulong arg_count = opline->extended_value;

			while (arg_count>0) {
				zend_verify_arg_type(fbc, ++i, *(p-arg_count), 0 TSRMLS_CC);
				arg_count--;
			}
		}

		if (EXPECTED(EG(exception) == NULL)) {
			temp_variable *ret = &EX_T(opline->result.var);

			MAKE_STD_ZVAL(ret->var.ptr);
			ZVAL_NULL(ret->var.ptr);
			ret->var.ptr_ptr = &ret->var.ptr;
			ret->var.fcall_returned_reference = (fbc->common.fn_flags & ZEND_ACC_RETURN_REFERENCE) != 0;

			if (!zend_execute_internal) {
				/* saves one function call if zend_execute_internal is not used */
				fbc->internal_function.handler(opline->extended_value, ret->var.ptr, (fbc->common.fn_flags & ZEND_ACC_RETURN_REFERENCE) ? &ret->var.ptr : NULL, EX(object), RETURN_VALUE_USED(opline) TSRMLS_CC);
			} else {
				zend_execute_internal(execute_data, NULL, RETURN_VALUE_USED(opline) TSRMLS_CC);
			}

			if (!RETURN_VALUE_USED(opline)) {
				zval_ptr_dtor(&ret->var.ptr);
			}
		} else if (RETURN_VALUE_USED(opline)) {
			EX_T(opline->result.var).var.ptr = NULL;
		}
	} else if (fbc->type == ZEND_USER_FUNCTION) {
		EX(original_return_value) = EG(return_value_ptr_ptr);
		EG(active_symbol_table) = NULL;
		EG(active_op_array) = &fbc->op_array;
		EG(return_value_ptr_ptr) = NULL;
		if (RETURN_VALUE_USED(opline)) {
			temp_variable *ret = &EX_T(opline->result.var);

			ret->var.ptr = NULL;
			EG(return_value_ptr_ptr) = &ret->var.ptr;
			ret->var.ptr_ptr = &ret->var.ptr;
			ret->var.fcall_returned_reference = (fbc->common.fn_flags & ZEND_ACC_RETURN_REFERENCE) != 0;
		}

		if (UNEXPECTED((EG(active_op_array)->fn_flags & ZEND_ACC_GENERATOR) != 0)) {
			if (RETURN_VALUE_USED(opline)) {
				EX_T(opline->result.var).var.ptr = zend_generator_create_zval(EG(active_op_array) TSRMLS_CC);
			}
		} else if (EXPECTED(zend_execute_ex == execute_ex)) {
			if (EXPECTED(EG(exception) == NULL)) {
				ZEND_VM_ENTER();
			}
		} else {
			zend_execute(EG(active_op_array) TSRMLS_CC);
		}

		EG(opline_ptr) = &EX(opline);
		EG(active_op_array) = EX(op_array);
		EG(return_value_ptr_ptr) = EX(original_return_value);
		if (EG(active_symbol_table)) {
			zend_clean_and_cache_symbol_table(EG(active_symbol_table) TSRMLS_CC);
		}
		EG(active_symbol_table) = EX(symbol_table);
	} else { /* ZEND_OVERLOADED_FUNCTION */
		MAKE_STD_ZVAL(EX_T(opline->result.var).var.ptr);
		ZVAL_NULL(EX_T(opline->result.var).var.ptr);

		/* Not sure what should be done here if it's a static method */
		if (EXPECTED(EX(object) != NULL)) {
			Z_OBJ_HT_P(EX(object))->call_method(fbc->common.function_name, opline->extended_value, EX_T(opline->result.var).var.ptr, &EX_T(opline->result.var).var.ptr, EX(object), RETURN_VALUE_USED(opline) TSRMLS_CC);
		} else {
			zend_error_noreturn(E_ERROR, "Cannot call overloaded function for non-object");
		}

		if (fbc->type == ZEND_OVERLOADED_FUNCTION_TEMPORARY) {
			efree((char*)fbc->common.function_name);
		}
		efree(fbc);

		if (!RETURN_VALUE_USED(opline)) {
			zval_ptr_dtor(&EX_T(opline->result.var).var.ptr);
		} else {
			Z_UNSET_ISREF_P(EX_T(opline->result.var).var.ptr);
			Z_SET_REFCOUNT_P(EX_T(opline->result.var).var.ptr, 1);
			EX_T(opline->result.var).var.fcall_returned_reference = 0;
			EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
		}
	}

	EX(function_state).function = (zend_function *) EX(op_array);
	EX(function_state).arguments = NULL;

	if (should_change_scope) {
		if (EG(This)) {
			if (UNEXPECTED(EG(exception) != NULL) && EX(call)->is_ctor_call) {
				if (EX(call)->is_ctor_result_used) {
					Z_DELREF_P(EG(This));
				}
				if (Z_REFCOUNT_P(EG(This)) == 1) {
					zend_object_store_ctor_failed(EG(This) TSRMLS_CC);
				}
			}
			zval_ptr_dtor(&EG(This));
		}
		EG(This) = EX(current_this);
		EG(scope) = EX(current_scope);
		EG(called_scope) = EX(current_called_scope);
	}

	EX(call)--;

	zend_vm_stack_clear_multiple(1 TSRMLS_CC);

	if (UNEXPECTED(EG(exception) != NULL)) {
		zend_throw_exception_internal(NULL TSRMLS_CC);
		if (RETURN_VALUE_USED(opline) && EX_T(opline->result.var).var.ptr) {
			zval_ptr_dtor(&EX_T(opline->result.var).var.ptr);
		}
		HANDLE_EXCEPTION();
	}

	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(42, ZEND_JMP, ANY, ANY)
{
	USE_OPLINE

#if DEBUG_ZEND>=2
	printf("Jumping to %d\n", opline->op1.opline_num);
#endif
	ZEND_VM_SET_OPCODE(opline->op1.jmp_addr);
	ZEND_VM_CONTINUE();
}

ZEND_VM_HANDLER(43, ZEND_JMPZ, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *val;
	int ret;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_TMP_VAR && EXPECTED(Z_TYPE_P(val) == IS_BOOL)) {
		ret = Z_LVAL_P(val);
	} else {
		ret = i_zend_is_true(val);
		FREE_OP1();
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
	}
	if (!ret) {
#if DEBUG_ZEND>=2
		printf("Conditional jmp to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_SET_OPCODE(opline->op2.jmp_addr);
		ZEND_VM_CONTINUE();
	}

	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(44, ZEND_JMPNZ, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *val;
	int ret;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_TMP_VAR && EXPECTED(Z_TYPE_P(val) == IS_BOOL)) {
		ret = Z_LVAL_P(val);
	} else {
		ret = i_zend_is_true(val);
		FREE_OP1();
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
	}
	if (ret) {
#if DEBUG_ZEND>=2
		printf("Conditional jmp to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_SET_OPCODE(opline->op2.jmp_addr);
		ZEND_VM_CONTINUE();
	}

	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(45, ZEND_JMPZNZ, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *val;
	int retval;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_TMP_VAR && EXPECTED(Z_TYPE_P(val) == IS_BOOL)) {
		retval = Z_LVAL_P(val);
	} else {
		retval = i_zend_is_true(val);
		FREE_OP1();
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
	}
	if (EXPECTED(retval != 0)) {
#if DEBUG_ZEND>=2
		printf("Conditional jmp on true to %d\n", opline->extended_value);
#endif
		ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->extended_value]);
		ZEND_VM_CONTINUE(); /* CHECK_ME */
	} else {
#if DEBUG_ZEND>=2
		printf("Conditional jmp on false to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->op2.opline_num]);
		ZEND_VM_CONTINUE(); /* CHECK_ME */
	}
}

ZEND_VM_HANDLER(46, ZEND_JMPZ_EX, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *val;
	int retval;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_TMP_VAR && EXPECTED(Z_TYPE_P(val) == IS_BOOL)) {
		retval = Z_LVAL_P(val);
	} else {
		retval = i_zend_is_true(val);
		FREE_OP1();
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
	}
	Z_LVAL(EX_T(opline->result.var).tmp_var) = retval;
	Z_TYPE(EX_T(opline->result.var).tmp_var) = IS_BOOL;
	if (!retval) {
#if DEBUG_ZEND>=2
		printf("Conditional jmp to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_SET_OPCODE(opline->op2.jmp_addr);
		ZEND_VM_CONTINUE();
	}
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(47, ZEND_JMPNZ_EX, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *val;
	int retval;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_TMP_VAR && EXPECTED(Z_TYPE_P(val) == IS_BOOL)) {
		retval = Z_LVAL_P(val);
	} else {
		retval = i_zend_is_true(val);
		FREE_OP1();
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
	}
	Z_LVAL(EX_T(opline->result.var).tmp_var) = retval;
	Z_TYPE(EX_T(opline->result.var).tmp_var) = IS_BOOL;
	if (retval) {
#if DEBUG_ZEND>=2
		printf("Conditional jmp to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_SET_OPCODE(opline->op2.jmp_addr);
		ZEND_VM_CONTINUE();
	}
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(70, ZEND_FREE, TMP|VAR, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (OP1_TYPE == IS_TMP_VAR) {
		zendi_zval_dtor(EX_T(opline->op1.var).tmp_var);
	} else {
		zval_ptr_dtor(&EX_T(opline->op1.var).var.ptr);
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(53, ZEND_INIT_STRING, ANY, ANY)
{
	USE_OPLINE
	zval *tmp = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	tmp->value.str.val = emalloc(1);
	tmp->value.str.val[0] = 0;
	tmp->value.str.len = 0;
	Z_SET_REFCOUNT_P(tmp, 1);
	tmp->type = IS_STRING;
	Z_UNSET_ISREF_P(tmp);
	/*CHECK_EXCEPTION();*/
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(54, ZEND_ADD_CHAR, TMP|UNUSED, CONST)
{
	USE_OPLINE
	zval *str = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();

	if (OP1_TYPE == IS_UNUSED) {
		/* Initialize for erealloc in add_char_to_string */
		Z_STRVAL_P(str) = NULL;
		Z_STRLEN_P(str) = 0;
		Z_TYPE_P(str) = IS_STRING;

		INIT_PZVAL(str);
	}

	add_char_to_string(str, str, opline->op2.zv);

	/* FREE_OP is missing intentionally here - we're always working on the same temporary variable */
	/*CHECK_EXCEPTION();*/
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(55, ZEND_ADD_STRING, TMP|UNUSED, CONST)
{
	USE_OPLINE
	zval *str = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();

	if (OP1_TYPE == IS_UNUSED) {
		/* Initialize for erealloc in add_string_to_string */
		Z_STRVAL_P(str) = NULL;
		Z_STRLEN_P(str) = 0;
		Z_TYPE_P(str) = IS_STRING;

		INIT_PZVAL(str);
	}

	add_string_to_string(str, str, opline->op2.zv);

	/* FREE_OP is missing intentionally here - we're always working on the same temporary variable */
	/*CHECK_EXCEPTION();*/
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(56, ZEND_ADD_VAR, TMP|UNUSED, TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op2;
	zval *str = &EX_T(opline->result.var).tmp_var;
	zval *var;
	zval var_copy;
	int use_copy = 0;

	SAVE_OPLINE();
	var = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_UNUSED) {
		/* Initialize for erealloc in add_string_to_string */
		Z_STRVAL_P(str) = NULL;
		Z_STRLEN_P(str) = 0;
		Z_TYPE_P(str) = IS_STRING;

		INIT_PZVAL(str);
	}

	if (Z_TYPE_P(var) != IS_STRING) {
		zend_make_printable_zval(var, &var_copy, &use_copy);

		if (use_copy) {
			var = &var_copy;
		}
	}
	add_string_to_string(str, str, var);

	if (use_copy) {
		zval_dtor(var);
	}
	/* original comment, possibly problematic:
	 * FREE_OP is missing intentionally here - we're always working on the same temporary variable
	 * (Zeev):  I don't think it's problematic, we only use variables
	 * which aren't affected by FREE_OP(Ts, )'s anyway, unless they're
	 * string offsets or overloaded objects
	 */
	FREE_OP2();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(109, ZEND_FETCH_CLASS, ANY, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (EG(exception)) {
		zend_exception_save(TSRMLS_C);
	}
	if (OP2_TYPE == IS_UNUSED) {
		EX_T(opline->result.var).class_entry = zend_fetch_class(NULL, 0, opline->extended_value TSRMLS_CC);
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	} else {
		zend_free_op free_op2;
		zval *class_name = GET_OP2_ZVAL_PTR(BP_VAR_R);

		if (OP2_TYPE == IS_CONST) {
			if (CACHED_PTR(opline->op2.literal->cache_slot)) {
				EX_T(opline->result.var).class_entry = CACHED_PTR(opline->op2.literal->cache_slot);
			} else {
				EX_T(opline->result.var).class_entry = zend_fetch_class_by_name(Z_STRVAL_P(class_name), Z_STRLEN_P(class_name), opline->op2.literal + 1, opline->extended_value TSRMLS_CC);
				CACHE_PTR(opline->op2.literal->cache_slot, EX_T(opline->result.var).class_entry);
			}
		} else if (Z_TYPE_P(class_name) == IS_OBJECT) {
			EX_T(opline->result.var).class_entry = Z_OBJCE_P(class_name);
		} else if (Z_TYPE_P(class_name) == IS_STRING) {
			EX_T(opline->result.var).class_entry = zend_fetch_class(Z_STRVAL_P(class_name), Z_STRLEN_P(class_name), opline->extended_value TSRMLS_CC);
		} else {
			if (UNEXPECTED(EG(exception) != NULL)) {
				HANDLE_EXCEPTION();
			}
			zend_error_noreturn(E_ERROR, "Class name must be a valid object or a string");
		}

		FREE_OP2();
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}
}

ZEND_VM_HANDLER(112, ZEND_INIT_METHOD_CALL, TMP|VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zval *function_name;
	char *function_name_strval;
	int function_name_strlen;
	zend_free_op free_op1, free_op2;
	call_slot *call = EX(call_slots) + opline->result.num;

	SAVE_OPLINE();

	function_name = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (OP2_TYPE != IS_CONST &&
	    UNEXPECTED(Z_TYPE_P(function_name) != IS_STRING)) {
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
		zend_error_noreturn(E_ERROR, "Method name must be a string");
	}

	function_name_strval = Z_STRVAL_P(function_name);
	function_name_strlen = Z_STRLEN_P(function_name);

	call->object = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_R);

	if (EXPECTED(call->object != NULL) &&
	    EXPECTED(Z_TYPE_P(call->object) == IS_OBJECT)) {
		call->called_scope = Z_OBJCE_P(call->object);

		if (OP2_TYPE != IS_CONST ||
		    (call->fbc = CACHED_POLYMORPHIC_PTR(opline->op2.literal->cache_slot, call->called_scope)) == NULL) {
		    zval *object = call->object;

			if (UNEXPECTED(Z_OBJ_HT_P(call->object)->get_method == NULL)) {
				zend_error_noreturn(E_ERROR, "Object does not support method calls");
			}

			/* First, locate the function. */
			call->fbc = Z_OBJ_HT_P(call->object)->get_method(&call->object, function_name_strval, function_name_strlen, ((OP2_TYPE == IS_CONST) ? (opline->op2.literal + 1) : NULL) TSRMLS_CC);
			if (UNEXPECTED(call->fbc == NULL)) {
				zend_error_noreturn(E_ERROR, "Call to undefined method %s::%s()", Z_OBJ_CLASS_NAME_P(call->object), function_name_strval);
			}
			if (OP2_TYPE == IS_CONST &&
			    EXPECTED(call->fbc->type <= ZEND_USER_FUNCTION) &&
			    EXPECTED((call->fbc->common.fn_flags & (ZEND_ACC_CALL_VIA_HANDLER|ZEND_ACC_NEVER_CACHE)) == 0) &&
			    EXPECTED(call->object == object)) {
				CACHE_POLYMORPHIC_PTR(opline->op2.literal->cache_slot, call->called_scope, call->fbc);
			}
		}
	} else {
		if (UNEXPECTED(EG(exception) != NULL)) {
			FREE_OP2();
			HANDLE_EXCEPTION();
		}
		zend_error_noreturn(E_ERROR, "Call to a member function %s() on a non-object", function_name_strval);
	}

	if ((call->fbc->common.fn_flags & ZEND_ACC_STATIC) != 0) {
		call->object = NULL;
	} else {
		if (!PZVAL_IS_REF(call->object)) {
			Z_ADDREF_P(call->object); /* For $this pointer */
		} else {
			zval *this_ptr;
			ALLOC_ZVAL(this_ptr);
			INIT_PZVAL_COPY(this_ptr, call->object);
			zval_copy_ctor(this_ptr);
			call->object = this_ptr;
		}
	}
	call->is_ctor_call = 0;
	EX(call) = call;

	FREE_OP2();
	FREE_OP1_IF_VAR();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(113, ZEND_INIT_STATIC_METHOD_CALL, CONST|VAR, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE
	zval *function_name;
	zend_class_entry *ce;
	call_slot *call = EX(call_slots) + opline->result.num;

	SAVE_OPLINE();

	if (OP1_TYPE == IS_CONST) {
		/* no function found. try a static method in class */
		if (CACHED_PTR(opline->op1.literal->cache_slot)) {
			ce = CACHED_PTR(opline->op1.literal->cache_slot);
		} else {
			ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), opline->op1.literal + 1, opline->extended_value TSRMLS_CC);
			if (UNEXPECTED(EG(exception) != NULL)) {
				HANDLE_EXCEPTION();
			}
			if (UNEXPECTED(ce == NULL)) {
				zend_error_noreturn(E_ERROR, "Class '%s' not found", Z_STRVAL_P(opline->op1.zv));
			}
			CACHE_PTR(opline->op1.literal->cache_slot, ce);
		}
		call->called_scope = ce;
	} else {
		ce = EX_T(opline->op1.var).class_entry;

		if (opline->extended_value == ZEND_FETCH_CLASS_PARENT || opline->extended_value == ZEND_FETCH_CLASS_SELF) {
			call->called_scope = EG(called_scope);
		} else {
			call->called_scope = ce;
		}
	}

	if (OP1_TYPE == IS_CONST &&
	    OP2_TYPE == IS_CONST &&
	    CACHED_PTR(opline->op2.literal->cache_slot)) {
		call->fbc = CACHED_PTR(opline->op2.literal->cache_slot);
	} else if (OP1_TYPE != IS_CONST &&
	           OP2_TYPE == IS_CONST &&
	           (call->fbc = CACHED_POLYMORPHIC_PTR(opline->op2.literal->cache_slot, ce))) {
		/* do nothing */
	} else if (OP2_TYPE != IS_UNUSED) {
		char *function_name_strval = NULL;
		int function_name_strlen = 0;
		zend_free_op free_op2;

		if (OP2_TYPE == IS_CONST) {
			function_name_strval = Z_STRVAL_P(opline->op2.zv);
			function_name_strlen = Z_STRLEN_P(opline->op2.zv);
		} else {
			function_name = GET_OP2_ZVAL_PTR(BP_VAR_R);

			if (UNEXPECTED(Z_TYPE_P(function_name) != IS_STRING)) {
				if (UNEXPECTED(EG(exception) != NULL)) {
					HANDLE_EXCEPTION();
				}
				zend_error_noreturn(E_ERROR, "Function name must be a string");
			} else {
				function_name_strval = Z_STRVAL_P(function_name);
				function_name_strlen = Z_STRLEN_P(function_name);
 			}
		}

		if (function_name_strval) {
			if (ce->get_static_method) {
				call->fbc = ce->get_static_method(ce, function_name_strval, function_name_strlen TSRMLS_CC);
			} else {
				call->fbc = zend_std_get_static_method(ce, function_name_strval, function_name_strlen, ((OP2_TYPE == IS_CONST) ? (opline->op2.literal + 1) : NULL) TSRMLS_CC);
			}
			if (UNEXPECTED(call->fbc == NULL)) {
				zend_error_noreturn(E_ERROR, "Call to undefined method %s::%s()", ce->name, function_name_strval);
			}
			if (OP2_TYPE == IS_CONST &&
			    EXPECTED(call->fbc->type <= ZEND_USER_FUNCTION) &&
			    EXPECTED((call->fbc->common.fn_flags & (ZEND_ACC_CALL_VIA_HANDLER|ZEND_ACC_NEVER_CACHE)) == 0)) {
				if (OP1_TYPE == IS_CONST) {
					CACHE_PTR(opline->op2.literal->cache_slot, call->fbc);
				} else {
					CACHE_POLYMORPHIC_PTR(opline->op2.literal->cache_slot, ce, call->fbc);
				}
			}
		}
		if (OP2_TYPE != IS_CONST) {
			FREE_OP2();
		}
	} else {
		if (UNEXPECTED(ce->constructor == NULL)) {
			zend_error_noreturn(E_ERROR, "Cannot call constructor");
		}
		if (EG(This) && Z_OBJCE_P(EG(This)) != ce->constructor->common.scope && (ce->constructor->common.fn_flags & ZEND_ACC_PRIVATE)) {
			zend_error_noreturn(E_ERROR, "Cannot call private %s::__construct()", ce->name);
		}
		call->fbc = ce->constructor;
	}

	if (call->fbc->common.fn_flags & ZEND_ACC_STATIC) {
		call->object = NULL;
	} else {
		if (EG(This) &&
		    Z_OBJ_HT_P(EG(This))->get_class_entry &&
		    !instanceof_function(Z_OBJCE_P(EG(This)), ce TSRMLS_CC)) {
		    /* We are calling method of the other (incompatible) class,
		       but passing $this. This is done for compatibility with php-4. */
			if (call->fbc->common.fn_flags & ZEND_ACC_ALLOW_STATIC) {
				zend_error(E_STRICT, "Non-static method %s::%s() should not be called statically, assuming $this from incompatible context", call->fbc->common.scope->name, call->fbc->common.function_name);
			} else {
				/* An internal function assumes $this is present and won't check that. So PHP would crash by allowing the call. */
				zend_error_noreturn(E_ERROR, "Non-static method %s::%s() cannot be called statically, assuming $this from incompatible context", call->fbc->common.scope->name, call->fbc->common.function_name);
			}
		}
		if ((call->object = EG(This))) {
			Z_ADDREF_P(call->object);
			call->called_scope = Z_OBJCE_P(call->object);
		}
	}
	call->is_ctor_call = 0;
	EX(call) = call;

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(59, ZEND_INIT_FCALL_BY_NAME, ANY, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zval *function_name;
	call_slot *call = EX(call_slots) + opline->result.num;

	if (OP2_TYPE == IS_CONST) {
		function_name = (zval*)(opline->op2.literal+1);
		if (CACHED_PTR(opline->op2.literal->cache_slot)) {
			call->fbc = CACHED_PTR(opline->op2.literal->cache_slot);
		} else if (UNEXPECTED(zend_hash_quick_find(EG(function_table), Z_STRVAL_P(function_name), Z_STRLEN_P(function_name)+1, Z_HASH_P(function_name), (void **) &call->fbc) == FAILURE)) {
			SAVE_OPLINE();
			zend_error_noreturn(E_ERROR, "Call to undefined function %s()", Z_STRVAL_P(opline->op2.zv));
		} else {
			CACHE_PTR(opline->op2.literal->cache_slot, call->fbc);
		}
		call->object = NULL;
		call->called_scope = NULL;
		call->is_ctor_call = 0;
		EX(call) = call;
		/*CHECK_EXCEPTION();*/
		ZEND_VM_NEXT_OPCODE();
	} else {
		char *function_name_strval, *lcname;
		int function_name_strlen;
		zend_free_op free_op2;

		SAVE_OPLINE();
		function_name = GET_OP2_ZVAL_PTR(BP_VAR_R);

		if (EXPECTED(Z_TYPE_P(function_name) == IS_STRING)) {
			function_name_strval = Z_STRVAL_P(function_name);
			function_name_strlen = Z_STRLEN_P(function_name);
			if (function_name_strval[0] == '\\') {
			    function_name_strlen -= 1;
				lcname = zend_str_tolower_dup(function_name_strval + 1, function_name_strlen);
			} else {
				lcname = zend_str_tolower_dup(function_name_strval, function_name_strlen);
			}
			if (UNEXPECTED(zend_hash_find(EG(function_table), lcname, function_name_strlen+1, (void **) &call->fbc) == FAILURE)) {
				zend_error_noreturn(E_ERROR, "Call to undefined function %s()", function_name_strval);
			}
			efree(lcname);
			FREE_OP2();
			call->object = NULL;
			call->called_scope = NULL;
			call->is_ctor_call = 0;
			EX(call) = call;
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE();
		} else if (OP2_TYPE != IS_CONST && OP2_TYPE != IS_TMP_VAR &&
		    EXPECTED(Z_TYPE_P(function_name) == IS_OBJECT) &&
			Z_OBJ_HANDLER_P(function_name, get_closure) &&
			Z_OBJ_HANDLER_P(function_name, get_closure)(function_name, &call->called_scope, &call->fbc, &call->object TSRMLS_CC) == SUCCESS) {
			if (call->object) {
				Z_ADDREF_P(call->object);
			}
			if (OP2_TYPE == IS_VAR && OP2_FREE &&
			    call->fbc->common.fn_flags & ZEND_ACC_CLOSURE) {
				/* Delay closure destruction until its invocation */
				call->fbc->common.prototype = (zend_function*)function_name;
			} else {
				FREE_OP2();
			}
			call->is_ctor_call = 0;
			EX(call) = call;
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE();
		} else if (OP2_TYPE != IS_CONST &&
				EXPECTED(Z_TYPE_P(function_name) == IS_ARRAY) &&
				zend_hash_num_elements(Z_ARRVAL_P(function_name)) == 2) {
			zend_class_entry *ce;
			zval **method = NULL;
			zval **obj = NULL;

			zend_hash_index_find(Z_ARRVAL_P(function_name), 0, (void **) &obj);
			zend_hash_index_find(Z_ARRVAL_P(function_name), 1, (void **) &method);

			if (!obj || !method) {
				zend_error_noreturn(E_ERROR, "Array callback has to contain indices 0 and 1");
			}

			if (Z_TYPE_PP(obj) != IS_STRING && Z_TYPE_PP(obj) != IS_OBJECT) {
				zend_error_noreturn(E_ERROR, "First array member is not a valid class name or object");
			}

			if (Z_TYPE_PP(method) != IS_STRING) {
				zend_error_noreturn(E_ERROR, "Second array member is not a valid method");
			}

			if (Z_TYPE_PP(obj) == IS_STRING) {
				ce = zend_fetch_class_by_name(Z_STRVAL_PP(obj), Z_STRLEN_PP(obj), NULL, 0 TSRMLS_CC);
				if (UNEXPECTED(ce == NULL)) {
					CHECK_EXCEPTION();
					ZEND_VM_NEXT_OPCODE();
				}
				call->called_scope = ce;
				call->object = NULL;

				if (ce->get_static_method) {
					call->fbc = ce->get_static_method(ce, Z_STRVAL_PP(method), Z_STRLEN_PP(method) TSRMLS_CC);
				} else {
					call->fbc = zend_std_get_static_method(ce, Z_STRVAL_PP(method), Z_STRLEN_PP(method), NULL TSRMLS_CC);
				}
			} else {
				call->object = *obj;
				ce = call->called_scope = Z_OBJCE_PP(obj);

				call->fbc = Z_OBJ_HT_P(call->object)->get_method(&call->object, Z_STRVAL_PP(method), Z_STRLEN_PP(method), NULL TSRMLS_CC);
				if (UNEXPECTED(call->fbc == NULL)) {
					zend_error_noreturn(E_ERROR, "Call to undefined method %s::%s()", Z_OBJ_CLASS_NAME_P(call->object), Z_STRVAL_PP(method));
				}

				if ((call->fbc->common.fn_flags & ZEND_ACC_STATIC) != 0) {
					call->object = NULL;
				} else {
					if (!PZVAL_IS_REF(call->object)) {
						Z_ADDREF_P(call->object); /* For $this pointer */
					} else {
						zval *this_ptr;
						ALLOC_ZVAL(this_ptr);
						INIT_PZVAL_COPY(this_ptr, call->object);
						zval_copy_ctor(this_ptr);
						call->object = this_ptr;
					}
				}
			}

			if (UNEXPECTED(call->fbc == NULL)) {
				zend_error_noreturn(E_ERROR, "Call to undefined method %s::%s()", ce->name, Z_STRVAL_PP(method));
			}
			call->is_ctor_call = 0;
			EX(call) = call;
			FREE_OP2();
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE();
		} else {
			if (UNEXPECTED(EG(exception) != NULL)) {
				HANDLE_EXCEPTION();
			}
			zend_error_noreturn(E_ERROR, "Function name must be a string");
			ZEND_VM_NEXT_OPCODE(); /* Never reached */
		}
	}
}


ZEND_VM_HANDLER(69, ZEND_INIT_NS_FCALL_BY_NAME, ANY, CONST)
{
	USE_OPLINE
	zend_literal *func_name;
	call_slot *call = EX(call_slots) + opline->result.num;

	func_name = opline->op2.literal + 1;
	if (CACHED_PTR(opline->op2.literal->cache_slot)) {
		call->fbc = CACHED_PTR(opline->op2.literal->cache_slot);
	} else if (zend_hash_quick_find(EG(function_table), Z_STRVAL(func_name->constant), Z_STRLEN(func_name->constant)+1, func_name->hash_value, (void **) &call->fbc)==FAILURE) {
		func_name++;
		if (UNEXPECTED(zend_hash_quick_find(EG(function_table), Z_STRVAL(func_name->constant), Z_STRLEN(func_name->constant)+1, func_name->hash_value, (void **) &call->fbc)==FAILURE)) {
			SAVE_OPLINE();
			zend_error_noreturn(E_ERROR, "Call to undefined function %s()", Z_STRVAL_P(opline->op2.zv));
		} else {
			CACHE_PTR(opline->op2.literal->cache_slot, call->fbc);
		}
	} else {
		CACHE_PTR(opline->op2.literal->cache_slot, call->fbc);
	}

	call->object = NULL;
	call->called_scope = NULL;
	call->is_ctor_call = 0;
	EX(call) = call;
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(61, ZEND_DO_FCALL_BY_NAME, ANY, ANY)
{
	EX(function_state).function = EX(call)->fbc;
	ZEND_VM_DISPATCH_TO_HELPER(zend_do_fcall_common_helper);
}

ZEND_VM_HANDLER(60, ZEND_DO_FCALL, CONST, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *fname = GET_OP1_ZVAL_PTR(BP_VAR_R);
	call_slot *call = EX(call_slots) + opline->op2.num;

	if (CACHED_PTR(opline->op1.literal->cache_slot)) {
		EX(function_state).function = CACHED_PTR(opline->op1.literal->cache_slot);
	} else if (UNEXPECTED(zend_hash_quick_find(EG(function_table), Z_STRVAL_P(fname), Z_STRLEN_P(fname)+1, Z_HASH_P(fname), (void **) &EX(function_state).function)==FAILURE)) {
	    SAVE_OPLINE();
		zend_error_noreturn(E_ERROR, "Call to undefined function %s()", fname->value.str.val);
	} else {
		CACHE_PTR(opline->op1.literal->cache_slot, EX(function_state).function);
	}
	call->fbc = EX(function_state).function;
	call->object = NULL;
	call->called_scope = NULL;
	call->is_ctor_call = 0;
	EX(call) = call;

	FREE_OP1();

	ZEND_VM_DISPATCH_TO_HELPER(zend_do_fcall_common_helper);
}

ZEND_VM_HANDLER(62, ZEND_RETURN, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zval *retval_ptr;
	zend_free_op free_op1;

	SAVE_OPLINE();
	retval_ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (!EG(return_value_ptr_ptr)) {
		if (OP1_TYPE == IS_TMP_VAR) {
			FREE_OP1();
		}
	} else {
		if (OP1_TYPE == IS_CONST ||
		    OP1_TYPE == IS_TMP_VAR ||
		    PZVAL_IS_REF(retval_ptr)) {
			zval *ret;

			ALLOC_ZVAL(ret);
			INIT_PZVAL_COPY(ret, retval_ptr);
			if (OP1_TYPE != IS_TMP_VAR) {
				zval_copy_ctor(ret);
			}
			*EG(return_value_ptr_ptr) = ret;
		} else if ((OP1_TYPE == IS_CV || OP1_TYPE == IS_VAR) &&
		           retval_ptr == &EG(uninitialized_zval)) {
			zval *ret;

			ALLOC_INIT_ZVAL(ret);
			*EG(return_value_ptr_ptr) = ret;
		} else {
			*EG(return_value_ptr_ptr) = retval_ptr;
			Z_ADDREF_P(retval_ptr);
		}
	}
	FREE_OP1_IF_VAR();
	ZEND_VM_DISPATCH_TO_HELPER(zend_leave_helper);
}

ZEND_VM_HANDLER(111, ZEND_RETURN_BY_REF, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zval *retval_ptr;
	zval **retval_ptr_ptr;
	zend_free_op free_op1;

	SAVE_OPLINE();

	do {
		if (OP1_TYPE == IS_CONST || OP1_TYPE == IS_TMP_VAR) {
			/* Not supposed to happen, but we'll allow it */
			zend_error(E_NOTICE, "Only variable references should be returned by reference");

			retval_ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
			if (!EG(return_value_ptr_ptr)) {
				if (OP1_TYPE == IS_TMP_VAR) {
					FREE_OP1();
				}
			} else if (!IS_OP1_TMP_FREE()) { /* Not a temp var */
				zval *ret;

				ALLOC_ZVAL(ret);
				INIT_PZVAL_COPY(ret, retval_ptr);
				zval_copy_ctor(ret);
				*EG(return_value_ptr_ptr) = ret;
			} else {
				zval *ret;

				ALLOC_ZVAL(ret);
				INIT_PZVAL_COPY(ret, retval_ptr);
				*EG(return_value_ptr_ptr) = ret;
			}
			break;
		}

		retval_ptr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

		if (OP1_TYPE == IS_VAR && UNEXPECTED(retval_ptr_ptr == NULL)) {
			zend_error_noreturn(E_ERROR, "Cannot return string offsets by reference");
		}

		if (OP1_TYPE == IS_VAR && !Z_ISREF_PP(retval_ptr_ptr)) {
			if (opline->extended_value == ZEND_RETURNS_FUNCTION &&
			    EX_T(opline->op1.var).var.fcall_returned_reference) {
			} else if (EX_T(opline->op1.var).var.ptr_ptr == &EX_T(opline->op1.var).var.ptr) {
				zend_error(E_NOTICE, "Only variable references should be returned by reference");
				if (EG(return_value_ptr_ptr)) {
					zval *ret;

					ALLOC_ZVAL(ret);
					INIT_PZVAL_COPY(ret, *retval_ptr_ptr);
					zval_copy_ctor(ret);
					*EG(return_value_ptr_ptr) = ret;
				}
				break;
			}
		}

		if (EG(return_value_ptr_ptr)) {
			SEPARATE_ZVAL_TO_MAKE_IS_REF(retval_ptr_ptr);
			Z_ADDREF_PP(retval_ptr_ptr);

			*EG(return_value_ptr_ptr) = *retval_ptr_ptr;
		}
	} while (0);

	FREE_OP1_IF_VAR();
	ZEND_VM_DISPATCH_TO_HELPER(zend_leave_helper);
}

ZEND_VM_HANDLER(161, ZEND_GENERATOR_RETURN, ANY, ANY)
{
	/* The generator object is stored in return_value_ptr_ptr */
	zend_generator *generator = (zend_generator *) EG(return_value_ptr_ptr);

	/* Close the generator to free up resources */
	zend_generator_close(generator, 1 TSRMLS_CC);

	/* Pass execution back to handling code */
	ZEND_VM_RETURN();
}

ZEND_VM_HANDLER(108, ZEND_THROW, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zval *value;
	zval *exception;
	zend_free_op free_op1;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_CONST || UNEXPECTED(Z_TYPE_P(value) != IS_OBJECT)) {
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
		zend_error_noreturn(E_ERROR, "Can only throw objects");
	}

	zend_exception_save(TSRMLS_C);
	/* Not sure if a complete copy is what we want here */
	ALLOC_ZVAL(exception);
	INIT_PZVAL_COPY(exception, value);
	if (!IS_OP1_TMP_FREE()) {
		zval_copy_ctor(exception);
	}

	zend_throw_exception_object(exception TSRMLS_CC);
	zend_exception_restore(TSRMLS_C);
	FREE_OP1_IF_VAR();
	HANDLE_EXCEPTION();
}

ZEND_VM_HANDLER(107, ZEND_CATCH, CONST, CV)
{
	USE_OPLINE
	zend_class_entry *ce, *catch_ce;
	zval *exception;

	SAVE_OPLINE();
	/* Check whether an exception has been thrown, if not, jump over code */
	zend_exception_restore(TSRMLS_C);
	if (EG(exception) == NULL) {
		ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->extended_value]);
		ZEND_VM_CONTINUE(); /* CHECK_ME */
	}
	if (CACHED_PTR(opline->op1.literal->cache_slot)) {
		catch_ce = CACHED_PTR(opline->op1.literal->cache_slot);
	} else {
		catch_ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), opline->op1.literal + 1, ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);

		CACHE_PTR(opline->op1.literal->cache_slot, catch_ce);
	}
	ce = Z_OBJCE_P(EG(exception));

#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_CAUGHT_ENABLED()) {
		DTRACE_EXCEPTION_CAUGHT((char *)ce->name);
	}
#endif /* HAVE_DTRACE */

	if (ce != catch_ce) {
		if (!instanceof_function(ce, catch_ce TSRMLS_CC)) {
			if (opline->result.num) {
				zend_throw_exception_internal(NULL TSRMLS_CC);
				HANDLE_EXCEPTION();
			}
			ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->extended_value]);
			ZEND_VM_CONTINUE(); /* CHECK_ME */
		}
	}

	exception = EG(exception);
	if (!EG(active_symbol_table)) {
		if (EX_CV(opline->op2.var)) {
			zval_ptr_dtor(EX_CV(opline->op2.var));
		}
		EX_CV(opline->op2.var) = (zval**)EX_CV_NUM(execute_data, EX(op_array)->last_var + opline->op2.var);
		*EX_CV(opline->op2.var) = EG(exception);
	} else {
		zend_compiled_variable *cv = &CV_DEF_OF(opline->op2.var);
		zend_hash_quick_update(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value,
		    &EG(exception), sizeof(zval *), (void**)&EX_CV(opline->op2.var));
	}
	if (UNEXPECTED(EG(exception) != exception)) {
		Z_ADDREF_P(EG(exception));
		HANDLE_EXCEPTION();
	} else {
		EG(exception) = NULL;
		ZEND_VM_NEXT_OPCODE();
	}
}

ZEND_VM_HANDLER(65, ZEND_SEND_VAL, CONST|TMP, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (opline->extended_value==ZEND_DO_FCALL_BY_NAME
		&& ARG_MUST_BE_SENT_BY_REF(EX(call)->fbc, opline->op2.opline_num)) {
			zend_error_noreturn(E_ERROR, "Cannot pass parameter %d by reference", opline->op2.opline_num);
	}
	{
		zval *valptr;
		zval *value;
		zend_free_op free_op1;

		value = GET_OP1_ZVAL_PTR(BP_VAR_R);

		ALLOC_ZVAL(valptr);
		INIT_PZVAL_COPY(valptr, value);
		if (!IS_OP1_TMP_FREE()) {
			zval_copy_ctor(valptr);
		}
		zend_vm_stack_push(valptr TSRMLS_CC);
		FREE_OP1_IF_VAR();
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HELPER(zend_send_by_var_helper, VAR|CV, ANY)
{
	USE_OPLINE
	zval *varptr;
	zend_free_op free_op1;
	varptr = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (varptr == &EG(uninitialized_zval)) {
		ALLOC_ZVAL(varptr);
		INIT_ZVAL(*varptr);
		Z_SET_REFCOUNT_P(varptr, 0);
	} else if (PZVAL_IS_REF(varptr)) {
		zval *original_var = varptr;

		ALLOC_ZVAL(varptr);
		ZVAL_COPY_VALUE(varptr, original_var);
		Z_UNSET_ISREF_P(varptr);
		Z_SET_REFCOUNT_P(varptr, 0);
		zval_copy_ctor(varptr);
	}
	Z_ADDREF_P(varptr);
	zend_vm_stack_push(varptr TSRMLS_CC);
	FREE_OP1();  /* for string offsets */

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(106, ZEND_SEND_VAR_NO_REF, VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *varptr;

	SAVE_OPLINE();
	if (opline->extended_value & ZEND_ARG_COMPILE_TIME_BOUND) { /* Had function_ptr at compile_time */
		if (!(opline->extended_value & ZEND_ARG_SEND_BY_REF)) {
			ZEND_VM_DISPATCH_TO_HELPER(zend_send_by_var_helper);
		}
	} else if (!ARG_SHOULD_BE_SENT_BY_REF(EX(call)->fbc, opline->op2.opline_num)) {
		ZEND_VM_DISPATCH_TO_HELPER(zend_send_by_var_helper);
	}

	if (OP1_TYPE == IS_VAR &&
		(opline->extended_value & ZEND_ARG_SEND_FUNCTION) &&
		EX_T(opline->op1.var).var.fcall_returned_reference &&
		EX_T(opline->op1.var).var.ptr) {
		varptr = EX_T(opline->op1.var).var.ptr;
		PZVAL_UNLOCK_EX(varptr, &free_op1, 0);
	} else {
		varptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
	}
	if ((!(opline->extended_value & ZEND_ARG_SEND_FUNCTION) ||
	     EX_T(opline->op1.var).var.fcall_returned_reference) &&
	    varptr != &EG(uninitialized_zval) &&
	    (PZVAL_IS_REF(varptr) ||
	     (Z_REFCOUNT_P(varptr) == 1 && (OP1_TYPE == IS_CV || free_op1.var)))) {
		Z_SET_ISREF_P(varptr);
		Z_ADDREF_P(varptr);
		zend_vm_stack_push(varptr TSRMLS_CC);
	} else {
		zval *valptr;

		if ((opline->extended_value & ZEND_ARG_COMPILE_TIME_BOUND) ?
			!(opline->extended_value & ZEND_ARG_SEND_SILENT) :
			!ARG_MAY_BE_SENT_BY_REF(EX(call)->fbc, opline->op2.opline_num)) {
			zend_error(E_STRICT, "Only variables should be passed by reference");
		}
		ALLOC_ZVAL(valptr);
		INIT_PZVAL_COPY(valptr, varptr);
		if (!IS_OP1_TMP_FREE()) {
			zval_copy_ctor(valptr);
		}
		zend_vm_stack_push(valptr TSRMLS_CC);
	}
	FREE_OP1_IF_VAR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(67, ZEND_SEND_REF, VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval **varptr_ptr;
	zval *varptr;

	SAVE_OPLINE();
	varptr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

	if (OP1_TYPE == IS_VAR && UNEXPECTED(varptr_ptr == NULL)) {
		zend_error_noreturn(E_ERROR, "Only variables can be passed by reference");
	}

	if (OP1_TYPE == IS_VAR && UNEXPECTED(*varptr_ptr == &EG(error_zval))) {
		ALLOC_INIT_ZVAL(varptr);
		zend_vm_stack_push(varptr TSRMLS_CC);
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	if (opline->extended_value == ZEND_DO_FCALL_BY_NAME &&
	    EX(function_state).function->type == ZEND_INTERNAL_FUNCTION &&
	    !ARG_SHOULD_BE_SENT_BY_REF(EX(call)->fbc, opline->op2.opline_num)) {
		ZEND_VM_DISPATCH_TO_HELPER(zend_send_by_var_helper);
	}

	SEPARATE_ZVAL_TO_MAKE_IS_REF(varptr_ptr);
	varptr = *varptr_ptr;
	Z_ADDREF_P(varptr);
	zend_vm_stack_push(varptr TSRMLS_CC);

	FREE_OP1_VAR_PTR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(66, ZEND_SEND_VAR, VAR|CV, ANY)
{
	USE_OPLINE

	if ((opline->extended_value == ZEND_DO_FCALL_BY_NAME)
		&& ARG_SHOULD_BE_SENT_BY_REF(EX(call)->fbc, opline->op2.opline_num)) {
		ZEND_VM_DISPATCH_TO_HANDLER(ZEND_SEND_REF);
	}
	SAVE_OPLINE();
	ZEND_VM_DISPATCH_TO_HELPER(zend_send_by_var_helper);
}

ZEND_VM_HANDLER(63, ZEND_RECV, ANY, ANY)
{
	USE_OPLINE
	zend_uint arg_num = opline->op1.num;
	zval **param = zend_vm_stack_get_arg(arg_num TSRMLS_CC);

	SAVE_OPLINE();
	if (UNEXPECTED(param == NULL)) {
		if (zend_verify_arg_type((zend_function *) EG(active_op_array), arg_num, NULL, opline->extended_value TSRMLS_CC)) {
			const char *space;
			const char *class_name;
			zend_execute_data *ptr;

			if (EG(active_op_array)->scope) {
				class_name = EG(active_op_array)->scope->name;
				space = "::";
			} else {
				class_name = space = "";
			}
			ptr = EX(prev_execute_data);

			if(ptr && ptr->op_array) {
				zend_error(E_WARNING, "Missing argument %u for %s%s%s(), called in %s on line %d and defined", opline->op1.num, class_name, space, get_active_function_name(TSRMLS_C), ptr->op_array->filename, ptr->opline->lineno);
			} else {
				zend_error(E_WARNING, "Missing argument %u for %s%s%s()", opline->op1.num, class_name, space, get_active_function_name(TSRMLS_C));
			}
		}
	} else {
		zval **var_ptr;

		zend_verify_arg_type((zend_function *) EG(active_op_array), arg_num, *param, opline->extended_value TSRMLS_CC);
		var_ptr = _get_zval_ptr_ptr_cv_BP_VAR_W(execute_data, opline->result.var TSRMLS_CC);
		Z_DELREF_PP(var_ptr);
		*var_ptr = *param;
		Z_ADDREF_PP(var_ptr);
	}

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(64, ZEND_RECV_INIT, ANY, CONST)
{
	USE_OPLINE
	zval *assignment_value;
	zend_uint arg_num = opline->op1.num;
	zval **param = zend_vm_stack_get_arg(arg_num TSRMLS_CC);
	zval **var_ptr;

	SAVE_OPLINE();
	if (param == NULL) {
		ALLOC_ZVAL(assignment_value);
		*assignment_value = *opline->op2.zv;
		if ((Z_TYPE_P(assignment_value) & IS_CONSTANT_TYPE_MASK) == IS_CONSTANT ||
		     Z_TYPE_P(assignment_value)==IS_CONSTANT_ARRAY) {
			Z_SET_REFCOUNT_P(assignment_value, 1);
			zval_update_constant(&assignment_value, 0 TSRMLS_CC);
		} else {
			zval_copy_ctor(assignment_value);
		}
		INIT_PZVAL(assignment_value);
	} else {
		assignment_value = *param;
		Z_ADDREF_P(assignment_value);
	}

	zend_verify_arg_type((zend_function *) EG(active_op_array), arg_num, assignment_value, opline->extended_value TSRMLS_CC);
	var_ptr = _get_zval_ptr_ptr_cv_BP_VAR_W(execute_data, opline->result.var TSRMLS_CC);
	zval_ptr_dtor(var_ptr);
	*var_ptr = assignment_value;

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(52, ZEND_BOOL, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *retval = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	/* PHP 3.0 returned "" for false and 1 for true, here we use 0 and 1 for now */
	ZVAL_BOOL(retval, i_zend_is_true(GET_OP1_ZVAL_PTR(BP_VAR_R)));
	FREE_OP1();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(50, ZEND_BRK, ANY, CONST)
{
	USE_OPLINE
	zend_brk_cont_element *el;

	SAVE_OPLINE();
	el = zend_brk_cont(Z_LVAL_P(opline->op2.zv), opline->op1.opline_num,
	                   EX(op_array), execute_data TSRMLS_CC);
	ZEND_VM_JMP(EX(op_array)->opcodes + el->brk);
}

ZEND_VM_HANDLER(51, ZEND_CONT, ANY, CONST)
{
	USE_OPLINE
	zend_brk_cont_element *el;

	SAVE_OPLINE();
	el = zend_brk_cont(Z_LVAL_P(opline->op2.zv), opline->op1.opline_num,
	                   EX(op_array), execute_data TSRMLS_CC);
	ZEND_VM_JMP(EX(op_array)->opcodes + el->cont);
}

ZEND_VM_HANDLER(100, ZEND_GOTO, ANY, CONST)
{
	zend_op *brk_opline;
	USE_OPLINE
	zend_brk_cont_element *el;

	SAVE_OPLINE();
	el = zend_brk_cont(Z_LVAL_P(opline->op2.zv), opline->extended_value,
 	                   EX(op_array), execute_data TSRMLS_CC);

	brk_opline = EX(op_array)->opcodes + el->brk;

	switch (brk_opline->opcode) {
		case ZEND_SWITCH_FREE:
			if (!(brk_opline->extended_value & EXT_TYPE_FREE_ON_RETURN)) {
				zval_ptr_dtor(&EX_T(brk_opline->op1.var).var.ptr);
			}
			break;
		case ZEND_FREE:
			if (!(brk_opline->extended_value & EXT_TYPE_FREE_ON_RETURN)) {
				zendi_zval_dtor(EX_T(brk_opline->op1.var).tmp_var);
			}
			break;
	}
	ZEND_VM_JMP(opline->op1.jmp_addr);
}

ZEND_VM_HANDLER(48, ZEND_CASE, CONST|TMP|VAR|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;

	SAVE_OPLINE();
	if (OP1_TYPE==IS_VAR) {
		PZVAL_LOCK(EX_T(opline->op1.var).var.ptr);
	}
	is_equal_function(&EX_T(opline->result.var).tmp_var,
				 GET_OP1_ZVAL_PTR(BP_VAR_R),
				 GET_OP2_ZVAL_PTR(BP_VAR_R) TSRMLS_CC);

	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(49, ZEND_SWITCH_FREE, VAR, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	zval_ptr_dtor(&EX_T(opline->op1.var).var.ptr);
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(68, ZEND_NEW, ANY, ANY)
{
	USE_OPLINE
	zval *object_zval;
	zend_function *constructor;

	SAVE_OPLINE();
	if (UNEXPECTED((EX_T(opline->op1.var).class_entry->ce_flags & (ZEND_ACC_INTERFACE|ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) != 0)) {
		if (EX_T(opline->op1.var).class_entry->ce_flags & ZEND_ACC_INTERFACE) {
			zend_error_noreturn(E_ERROR, "Cannot instantiate interface %s", EX_T(opline->op1.var).class_entry->name);
		} else if ((EX_T(opline->op1.var).class_entry->ce_flags & ZEND_ACC_TRAIT) == ZEND_ACC_TRAIT) {
			zend_error_noreturn(E_ERROR, "Cannot instantiate trait %s", EX_T(opline->op1.var).class_entry->name);
		} else {
			zend_error_noreturn(E_ERROR, "Cannot instantiate abstract class %s", EX_T(opline->op1.var).class_entry->name);
		}
	}
	ALLOC_ZVAL(object_zval);
	object_init_ex(object_zval, EX_T(opline->op1.var).class_entry);
	INIT_PZVAL(object_zval);

	constructor = Z_OBJ_HT_P(object_zval)->get_constructor(object_zval TSRMLS_CC);

	if (constructor == NULL) {
		if (RETURN_VALUE_USED(opline)) {
			AI_SET_PTR(&EX_T(opline->result.var), object_zval);
		} else {
			zval_ptr_dtor(&object_zval);
		}
		ZEND_VM_JMP(EX(op_array)->opcodes + opline->op2.opline_num);
	} else {
		call_slot *call = EX(call_slots) + opline->extended_value;

		if (RETURN_VALUE_USED(opline)) {
			PZVAL_LOCK(object_zval);
			AI_SET_PTR(&EX_T(opline->result.var), object_zval);
		}

		/* We are not handling overloaded classes right now */
		call->fbc = constructor;
		call->object = object_zval;
		call->called_scope = EX_T(opline->op1.var).class_entry;
		call->is_ctor_call = 1;
		call->is_ctor_result_used = RETURN_VALUE_USED(opline);
		EX(call) = call;

		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}
}

ZEND_VM_HANDLER(110, ZEND_CLONE, CONST|TMP|VAR|UNUSED|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *obj;
	zend_class_entry *ce;
	zend_function *clone;
	zend_object_clone_obj_t clone_call;

	SAVE_OPLINE();
	obj = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_CONST ||
	    UNEXPECTED(Z_TYPE_P(obj) != IS_OBJECT)) {
		if (UNEXPECTED(EG(exception) != NULL)) {
			HANDLE_EXCEPTION();
		}
		zend_error_noreturn(E_ERROR, "__clone method called on non-object");
	}

	ce = Z_OBJCE_P(obj);
	clone = ce ? ce->clone : NULL;
	clone_call =  Z_OBJ_HT_P(obj)->clone_obj;
	if (UNEXPECTED(clone_call == NULL)) {
		if (ce) {
			zend_error_noreturn(E_ERROR, "Trying to clone an uncloneable object of class %s", ce->name);
		} else {
			zend_error_noreturn(E_ERROR, "Trying to clone an uncloneable object");
		}
	}

	if (ce && clone) {
		if (clone->op_array.fn_flags & ZEND_ACC_PRIVATE) {
			/* Ensure that if we're calling a private function, we're allowed to do so.
			 */
			if (UNEXPECTED(ce != EG(scope))) {
				zend_error_noreturn(E_ERROR, "Call to private %s::__clone() from context '%s'", ce->name, EG(scope) ? EG(scope)->name : "");
			}
		} else if ((clone->common.fn_flags & ZEND_ACC_PROTECTED)) {
			/* Ensure that if we're calling a protected function, we're allowed to do so.
			 */
			if (UNEXPECTED(!zend_check_protected(zend_get_function_root_class(clone), EG(scope)))) {
				zend_error_noreturn(E_ERROR, "Call to protected %s::__clone() from context '%s'", ce->name, EG(scope) ? EG(scope)->name : "");
			}
		}
	}

	if (EXPECTED(EG(exception) == NULL)) {
		zval *retval;

		ALLOC_ZVAL(retval);
		Z_OBJVAL_P(retval) = clone_call(obj TSRMLS_CC);
		Z_TYPE_P(retval) = IS_OBJECT;
		Z_SET_REFCOUNT_P(retval, 1);
		Z_SET_ISREF_P(retval);
		if (!RETURN_VALUE_USED(opline) || UNEXPECTED(EG(exception) != NULL)) {
			zval_ptr_dtor(&retval);
		} else {
			AI_SET_PTR(&EX_T(opline->result.var), retval);
		}
	}
	FREE_OP1_IF_VAR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(99, ZEND_FETCH_CONSTANT, VAR|CONST|UNUSED, CONST)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (OP1_TYPE == IS_UNUSED) {
		zend_constant *c;
		zval *retval;

		if (CACHED_PTR(opline->op2.literal->cache_slot)) {
			c = CACHED_PTR(opline->op2.literal->cache_slot);
		} else if ((c = zend_quick_get_constant(opline->op2.literal + 1, opline->extended_value TSRMLS_CC)) == NULL) {
			if ((opline->extended_value & IS_CONSTANT_UNQUALIFIED) != 0) {
				char *actual = (char *)zend_memrchr(Z_STRVAL_P(opline->op2.zv), '\\', Z_STRLEN_P(opline->op2.zv));
				if(!actual) {
					actual = Z_STRVAL_P(opline->op2.zv);
				} else {
					actual++;
				}
				/* non-qualified constant - allow text substitution */
				zend_error(E_NOTICE, "Use of undefined constant %s - assumed '%s'", actual, actual);
				ZVAL_STRINGL(&EX_T(opline->result.var).tmp_var, actual, Z_STRLEN_P(opline->op2.zv)-(actual - Z_STRVAL_P(opline->op2.zv)), 1);
				CHECK_EXCEPTION();
				ZEND_VM_NEXT_OPCODE();
			} else {
				zend_error_noreturn(E_ERROR, "Undefined constant '%s'", Z_STRVAL_P(opline->op2.zv));
			}
		} else {
			CACHE_PTR(opline->op2.literal->cache_slot, c);
		}
		retval = &EX_T(opline->result.var).tmp_var;
		ZVAL_COPY_VALUE(retval, &c->value);
		zval_copy_ctor(retval);
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	} else {
		/* class constant */
		zend_class_entry *ce;
		zval **value;

		if (OP1_TYPE == IS_CONST) {
			if (CACHED_PTR(opline->op2.literal->cache_slot)) {
				value = CACHED_PTR(opline->op2.literal->cache_slot);
				ZVAL_COPY_VALUE(&EX_T(opline->result.var).tmp_var, *value);
				zval_copy_ctor(&EX_T(opline->result.var).tmp_var);
				CHECK_EXCEPTION();
				ZEND_VM_NEXT_OPCODE();
			} else if (CACHED_PTR(opline->op1.literal->cache_slot)) {
				ce = CACHED_PTR(opline->op1.literal->cache_slot);
			} else {
				ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), opline->op1.literal + 1, opline->extended_value TSRMLS_CC);
				if (UNEXPECTED(EG(exception) != NULL)) {
					HANDLE_EXCEPTION();
				}
				if (UNEXPECTED(ce == NULL)) {
					zend_error_noreturn(E_ERROR, "Class '%s' not found", Z_STRVAL_P(opline->op1.zv));
				}
				CACHE_PTR(opline->op1.literal->cache_slot, ce);
			}
		} else {
			ce = EX_T(opline->op1.var).class_entry;
			if ((value = CACHED_POLYMORPHIC_PTR(opline->op2.literal->cache_slot, ce)) != NULL) {
				ZVAL_COPY_VALUE(&EX_T(opline->result.var).tmp_var, *value);
				zval_copy_ctor(&EX_T(opline->result.var).tmp_var);
				CHECK_EXCEPTION();
				ZEND_VM_NEXT_OPCODE();
			}
		}

		if (EXPECTED(zend_hash_quick_find(&ce->constants_table, Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv)+1, Z_HASH_P(opline->op2.zv), (void **) &value) == SUCCESS)) {
			if (Z_TYPE_PP(value) == IS_CONSTANT_ARRAY ||
			    (Z_TYPE_PP(value) & IS_CONSTANT_TYPE_MASK) == IS_CONSTANT) {
				zend_class_entry *old_scope = EG(scope);

				EG(scope) = ce;
				zval_update_constant(value, (void *) 1 TSRMLS_CC);
				EG(scope) = old_scope;
			}
			if (OP1_TYPE == IS_CONST) {
				CACHE_PTR(opline->op2.literal->cache_slot, value);
			} else {
				CACHE_POLYMORPHIC_PTR(opline->op2.literal->cache_slot, ce, value);
			}
			ZVAL_COPY_VALUE(&EX_T(opline->result.var).tmp_var, *value);
			zval_copy_ctor(&EX_T(opline->result.var).tmp_var);
		} else if (Z_STRLEN_P(opline->op2.zv) == sizeof("class")-1 && strcmp(Z_STRVAL_P(opline->op2.zv), "class") == 0) {
			/* "class" is assigned as a case-sensitive keyword from zend_do_resolve_class_name */
			ZVAL_STRINGL(&EX_T(opline->result.var).tmp_var, ce->name, ce->name_length, 1);
		} else {
			zend_error_noreturn(E_ERROR, "Undefined class constant '%s'", Z_STRVAL_P(opline->op2.zv));
		}

		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}
}

ZEND_VM_HANDLER(72, ZEND_ADD_ARRAY_ELEMENT, CONST|TMP|VAR|CV, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *expr_ptr;

	SAVE_OPLINE();
	if ((OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) && opline->extended_value) {
		zval **expr_ptr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

		if (OP1_TYPE == IS_VAR && UNEXPECTED(expr_ptr_ptr == NULL)) {
			zend_error_noreturn(E_ERROR, "Cannot create references to/from string offsets");
		}
		SEPARATE_ZVAL_TO_MAKE_IS_REF(expr_ptr_ptr);
		expr_ptr = *expr_ptr_ptr;
		Z_ADDREF_P(expr_ptr);
	} else {
		expr_ptr=GET_OP1_ZVAL_PTR(BP_VAR_R);
		if (IS_OP1_TMP_FREE()) { /* temporary variable */
			zval *new_expr;

			ALLOC_ZVAL(new_expr);
			INIT_PZVAL_COPY(new_expr, expr_ptr);
			expr_ptr = new_expr;
		} else if (OP1_TYPE == IS_CONST || PZVAL_IS_REF(expr_ptr)) {
			zval *new_expr;

			ALLOC_ZVAL(new_expr);
			INIT_PZVAL_COPY(new_expr, expr_ptr);
			expr_ptr = new_expr;
			zendi_zval_copy_ctor(*expr_ptr);
		} else {
			Z_ADDREF_P(expr_ptr);
		}
	}

	if (OP2_TYPE != IS_UNUSED) {
		zend_free_op free_op2;
		zval *offset = GET_OP2_ZVAL_PTR(BP_VAR_R);
		ulong hval;

		switch (Z_TYPE_P(offset)) {
			case IS_DOUBLE:
				hval = zend_dval_to_lval(Z_DVAL_P(offset));
				ZEND_VM_C_GOTO(num_index);
			case IS_LONG:
			case IS_BOOL:
				hval = Z_LVAL_P(offset);
ZEND_VM_C_LABEL(num_index):
				zend_hash_index_update(Z_ARRVAL(EX_T(opline->result.var).tmp_var), hval, &expr_ptr, sizeof(zval *), NULL);
				break;
			case IS_STRING:
				if (OP2_TYPE == IS_CONST) {
					hval = Z_HASH_P(offset);
				} else {
					ZEND_HANDLE_NUMERIC_EX(Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1, hval, ZEND_VM_C_GOTO(num_index));
					if (IS_INTERNED(Z_STRVAL_P(offset))) {
						hval = INTERNED_HASH(Z_STRVAL_P(offset));
					} else {
						hval = zend_hash_func(Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1);
					}
				}
				zend_hash_quick_update(Z_ARRVAL(EX_T(opline->result.var).tmp_var), Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1, hval, &expr_ptr, sizeof(zval *), NULL);
				break;
			case IS_NULL:
				zend_hash_update(Z_ARRVAL(EX_T(opline->result.var).tmp_var), "", sizeof(""), &expr_ptr, sizeof(zval *), NULL);
				break;
			default:
				zend_error(E_WARNING, "Illegal offset type");
				zval_ptr_dtor(&expr_ptr);
				/* do nothing */
				break;
		}
		FREE_OP2();
	} else {
		zend_hash_next_index_insert(Z_ARRVAL(EX_T(opline->result.var).tmp_var), &expr_ptr, sizeof(zval *), NULL);
	}
	if ((OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) && opline->extended_value) {
		FREE_OP1_VAR_PTR();
	} else {
		FREE_OP1_IF_VAR();
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(71, ZEND_INIT_ARRAY, CONST|TMP|VAR|UNUSED|CV, CONST|TMP|VAR|UNUSED|CV)
{
	USE_OPLINE

	array_init(&EX_T(opline->result.var).tmp_var);
	if (OP1_TYPE == IS_UNUSED) {
		ZEND_VM_NEXT_OPCODE();
#if !defined(ZEND_VM_SPEC) || OP1_TYPE != IS_UNUSED
	} else {
		ZEND_VM_DISPATCH_TO_HANDLER(ZEND_ADD_ARRAY_ELEMENT);
#endif
	}
}

ZEND_VM_HANDLER(21, ZEND_CAST, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *expr;
	zval *result = &EX_T(opline->result.var).tmp_var;

	SAVE_OPLINE();
	expr = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (opline->extended_value != IS_STRING) {
		ZVAL_COPY_VALUE(result, expr);
		if (!IS_OP1_TMP_FREE()) {
			zendi_zval_copy_ctor(*result);
		}
	}
	switch (opline->extended_value) {
		case IS_NULL:
			convert_to_null(result);
			break;
		case IS_BOOL:
			convert_to_boolean(result);
			break;
		case IS_LONG:
			convert_to_long(result);
			break;
		case IS_DOUBLE:
			convert_to_double(result);
			break;
		case IS_STRING: {
			zval var_copy;
			int use_copy;

			zend_make_printable_zval(expr, &var_copy, &use_copy);
			if (use_copy) {
				ZVAL_COPY_VALUE(result, &var_copy);
				if (IS_OP1_TMP_FREE()) {
					FREE_OP1();
				}
			} else {
				ZVAL_COPY_VALUE(result, expr);
				if (!IS_OP1_TMP_FREE()) {
					zendi_zval_copy_ctor(*result);
				}
			}
			break;
		}
		case IS_ARRAY:
			convert_to_array(result);
			break;
		case IS_OBJECT:
			convert_to_object(result);
			break;
	}
	FREE_OP1_IF_VAR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(73, ZEND_INCLUDE_OR_EVAL, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_op_array *new_op_array=NULL;
	zend_free_op free_op1;
	zval *inc_filename;
	zval *tmp_inc_filename = NULL;
	zend_bool failure_retval=0;

	SAVE_OPLINE();
	inc_filename = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (inc_filename->type!=IS_STRING) {
		MAKE_STD_ZVAL(tmp_inc_filename);
		ZVAL_COPY_VALUE(tmp_inc_filename, inc_filename);
		zval_copy_ctor(tmp_inc_filename);
		convert_to_string(tmp_inc_filename);
		inc_filename = tmp_inc_filename;
	}

	if (opline->extended_value != ZEND_EVAL && strlen(Z_STRVAL_P(inc_filename)) != Z_STRLEN_P(inc_filename)) {
		if (opline->extended_value == ZEND_INCLUDE_ONCE || opline->extended_value == ZEND_INCLUDE) {
			zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, Z_STRVAL_P(inc_filename) TSRMLS_CC);
		} else {
			zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, Z_STRVAL_P(inc_filename) TSRMLS_CC);
		}
	} else {
		switch (opline->extended_value) {
			case ZEND_INCLUDE_ONCE:
			case ZEND_REQUIRE_ONCE: {
					zend_file_handle file_handle;
					char *resolved_path;

					resolved_path = zend_resolve_path(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename) TSRMLS_CC);
					if (resolved_path) {
						failure_retval = zend_hash_exists(&EG(included_files), resolved_path, strlen(resolved_path)+1);
					} else {
						resolved_path = Z_STRVAL_P(inc_filename);
					}

					if (failure_retval) {
						/* do nothing, file already included */
					} else if (SUCCESS == zend_stream_open(resolved_path, &file_handle TSRMLS_CC)) {

						if (!file_handle.opened_path) {
							file_handle.opened_path = estrdup(resolved_path);
						}

						if (zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1)==SUCCESS) {
							new_op_array = zend_compile_file(&file_handle, (opline->extended_value==ZEND_INCLUDE_ONCE?ZEND_INCLUDE:ZEND_REQUIRE) TSRMLS_CC);
							zend_destroy_file_handle(&file_handle TSRMLS_CC);
						} else {
							zend_file_handle_dtor(&file_handle TSRMLS_CC);
							failure_retval=1;
						}
					} else {
						if (opline->extended_value == ZEND_INCLUDE_ONCE) {
							zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, Z_STRVAL_P(inc_filename) TSRMLS_CC);
						} else {
							zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, Z_STRVAL_P(inc_filename) TSRMLS_CC);
						}
					}
					if (resolved_path != Z_STRVAL_P(inc_filename)) {
						efree(resolved_path);
					}
				}
				break;
			case ZEND_INCLUDE:
			case ZEND_REQUIRE:
				new_op_array = compile_filename(opline->extended_value, inc_filename TSRMLS_CC);
				break;
			case ZEND_EVAL: {
					char *eval_desc = zend_make_compiled_string_description("eval()'d code" TSRMLS_CC);

					new_op_array = zend_compile_string(inc_filename, eval_desc TSRMLS_CC);
					efree(eval_desc);
				}
				break;
			EMPTY_SWITCH_DEFAULT_CASE()
		}
	}
	if (tmp_inc_filename) {
		zval_ptr_dtor(&tmp_inc_filename);
	}
	FREE_OP1();
	if (UNEXPECTED(EG(exception) != NULL)) {
		HANDLE_EXCEPTION();
	} else if (EXPECTED(new_op_array != NULL)) {
		EX(original_return_value) = EG(return_value_ptr_ptr);
		EG(active_op_array) = new_op_array;
		if (RETURN_VALUE_USED(opline)) {
			EX_T(opline->result.var).var.ptr = NULL;
			EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
			EG(return_value_ptr_ptr) = EX_T(opline->result.var).var.ptr_ptr;
		} else {
			EG(return_value_ptr_ptr) = NULL;
		}

		EX(function_state).function = (zend_function *) new_op_array;
		EX(object) = NULL;

		if (!EG(active_symbol_table)) {
			zend_rebuild_symbol_table(TSRMLS_C);
		}

		if (EXPECTED(zend_execute_ex == execute_ex)) {
			ZEND_VM_ENTER();
		} else {
			zend_execute(new_op_array TSRMLS_CC);
		}

		EX(function_state).function = (zend_function *) EX(op_array);

		EG(opline_ptr) = &EX(opline);
		EG(active_op_array) = EX(op_array);
		EG(return_value_ptr_ptr) = EX(original_return_value);
		destroy_op_array(new_op_array TSRMLS_CC);
		efree(new_op_array);
		if (UNEXPECTED(EG(exception) != NULL)) {
			zend_throw_exception_internal(NULL TSRMLS_CC);
			HANDLE_EXCEPTION();
		}

	} else if (RETURN_VALUE_USED(opline)) {
		zval *retval;

		ALLOC_ZVAL(retval);
		ZVAL_BOOL(retval, failure_retval);
		INIT_PZVAL(retval);
		AI_SET_PTR(&EX_T(opline->result.var), retval);
	}
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(74, ZEND_UNSET_VAR, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	USE_OPLINE
	zval tmp, *varname;
	HashTable *target_symbol_table;
	zend_free_op free_op1;

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CV &&
	    OP2_TYPE == IS_UNUSED &&
	    (opline->extended_value & ZEND_QUICK_SET)) {
		if (EG(active_symbol_table)) {
			zend_compiled_variable *cv = &CV_DEF_OF(opline->op1.var);

			zend_delete_variable(EX(prev_execute_data), EG(active_symbol_table),  cv->name, cv->name_len+1, cv->hash_value TSRMLS_CC);
			EX_CV(opline->op1.var) = NULL;
		} else if (EX_CV(opline->op1.var)) {
			zval_ptr_dtor(EX_CV(opline->op1.var));
			EX_CV(opline->op1.var) = NULL;
		}
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}

	varname = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE != IS_CONST && Z_TYPE_P(varname) != IS_STRING) {
		ZVAL_COPY_VALUE(&tmp, varname);
		zval_copy_ctor(&tmp);
		convert_to_string(&tmp);
		varname = &tmp;
	} else if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
		Z_ADDREF_P(varname);
	}

	if (OP2_TYPE != IS_UNUSED) {
		zend_class_entry *ce;

		if (OP2_TYPE == IS_CONST) {
			if (CACHED_PTR(opline->op2.literal->cache_slot)) {
				ce = CACHED_PTR(opline->op2.literal->cache_slot);
			} else {
				ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv), opline->op2.literal + 1, 0 TSRMLS_CC);
				if (UNEXPECTED(EG(exception) != NULL)) {
					if (OP1_TYPE != IS_CONST && varname == &tmp) {
						zval_dtor(&tmp);
					} else if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
						zval_ptr_dtor(&varname);
					}
					FREE_OP1();
					HANDLE_EXCEPTION();
				}
				if (UNEXPECTED(ce == NULL)) {
					zend_error_noreturn(E_ERROR, "Class '%s' not found", Z_STRVAL_P(opline->op2.zv));
				}
				CACHE_PTR(opline->op2.literal->cache_slot, ce);
			}
		} else {
			ce = EX_T(opline->op2.var).class_entry;
		}
		zend_std_unset_static_property(ce, Z_STRVAL_P(varname), Z_STRLEN_P(varname), ((OP1_TYPE == IS_CONST) ? opline->op1.literal : NULL) TSRMLS_CC);
	} else {
		ulong hash_value = zend_inline_hash_func(varname->value.str.val, varname->value.str.len+1);

		target_symbol_table = zend_get_target_symbol_table(opline->extended_value & ZEND_FETCH_TYPE_MASK TSRMLS_CC);
		zend_delete_variable(execute_data, target_symbol_table, varname->value.str.val, varname->value.str.len+1, hash_value TSRMLS_CC);
	}

	if (OP1_TYPE != IS_CONST && varname == &tmp) {
		zval_dtor(&tmp);
	} else if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
		zval_ptr_dtor(&varname);
	}
	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(75, ZEND_UNSET_DIM, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **container;
	zval *offset;
	ulong hval;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_UNSET);
	if (OP1_TYPE == IS_CV && container != &EG(uninitialized_zval_ptr)) {
		SEPARATE_ZVAL_IF_NOT_REF(container);
	}
	offset = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE != IS_VAR || container) {
		switch (Z_TYPE_PP(container)) {
			case IS_ARRAY: {
				HashTable *ht = Z_ARRVAL_PP(container);

				switch (Z_TYPE_P(offset)) {
					case IS_DOUBLE:
						hval = zend_dval_to_lval(Z_DVAL_P(offset));
						zend_hash_index_del(ht, hval);
						break;
					case IS_RESOURCE:
					case IS_BOOL:
					case IS_LONG:
						hval = Z_LVAL_P(offset);
						zend_hash_index_del(ht, hval);
						break;
					case IS_STRING:
						if (OP2_TYPE == IS_CV || OP2_TYPE == IS_VAR) {
							Z_ADDREF_P(offset);
						}
						if (OP2_TYPE == IS_CONST) {
							hval = Z_HASH_P(offset);
						} else {
							ZEND_HANDLE_NUMERIC_EX(Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1, hval, ZEND_VM_C_GOTO(num_index_dim));
							if (IS_INTERNED(Z_STRVAL_P(offset))) {
								hval = INTERNED_HASH(Z_STRVAL_P(offset));
							} else {
								hval = zend_hash_func(Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1);
							}
						}
						if (ht == &EG(symbol_table)) {
							zend_delete_global_variable_ex(offset->value.str.val, offset->value.str.len, hval TSRMLS_CC);
						} else {
							zend_hash_quick_del(ht, Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1, hval);
						}
						if (OP2_TYPE == IS_CV || OP2_TYPE == IS_VAR) {
							zval_ptr_dtor(&offset);
						}
						break;
ZEND_VM_C_LABEL(num_index_dim):
						zend_hash_index_del(ht, hval);
						if (OP2_TYPE == IS_CV || OP2_TYPE == IS_VAR) {
							zval_ptr_dtor(&offset);
						}
						break;
					case IS_NULL:
						zend_hash_del(ht, "", sizeof(""));
						break;
					default:
						zend_error(E_WARNING, "Illegal offset type in unset");
						break;
				}
				FREE_OP2();
				break;
			}
			case IS_OBJECT:
				if (UNEXPECTED(Z_OBJ_HT_P(*container)->unset_dimension == NULL)) {
					zend_error_noreturn(E_ERROR, "Cannot use object as array");
				}
				if (IS_OP2_TMP_FREE()) {
					MAKE_REAL_ZVAL_PTR(offset);
				}
				Z_OBJ_HT_P(*container)->unset_dimension(*container, offset TSRMLS_CC);
				if (IS_OP2_TMP_FREE()) {
					zval_ptr_dtor(&offset);
				} else {
					FREE_OP2();
				}
				break;
			case IS_STRING:
				zend_error_noreturn(E_ERROR, "Cannot unset string offsets");
				ZEND_VM_CONTINUE(); /* bailed out before */
			default:
				FREE_OP2();
				break;
		}
	} else {
		FREE_OP2();
	}
	FREE_OP1_VAR_PTR();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(76, ZEND_UNSET_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval **container;
	zval *offset;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR_PTR(BP_VAR_UNSET);
	offset = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE != IS_VAR || container) {
		if (OP1_TYPE == IS_CV && container != &EG(uninitialized_zval_ptr)) {
			SEPARATE_ZVAL_IF_NOT_REF(container);
		}
		if (Z_TYPE_PP(container) == IS_OBJECT) {
			if (IS_OP2_TMP_FREE()) {
				MAKE_REAL_ZVAL_PTR(offset);
			}
			if (Z_OBJ_HT_P(*container)->unset_property) {
				Z_OBJ_HT_P(*container)->unset_property(*container, offset, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
			} else {
				zend_error(E_NOTICE, "Trying to unset property of non-object");
			}
			if (IS_OP2_TMP_FREE()) {
				zval_ptr_dtor(&offset);
			} else {
				FREE_OP2();
			}
		} else {
			FREE_OP2();
		}
	} else {
		FREE_OP2();
	}
	FREE_OP1_VAR_PTR();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(77, ZEND_FE_RESET, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *array_ptr, **array_ptr_ptr;
	HashTable *fe_ht;
	zend_object_iterator *iter = NULL;
	zend_class_entry *ce = NULL;
	zend_bool is_empty = 0;

	SAVE_OPLINE();

	if ((OP1_TYPE == IS_CV || OP1_TYPE == IS_VAR) &&
	    (opline->extended_value & ZEND_FE_RESET_VARIABLE)) {
		array_ptr_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_R);
		if (array_ptr_ptr == NULL || array_ptr_ptr == &EG(uninitialized_zval_ptr)) {
			MAKE_STD_ZVAL(array_ptr);
			ZVAL_NULL(array_ptr);
		} else if (Z_TYPE_PP(array_ptr_ptr) == IS_OBJECT) {
			if(Z_OBJ_HT_PP(array_ptr_ptr)->get_class_entry == NULL) {
				zend_error(E_WARNING, "foreach() cannot iterate over objects without PHP class");
				ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);
			}

			ce = Z_OBJCE_PP(array_ptr_ptr);
			if (!ce || ce->get_iterator == NULL) {
				SEPARATE_ZVAL_IF_NOT_REF(array_ptr_ptr);
				Z_ADDREF_PP(array_ptr_ptr);
			}
			array_ptr = *array_ptr_ptr;
		} else {
			if (Z_TYPE_PP(array_ptr_ptr) == IS_ARRAY) {
				SEPARATE_ZVAL_IF_NOT_REF(array_ptr_ptr);
				if (opline->extended_value & ZEND_FE_FETCH_BYREF) {
					Z_SET_ISREF_PP(array_ptr_ptr);
				}
			}
			array_ptr = *array_ptr_ptr;
			Z_ADDREF_P(array_ptr);
		}
	} else {
		array_ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);
		if (IS_OP1_TMP_FREE()) { /* IS_TMP_VAR */
			zval *tmp;

			ALLOC_ZVAL(tmp);
			INIT_PZVAL_COPY(tmp, array_ptr);
			array_ptr = tmp;
			if (Z_TYPE_P(array_ptr) == IS_OBJECT) {
				ce = Z_OBJCE_P(array_ptr);
				if (ce && ce->get_iterator) {
					Z_DELREF_P(array_ptr);
				}
			}
		} else if (Z_TYPE_P(array_ptr) == IS_OBJECT) {
			ce = Z_OBJCE_P(array_ptr);
			if (!ce || !ce->get_iterator) {
				Z_ADDREF_P(array_ptr);
			}
		} else if (OP1_TYPE == IS_CONST ||
		           ((OP1_TYPE == IS_CV || OP1_TYPE == IS_VAR) &&
		            !Z_ISREF_P(array_ptr) &&
		            Z_REFCOUNT_P(array_ptr) > 1)) {
			zval *tmp;

			ALLOC_ZVAL(tmp);
			INIT_PZVAL_COPY(tmp, array_ptr);
			zval_copy_ctor(tmp);
			array_ptr = tmp;
		} else {
			Z_ADDREF_P(array_ptr);
		}
	}

	if (ce && ce->get_iterator) {
		iter = ce->get_iterator(ce, array_ptr, opline->extended_value & ZEND_FE_RESET_REFERENCE TSRMLS_CC);

		if (iter && EXPECTED(EG(exception) == NULL)) {
			array_ptr = zend_iterator_wrap(iter TSRMLS_CC);
		} else {
			FREE_OP1_IF_VAR();
			if (!EG(exception)) {
				zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Object of type %s did not create an Iterator", ce->name);
			}
			zend_throw_exception_internal(NULL TSRMLS_CC);
			HANDLE_EXCEPTION();
		}
	}

	EX_T(opline->result.var).fe.ptr = array_ptr;

	if (iter) {
		iter->index = 0;
		if (iter->funcs->rewind) {
			iter->funcs->rewind(iter TSRMLS_CC);
			if (UNEXPECTED(EG(exception) != NULL)) {
				zval_ptr_dtor(&array_ptr);
				FREE_OP1_IF_VAR();
				HANDLE_EXCEPTION();
			}
		}
		is_empty = iter->funcs->valid(iter TSRMLS_CC) != SUCCESS;
		if (UNEXPECTED(EG(exception) != NULL)) {
			zval_ptr_dtor(&array_ptr);
			FREE_OP1_IF_VAR();
			HANDLE_EXCEPTION();
		}
		iter->index = -1; /* will be set to 0 before using next handler */
	} else if ((fe_ht = HASH_OF(array_ptr)) != NULL) {
		zend_hash_internal_pointer_reset(fe_ht);
		if (ce) {
			zend_object *zobj = zend_objects_get_address(array_ptr TSRMLS_CC);
			while (zend_hash_has_more_elements(fe_ht) == SUCCESS) {
				char *str_key;
				uint str_key_len;
				ulong int_key;
				zend_uchar key_type;

				key_type = zend_hash_get_current_key_ex(fe_ht, &str_key, &str_key_len, &int_key, 0, NULL);
				if (key_type != HASH_KEY_NON_EXISTENT &&
					(key_type == HASH_KEY_IS_LONG ||
				     zend_check_property_access(zobj, str_key, str_key_len-1 TSRMLS_CC) == SUCCESS)) {
					break;
				}
				zend_hash_move_forward(fe_ht);
			}
		}
		is_empty = zend_hash_has_more_elements(fe_ht) != SUCCESS;
		zend_hash_get_pointer(fe_ht, &EX_T(opline->result.var).fe.fe_pos);
	} else {
		zend_error(E_WARNING, "Invalid argument supplied for foreach()");
		is_empty = 1;
	}

	FREE_OP1_IF_VAR();
	if (is_empty) {
		ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);
	} else {
		CHECK_EXCEPTION();
		ZEND_VM_NEXT_OPCODE();
	}
}

ZEND_VM_HANDLER(78, ZEND_FE_FETCH, VAR, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *array = EX_T(opline->op1.var).fe.ptr;
	zval **value;
	HashTable *fe_ht;
	zend_object_iterator *iter = NULL;

	zval *key = NULL;
	if (opline->extended_value & ZEND_FE_FETCH_WITH_KEY) {
		key = &EX_T((opline+1)->result.var).tmp_var;
	}

	SAVE_OPLINE();

	switch (zend_iterator_unwrap(array, &iter TSRMLS_CC)) {
		default:
		case ZEND_ITER_INVALID:
			zend_error(E_WARNING, "Invalid argument supplied for foreach()");
			ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);

		case ZEND_ITER_PLAIN_OBJECT: {
			zend_object *zobj = zend_objects_get_address(array TSRMLS_CC);
			int key_type;
			char *str_key;
			zend_uint str_key_len;
			zend_ulong int_key;

			fe_ht = Z_OBJPROP_P(array);
			zend_hash_set_pointer(fe_ht, &EX_T(opline->op1.var).fe.fe_pos);
			do {
				if (zend_hash_get_current_data(fe_ht, (void **) &value)==FAILURE) {
					/* reached end of iteration */
					ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);
				}
				key_type = zend_hash_get_current_key_ex(fe_ht, &str_key, &str_key_len, &int_key, 0, NULL);

				zend_hash_move_forward(fe_ht);
			} while (key_type != HASH_KEY_IS_LONG &&
			         zend_check_property_access(zobj, str_key, str_key_len - 1 TSRMLS_CC) != SUCCESS);

			if (key) {
				if (key_type == HASH_KEY_IS_LONG) {
					ZVAL_LONG(key, int_key);
				} else {
					const char *class_name, *prop_name;
					int prop_name_len;
					zend_unmangle_property_name_ex(
						str_key, str_key_len - 1, &class_name, &prop_name, &prop_name_len
					);
					ZVAL_STRINGL(key, prop_name, prop_name_len, 1);
				}
			}

			zend_hash_get_pointer(fe_ht, &EX_T(opline->op1.var).fe.fe_pos);
			break;
		}

		case ZEND_ITER_PLAIN_ARRAY:
			fe_ht = Z_ARRVAL_P(array);
			zend_hash_set_pointer(fe_ht, &EX_T(opline->op1.var).fe.fe_pos);
			if (zend_hash_get_current_data(fe_ht, (void **) &value)==FAILURE) {
				/* reached end of iteration */
				ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);
			}
			if (key) {
				zend_hash_get_current_key_zval(fe_ht, key);
			}
			zend_hash_move_forward(fe_ht);
			zend_hash_get_pointer(fe_ht, &EX_T(opline->op1.var).fe.fe_pos);
			break;

		case ZEND_ITER_OBJECT:
			/* !iter happens from exception */
			if (iter && ++iter->index > 0) {
				/* This could cause an endless loop if index becomes zero again.
				 * In case that ever happens we need an additional flag. */
				iter->funcs->move_forward(iter TSRMLS_CC);
				if (UNEXPECTED(EG(exception) != NULL)) {
					zval_ptr_dtor(&array);
					HANDLE_EXCEPTION();
				}
			}
			/* If index is zero we come from FE_RESET and checked valid() already. */
			if (!iter || (iter->index > 0 && iter->funcs->valid(iter TSRMLS_CC) == FAILURE)) {
				/* reached end of iteration */
				if (UNEXPECTED(EG(exception) != NULL)) {
					zval_ptr_dtor(&array);
					HANDLE_EXCEPTION();
				}
				ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);
			}
			iter->funcs->get_current_data(iter, &value TSRMLS_CC);
			if (UNEXPECTED(EG(exception) != NULL)) {
				zval_ptr_dtor(&array);
				HANDLE_EXCEPTION();
			}
			if (!value) {
				/* failure in get_current_data */
				ZEND_VM_JMP(EX(op_array)->opcodes+opline->op2.opline_num);
			}
			if (key) {
				if (iter->funcs->get_current_key) {
					iter->funcs->get_current_key(iter, key TSRMLS_CC);
					if (UNEXPECTED(EG(exception) != NULL)) {
						zval_ptr_dtor(&array);
						HANDLE_EXCEPTION();
					}
				} else {
					ZVAL_LONG(key, iter->index);
				}
			}
			break;
	}

	if (opline->extended_value & ZEND_FE_FETCH_BYREF) {
		SEPARATE_ZVAL_IF_NOT_REF(value);
		Z_SET_ISREF_PP(value);
		EX_T(opline->result.var).var.ptr_ptr = value;
		Z_ADDREF_PP(value);
	} else {
		PZVAL_LOCK(*value);
		AI_SET_PTR(&EX_T(opline->result.var), *value);
	}

	CHECK_EXCEPTION();
	ZEND_VM_INC_OPCODE();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(114, ZEND_ISSET_ISEMPTY_VAR, CONST|TMP|VAR|CV, UNUSED|CONST|VAR)
{
	USE_OPLINE
	zval **value;
	zend_bool isset = 1;

	SAVE_OPLINE();
	if (OP1_TYPE == IS_CV &&
	    OP2_TYPE == IS_UNUSED &&
	    (opline->extended_value & ZEND_QUICK_SET)) {
		if (EX_CV(opline->op1.var)) {
			value = EX_CV(opline->op1.var);
		} else if (EG(active_symbol_table)) {
			zend_compiled_variable *cv = &CV_DEF_OF(opline->op1.var);

			if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void **) &value) == FAILURE) {
				isset = 0;
			}
		} else {
			isset = 0;
		}
	} else {
		HashTable *target_symbol_table;
		zend_free_op free_op1;
		zval tmp, *varname = GET_OP1_ZVAL_PTR(BP_VAR_IS);

		if (OP1_TYPE != IS_CONST && Z_TYPE_P(varname) != IS_STRING) {
			ZVAL_COPY_VALUE(&tmp, varname);
			zval_copy_ctor(&tmp);
			convert_to_string(&tmp);
			varname = &tmp;
		}

		if (OP2_TYPE != IS_UNUSED) {
			zend_class_entry *ce;

			if (OP2_TYPE == IS_CONST) {
				if (CACHED_PTR(opline->op2.literal->cache_slot)) {
					ce = CACHED_PTR(opline->op2.literal->cache_slot);
				} else {
					ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv), opline->op2.literal + 1, 0 TSRMLS_CC);
					if (UNEXPECTED(ce == NULL)) {
						CHECK_EXCEPTION();
						ZEND_VM_NEXT_OPCODE();
					}
					CACHE_PTR(opline->op2.literal->cache_slot, ce);
				}
			} else {
				ce = EX_T(opline->op2.var).class_entry;
			}
			value = zend_std_get_static_property(ce, Z_STRVAL_P(varname), Z_STRLEN_P(varname), 1, ((OP1_TYPE == IS_CONST) ? opline->op1.literal : NULL) TSRMLS_CC);
			if (!value) {
				isset = 0;
			}
		} else {
			target_symbol_table = zend_get_target_symbol_table(opline->extended_value & ZEND_FETCH_TYPE_MASK TSRMLS_CC);
			if (zend_hash_find(target_symbol_table, varname->value.str.val, varname->value.str.len+1, (void **) &value) == FAILURE) {
				isset = 0;
			}
		}

		if (OP1_TYPE != IS_CONST && varname == &tmp) {
			zval_dtor(&tmp);
		}
		FREE_OP1();
	}

	if (opline->extended_value & ZEND_ISSET) {
		if (isset && Z_TYPE_PP(value) != IS_NULL) {
			ZVAL_BOOL(&EX_T(opline->result.var).tmp_var, 1);
		} else {
			ZVAL_BOOL(&EX_T(opline->result.var).tmp_var, 0);
		}
	} else /* if (opline->extended_value & ZEND_ISEMPTY) */ {
		if (!isset || !i_zend_is_true(*value)) {
			ZVAL_BOOL(&EX_T(opline->result.var).tmp_var, 1);
		} else {
			ZVAL_BOOL(&EX_T(opline->result.var).tmp_var, 0);
		}
	}

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HELPER_EX(zend_isset_isempty_dim_prop_obj_handler, VAR|UNUSED|CV, CONST|TMP|VAR|CV, int prop_dim)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *container;
	zval **value = NULL;
	int result = 0;
	ulong hval;
	zval *offset;

	SAVE_OPLINE();
	container = GET_OP1_OBJ_ZVAL_PTR(BP_VAR_IS);

	offset = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if (Z_TYPE_P(container) == IS_ARRAY && !prop_dim) {
		HashTable *ht;
		int isset = 0;

		ht = Z_ARRVAL_P(container);

		switch (Z_TYPE_P(offset)) {
			case IS_DOUBLE:
				hval = zend_dval_to_lval(Z_DVAL_P(offset));
				ZEND_VM_C_GOTO(num_index_prop);
			case IS_RESOURCE:
			case IS_BOOL:
			case IS_LONG:
				hval = Z_LVAL_P(offset);
ZEND_VM_C_LABEL(num_index_prop):
				if (zend_hash_index_find(ht, hval, (void **) &value) == SUCCESS) {
					isset = 1;
				}
				break;
			case IS_STRING:
				if (OP2_TYPE == IS_CONST) {
					hval = Z_HASH_P(offset);
				} else {
					ZEND_HANDLE_NUMERIC_EX(Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1, hval, ZEND_VM_C_GOTO(num_index_prop));
					if (IS_INTERNED(Z_STRVAL_P(offset))) {
						hval = INTERNED_HASH(Z_STRVAL_P(offset));
					} else {
						hval = zend_hash_func(Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1);
					}
				}
				if (zend_hash_quick_find(ht, Z_STRVAL_P(offset), Z_STRLEN_P(offset)+1, hval, (void **) &value) == SUCCESS) {
					isset = 1;
				}
				break;
			case IS_NULL:
				if (zend_hash_find(ht, "", sizeof(""), (void **) &value) == SUCCESS) {
					isset = 1;
				}
				break;
			default:
				zend_error(E_WARNING, "Illegal offset type in isset or empty");
				break;
		}

		if (opline->extended_value & ZEND_ISSET) {
			if (isset && Z_TYPE_PP(value) == IS_NULL) {
				result = 0;
			} else {
				result = isset;
			}
		} else /* if (opline->extended_value & ZEND_ISEMPTY) */ {
			if (!isset || !i_zend_is_true(*value)) {
				result = 0;
			} else {
				result = 1;
			}
		}
		FREE_OP2();
	} else if (Z_TYPE_P(container) == IS_OBJECT) {
		if (IS_OP2_TMP_FREE()) {
			MAKE_REAL_ZVAL_PTR(offset);
		}
		if (prop_dim) {
			if (Z_OBJ_HT_P(container)->has_property) {
				result = Z_OBJ_HT_P(container)->has_property(container, offset, (opline->extended_value & ZEND_ISEMPTY) != 0, ((OP2_TYPE == IS_CONST) ? opline->op2.literal : NULL) TSRMLS_CC);
			} else {
				zend_error(E_NOTICE, "Trying to check property of non-object");
				result = 0;
			}
		} else {
			if (Z_OBJ_HT_P(container)->has_dimension) {
				result = Z_OBJ_HT_P(container)->has_dimension(container, offset, (opline->extended_value & ZEND_ISEMPTY) != 0 TSRMLS_CC);
			} else {
				zend_error(E_NOTICE, "Trying to check element of non-array");
				result = 0;
			}
		}
		if (IS_OP2_TMP_FREE()) {
			zval_ptr_dtor(&offset);
		} else {
			FREE_OP2();
		}
	} else if (Z_TYPE_P(container) == IS_STRING && !prop_dim) { /* string offsets */
		zval tmp;

		if (Z_TYPE_P(offset) != IS_LONG) {
			if (Z_TYPE_P(offset) <= IS_BOOL /* simple scalar types */
					|| (Z_TYPE_P(offset) == IS_STRING /* or numeric string */
						&& IS_LONG == is_numeric_string(Z_STRVAL_P(offset), Z_STRLEN_P(offset), NULL, NULL, 0))) {
				ZVAL_COPY_VALUE(&tmp, offset);
				zval_copy_ctor(&tmp);
				convert_to_long(&tmp);
				offset = &tmp;
			} else {
				/* can not be converted to proper offset, return "not set" */
				result = 0;
			}
		}
		if (Z_TYPE_P(offset) == IS_LONG) {
			if (opline->extended_value & ZEND_ISSET) {
				if (offset->value.lval >= 0 && offset->value.lval < Z_STRLEN_P(container)) {
					result = 1;
				}
			} else /* if (opline->extended_value & ZEND_ISEMPTY) */ {
				if (offset->value.lval >= 0 && offset->value.lval < Z_STRLEN_P(container) && Z_STRVAL_P(container)[offset->value.lval] != '0') {
					result = 1;
				}
			}
		}
		FREE_OP2();
	} else {
		FREE_OP2();
	}

	Z_TYPE(EX_T(opline->result.var).tmp_var) = IS_BOOL;
	if (opline->extended_value & ZEND_ISSET) {
		Z_LVAL(EX_T(opline->result.var).tmp_var) = result;
	} else {
		Z_LVAL(EX_T(opline->result.var).tmp_var) = !result;
	}

	FREE_OP1_VAR_PTR();

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(115, ZEND_ISSET_ISEMPTY_DIM_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_isset_isempty_dim_prop_obj_handler, prop_dim, 0);
}

ZEND_VM_HANDLER(148, ZEND_ISSET_ISEMPTY_PROP_OBJ, VAR|UNUSED|CV, CONST|TMP|VAR|CV)
{
	ZEND_VM_DISPATCH_TO_HELPER_EX(zend_isset_isempty_dim_prop_obj_handler, prop_dim, 1);
}

ZEND_VM_HANDLER(79, ZEND_EXIT, CONST|TMP|VAR|UNUSED|CV, ANY)
{
#if !defined(ZEND_VM_SPEC) || (OP1_TYPE != IS_UNUSED)
	USE_OPLINE

	SAVE_OPLINE();
	if (OP1_TYPE != IS_UNUSED) {
		zend_free_op free_op1;
		zval *ptr = GET_OP1_ZVAL_PTR(BP_VAR_R);

		if (Z_TYPE_P(ptr) == IS_LONG) {
			EG(exit_status) = Z_LVAL_P(ptr);
		} else {
			zend_print_variable(ptr);
		}
		FREE_OP1();
	}
#endif
	zend_bailout();
	ZEND_VM_NEXT_OPCODE(); /* Never reached */
}

ZEND_VM_HANDLER(57, ZEND_BEGIN_SILENCE, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	Z_LVAL(EX_T(opline->result.var).tmp_var) = EG(error_reporting);
	Z_TYPE(EX_T(opline->result.var).tmp_var) = IS_LONG;  /* shouldn't be necessary */
	if (EX(old_error_reporting) == NULL) {
		EX(old_error_reporting) = &EX_T(opline->result.var).tmp_var;
	}

	if (EG(error_reporting)) {
		do {
			EG(error_reporting) = 0;
			if (!EG(error_reporting_ini_entry)) {
				if (UNEXPECTED(zend_hash_find(EG(ini_directives), "error_reporting", sizeof("error_reporting"), (void **) &EG(error_reporting_ini_entry)) == FAILURE)) {
					break;
				}
			}
			if (!EG(error_reporting_ini_entry)->modified) {
				if (!EG(modified_ini_directives)) {
					ALLOC_HASHTABLE(EG(modified_ini_directives));
					zend_hash_init(EG(modified_ini_directives), 8, NULL, NULL, 0);
				}
				if (EXPECTED(zend_hash_add(EG(modified_ini_directives), "error_reporting", sizeof("error_reporting"), &EG(error_reporting_ini_entry), sizeof(zend_ini_entry*), NULL) == SUCCESS)) {
					EG(error_reporting_ini_entry)->orig_value = EG(error_reporting_ini_entry)->value;
					EG(error_reporting_ini_entry)->orig_value_length = EG(error_reporting_ini_entry)->value_length;
					EG(error_reporting_ini_entry)->orig_modifiable = EG(error_reporting_ini_entry)->modifiable;
					EG(error_reporting_ini_entry)->modified = 1;
				}
			} else if (EG(error_reporting_ini_entry)->value != EG(error_reporting_ini_entry)->orig_value) {
				efree(EG(error_reporting_ini_entry)->value);
			}
			EG(error_reporting_ini_entry)->value = estrndup("0", sizeof("0")-1);
			EG(error_reporting_ini_entry)->value_length = sizeof("0")-1;
		} while (0);
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(142, ZEND_RAISE_ABSTRACT_ERROR, ANY, ANY)
{
	SAVE_OPLINE();
	zend_error_noreturn(E_ERROR, "Cannot call abstract method %s::%s()", EG(scope)->name, EX(op_array)->function_name);
	ZEND_VM_NEXT_OPCODE(); /* Never reached */
}

ZEND_VM_HANDLER(58, ZEND_END_SILENCE, TMP, ANY)
{
	USE_OPLINE
	zval restored_error_reporting;

	SAVE_OPLINE();
	if (!EG(error_reporting) && Z_LVAL(EX_T(opline->op1.var).tmp_var) != 0) {
		Z_TYPE(restored_error_reporting) = IS_LONG;
		Z_LVAL(restored_error_reporting) = Z_LVAL(EX_T(opline->op1.var).tmp_var);
		EG(error_reporting) = Z_LVAL(restored_error_reporting);
		convert_to_string(&restored_error_reporting);
		if (EXPECTED(EG(error_reporting_ini_entry) != NULL)) {
			if (EXPECTED(EG(error_reporting_ini_entry)->modified &&
			    EG(error_reporting_ini_entry)->value != EG(error_reporting_ini_entry)->orig_value)) {
				efree(EG(error_reporting_ini_entry)->value);
			}
			EG(error_reporting_ini_entry)->value = Z_STRVAL(restored_error_reporting);
			EG(error_reporting_ini_entry)->value_length = Z_STRLEN(restored_error_reporting);
		} else {
			zendi_zval_dtor(restored_error_reporting);
		}
	}
	if (EX(old_error_reporting) == &EX_T(opline->op1.var).tmp_var) {
		EX(old_error_reporting) = NULL;
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(152, ZEND_JMP_SET, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *value;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (i_zend_is_true(value)) {
		ZVAL_COPY_VALUE(&EX_T(opline->result.var).tmp_var, value);
		if (!IS_OP1_TMP_FREE()) {
			zendi_zval_copy_ctor(EX_T(opline->result.var).tmp_var);
		}
		FREE_OP1_IF_VAR();
#if DEBUG_ZEND>=2
		printf("Conditional jmp to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_JMP(opline->op2.jmp_addr);
	}

	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(158, ZEND_JMP_SET_VAR, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *value, *ret;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (i_zend_is_true(value)) {
		if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
			Z_ADDREF_P(value);
			EX_T(opline->result.var).var.ptr = value;
			EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
		} else {
			ALLOC_ZVAL(ret);
			INIT_PZVAL_COPY(ret, value);
			EX_T(opline->result.var).var.ptr = ret;
			EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
			if (!IS_OP1_TMP_FREE()) {
				zval_copy_ctor(EX_T(opline->result.var).var.ptr);
			}
		}
		FREE_OP1_IF_VAR();
#if DEBUG_ZEND>=2
		printf("Conditional jmp to %d\n", opline->op2.opline_num);
#endif
		ZEND_VM_JMP(opline->op2.jmp_addr);
	}

	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(22, ZEND_QM_ASSIGN, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *value;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);

	ZVAL_COPY_VALUE(&EX_T(opline->result.var).tmp_var, value);
	if (!IS_OP1_TMP_FREE()) {
		zval_copy_ctor(&EX_T(opline->result.var).tmp_var);
	}
	FREE_OP1_IF_VAR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(157, ZEND_QM_ASSIGN_VAR, CONST|TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *value, *ret;

	SAVE_OPLINE();
	value = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (OP1_TYPE == IS_VAR || OP1_TYPE == IS_CV) {
		Z_ADDREF_P(value);
		EX_T(opline->result.var).var.ptr = value;
		EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
	} else {
		ALLOC_ZVAL(ret);
		INIT_PZVAL_COPY(ret, value);
		EX_T(opline->result.var).var.ptr = ret;
		EX_T(opline->result.var).var.ptr_ptr = &EX_T(opline->result.var).var.ptr;
		if (!IS_OP1_TMP_FREE()) {
			zval_copy_ctor(EX_T(opline->result.var).var.ptr);
		}
	}

	FREE_OP1_IF_VAR();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(101, ZEND_EXT_STMT, ANY, ANY)
{
	SAVE_OPLINE();
	if (!EG(no_extensions)) {
		zend_llist_apply_with_argument(&zend_extensions, (llist_apply_with_arg_func_t) zend_extension_statement_handler, EX(op_array) TSRMLS_CC);
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(102, ZEND_EXT_FCALL_BEGIN, ANY, ANY)
{
	SAVE_OPLINE();
	if (!EG(no_extensions)) {
		zend_llist_apply_with_argument(&zend_extensions, (llist_apply_with_arg_func_t) zend_extension_fcall_begin_handler, EX(op_array) TSRMLS_CC);
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(103, ZEND_EXT_FCALL_END, ANY, ANY)
{
	SAVE_OPLINE();
	if (!EG(no_extensions)) {
		zend_llist_apply_with_argument(&zend_extensions, (llist_apply_with_arg_func_t) zend_extension_fcall_end_handler, EX(op_array) TSRMLS_CC);
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(139, ZEND_DECLARE_CLASS, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	EX_T(opline->result.var).class_entry = do_bind_class(EX(op_array), opline, EG(class_table), 0 TSRMLS_CC);
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(140, ZEND_DECLARE_INHERITED_CLASS, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	EX_T(opline->result.var).class_entry = do_bind_inherited_class(EX(op_array), opline, EG(class_table), EX_T(opline->extended_value).class_entry, 0 TSRMLS_CC);
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(145, ZEND_DECLARE_INHERITED_CLASS_DELAYED, ANY, ANY)
{
	USE_OPLINE
	zend_class_entry **pce, **pce_orig;

	SAVE_OPLINE();
	if (zend_hash_quick_find(EG(class_table), Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv)+1, Z_HASH_P(opline->op2.zv), (void**)&pce) == FAILURE ||
	    (zend_hash_quick_find(EG(class_table), Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), Z_HASH_P(opline->op1.zv), (void**)&pce_orig) == SUCCESS &&
	     *pce != *pce_orig)) {
		do_bind_inherited_class(EX(op_array), opline, EG(class_table), EX_T(opline->extended_value).class_entry, 0 TSRMLS_CC);
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(141, ZEND_DECLARE_FUNCTION, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	do_bind_function(EX(op_array), opline, EG(function_table), 0);
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(105, ZEND_TICKS, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	if (++EG(ticks_count)>=opline->extended_value) {
		EG(ticks_count)=0;
		if (zend_ticks_function) {
			zend_ticks_function(opline->extended_value);
		}
	}
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(138, ZEND_INSTANCEOF, TMP|VAR|CV, ANY)
{
	USE_OPLINE
	zend_free_op free_op1;
	zval *expr;
	zend_bool result;

	SAVE_OPLINE();
	expr = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (Z_TYPE_P(expr) == IS_OBJECT && Z_OBJ_HT_P(expr)->get_class_entry) {
		result = instanceof_function(Z_OBJCE_P(expr), EX_T(opline->op2.var).class_entry TSRMLS_CC);
	} else {
		result = 0;
	}
	ZVAL_BOOL(&EX_T(opline->result.var).tmp_var, result);
	FREE_OP1();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(104, ZEND_EXT_NOP, ANY, ANY)
{
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(0, ZEND_NOP, ANY, ANY)
{
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(144, ZEND_ADD_INTERFACE, ANY, CONST)
{
	USE_OPLINE
	zend_class_entry *ce = EX_T(opline->op1.var).class_entry;
	zend_class_entry *iface;

	SAVE_OPLINE();
	if (CACHED_PTR(opline->op2.literal->cache_slot)) {
		iface = CACHED_PTR(opline->op2.literal->cache_slot);
	} else {
		iface = zend_fetch_class_by_name(Z_STRVAL_P(opline->op2.zv), Z_STRLEN_P(opline->op2.zv), opline->op2.literal + 1, opline->extended_value TSRMLS_CC);
		if (UNEXPECTED(iface == NULL)) {
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE();
		}
		CACHE_PTR(opline->op2.literal->cache_slot, iface);
	}

	if (UNEXPECTED((iface->ce_flags & ZEND_ACC_INTERFACE) == 0)) {
		zend_error_noreturn(E_ERROR, "%s cannot implement %s - it is not an interface", ce->name, iface->name);
	}
	zend_do_implement_interface(ce, iface TSRMLS_CC);

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(154, ZEND_ADD_TRAIT, ANY, ANY)
{
	USE_OPLINE
	zend_class_entry *ce = EX_T(opline->op1.var).class_entry;
	zend_class_entry *trait;

	SAVE_OPLINE();
	if (CACHED_PTR(opline->op2.literal->cache_slot)) {
		trait = CACHED_PTR(opline->op2.literal->cache_slot);
	} else {
		trait = zend_fetch_class_by_name(Z_STRVAL_P(opline->op2.zv),
		                                 Z_STRLEN_P(opline->op2.zv),
		                                 opline->op2.literal + 1,
		                                 opline->extended_value TSRMLS_CC);
		if (UNEXPECTED(trait == NULL)) {
			CHECK_EXCEPTION();
			ZEND_VM_NEXT_OPCODE();
		}
		if (!((trait->ce_flags & ZEND_ACC_TRAIT) == ZEND_ACC_TRAIT)) {
			zend_error_noreturn(E_ERROR, "%s cannot use %s - it is not a trait", ce->name, trait->name);
		}
		CACHE_PTR(opline->op2.literal->cache_slot, trait);
	}

	zend_do_implement_trait(ce, trait TSRMLS_CC);

 	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(155, ZEND_BIND_TRAITS, ANY, ANY)
{
	USE_OPLINE
	zend_class_entry *ce = EX_T(opline->op1.var).class_entry;

	SAVE_OPLINE();
	zend_do_bind_traits(ce TSRMLS_CC);
 	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(149, ZEND_HANDLE_EXCEPTION, ANY, ANY)
{
	zend_uint op_num = EG(opline_before_exception)-EG(active_op_array)->opcodes;
	int i;
	zend_uint catch_op_num = 0, finally_op_num = 0;
	void **stack_frame;

	/* Figure out where the next stack frame (which maybe contains pushed
	 * arguments that have to be dtor'ed) starts */
	stack_frame = zend_vm_stack_frame_base(execute_data);

	/* If the exception was thrown during a function call there might be
	 * arguments pushed to the stack that have to be dtor'ed. */
	while (zend_vm_stack_top(TSRMLS_C) != stack_frame) {
		zval *stack_zval_p = zend_vm_stack_pop(TSRMLS_C);
		zval_ptr_dtor(&stack_zval_p);
	}

	for (i=0; i<EG(active_op_array)->last_try_catch; i++) {
		if (EG(active_op_array)->try_catch_array[i].try_op > op_num) {
			/* further blocks will not be relevant... */
			break;
		}
		if (op_num < EG(active_op_array)->try_catch_array[i].catch_op) {
			catch_op_num = EX(op_array)->try_catch_array[i].catch_op;
		}
		if (op_num < EG(active_op_array)->try_catch_array[i].finally_op) {
			finally_op_num = EX(op_array)->try_catch_array[i].finally_op;
		}
	}

	if (EX(call) >= EX(call_slots)) {
		call_slot *call = EX(call);
		do {
			if (call->object) {
				if (call->is_ctor_call) {
					if (call->is_ctor_result_used) {
						Z_DELREF_P(call->object);
					}
					if (Z_REFCOUNT_P(call->object) == 1) {
						zend_object_store_ctor_failed(call->object TSRMLS_CC);
					}
				}
				zval_ptr_dtor(&call->object);
			}
			call--;
		} while (call >= EX(call_slots));
		EX(call) = NULL;
	}

	for (i=0; i<EX(op_array)->last_brk_cont; i++) {
		if (EX(op_array)->brk_cont_array[i].start < 0) {
			continue;
		} else if (EX(op_array)->brk_cont_array[i].start > op_num) {
			/* further blocks will not be relevant... */
			break;
		} else if (op_num < EX(op_array)->brk_cont_array[i].brk) {
			if (!catch_op_num ||
			    catch_op_num >= EX(op_array)->brk_cont_array[i].brk) {
				zend_op *brk_opline = &EX(op_array)->opcodes[EX(op_array)->brk_cont_array[i].brk];

				switch (brk_opline->opcode) {
					case ZEND_SWITCH_FREE:
						if (!(brk_opline->extended_value & EXT_TYPE_FREE_ON_RETURN)) {
							zval_ptr_dtor(&EX_T(brk_opline->op1.var).var.ptr);
						}
						break;
					case ZEND_FREE:
						if (!(brk_opline->extended_value & EXT_TYPE_FREE_ON_RETURN)) {
							zendi_zval_dtor(EX_T(brk_opline->op1.var).tmp_var);
						}
						break;
				}
			}
		}
	}

	/* restore previous error_reporting value */
	if (!EG(error_reporting) && EX(old_error_reporting) != NULL && Z_LVAL_P(EX(old_error_reporting)) != 0) {
		zval restored_error_reporting;

		Z_TYPE(restored_error_reporting) = IS_LONG;
		Z_LVAL(restored_error_reporting) = Z_LVAL_P(EX(old_error_reporting));
		convert_to_string(&restored_error_reporting);
		zend_alter_ini_entry_ex("error_reporting", sizeof("error_reporting"), Z_STRVAL(restored_error_reporting), Z_STRLEN(restored_error_reporting), ZEND_INI_USER, ZEND_INI_STAGE_RUNTIME, 1 TSRMLS_CC);
		zendi_zval_dtor(restored_error_reporting);
	}
	EX(old_error_reporting) = NULL;

	if (finally_op_num && (!catch_op_num || catch_op_num >= finally_op_num)) {
		zend_exception_save(TSRMLS_C);
		EX(fast_ret) = NULL;
		ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[finally_op_num]);
		ZEND_VM_CONTINUE();
	} else if (catch_op_num) {
		ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[catch_op_num]);
		ZEND_VM_CONTINUE();
	} else {
		if (UNEXPECTED((EX(op_array)->fn_flags & ZEND_ACC_GENERATOR) != 0)) {
			ZEND_VM_DISPATCH_TO_HANDLER(ZEND_GENERATOR_RETURN);
		} else {
			ZEND_VM_DISPATCH_TO_HELPER(zend_leave_helper);
		}
	}
}

ZEND_VM_HANDLER(146, ZEND_VERIFY_ABSTRACT_CLASS, ANY, ANY)
{
	USE_OPLINE

	SAVE_OPLINE();
	zend_verify_abstract_class(EX_T(opline->op1.var).class_entry TSRMLS_CC);
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(150, ZEND_USER_OPCODE, ANY, ANY)
{
	USE_OPLINE
	int ret;

	SAVE_OPLINE();
	ret = zend_user_opcode_handlers[opline->opcode](ZEND_OPCODE_HANDLER_ARGS_PASSTHRU_INTERNAL);
	LOAD_OPLINE();

	switch (ret) {
		case ZEND_USER_OPCODE_CONTINUE:
			ZEND_VM_CONTINUE();
		case ZEND_USER_OPCODE_RETURN:
			if (UNEXPECTED((EX(op_array)->fn_flags & ZEND_ACC_GENERATOR) != 0)) {
				ZEND_VM_DISPATCH_TO_HANDLER(ZEND_GENERATOR_RETURN);
			} else {
				ZEND_VM_DISPATCH_TO_HELPER(zend_leave_helper);
			}
		case ZEND_USER_OPCODE_ENTER:
			ZEND_VM_ENTER();
		case ZEND_USER_OPCODE_LEAVE:
			ZEND_VM_LEAVE();
		case ZEND_USER_OPCODE_DISPATCH:
			ZEND_VM_DISPATCH(opline->opcode, opline);
		default:
			ZEND_VM_DISPATCH((zend_uchar)(ret & 0xff), opline);
	}
}

ZEND_VM_HANDLER(143, ZEND_DECLARE_CONST, CONST, CONST)
{
	USE_OPLINE
	zend_free_op free_op1, free_op2;
	zval *name;
	zval *val;
	zend_constant c;

	SAVE_OPLINE();
	name  = GET_OP1_ZVAL_PTR(BP_VAR_R);
	val   = GET_OP2_ZVAL_PTR(BP_VAR_R);

	if ((Z_TYPE_P(val) & IS_CONSTANT_TYPE_MASK) == IS_CONSTANT || Z_TYPE_P(val) == IS_CONSTANT_ARRAY) {
		zval tmp;
		zval *tmp_ptr = &tmp;

		ZVAL_COPY_VALUE(&tmp, val);
		if (Z_TYPE_P(val) == IS_CONSTANT_ARRAY) {
			zval_copy_ctor(&tmp);
		}
		INIT_PZVAL(&tmp);
		zval_update_constant(&tmp_ptr, NULL TSRMLS_CC);
		c.value = *tmp_ptr;
	} else {
		INIT_PZVAL_COPY(&c.value, val);
		zval_copy_ctor(&c.value);
	}
	c.flags = CONST_CS; /* non persistent, case sensetive */
	c.name = IS_INTERNED(Z_STRVAL_P(name)) ? Z_STRVAL_P(name) : zend_strndup(Z_STRVAL_P(name), Z_STRLEN_P(name));
	c.name_len = Z_STRLEN_P(name)+1;
	c.module_number = PHP_USER_CONSTANT;

	if (zend_register_constant(&c TSRMLS_CC) == FAILURE) {
	}

	FREE_OP1();
	FREE_OP2();
	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(153, ZEND_DECLARE_LAMBDA_FUNCTION, CONST, UNUSED)
{
	USE_OPLINE
	zend_function *op_array;
	int closure_is_static, closure_is_being_defined_inside_static_context;

	SAVE_OPLINE();

	if (UNEXPECTED(zend_hash_quick_find(EG(function_table), Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), Z_HASH_P(opline->op1.zv), (void *) &op_array) == FAILURE) ||
	    UNEXPECTED(op_array->type != ZEND_USER_FUNCTION)) {
		zend_error_noreturn(E_ERROR, "Base lambda function for closure not found");
	}

	closure_is_static = op_array->common.fn_flags & ZEND_ACC_STATIC;
	closure_is_being_defined_inside_static_context = EX(prev_execute_data) && EX(prev_execute_data)->function_state.function->common.fn_flags & ZEND_ACC_STATIC;
	if (closure_is_static || closure_is_being_defined_inside_static_context) {
		zend_create_closure(&EX_T(opline->result.var).tmp_var, (zend_function *) op_array,  EG(called_scope), NULL TSRMLS_CC);
	} else {
		zend_create_closure(&EX_T(opline->result.var).tmp_var, (zend_function *) op_array,  EG(scope), EG(This) TSRMLS_CC);
	}

	CHECK_EXCEPTION();
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(156, ZEND_SEPARATE, VAR, UNUSED)
{
	USE_OPLINE
	zval *var_ptr, *new_zv;

	SAVE_OPLINE();
	var_ptr = EX_T(opline->op1.var).var.ptr;
	if (Z_TYPE_P(var_ptr) != IS_OBJECT &&
			!PZVAL_IS_REF(var_ptr) &&
			Z_REFCOUNT_P(var_ptr) > 1) {

		Z_DELREF_P(var_ptr);
		ALLOC_ZVAL(new_zv);
		INIT_PZVAL_COPY(new_zv, var_ptr);
		var_ptr = new_zv;
		zval_copy_ctor(var_ptr);
		EX_T(opline->op1.var).var.ptr = var_ptr;
	}
	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(160, ZEND_YIELD, CONST|TMP|VAR|CV|UNUSED, CONST|TMP|VAR|CV|UNUSED)
{
	USE_OPLINE

	/* The generator object is stored in return_value_ptr_ptr */
	zend_generator *generator = (zend_generator *) EG(return_value_ptr_ptr);

	if (generator->flags & ZEND_GENERATOR_FORCED_CLOSE) {
		zend_error_noreturn(E_ERROR, "Cannot yield from finally in a force-closed generator");
	}

	/* Destroy the previously yielded value */
	if (generator->value) {
		zval_ptr_dtor(&generator->value);
	}

	/* Destroy the previously yielded key */
	if (generator->key) {
		zval_ptr_dtor(&generator->key);
	}

	/* Set the new yielded value */
	if (OP1_TYPE != IS_UNUSED) {
		zend_free_op free_op1;

		if (EX(op_array)->fn_flags & ZEND_ACC_RETURN_REFERENCE) {
			/* Constants and temporary variables aren't yieldable by reference,
			 * but we still allow them with a notice. */
			if (OP1_TYPE == IS_CONST || OP1_TYPE == IS_TMP_VAR) {
				zval *value, *copy;

				zend_error(E_NOTICE, "Only variable references should be yielded by reference");

				value = GET_OP1_ZVAL_PTR(BP_VAR_R);
				ALLOC_ZVAL(copy);
				INIT_PZVAL_COPY(copy, value);

				/* Temporary variables don't need ctor copying */
				if (!IS_OP1_TMP_FREE()) {
					zval_copy_ctor(copy);
				}

				generator->value = copy;
			} else {
				zval **value_ptr = GET_OP1_ZVAL_PTR_PTR(BP_VAR_W);

				if (OP1_TYPE == IS_VAR && UNEXPECTED(value_ptr == NULL)) {
					zend_error_noreturn(E_ERROR, "Cannot yield string offsets by reference");
				}

				/* If a function call result is yielded and the function did
				 * not return by reference we throw a notice. */
				if (OP1_TYPE == IS_VAR && !Z_ISREF_PP(value_ptr)
				    && !(opline->extended_value == ZEND_RETURNS_FUNCTION
				         && EX_T(opline->op1.var).var.fcall_returned_reference)
				    && EX_T(opline->op1.var).var.ptr_ptr == &EX_T(opline->op1.var).var.ptr) {
					zend_error(E_NOTICE, "Only variable references should be yielded by reference");

					Z_ADDREF_PP(value_ptr);
					generator->value = *value_ptr;
				} else {
					SEPARATE_ZVAL_TO_MAKE_IS_REF(value_ptr);
					Z_ADDREF_PP(value_ptr);
					generator->value = *value_ptr;
				}

				FREE_OP1_IF_VAR();
			}
		} else {
			zval *value = GET_OP1_ZVAL_PTR(BP_VAR_R);

			/* Consts, temporary variables and references need copying */
			if (OP1_TYPE == IS_CONST || OP1_TYPE == IS_TMP_VAR
				|| (PZVAL_IS_REF(value) && Z_REFCOUNT_P(value) > 0)
			) {
				zval *copy;

				ALLOC_ZVAL(copy);
				INIT_PZVAL_COPY(copy, value);

				/* Temporary variables don't need ctor copying */
				if (!IS_OP1_TMP_FREE()) {
					zval_copy_ctor(copy);
				}

				generator->value = copy;
			} else {
				Z_ADDREF_P(value);
				generator->value = value;
			}

			FREE_OP1_IF_VAR();
		}
	} else {
		/* If no value was specified yield null */
		Z_ADDREF(EG(uninitialized_zval));
		generator->value = &EG(uninitialized_zval);
	}

	/* Set the new yielded key */
	if (OP2_TYPE != IS_UNUSED) {
		zend_free_op free_op2;
		zval *key = GET_OP2_ZVAL_PTR(BP_VAR_R);

		/* Consts, temporary variables and references need copying */
		if (OP2_TYPE == IS_CONST || OP2_TYPE == IS_TMP_VAR
			|| (PZVAL_IS_REF(key) && Z_REFCOUNT_P(key) > 0)
		) {
			zval *copy;

			ALLOC_ZVAL(copy);
			INIT_PZVAL_COPY(copy, key);

			/* Temporary variables don't need ctor copying */
			if (!IS_OP2_TMP_FREE()) {
				zval_copy_ctor(copy);
			}

			generator->key = copy;
		} else {
			Z_ADDREF_P(key);
			generator->key = key;
		}

		if (Z_TYPE_P(generator->key) == IS_LONG
		    && Z_LVAL_P(generator->key) > generator->largest_used_integer_key
		) {
			generator->largest_used_integer_key = Z_LVAL_P(generator->key);
		}

		FREE_OP2_IF_VAR();
	} else {
		/* If no key was specified we use auto-increment keys */
		generator->largest_used_integer_key++;

		ALLOC_INIT_ZVAL(generator->key);
		ZVAL_LONG(generator->key, generator->largest_used_integer_key);
	}

	if (RETURN_VALUE_USED(opline)) {
		/* If the return value of yield is used set the send
		 * target and initialize it to NULL */
		generator->send_target = &EX_T(opline->result.var).var.ptr;
		Z_ADDREF(EG(uninitialized_zval));
		EX_T(opline->result.var).var.ptr = &EG(uninitialized_zval);
	} else {
		generator->send_target = NULL;
	}

	/* We increment to the next op, so we are at the correct position when the
	 * generator is resumed. */
	ZEND_VM_INC_OPCODE();

	/* The GOTO VM uses a local opline variable. We need to set the opline
	 * variable in execute_data so we don't resume at an old position. */
	SAVE_OPLINE();

	ZEND_VM_RETURN();
}

ZEND_VM_HANDLER(159, ZEND_DISCARD_EXCEPTION, ANY, ANY)
{
	if (EG(prev_exception) != NULL) {
		/* discard the previously thrown exception */
		zval_ptr_dtor(&EG(prev_exception));
		EG(prev_exception) = NULL;
	}

	ZEND_VM_NEXT_OPCODE();
}

ZEND_VM_HANDLER(162, ZEND_FAST_CALL, ANY, ANY)
{
	USE_OPLINE

	if (opline->extended_value &&
	    UNEXPECTED(EG(prev_exception) != NULL)) {
	    /* in case of unhandled exception jump to catch block instead of finally */
		ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->op2.opline_num]);
		ZEND_VM_CONTINUE();
	}
	EX(fast_ret) = opline + 1;
	ZEND_VM_SET_OPCODE(opline->op1.jmp_addr);
	ZEND_VM_CONTINUE();
}

ZEND_VM_HANDLER(163, ZEND_FAST_RET, ANY, ANY)
{
	if (EX(fast_ret)) {
		ZEND_VM_SET_OPCODE(EX(fast_ret));
		ZEND_VM_CONTINUE();
	} else {
		/* special case for unhandled exceptions */
		USE_OPLINE

		if (opline->extended_value == ZEND_FAST_RET_TO_FINALLY) {
			ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->op2.opline_num]);
			ZEND_VM_CONTINUE();
		} else if (opline->extended_value == ZEND_FAST_RET_TO_CATCH) {
			zend_exception_restore(TSRMLS_C);
			ZEND_VM_SET_OPCODE(&EX(op_array)->opcodes[opline->op2.opline_num]);
			ZEND_VM_CONTINUE();
		} else if (UNEXPECTED((EX(op_array)->fn_flags & ZEND_ACC_GENERATOR) != 0)) {
			zend_exception_restore(TSRMLS_C);
			ZEND_VM_DISPATCH_TO_HANDLER(ZEND_GENERATOR_RETURN);
		} else {
			zend_exception_restore(TSRMLS_C);
			ZEND_VM_DISPATCH_TO_HELPER(zend_leave_helper);
		}
	}
}

ZEND_VM_EXPORT_HELPER(zend_do_fcall, zend_do_fcall_common_helper)
