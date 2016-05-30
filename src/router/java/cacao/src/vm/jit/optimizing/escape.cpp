/* src/vm/jit/optimizing/escape.cpp

   Copyright (C) 2008-2013
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

#include "mm/dumpmemory.hpp"

#include "vm/class.hpp"
#include "vm/classcache.hpp"

#include "vm/field.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/show.hpp"
#include "vm/jit/optimizing/escape.hpp"

#include <stdarg.h>

#if defined(ENABLE_ESCAPE_REASON)
#define ENABLE_REASON
#endif

#if defined(ENABLE_REASON)
#define I2(why, tov, es) escape_analysis_record_reason(e, why, iptr, tov, es);
#else
#define I2(why, tov, es)
#endif
#define I(why, to, from) I2(why, instruction_ ## to (iptr), escape_analysis_get_state(e, instruction_ ## from (iptr)))
#define E2(why, var) I2(why, var, ESCAPE_GLOBAL)
#define E(why, which) E2(why, instruction_ ## which (iptr))

typedef enum {
	RED = 31,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE,
	COLOR_END
} color_t;

#define ENABLE_COLOR

static void color_start(color_t color) {
#if defined(ENABLE_COLOR)
	if (RED <= color && color < COLOR_END) {
		printf("\033[%dm", color);
	}
#endif
}

static void color_end() {
#if defined(ENABLE_COLOR)
	printf("\033[m");
	fflush(stdout);
#endif
}

static void color_printf(color_t color, const char *fmt, ...) {
	va_list ap;
	color_start(color);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	color_end();
}


/*** escape_state *************************************************************/

const char *escape_state_to_string(escape_state_t escape_state) {
#	define str(x) case x: return #x;
	switch (escape_state) {
		str(ESCAPE_UNKNOWN)
		str(ESCAPE_NONE)
		str(ESCAPE_METHOD)
		str(ESCAPE_METHOD_RETURN)
		str(ESCAPE_GLOBAL)
		default: return "???";
	}
#	undef str
}

/*** instruction **************************************************************/

static inline s2 instruction_get_opcode(const instruction *iptr) {
	if (iptr->opc == ICMD_BUILTIN) {
		return iptr->sx.s23.s3.bte->opcode;
	} else {
		return iptr->opc;
	}
}

static inline bool instruction_is_unresolved(const instruction *iptr) {
	return iptr->flags.bits & INS_FLAG_UNRESOLVED;
}

static inline s4 instruction_field_type(const instruction *iptr) {
	if (instruction_is_unresolved(iptr)) {
		return iptr->sx.s23.s3.uf->fieldref->parseddesc.fd->type;
	} else {
		return iptr->sx.s23.s3.fmiref->p.field->type;
	}
}

static inline s4 instruction_s1(const instruction *iptr) {
	return iptr->s1.varindex;
}

static inline s4 instruction_s2(const instruction *iptr) {
	return iptr->sx.s23.s2.varindex;
}

static inline s4 instruction_s3(const instruction *iptr) {
	return iptr->sx.s23.s3.varindex;
}

static inline s4 instruction_dst(const instruction *iptr) {
	return iptr->dst.varindex;
}

static inline s4 instruction_arg(const instruction *iptr, int arg) {
	return iptr->sx.s23.s2.args[arg];
}

static inline bool instruction_is_class_constant(const instruction *iptr) {
	return iptr->flags.bits & INS_FLAG_CLASS;
}

static inline classinfo *instruction_classinfo(const instruction *iptr) {
	return iptr->sx.val.c.cls;
}

static inline methodinfo *instruction_local_methodinfo(const instruction *iptr) {
	if (instruction_is_unresolved(iptr)) {
		return NULL;
	} else {
		return iptr->sx.s23.s3.fmiref->p.method;
	}
}

static inline int instruction_dst_type(const instruction *iptr, jitdata *jd) {
	return VAROP(iptr->dst)->type;
}

static inline int instruction_return_type(const instruction *iptr) {
	return instruction_call_site(iptr)->returntype.type;
}

static inline s4 instruction_arg_type(const instruction *iptr, int arg) {
	methoddesc *md = instruction_call_site(iptr);
	assert(0 <= arg && arg < md->paramcount);
	return md->paramtypes[arg].type;
}

static inline int instruction_arg_count(const instruction *iptr) {
	return instruction_call_site(iptr)->paramcount;
}

/*** instruction_list ********************************************************/

typedef struct instruction_list_item {
	instruction *instr;
	struct instruction_list_item *next;
} instruction_list_item_t;

typedef struct {
	instruction_list_item_t *first;
} instruction_list_t;

void instruction_list_init(instruction_list_t *list) {
	list->first = NULL;
}

void instruction_list_add(instruction_list_t *list, instruction *instr) {
	instruction_list_item_t *item = DNEW(instruction_list_item_t);
	item->instr = instr;
	item->next = list->first;
	list->first = item;
}

