/*
  +----------------------------------------------------------------------+
  | APC                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Daniel Cowgill <dcowgill@communityconnect.com>              |
  |          Arun C. Murthy <arunc@yahoo-inc.com>                        |
  |          Gopal Vijayaraghavan <gopalv@yahoo-inc.com>                 |
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

/* $Id: apc_compile.h 303388 2010-09-15 09:50:28Z dmitry $ */

#ifndef APC_COMPILE_H
#define APC_COMPILE_H

/*
 * This module encapsulates most of the complexity involved in deep-copying
 * the Zend compiler data structures. The routines are allocator-agnostic, so
 * the same function can be used for copying to and from shared memory.
 */

#include "apc.h"
#include "apc_php.h"
#include "apc_main.h"

/* {{{ struct definition: apc_function_t */
typedef struct apc_function_t apc_function_t;
struct apc_function_t {
    char* name;                 /* the function name */
    int name_len;               /* length of name */
    zend_function* function;    /* the zend function data structure */
};
/* }}} */

/* {{{ struct definition: apc_class_t */
typedef struct apc_class_t apc_class_t;
struct apc_class_t {
    char* name;                     /* the class name */
    int name_len;                   /* length of name */
    char* parent_name;              /* the parent class name */
    zend_class_entry* class_entry;  /* the zend class data structure */
};
/* }}} */

/* {{{ struct definition: apc_opflags_t */
typedef struct apc_opflags_t apc_opflags_t;
struct apc_opflags_t {
    unsigned int has_jumps      : 1; /* has jump offsets */
    unsigned int deep_copy      : 1; /* needs deep copy */

    /* autoglobal bits */
    unsigned int _POST          : 1;
    unsigned int _GET           : 1;
    unsigned int _COOKIE        : 1;
    unsigned int _SERVER        : 1;
    unsigned int _ENV           : 1;
    unsigned int _FILES         : 1;
    unsigned int _REQUEST       : 1;
    unsigned int _SESSION       : 1;
#ifdef ZEND_ENGINE_2_4
    unsigned int GLOBALS        : 1;
#endif
    unsigned int unknown_global : 1;
};
/* }}} */

/*
 * These are the top-level copy functions.
 */

extern zend_op_array* apc_copy_op_array(zend_op_array* dst, zend_op_array* src, apc_context_t* ctxt TSRMLS_DC);
extern zend_class_entry* apc_copy_class_entry(zend_class_entry* dst, zend_class_entry* src, apc_context_t* ctxt TSRMLS_DC);
extern apc_function_t* apc_copy_new_functions(int old_count, apc_context_t* ctxt TSRMLS_DC);
extern apc_class_t* apc_copy_new_classes(zend_op_array* op_array, int old_count, apc_context_t* ctxt TSRMLS_DC);
extern zval* apc_copy_zval(zval* dst, const zval* src, apc_context_t* ctxt TSRMLS_DC);

#if 0
/*
 * Deallocation functions corresponding to the copy functions above.
 */

extern void apc_free_op_array(zend_op_array* src, apc_context_t* ctxt);
extern void apc_free_functions(apc_function_t* src, apc_context_t* ctxt);
extern void apc_free_classes(apc_class_t* src, apc_context_t* ctxt);
extern void apc_free_zval(zval* src, apc_context_t* ctxt);
#endif

/*
 * These "copy-for-execution" functions must be called after retrieving an
 * object from the shared cache. They do the minimal amount of work necessary
 * to allow multiple processes to concurrently execute the same VM data
 * structures.
 */

extern zend_op_array* apc_copy_op_array_for_execution(zend_op_array* dst, zend_op_array* src, apc_context_t* ctxt TSRMLS_DC);
extern zend_function* apc_copy_function_for_execution(zend_function* src, apc_context_t* ctxt TSRMLS_DC);
extern zend_class_entry* apc_copy_class_entry_for_execution(zend_class_entry* src, apc_context_t* ctxt TSRMLS_DC);

/*
 * The "free-after-execution" function performs a cursory clean up of the class data
 * This is required to minimize memory leak warnings and to ensure correct destructor
 * ordering of some variables.
 */
extern void apc_free_class_entry_after_execution(zend_class_entry* src TSRMLS_DC);

/*
 * Optimization callback definition and registration function. 
 */
typedef zend_op_array* (*apc_optimize_function_t) (zend_op_array* TSRMLS_DC);
extern apc_optimize_function_t apc_register_optimizer(apc_optimize_function_t optimizer TSRMLS_DC);

/*
 * To handle __COMPILER_HALT_OFFSET__
 */
long apc_file_halt_offset(const char* filename TSRMLS_DC);
void apc_do_halt_compiler_register(const char *filename, long halt_offset TSRMLS_DC);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: expandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: expandtab sw=4 ts=4 sts=4
 */
