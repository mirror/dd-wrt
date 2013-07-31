/*
 *	Filters: utility functions
 *
 *	Copyright 1998 Pavel Machek <pavel@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "conf/conf.h"
#include "filter/filter.h"

#define P(a,b) ((a<<8) | b)

struct f_inst *
f_new_inst(void)
{
  struct f_inst * ret;
  ret = cfg_alloc(sizeof(struct f_inst));
  ret->code = ret->aux = 0;
  ret->arg1 = ret->arg2 = ret->next = NULL;
  ret->lineno = ifs->lino;
  return ret;
}

struct f_inst *
f_new_dynamic_attr(int type, int f_type UNUSED, int code)
{
  /* FIXME: Remove the f_type parameter? */
  struct f_inst *f = f_new_inst();
  f->aux = type;
  f->a2.i = code;
  return f;
}

/*
 * Generate set_dynamic( operation( get_dynamic(), argument ) )
 */
struct f_inst *
f_generate_complex(int operation, int operation_aux, struct f_inst *dyn, struct f_inst *argument)
{
  struct f_inst *set_dyn = f_new_inst(),
                *oper = f_new_inst(),
                *get_dyn = dyn;

  *set_dyn = *get_dyn;
  get_dyn->code = P('e','a');
  oper->code = operation;
  oper->aux = operation_aux;
  oper->a1.p = get_dyn;
  oper->a2.p = argument;
  set_dyn->code = P('e','S');
  set_dyn->a1.p = oper;
  return set_dyn;
}


struct f_inst *
f_generate_roa_check(struct symbol *sym, struct f_inst *prefix, struct f_inst *asn)
{
  struct f_inst_roa_check *ret = cfg_allocz(sizeof(struct f_inst_roa_check));
  ret->i.code = P('R','C');
  ret->i.lineno = ifs->lino;
  ret->i.arg1 = prefix;
  ret->i.arg2 = asn;
  /* prefix == NULL <-> asn == NULL */

  if ((sym->class != SYM_ROA) || ! sym->def)
    cf_error("%s is not a ROA table", sym->name);
  ret->rtc = sym->def;

  return &ret->i;
}

char *
filter_name(struct filter *filter)
{
  if (!filter)
    return "ACCEPT";
  else if (filter == FILTER_REJECT)
    return "REJECT";
  else if (!filter->name)
    return "(unnamed)";
  else
    return filter->name;
}