#define FOR_EACH_INSTRUCTION_LIST(list, it) \
	for ((it) = (list)->first; (it) != NULL; (it) = (it)->next)

/*** escape_analysis *********************************************************/

struct var_extra;

typedef struct {
	jitdata *jd;

	instruction_list_t *allocations;
	instruction_list_t *getfields;
	instruction_list_t *monitors;
	instruction_list_t *returns;

	struct var_extra **var;

	unsigned adr_args_count;

	bool verbose;

} escape_analysis_t;

/*** dependency_list_item ****************************************************/

typedef struct dependency_list_item {
	instruction *store;
	struct dependency_list_item *next;
} dependency_list_item_t;

bool dependency_list_item_compare(const dependency_list_item_t *item, const instruction *load) {

	instruction *store = item->store;
	Utf8String storen;
	Utf8String loadn;

	if (load->opc == ICMD_AALOAD) {

		if (store->opc != ICMD_AASTORE) {
			return false;
		}

		return true;

	} else {

		if (store->opc != ICMD_PUTFIELD) {
			return false;
		}

		if (
			instruction_is_unresolved(store) !=
			instruction_is_unresolved(load)
		) {
			return false;
		}

		if (instruction_is_unresolved(store)) {
			storen = store->sx.s23.s3.uf->fieldref->name;
			loadn  = load->sx.s23.s3.uf->fieldref->name;
		} else {
			storen = store->sx.s23.s3.fmiref->name;
			loadn  = load->sx.s23.s3.fmiref->name;
		}

		return storen == loadn;
	}
}

/* TODO rename */
s4 dependency_list_item_get_dependency(const dependency_list_item_t *item) {
	switch (item->store->opc) {
		case ICMD_AASTORE:
			return instruction_s3(item->store);
		case ICMD_PUTFIELD:
			return instruction_s2(item->store);
		default:
			assert(0);
			return 0;
	}
}

/*** dependency_list *********************************************************/

typedef struct {
	dependency_list_item_t *first;
	dependency_list_item_t *last;
} dependency_list_t;

void dependency_list_init(dependency_list_t *dl) {
	dl->first = NULL;
	dl->last = NULL;
}

void dependency_list_add(dependency_list_t *dl, instruction *store) {
	dependency_list_item_t *item = DNEW(dependency_list_item_t);

	item->store = store;
	item->next = NULL;

	if (dl->first == NULL) {
		dl->first = item;
		dl->last = item;
	} else {
		dl->last->next = item;
		dl->last = item;
	}
}

void dependenCy_list_import(dependency_list_t *dl, dependency_list_t *other) {

	if (other == NULL) {
		return;
	}

	if (dl->first == NULL) {
		*dl = *other;
	} else {
		dl->last->next = other->first;
		dl->last = other->last;
	}

	other->first = NULL;
	other->last = NULL;

}

#define FOR_EACH_DEPENDENCY_LIST(dl, it) \
	for ((it) = (dl)->first; (it) != NULL; (it) = (it)->next)

/*** var_extra ***************************************************************/

#if defined(ENABLE_REASON)
typedef struct reason {
	const char *why;
	instruction *iptr;
	struct reason *next;
} reason_t;
#endif

typedef struct var_extra {
	instruction *allocation;
	escape_state_t escape_state;
	s4 representant;
	dependency_list_t *dependency_list;
	unsigned contains_arg:1;
	unsigned contains_only_args:1;
	/*signed adr_arg_num:30;*/
#if defined(ENABLE_REASON)
	reason_t *reasons;
#endif
} var_extra_t;

static void var_extra_init(var_extra_t *ve) {
	ve->allocation = NULL;
	ve->escape_state = ESCAPE_NONE;
	ve->representant = -1;
	ve->dependency_list = NULL;
	ve->contains_arg = false;
	ve->contains_only_args = false;
	/*ve->adr_arg_num = -1;*/
#if defined(ENABLE_REASON)
	ve->reasons = NULL;
#endif
}

static inline var_extra_t *var_extra_get_no_alloc(const escape_analysis_t *e, s4 var) {
	return e->var[var];
}

static var_extra_t* var_extra_get(escape_analysis_t *e, s4 var) {
	var_extra_t *ve;

	assert(0 <= var && var <= e->jd->vartop);

	ve = var_extra_get_no_alloc(e, var);

	if (ve == NULL) {
		ve = DNEW(var_extra_t);
		var_extra_init(ve);
		e->var[var] = ve;
	}

	return ve;
}

static s4 var_extra_get_representant(escape_analysis_t *e, s4 var) {
	var_extra_t *ve;
#if !defined(NDEBUG)
	int ctr = 0;
#endif

	ve = var_extra_get(e, var);

	while (ve->representant != -1) {
		assert(ctr++ < 10000);
		var = ve->representant;
		ve = var_extra_get_no_alloc(e, var);
		assert(ve);
	}

	return var;
}

