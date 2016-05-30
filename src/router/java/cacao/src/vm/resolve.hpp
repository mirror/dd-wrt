/* src/vm/resolve.hpp - resolving classes/interfaces/fields/methods

   Copyright (C) 1996-2014
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


#ifndef RESOLVE_HPP_
#define RESOLVE_HPP_ 1

#include <stdio.h>                      // for FILE, NULL
#include "config.h"                     // for ENABLE_VERIFIER
#include "vm/references.hpp"            // for classref_or_classinfo
#include "vm/types.hpp"                 // for s4
#include "vm/utf8.hpp"                  // for Utf8String

struct classinfo;
struct fieldinfo;
struct instruction;
struct jitdata;
struct methodinfo;
struct typedesc;
struct unresolved_class;
struct unresolved_field;
struct unresolved_method;
struct unresolved_subtype_set;
struct typedescriptor_t;
struct typeinfo_t;

/* constants ******************************************************************/

#define RESOLVE_STATIC    0x0001  /* ref to static fields/methods             */
#define RESOLVE_PUTFIELD  0x0002  /* field ref inside a PUT{FIELD,STATIC}...  */
#define RESOLVE_SPECIAL   0x0004  /* method ref inside INVOKESPECIAL          */


/* enums **********************************************************************/

typedef enum {
	resolveLazy,
	resolveEager
} resolve_mode_t;

typedef enum {
	resolveLinkageError,
	resolveIllegalAccessError
} resolve_err_t;

typedef enum {
	resolveFailed = false,  /* this must be a false value */
	resolveDeferred = true, /* this must be a true value  */
	resolveSucceeded
} resolve_result_t;

/* structs ********************************************************************/

struct unresolved_subtype_set {
	classref_or_classinfo *subtyperefs;     /* NULL terminated list */
};

struct unresolved_class {
	constant_classref      *classref;
	methodinfo		       *referermethod;
	unresolved_subtype_set  subtypeconstraints;
};

/* XXX unify heads of unresolved_field and unresolved_method? */

struct unresolved_field {
	constant_FMIref *fieldref;
	methodinfo      *referermethod;
	s4               flags;
	
	unresolved_subtype_set  instancetypes;
	unresolved_subtype_set  valueconstraints;
};

struct unresolved_method {
	constant_FMIref *methodref;
	methodinfo      *referermethod;
	s4               flags;
	
	unresolved_subtype_set  instancetypes;
	unresolved_subtype_set *paramconstraints;
};

#define SUBTYPESET_IS_EMPTY(stset) \
	((stset).subtyperefs == NULL)

#define UNRESOLVED_SUBTYPE_SET_EMTPY(stset) \
	do { (stset).subtyperefs = NULL; } while(0)


/* function prototypes ********************************************************/

void resolve_handle_pending_exception(bool throwError);

bool resolve_class_from_name(classinfo* referer,methodinfo *refmethod,
			  			Utf8String classname,
			  			resolve_mode_t mode,
						bool checkaccess,
						bool link,
			  			classinfo **result);

bool resolve_classref(methodinfo *refmethod,
				 constant_classref *ref,
				 resolve_mode_t mode,
				 bool checkaccess,
			     bool link,
				 classinfo **result);

bool resolve_classref_or_classinfo(methodinfo *refmethod,
							  classref_or_classinfo cls,
							  resolve_mode_t mode,
							  bool checkaccess,
							  bool link,
							  classinfo **result);

classinfo *resolve_classref_or_classinfo_eager(classref_or_classinfo cls, bool checkaccess);

bool resolve_class_from_typedesc(typedesc *d,bool checkaccess,bool link,classinfo **result);

#ifdef ENABLE_VERIFIER
bool resolve_class(unresolved_class *ref,
			  resolve_mode_t mode,
			  bool checkaccess,
			  classinfo **result);
#endif /* ENABLE_VERIFIER */

bool resolve_field(unresolved_field *ref,
			  resolve_mode_t mode,
			  fieldinfo **result);

