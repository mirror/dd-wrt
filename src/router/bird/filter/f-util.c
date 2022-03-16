/*
 *	Filters: utility functions
 *
 *	Copyright 1998 Pavel Machek <pavel@ucw.cz>
 *		  2017 Jan Maria Matejka <mq@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "conf/conf.h"
#include "filter/filter.h"

#define P(a,b) ((a<<8) | b)

struct f_inst *
f_new_inst(enum f_instruction_code fi_code)
{
  struct f_inst * ret;
  ret = cfg_allocz(sizeof(struct f_inst));
  ret->fi_code = fi_code;
  ret->lineno = ifs->lino;
  return ret;
}

struct f_inst *
f_new_inst_da(enum f_instruction_code fi_code, struct f_dynamic_attr da)
{
  struct f_inst *ret = f_new_inst(fi_code);
  ret->aux = da.type;
  ret->a2.i = da.ea_code;
  return ret;
}

struct f_inst *
f_new_inst_sa(enum f_instruction_code fi_code, struct f_static_attr sa)
{
  struct f_inst *ret = f_new_inst(fi_code);
  ret->aux = sa.f_type;
  ret->a2.i = sa.sa_code;
  ret->a1.i = sa.readonly;
  return ret;
}

/*
 * Generate set_dynamic( operation( get_dynamic(), argument ) )
 */
struct f_inst *
f_generate_complex(int operation, int operation_aux, struct f_dynamic_attr da, struct f_inst *argument)
{
  struct f_inst *set_dyn = f_new_inst_da(FI_EA_SET, da),
                *oper = f_new_inst(operation),
                *get_dyn = f_new_inst_da(FI_EA_GET, da);

  oper->aux = operation_aux;
  oper->a1.p = get_dyn;
  oper->a2.p = argument;

  set_dyn->a1.p = oper;
  return set_dyn;
}


struct f_inst *
f_generate_roa_check(struct symbol *sym, struct f_inst *prefix, struct f_inst *asn)
{
  struct f_inst_roa_check *ret = cfg_allocz(sizeof(struct f_inst_roa_check));
  ret->i.fi_code = FI_ROA_CHECK;
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