static escape_state_t var_extra_get_escape_state(escape_analysis_t *e, s4 var) {
	var_extra_t *ve;

	var = var_extra_get_representant(e, var);
	ve = var_extra_get(e, var);

	return ve->escape_state;
}

static void var_extra_set_escape_state(escape_analysis_t *e, s4 var, escape_state_t escape_state) {
	var_extra_t *ve;

	var = var_extra_get_representant(e, var);
	ve = var_extra_get(e, var);

	ve->escape_state = escape_state;
}

static dependency_list_t *var_extra_get_dependency_list(escape_analysis_t *e, s4 var) {
	var_extra_t *ve;

	var = var_extra_get_representant(e, var);
	ve = var_extra_get(e, var);

	if (ve->dependency_list == NULL) {
		ve->dependency_list = DNEW(dependency_list_t);
		dependency_list_init(ve->dependency_list);
	}

	return ve->dependency_list;
}

/*** escape_analysis *********************************************************/

static void escape_analysis_init(escape_analysis_t *e, jitdata *jd) {
	e->jd = jd;

	e->allocations = DNEW(instruction_list_t);
	instruction_list_init(e->allocations);

	e->getfields = DNEW(instruction_list_t);
	instruction_list_init(e->getfields);

	e->monitors = DNEW(instruction_list_t);
	instruction_list_init(e->monitors);

	e->returns = DNEW(instruction_list_t);
	instruction_list_init(e->returns);

	e->var = DMNEW(var_extra_t *, jd->vartop);
	MZERO(e->var, var_extra_t *, jd->vartop);

	e->adr_args_count = 0;

	e->verbose = 1;
	e->verbose = strcmp(jd->m->name.begin(), "<init>") == 0;
	e->verbose = getenv("EV") != NULL;
}

#if defined(ENABLE_REASON)
static void escape_analysis_record_reason(escape_analysis_t *e, const char *why, instruction *iptr, s4 var, escape_state_t es) {
	var_extra_t *ve;
	reason_t *re;
	if (es == ESCAPE_GLOBAL || es == ESCAPE_METHOD_RETURN) {
		var = var_extra_get_representant(e, var);
		ve = var_extra_get(e, var);
		re = NEW(reason_t);
		re->why = why;
		re->iptr= iptr;
		re->next = ve->reasons;
		ve->reasons = re;
		if (e->verbose) {
			printf("%d escapes because %s\n", var, why);
		}
	}
}
#endif

static void escape_analysis_set_allocation(escape_analysis_t *e, s4 var, instruction *iptr) {
	var_extra_get(e, var)->allocation = iptr;
}

static instruction *escape_analysis_get_allocation(const escape_analysis_t *e, s4 var) {
	var_extra_t *ve = var_extra_get_no_alloc(e, var);

	assert(ve != NULL);
	assert(ve->allocation != NULL);

	return ve->allocation;
}

static void escape_analysis_set_contains_argument(escape_analysis_t *e, s4 var) {
	var = var_extra_get_representant(e, var);
	var_extra_get(e, var)->contains_arg = true;
}

static bool escape_analysis_get_contains_argument(escape_analysis_t *e, s4 var) {
	var = var_extra_get_representant(e, var);
	return var_extra_get(e, var)->contains_arg;
}

static void escape_analysis_set_contains_only_arguments(escape_analysis_t *e, s4 var) {
	var = var_extra_get_representant(e, var);
	var_extra_get(e, var)->contains_only_args = true;
}

static bool escape_analysis_get_contains_only_arguments(escape_analysis_t *e, s4 var) {
	var = var_extra_get_representant(e, var);
	return var_extra_get(e, var)->contains_only_args;
}

/*
static void escape_analysis_set_adr_arg_num(escape_analysis_t *e, s4 var, s4 num) {
	var_extra_get(e, var)->adr_arg_num = num;
}

static s4 escape_analysis_get_adr_arg_num(escape_analysis_t *e, s4 var) {
	return var_extra_get(e, var)->adr_arg_num;
}
*/

static bool escape_analysis_in_same_set(escape_analysis_t *e, s4 var1, s4 var2) {
	return var_extra_get_representant(e, var1) == var_extra_get_representant(e, var2);
}