bool resolve_method(unresolved_method *ref,
			  resolve_mode_t mode,
			   methodinfo **result);

#ifdef ENABLE_VERIFIER
unresolved_class * create_unresolved_class(methodinfo *refmethod,
						constant_classref *classref,
						typeinfo_t *valuetype);
#endif

unresolved_field *resolve_create_unresolved_field(classinfo *referer,
											  methodinfo *refmethod,
											  instruction *iptr);

unresolved_method * resolve_create_unresolved_method(classinfo *referer,
													 methodinfo *refmethod,
													 constant_FMIref *methodref,
													 bool invokestatic,
													 bool invokespecial);

void unresolved_class_free(unresolved_class *ref);
void unresolved_field_free(unresolved_field *ref);
void unresolved_method_free(unresolved_method *ref);

resolve_result_t resolve_method_lazy(methodinfo *refmethod,
									 constant_FMIref *methodref,
									 bool invokespecial);

resolve_result_t resolve_field_lazy(methodinfo *refmethod,
									constant_FMIref *fieldref);

#if defined(ENABLE_VERIFIER)
resolve_result_t resolve_field_verifier_checks(methodinfo *refmethod,
											   constant_FMIref *fieldref,
											   classinfo *container,
											   fieldinfo *fi,
											   typeinfo_t *instanceti,
											   typeinfo_t *valueti,
											   bool isstatic,
											   bool isput);

bool resolve_constrain_unresolved_field(unresolved_field *ref,
										classinfo *referer, 
										methodinfo *refmethod,
									    typeinfo_t *instanceti,
									    typeinfo_t *valueti);

resolve_result_t resolve_method_verifier_checks(methodinfo *refmethod,
												constant_FMIref *methodref,
												methodinfo *mi,
												bool invokestatic);

resolve_result_t resolve_method_instance_type_checks(methodinfo *refmethod,
													 methodinfo *mi,
													 typeinfo_t *instanceti,
													 bool invokespecial);

resolve_result_t resolve_method_param_type_checks(jitdata *jd, 
												  methodinfo *refmethod,
												  instruction *iptr, 
												  methodinfo *mi,
												  bool invokestatic);

resolve_result_t resolve_method_param_type_checks_stackbased(
		methodinfo *refmethod, 
		methodinfo *mi,
		bool invokestatic, 
		typedescriptor_t *stack);

bool resolve_method_loading_constraints(classinfo *referer,
										methodinfo *mi);

bool resolve_constrain_unresolved_method_instance(unresolved_method *ref,
												  methodinfo *refmethod,
												  typeinfo_t *instanceti,
												  bool invokespecial);

bool resolve_constrain_unresolved_method_params(jitdata *jd,
												unresolved_method *ref,
												methodinfo *refmethod,
												instruction *iptr);

bool resolve_constrain_unresolved_method_params_stackbased(
		unresolved_method *ref,
		methodinfo *refmethod,
		typedescriptor_t *stack);

#endif /* defined(ENABLE_VERIFIER) */

#ifndef NDEBUG
void unresolved_class_debug_dump(unresolved_class *ref,FILE *file);
void unresolved_field_debug_dump(unresolved_field *ref,FILE *file);
void unresolved_method_debug_dump(unresolved_method *ref,FILE *file);
void unresolved_subtype_set_debug_dump(unresolved_subtype_set *stset,FILE *file);
#endif

classinfo  *resolve_classref_eager(constant_classref *ref);
classinfo  *resolve_classref_eager_nonabstract(constant_classref *ref);
fieldinfo  *resolve_field_eager(unresolved_field *ref);
methodinfo *resolve_method_eager(unresolved_method *ref);

#ifdef ENABLE_VERIFIER
classinfo *resolve_class_eager(unresolved_class *ref);
classinfo *resolve_class_eager_no_access_check(unresolved_class *ref);
#endif

#endif // RESOLVE_HPP_

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