static void escape_analysis_ensure_state(escape_analysis_t *e, s4 var, escape_state_t escape_state) {

	var_extra_t *ve;
	dependency_list_item_t *it;

	var = var_extra_get_representant(e, var);
	ve = var_extra_get(e, var);

	if (ve->escape_state < escape_state) {
		if (e->verbose) {
			printf(
				"escape state of %d %s => %s\n",
				var,
				escape_state_to_string(ve->escape_state),
				escape_state_to_string(escape_state)
			);
		}
		ve->escape_state = escape_state;
		if (ve->dependency_list != NULL) {
			FOR_EACH_DEPENDENCY_LIST(ve->dependency_list, it) {
				if (e->verbose) {
					printf("propagating to %s@%d\n", icmd_table[it->store->opc].name, it->store->line);
				}
				escape_analysis_ensure_state(
					e,
					dependency_list_item_get_dependency(it),
					escape_state
				);
				{
				instruction *iptr = NULL;
				I2("propagated by dependency", dependency_list_item_get_dependency(it), escape_state);
				}
			}
		}
	}
}

static escape_state_t escape_analysis_get_state(escape_analysis_t *e, s4 var) {
	return var_extra_get_escape_state(e, var);
}

static classinfo *escape_analysis_classinfo_in_var(escape_analysis_t *e, s4 var) {
	instruction *iptr = escape_analysis_get_allocation(e, var);

	if (iptr == NULL) {
		return NULL;
	}

	if (! instruction_is_class_constant(iptr)) {
		return NULL;
	}

	if (instruction_dst(iptr) != var) {
		return NULL;
	}

	if (instruction_is_unresolved(iptr)) {
		return NULL;
	}

	return instruction_classinfo(iptr);
}

static void escape_analysis_merge(escape_analysis_t *e, s4 var1, s4 var2) {

	var_extra_t *ve1, *ve2;
	dependency_list_item_t *itd;
	bool has_become_arg;

	var1 = var_extra_get_representant(e, var1);
	var2 = var_extra_get_representant(e, var2);

	/* Don't merge the same escape sets. */

	if (var1 == var2) {
		return;
	}

	if (e->verbose) printf("Merging (%d,%d)\n", var1, var2);

	ve1 = var_extra_get(e, var1);
	ve2 = var_extra_get(e, var2);

	/* Adjust escape state to maximal escape state. */

	escape_analysis_ensure_state(e, var1, ve2->escape_state);
	escape_analysis_ensure_state(e, var2, ve1->escape_state);

	/* Representant of var1 becomes the representant of var2. */

	ve2->representant = var1;

	/* Adjust is_arg to logical or. */

	has_become_arg = ve1->contains_arg != ve2->contains_arg;
	ve1->contains_arg = ve1->contains_arg || ve2->contains_arg;

	if (e->verbose && has_become_arg) printf("(%d,%d) has become arg.\n", var1, var2);

	/* Merge list of dependencies. */

	if (ve1->dependency_list == NULL) {
		ve1->dependency_list = ve2->dependency_list;
	} else {
		dependenCy_list_import(ve1->dependency_list, ve2->dependency_list);
	}

	/* If one of the merged values is an argument but the other not,
	   all dependencies of the newly created value escape globally. */

	if (has_become_arg && ve1->dependency_list != NULL) {
		FOR_EACH_DEPENDENCY_LIST(ve1->dependency_list, itd) {
			escape_analysis_ensure_state(
				e,
				dependency_list_item_get_dependency(itd),
				ESCAPE_GLOBAL
			);
			{
			instruction *iptr = NULL;
			E2("has become arg", dependency_list_item_get_dependency(itd));
			}
		}
	}

	/* Adjust contains_only_args to logical and. */

	ve1->contains_only_args = ve1->contains_only_args && ve2->contains_only_args;

	/* Adjust address argument number contained in this var. */

	/*
	if (ve1->adr_arg_num != ve2->adr_arg_num) {
		ve1->adr_arg_num = -1;
	}
	*/
#if defined(ENABLE_REASON)
	if (ve1->reasons) {
		reason_t *re = ve1->reasons;
		while (re->next != NULL) {
			re = re->next;
		}
		re->next = ve2->reasons;
	} else {
		ve1->reasons = ve2->reasons;
	}
#endif
}

static void escape_analysis_add_dependency(escape_analysis_t *e, instruction *store) {
	s4 obj = instruction_s1(store);
	dependency_list_t *dl = var_extra_get_dependency_list(e, obj);

	assert(store->opc == ICMD_PUTFIELD || store->opc == ICMD_AASTORE);

	dependency_list_add(dl, store);

	if (e->verbose) {
		printf("dependency_list_add: %d.dependency_list.add( { ", obj);
		show_icmd(e->jd, store, 0, 3);
		printf(" } )\n");
	}
}

static void escape_analysis_process_instruction(escape_analysis_t *e, instruction *iptr) {
	jitdata *jd = e->jd;
	classinfo *c;
	s4 count;
	u1 *paramescape;
	unsigned i;
	instruction **iarg;
	methodinfo *mi;
	escape_state_t es;
	const char *why;

	if (e->verbose) {
		color_start(CYAN);
		printf("%d: ", iptr->line);
		show_icmd(e->jd, iptr, 0, 3);
		color_end();
		printf("\n");
	}

	switch (instruction_get_opcode(iptr)) {
		case ICMD_ACONST:

			escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);
			escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_NONE);

			break;

		case ICMD_NEW:
			c = escape_analysis_classinfo_in_var(e, instruction_arg(iptr, 0));

			escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);

			if (c == NULL) {
				escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
				E("unresolved class", dst)
			} else if (c->finalizer != NULL) {
				escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
				E("finalizer", dst)
			} else {
				escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_NONE);
			}

			instruction_list_add(e->allocations, iptr);

			break;

		case ICMD_MONITORENTER:
		case ICMD_MONITOREXIT:

			instruction_list_add(e->monitors, iptr);

			break;

		case ICMD_NEWARRAY:
		case ICMD_ANEWARRAY:

			escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
			escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);
			instruction_list_add(e->allocations, iptr);
			E("untracked array", dst)
			break;

		case ICMD_PUTSTATIC:
			if (instruction_field_type(iptr) == TYPE_ADR) {
				escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_GLOBAL);
				E("putstatic", s1)
			}
			break;

		case ICMD_PUTFIELD:
			if (instruction_field_type(iptr) == TYPE_ADR) {
				if (escape_analysis_get_contains_argument(e, instruction_s1(iptr))) {
					escape_analysis_ensure_state(e, instruction_s2(iptr), ESCAPE_GLOBAL);
					/* If s1 is currently not an argument, but can contain one later because
					   of a phi function, the merge function takes care to make all
					   dependencies escape globally. */
					E("putfield into argument", s2)
				} else {
					I("putfield inherit", s2, s1);
					escape_analysis_ensure_state(e, instruction_s2(iptr), escape_analysis_get_state(e, instruction_s1(iptr)));
					escape_analysis_add_dependency(e, iptr);
				}
			}
			break;

		case ICMD_AASTORE:
			if (escape_analysis_get_contains_argument(e, instruction_s1(iptr))) {
				if (e->verbose) printf("Contains argument.\n");
				escape_analysis_ensure_state(e, instruction_s3(iptr), ESCAPE_GLOBAL);
				E("aastore into argument", s3)
			} else {
				if (e->verbose) printf("Contains no argument.\n");
				I("aastore", s3, s1)
				escape_analysis_ensure_state(e, instruction_s3(iptr), escape_analysis_get_state(e, instruction_s1(iptr)));
				escape_analysis_add_dependency(e, iptr);
			}
			break;

		case ICMD_GETSTATIC:
			if (instruction_field_type(iptr) == TYPE_ADR) {
				escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
				E("loaded from static var", dst)
				escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);
			}
			break;

		case ICMD_GETFIELD:
			if (instruction_field_type(iptr) == TYPE_ADR) {

				if (escape_analysis_get_contains_argument(e, instruction_s1(iptr))) {
					/* Fields loaded from arguments escape globally.
					   x = arg.foo;
					   x.bar = y;
					   => y escapes globally. */
					escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
					E("loaded from arg", dst)
				} else {
					escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_NONE);
				}

				escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);

				instruction_list_add(e->getfields, iptr);
			}
			break;

		case ICMD_ARRAYLENGTH:
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD);
			break;

		case ICMD_AALOAD:

			if (escape_analysis_get_contains_argument(e, instruction_s1(iptr))) {
				/* If store into argument, escapes globally. See ICMD_GETFIELD. */
				escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
				E("aaload from argument", dst)
			} else {
				escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_NONE);
			}

			escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);

			instruction_list_add(e->getfields, iptr);
			break;

		case ICMD_IF_ACMPEQ:
		case ICMD_IF_ACMPNE:
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD);
			escape_analysis_ensure_state(e, instruction_s2(iptr), ESCAPE_METHOD);
			break;

		case ICMD_IFNULL:
		case ICMD_IFNONNULL:
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD);
			break;

		case ICMD_CHECKNULL:
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD);
			escape_analysis_merge(e, instruction_s1(iptr), instruction_dst(iptr));
			break;

		case ICMD_CHECKCAST:
			escape_analysis_merge(e, instruction_s1(iptr), instruction_dst(iptr));
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD);
			escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);
			break;

		case ICMD_INSTANCEOF:
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD);
			break;

		case ICMD_INVOKESPECIAL:
		case ICMD_INVOKEVIRTUAL:
		case ICMD_INVOKEINTERFACE:
		case ICMD_INVOKESTATIC:
			count = instruction_arg_count(iptr);
			mi = instruction_local_methodinfo(iptr);
			paramescape = NULL;
			why = "???";

			if (mi != NULL) {
				/* If the method could be resolved, it already is. */
				paramescape = mi->paramescape;

				if (e->verbose) {
					if (paramescape) {
						printf("Paramescape for callee available.\n");
					}
				}

				if (paramescape) why = "Available param escape";

				if (paramescape == NULL) {
					if (e->verbose) {
						printf("BC escape analyzing callee.\n");
					}
					why = "BC param escape";
					bc_escape_analysis_perform(mi);
					paramescape = mi->paramescape;
				}
			} else {
				if (e->verbose) {
					printf("Unresolved callee.\n");
				}
				why = "Unresolved callee";
			}

			if (iptr->opc == ICMD_INVOKEVIRTUAL || iptr->opc == ICMD_INVOKEINTERFACE) {
				if (mi != NULL && !escape_is_monomorphic(e->jd->m, mi)) {
					if (e->verbose) {
						printf("Not monomorphic.\n");
					}
					why = "Polymorphic";
					paramescape = NULL;
				}
			}

			/* Set the escape state of the return value.
			   This is: global if we down have information of the callee, or the callee
			   supplied escape state. */

			if (instruction_return_type(iptr) == TYPE_ADR) {
				if (paramescape == NULL) {
					escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_GLOBAL);
					E(why, dst);
				} else {
					es = escape_state_from_u1(paramescape[-1]);
					I2(why, instruction_dst(iptr), es)
					escape_analysis_ensure_state(e, instruction_dst(iptr), es);
				}
				escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);
			}

			for (i = 0; i < count; ++i) {
				if (instruction_arg_type(iptr, i) == TYPE_ADR) {

					if (paramescape == NULL) {
						escape_analysis_ensure_state(
							e,
							instruction_arg(iptr, i),
							ESCAPE_GLOBAL
						);
						E2(why, instruction_arg(iptr, i));
					} else if (escape_state_from_u1(*paramescape) <= ESCAPE_METHOD) {
						es = escape_state_from_u1(*paramescape);

						if (es < ESCAPE_METHOD) {
							es = ESCAPE_METHOD;
						}

						I2(why, instruction_arg(iptr, i), es);
						escape_analysis_ensure_state(e, instruction_arg(iptr, i), es);

						if (*paramescape & 0x80) {
							/* Parameter can be returned from method.
							   This creates an alias to the retur value.
							   If the return value escapes, the ES of the parameter needs
							   to be adjusted. */
							escape_analysis_merge(e, instruction_arg(iptr, i), instruction_dst(iptr));
							I2("return alias", instruction_arg(iptr, i), instruction_dst(iptr));
						}
					} else {
						escape_analysis_ensure_state(e, instruction_arg(iptr, i), ESCAPE_GLOBAL);
						E2(why, instruction_arg(iptr, i));
					}

					if (paramescape != NULL) {
						++paramescape;
					}
				}
			}

			break;

		case ICMD_ATHROW:
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_GLOBAL);
			E("throw", s1)
			break;

		case ICMD_ARETURN:
			/* If we return only arguments, the return value escapes only the method.
			   ESCAPE_METHOD for now, and check later, if a different value than an
			   argument is possibly returned. */
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_METHOD_RETURN);
			E("return", s1)
			instruction_list_add(e->returns, iptr);
			break;

		case ICMD_ALOAD:
		case ICMD_ASTORE:
		case ICMD_MOVE:
		case ICMD_COPY:
			if (instruction_dst_type(iptr, jd) == TYPE_ADR) {
				escape_analysis_merge(e, instruction_s1(iptr), instruction_dst(iptr));
				escape_analysis_set_allocation(e, instruction_dst(iptr), iptr);
			}
			break;

		case ICMD_PHI:
			for (
				iarg = iptr->sx.s23.s2.iargs;
				iarg != iptr->sx.s23.s2.iargs + iptr->s1.argcount;
				++iarg
			) {
				escape_analysis_merge(e, instruction_dst(iptr), instruction_dst(*iarg));
			}
			break;

		case ICMD_GETEXCEPTION:
			escape_analysis_ensure_state(e, instruction_dst(iptr), ESCAPE_NONE);
			break;
	}
}

static void escape_analysis_process_instructions(escape_analysis_t *e) {
	basicblock *bptr;
	instruction *iptr;

	FOR_EACH_BASICBLOCK(e->jd, bptr) {

		if (e->verbose) {
			color_printf(CYAN, "=== BB %d ===\n", bptr->nr);
		}

		for (iptr = bptr->phis; iptr != bptr->phis + bptr->phicount; ++iptr) {
			escape_analysis_process_instruction(e, iptr);
		}

		FOR_EACH_INSTRUCTION(bptr, iptr) {
			escape_analysis_process_instruction(e, iptr);
		}

	}
}

static void escape_analysis_post_process_returns(escape_analysis_t *e) {
	instruction_list_item_t *iti;
	instruction *iptr;

	if (e->verbose) printf("Post processing returns:\n");

	FOR_EACH_INSTRUCTION_LIST(e->getfields, iti) {
		iptr = iti->instr;

		if (! escape_analysis_get_contains_only_arguments(e, instruction_s1(iptr))) {
			escape_analysis_ensure_state(e, instruction_s1(iptr), ESCAPE_GLOBAL);
			E("return of not argument", s1)
		}
	}
}

static void escape_analysis_post_process_getfields(escape_analysis_t *e) {
	instruction_list_item_t *iti;
	dependency_list_item_t *itd;
	instruction *iptr;
	dependency_list_t *dl;

	if (e->verbose) printf("Post processing getfields:\n");

	FOR_EACH_INSTRUCTION_LIST(e->getfields, iti) {

		iptr = iti->instr;

		/* Get the object the field/element is loaded from. */

		dl = var_extra_get_dependency_list(e, instruction_s1(iptr));

		/* Adjust escape state of all objects in the dependency list,
		   referenced via the field of this getfield/arraystore. */

		if (dl != NULL) {
			FOR_EACH_DEPENDENCY_LIST(dl, itd) {
				if (dependency_list_item_compare(itd, iptr)) {

					/* Fields match. Adjust escape state. */

					escape_analysis_ensure_state(
						e,
						dependency_list_item_get_dependency(itd),
						escape_analysis_get_state(e, instruction_dst(iptr))
					);
					I2("post process getfield", dependency_list_item_get_dependency(itd), escape_analysis_get_state(e, instruction_dst(iptr)));
				}
			}
		}

	}
}

static void escape_analysis_mark_monitors(escape_analysis_t *e) {
	instruction_list_item_t *iti;
	instruction *iptr;

	FOR_EACH_INSTRUCTION_LIST(e->monitors, iti) {
		iptr = iti->instr;

		/* TODO if argument does not escape, mark. */
		if (escape_analysis_get_state(e, instruction_arg(iptr, 0)) != ESCAPE_GLOBAL) {
			if (e->verbose) {
				printf("Monitor on thread local object!\n");
			}
		}
	}
}

static void escape_analysis_mark_allocations(escape_analysis_t *e) {
	instruction_list_item_t *iti;
	instruction *iptr;
	escape_state_t es;
	FOR_EACH_INSTRUCTION_LIST(e->allocations, iti) {
		iptr = iti->instr;
		es = escape_analysis_get_state(e, instruction_dst(iptr));

#if defined(ENABLE_REASON)
		if (instruction_get_opcode(iptr) == ICMD_NEW) {
			var_extra_t *ve;
			iptr->sx.s23.s3.bte = builtintable_get_internal(BUILTIN_escape_reason_new);
			ve = var_extra_get(e, var_extra_get_representant(e, instruction_dst(iptr)));
			iptr->escape_reasons = ve->reasons;
			if (es < ESCAPE_METHOD_RETURN) {
				assert(!ve->reasons);
				reason_t *r = NEW(reason_t);
				r->why = "No escape\n";
				r->iptr = NULL;
				r->next = NULL;
				iptr->escape_reasons = r;
			} else {
				assert(iptr->escape_reasons);
			}
		}
#endif

/*
		if (instruction_get_opcode(iptr) == ICMD_NEW) {
			es = escape_analysis_get_state(e, instruction_dst(iptr));
			if (es < ESCAPE_METHOD_RETURN) {
				iptr->sx.s23.s3.bte = builtintable_get_internal(BUILTIN_tlh_new);
				e->jd->code->flags |= CODE_FLAG_TLH;
			}
		}
*/
	}
}

static void escape_analysis_process_arguments(escape_analysis_t *e) {
	s4 p;
	s4 t;
	s4 l;
	s4 varindex;
	methoddesc *md;

	md = e->jd->m->parseddesc;

	for (p = 0, l = 0; p < md->paramcount; ++p) {
		t = md->paramtypes[p].type;
		varindex = e->jd->local_map[l * 5 + t];
		if (t == TYPE_ADR) {
			if (varindex != jitdata::UNUSED) {
				escape_analysis_ensure_state(e, varindex, ESCAPE_NONE);
				escape_analysis_set_contains_argument(e, varindex);
				escape_analysis_set_contains_only_arguments(e, varindex);
				/*escape_analysis_set_adr_arg_num(e, varindex, e->adr_args_count);*/
			}
			e->adr_args_count += 1;
		}
		l += IS_2_WORD_TYPE(t) ? 2 : 1;
	}
}

static void escape_analysis_export_arguments(escape_analysis_t *e) {
	s4 p;
	s4 t;
	s4 l;
	s4 varindex;
	methoddesc *md;
	u1 *paramescape;
	instruction_list_item_t *iti;
	instruction *iptr;
	escape_state_t es, re;
	int ret_val_is_adr;

	md = e->jd->m->parseddesc;

	ret_val_is_adr = (md->returntype.type == TYPE_ADR) ? 1 : 0;

	paramescape = MNEW(u1, e->adr_args_count + ret_val_is_adr);

	e->jd->m->paramescape = paramescape + ret_val_is_adr;

	for (p = 0, l = 0; p < md->paramcount; ++p) {
		t = md->paramtypes[p].type;
		varindex = e->jd->local_map[l * 5 + t];
		if (t == TYPE_ADR) {
			if (varindex == jitdata::UNUSED) {
				*paramescape = (u1)ESCAPE_NONE;
			} else {
				es = escape_analysis_get_state(e, varindex);

				if (es == ESCAPE_METHOD_RETURN) {
					*paramescape = escape_state_to_u1(ESCAPE_METHOD) | 0x80;
					if (e->verbose) {
						printf("non-escaping adr parameter returned: %d\n", p);
					}
				} else {
					*paramescape = escape_state_to_u1(es);
				}

			}
			paramescape += 1;
		}
		l += IS_2_WORD_TYPE(t) ? 2 : 1;
	}

	if (ret_val_is_adr) {
		/* Calculate escape state of return value as maximum escape state of all
		   values returned. */

		re = ESCAPE_NONE;

		FOR_EACH_INSTRUCTION_LIST(e->returns, iti) {
			iptr = iti->instr;
			es = escape_analysis_get_state(e, instruction_s1(iptr));

			if (es > re) {
				re = es;
			}
		}

		e->jd->m->paramescape[-1] = escape_state_to_u1(re);
	}
}

#if defined(ENABLE_REASON)
void print_escape_reasons() {
	reason_t *re = THREADOBJECT->escape_reasons;

	fprintf(stderr, "DYN_REASON");

	for (; re; re = re->next) {
		fprintf(stderr,":%s", re->why);
	}

	fprintf(stderr, "\n");
}

void set_escape_reasons(void *vp) {
	THREADOBJECT->escape_reasons = vp;
}
#endif

static void escape_analysis_display(escape_analysis_t *e) {
	instruction_list_item_t *iti;
	var_extra_t *ve;
	instruction *iptr;

	FOR_EACH_INSTRUCTION_LIST(e->allocations, iti) {
		iptr = iti->instr;
		ve = var_extra_get(e, var_extra_get_representant(e, instruction_dst(iptr)));
		show_icmd(e->jd, iptr-1, 0, 3);
		printf("\n");
		show_icmd(e->jd, iptr, 0, 3);
		printf("\n");
		printf(
			"%s@%d: --%s-- %d\n\n",
			icmd_table[iptr->opc].name,
			iptr->line,
			escape_state_to_string(ve->escape_state),
			ve->representant
		);
#if defined(ENABLE_REASON)
		{
			reason_t *re;
			for (re = ve->reasons; re; re = re->next) {
				printf("ESCAPE_REASON: %s\n", re->why);
			}
		}
#endif
	}
}

void escape_analysis_perform(jitdata *jd) {
	escape_analysis_t *e;

	jd->m->flags |= ACC_METHOD_EA;

	e = DNEW(escape_analysis_t);
	escape_analysis_init(e, jd);

	if (e->verbose)
		color_printf(RED, "\n\n==== %s/%s ====\n\n", e->jd->m->clazz->name.begin(), e->jd->m->name.begin());

	escape_analysis_process_arguments(e);
	escape_analysis_process_instructions(e);
	escape_analysis_post_process_getfields(e);
	escape_analysis_post_process_returns(e);

	escape_analysis_export_arguments(e);
	if (e->verbose) escape_analysis_display(e);

	if (e->verbose) {
		int i, j, r;
		for (i = 0; i < jd->vartop; ++i) {
			r = var_extra_get_representant(e, i);
			if (i == r) {
				printf("EES of %d: ", i);
				for (j = 0; j < jd->vartop; ++j) {
					if (var_extra_get_representant(e, j) == r) {
						printf("%d, ", j);
					}
				}
				printf("\n");
			}
		}
		printf("\n");
	}

	escape_analysis_mark_allocations(e);
	escape_analysis_mark_monitors(e);

	jd->m->flags &= ~ACC_METHOD_EA;
}

void escape_analysis_escape_check(void *vp) {
}

/*** monomorphic *************************************************************/

bool escape_is_monomorphic(methodinfo *caller, methodinfo *callee) {

	/* Non-speculative case */

	if (callee->flags & (ACC_STATIC | ACC_FINAL | ACC_PRIVATE)) {
		return true;
	}

	if (
		(callee->flags & (ACC_METHOD_MONOMORPHIC | ACC_METHOD_IMPLEMENTED| ACC_ABSTRACT))
		== (ACC_METHOD_MONOMORPHIC | ACC_METHOD_IMPLEMENTED)
	) {

		/* Mark that we have used the information about monomorphy. */

		callee->flags |= ACC_METHOD_MONOMORPHY_USED;

		/* We assume the callee is monomorphic. */

		method_add_assumption_monomorphic(caller, callee);

		return true;
	}

	return false;
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
