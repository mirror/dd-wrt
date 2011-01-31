/*
 * Configuration file loader
 *
 * $Id: load.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <syslog.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

/* Local includes */
#include "load.h"
#include "lane.h"
#include "units.h"
#include "dump.h"
#include "mem.h"
#include "load_lex.h"

/* Type definitions */
typedef enum {
  VT_INT, VT_STR, VT_BOOL, VT_ADDR, VT_PVC
} VarType_t;

typedef struct {
  const Unit_t *unit;
  const char *name;
  VarType_t type;
  union {
    int intval;
    const char *strval;
    Bool_t boolval;
    const AtmAddr_t *addrval;
    const InitPvc_t *init;
  } val_u;
} Var_t;

typedef struct _VarList_t {
  Var_t *var;
  struct _VarList_t *next;
} VarList_t;

/* Local function prototypes */
static void load_init0(void);
static void load_init1(void);
static void load_dump(void);
static void load_release(void);

static Var_t *find_var(const Unit_t *unit, const char *varname);
static void add_var(Var_t *var);
static void load_vars(const char *filename);


/* Data */
#define BUFLEN 256
static const char *rcsid = "$Id: load.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $";

static VarList_t *varlist;
extern char *var_file;

const Unit_t load_unit = {
  "load",
  &load_init0,
  &load_init1,
  &load_dump,
  &load_release
};

/* Functions */

/* Initialize local data */
static void
load_init0(void)
{
  varlist = NULL;
}

/* Initialization for data that needs other units */
static void
load_init1(void)
{
  set_var_str(&load_unit, "version", rcsid);
  /*
   * Load variables from file
   * For example:
   * [main]
   * debug=True
   * memdebug=True
   * [conn]
   * #S4, Join Timeout, s
   * S4=60
   */
  load_vars(var_file);

  Debug_unit(&load_unit, "Initialized.");
}

/* Dump status, local data etc. */
static void
load_dump(void)
{
  dump_vars(NULL);
}

/* Release allocated memory, close files etc. */
static void
load_release(void)
{
  VarList_t *tmp;
  LaneDestList_t *ltmp, *ltmp2;

  Debug_unit(&load_unit, "Releasing unit");
  for (tmp = varlist; tmp != NULL;) {
    Debug_unit(&load_unit, "Freeing var %s/%s", tmp->var->unit->name, tmp->var->name);
    assert(tmp->var != NULL);
    assert(tmp->var->name != NULL);
    varlist = varlist->next;
    if (tmp->var->type == VT_STR){
      assert(tmp->var->val_u.strval != NULL);
      mem_free(&load_unit, tmp->var->val_u.strval);
    }
    if (tmp->var->type == VT_ADDR){
      assert(tmp->var->val_u.addrval != NULL);
      mem_free(&load_unit, tmp->var->val_u.addrval);
    }
    if (tmp->var->type == VT_PVC){
      assert(tmp->var->val_u.init != NULL);
      assert(tmp->var->val_u.init->pvc != NULL);
      mem_free(&load_unit, tmp->var->val_u.init->pvc);
      assert(tmp->var->val_u.init->address != NULL);
      mem_free(&load_unit, tmp->var->val_u.init->address);
      ltmp = tmp->var->val_u.init->destinations;
      while (ltmp != NULL) {
	ltmp2 = ltmp->next;
	assert(ltmp->addr != NULL);
	mem_free(&load_unit, ltmp->addr);
	mem_free(&load_unit, ltmp);
	ltmp = ltmp2;
      }
      mem_free(&load_unit, tmp->var->val_u.init);
    }
    mem_free(&load_unit, tmp->var->name);
    mem_free(&load_unit, tmp->var);
    mem_free(&load_unit, tmp);
    tmp = varlist;
  }
}

static Var_t *
find_var(const Unit_t *unit, const char *varname)
{
  VarList_t *tmp;

  assert(unit != NULL);
  assert(unit->name != NULL);
  assert(varname != NULL);

  for (tmp = varlist; tmp != NULL; tmp = tmp->next) {
    assert(tmp->var != NULL);
    assert(tmp->var->unit != NULL);
    assert(tmp->var->unit->name != NULL);
    assert(tmp->var->name != NULL);
    if (strcmp(unit->name, tmp->var->unit->name) == 0 && strcmp(tmp->var->name, varname) == 0) {
      break;
    }
  }
  if (tmp) {
    return tmp->var;
  }
  else {
    return NULL;
  }
}

/* Get or initialize variable */
int
get_var_int(const Unit_t *unit, const char *varname)
{
  const Var_t *tmp;

  tmp = find_var(unit, varname);
  if (tmp) {
    assert(tmp->type == VT_INT);
    return tmp->val_u.intval;
  }
  else {
    return 0;
  }
}

const InitPvc_t *get_var_vcc(const Unit_t *unit, const char *varname)
{
  const Var_t *tmp;

  tmp=find_var(unit, varname);
  if(tmp) {
    assert(tmp->type == VT_PVC);
    return tmp->val_u.init;
  } else {
    return NULL;
  }
}

const char *
get_var_str(const Unit_t *unit, const char *varname)
{
  const Var_t *tmp;

  tmp=find_var(unit, varname);
  if (tmp) {
    assert(tmp->type == VT_STR);
    return tmp->val_u.strval;
  }
  else {
    return NULL;
  }
}

Bool_t
get_var_bool(const Unit_t *unit, const char *varname)
{
  const Var_t *tmp;

  tmp = find_var(unit, varname);
  if (tmp) {
    assert(tmp->type == VT_BOOL);
    return tmp->val_u.boolval;
  }
  else {
    return BL_FALSE;
  }
}

const AtmAddr_t *
get_var_addr(const Unit_t *unit, const char *varname)
{
  const Var_t *tmp;

  tmp = find_var(unit, varname);
  if (tmp) {
    assert(tmp->type == VT_ADDR);
    return tmp->val_u.addrval;
  }
  else {
    return NULL;
  }
}

static void
add_var(Var_t *var)
{
  VarList_t *tmpl;

  tmpl = (VarList_t *)mem_alloc(&load_unit, sizeof(VarList_t));
  tmpl->var = var;
  tmpl->next = varlist;
  varlist = tmpl;
}

/* Set or initialize variable */

void 
set_var_vcc(const Unit_t *unit, const char *varname, 
	    const InitPvc_t *vcc)
{
  Var_t *tmp;
  LaneDestList_t *ltmp, *ltmp2;

  assert(unit != NULL && unit->name != NULL && varname != NULL);
  tmp = find_var(unit, varname);
  if (tmp == NULL) {
    tmp = (Var_t *)mem_alloc(&load_unit, sizeof(Var_t));
    tmp->unit = unit;
    tmp->name = varname;
    tmp->val_u.init = vcc;
    tmp->type = VT_PVC;
    add_var(tmp);
  } else {
    assert(tmp->type == VT_PVC);
    mem_free(&load_unit, tmp->name);
    tmp->name = varname;
    assert(tmp->val_u.init->pvc != NULL);
    mem_free(&load_unit, tmp->val_u.init->pvc);
    assert(tmp->val_u.init->address != NULL);
    mem_free(&load_unit, tmp->val_u.init->address);
    ltmp = tmp->val_u.init->destinations;
    while(ltmp != NULL) {
      ltmp2 = ltmp->next;
      assert(ltmp->addr != NULL);
      mem_free(&load_unit, ltmp->addr);
      mem_free(&load_unit, ltmp);
      ltmp = ltmp2;
    }
    mem_free(&load_unit, tmp->val_u.init);
    tmp->val_u.init = vcc;
  }
}

void
set_var_int(const Unit_t *unit, const char *varname, int intval)
{
  Var_t *tmp;

  assert(unit != NULL && unit->name != NULL && varname != NULL);
  tmp = find_var(unit, varname);
  if (tmp == NULL) {
    tmp = (Var_t *)mem_alloc(&load_unit, sizeof(Var_t));
    tmp->unit = unit;
    tmp->name = varname;
    tmp->val_u.intval = intval;
    tmp->type = VT_INT;
    add_var(tmp);
  }
  else {
    assert(tmp->type == VT_INT);
    mem_free(&load_unit, tmp->name);
    tmp->name = varname;
    tmp->val_u.intval = intval;
  }
}

void
set_var_str(const Unit_t *unit, const char *varname, const char *strval)
{
  Var_t *tmp;

  assert(unit != NULL && unit->name != NULL && varname != NULL);
  tmp = find_var(unit, varname);
  if (tmp == NULL) {
    tmp = (Var_t *)mem_alloc(&load_unit, sizeof(Var_t));
    tmp->unit = unit;
    tmp->name = varname;
    tmp->val_u.strval = strval;
    tmp->type = VT_STR;
    add_var(tmp);
  } else {
    assert(tmp->type == VT_STR);
    mem_free(&load_unit, tmp->name);
    tmp->name = varname;
    mem_free(&load_unit, tmp->val_u.strval);
    tmp->val_u.strval = strval;
  }
}


void
set_var_addr(const Unit_t *unit, const char *varname, const AtmAddr_t *addr)
{
  Var_t *tmp;

  assert(unit != NULL && unit->name != NULL && varname != NULL);
  tmp = find_var(unit, varname);
  if (tmp == NULL) {
    tmp = (Var_t *)mem_alloc(&load_unit, sizeof(Var_t));
    tmp->unit = unit;
    tmp->name = varname;
    tmp->val_u.addrval = addr;
    tmp->type = VT_ADDR;
    add_var(tmp);
  }
  else {
    assert(tmp->type == VT_ADDR);
    mem_free(&load_unit, tmp->name);
    tmp->name = varname;
    mem_free(&load_unit, tmp->val_u.addrval);
    tmp->val_u.addrval = addr;
  }
}

void
set_var_bool(const Unit_t *unit, const char *varname, Bool_t boolval)
{
  Var_t *tmp;

  assert(unit != NULL && unit->name != NULL && varname != NULL);
  tmp = find_var(unit, varname);
  if (tmp == NULL) {
    tmp = (Var_t *)mem_alloc(&load_unit, sizeof(Var_t));
    tmp->unit = unit;
    tmp->name = varname;
    tmp->val_u.boolval = boolval;
    tmp->type = VT_BOOL;
    add_var(tmp);
  }
  else {
    mem_free(&load_unit, tmp->name);
    tmp->name = varname;
    assert(tmp->type == VT_BOOL);
    tmp->val_u.boolval = boolval;
  }
}

void
dump_vars(const Unit_t *unit)
{
  const VarList_t *tmp;
  LaneDestList_t *ltmp;

  Debug_unit(&load_unit, "Dumping variables");
  for (tmp = varlist; tmp != NULL; tmp = tmp->next) {
    assert(tmp->var != NULL);
    assert(tmp->var->unit != NULL);
    assert(tmp->var->unit->name != NULL);
    assert(tmp->var->name != NULL);
    if (unit == NULL || strcmp(unit->name, tmp->var->unit->name) == 0) {
      switch (tmp->var->type) {
      case VT_INT:
	Debug_unit(&load_unit, "%s/%s = %d", tmp->var->unit->name, tmp->var->name, tmp->var->val_u.intval);
	break;
      case VT_STR:
	Debug_unit(&load_unit, "%s/%s = \"%s\"", tmp->var->unit->name, tmp->var->name, tmp->var->val_u.strval);
	break;
      case VT_BOOL:
	Debug_unit(&load_unit, "%s/%s = %s", tmp->var->unit->name, tmp->var->name, tmp->var->val_u.boolval == BL_TRUE? "True" : "False" );
	break;
      case VT_ADDR:
	Debug_unit(&load_unit, "%s/%s =", tmp->var->unit->name, tmp->var->name);
	dump_atmaddr(tmp->var->val_u.addrval);
	break;
      case VT_PVC:
	Debug_unit(&load_unit,"%s/%s = %d,%d,%d with lecid:%d ",
		   tmp->var->unit->name, 
		   tmp->var->name, 
		   tmp->var->val_u.init->pvc->port,
		   tmp->var->val_u.init->pvc->vpi,
		   tmp->var->val_u.init->pvc->vci,
		   tmp->var->val_u.init->lecid);
	dump_atmaddr(tmp->var->val_u.init->address);
	ltmp = tmp->var->val_u.init->destinations;
	while(ltmp) {
	  dump_printf(EL_CONT,"\t");
	  dump_addr(ltmp->addr);
	  dump_printf(EL_CONT,"\n");
	  ltmp = ltmp->next;
	}
	break;
      }
    }
  }
}

void
load_vars(const char *file)
{
  const Unit_t *curr_unit = NULL;

  int ret = 0;   /* to silence gcc 2.7.2.1 */
  char *varname;
  InitPvc_t *pvc;
  LaneDestList_t *ltmp;
  int read_flag = 1;

  assert(file != NULL);
  Debug_unit(&load_unit, "Loading variables from file %s", file);
  yyin = fopen(file, "r");
  if (!yyin) {
    Debug_unit(&load_unit, "Cannot open file %s: %s", file, strerror(errno));
    return;
  }
  g_buf_index = 0;
  do {
    if (read_flag)
      ret = yylex();
    else
      read_flag =1;
    switch(ret) {
    case END:
      Debug_unit(&load_unit, "EOF");
      break;
    case UNIT:
      Debug_unit(&load_unit, "Got unit %s", g_return.stringgi);
      curr_unit = find_unit(g_return.stringgi);
      if (curr_unit == NULL) {
	Debug_unit(&load_unit, "Unknown unit %s", g_return.stringgi);
      }
      Debug_unit(&load_unit, "Got unit %s", g_return.stringgi);
      mem_free(&load_unit,g_return.stringgi);
      break;
    case VARNAME:
      varname = g_return.stringgi;
      Debug_unit(&load_unit, "Got variable name %s", varname);
      ret = yylex();
      switch(ret) {
      case STRING:
	Debug_unit(&load_unit, "Variable is string: %s", g_return.stringgi);
	set_var_str(curr_unit, varname, g_return.stringgi);
	break;
      case BOOLEAN:
	Debug_unit(&load_unit, "Variable is boolean: %s", 
		   g_return.bool==BL_TRUE?"True":"False");
	set_var_bool(curr_unit, varname, g_return.bool);
	break;
      case INTEGER:
	Debug_unit(&load_unit, "Variable is integer: %d", g_return.intti);
	set_var_int(curr_unit, varname, g_return.intti);
	break;
      case ATMADDRESS:
	Debug_unit(&load_unit, "Variable is atmaddress ");
	dump_atmaddr(g_return.atmaddress);
	set_var_addr(curr_unit, varname, g_return.atmaddress);
	break;
      case LANEDEST:
	Debug_unit(&load_unit, "Invalid variable value for %s", varname);
	mem_free(&load_unit, g_return.destaddr);
	break;
      case UNIT:
	Debug_unit(&load_unit, "Invalid variable value for %s", varname);
	mem_free(&load_unit, g_return.stringgi);
	break;
      case VCC:
	Debug_unit(&load_unit, "Variable is vcc");
	pvc = (InitPvc_t *)mem_alloc(curr_unit, sizeof(InitPvc_t));
	pvc->pvc = (LaneVcc_t *)mem_alloc(curr_unit, sizeof(LaneVcc_t));
	pvc->pvc->port = g_return.vcc.port;
	pvc->pvc->vpi = g_return.vcc.vpi;
	pvc->pvc->vci = g_return.vcc.vci;
	pvc->address = NULL;
	pvc->lecid = 0;
	pvc->destinations = NULL;
	ret = yylex();
	if (ret != ATMADDRESS) {
	  Debug_unit(&load_unit, "Invalid atm_address for pvc %d,%d,%d",
		     pvc->pvc->port, pvc->pvc->vpi, pvc->pvc->vci);
	  switch(ret) {
	  case UNIT:
	  case STRING:
	  case VARNAME:
	    mem_free(&load_unit, g_return.stringgi);
	    break;
	  case LANEDEST:
	    mem_free(&load_unit, g_return.destaddr);
	    break;
	  }
	} else {
	  pvc->address = g_return.atmaddress;
	}
	ret = yylex();
	if (ret != INTEGER) {
	  Debug_unit(&load_unit, "Invalid lecid for pvc %d,%d,%d\n",
		     pvc->pvc->port,pvc->pvc->vpi,pvc->pvc->vci);
	  switch(ret) {
	  case UNIT:
	  case STRING:
	  case VARNAME:
	    mem_free(&load_unit, g_return.stringgi);
	    break;
	  case LANEDEST:
	    mem_free(&load_unit, g_return.destaddr);
	    break;
	  case ATMADDRESS:
	    mem_free(&load_unit, g_return.atmaddress);
	    break;
	  }
	} else {
	  pvc->lecid = g_return.intti;
	}
	while((ret=yylex())==LANEDEST) {
	  ltmp=(LaneDestList_t *)mem_alloc(&load_unit, sizeof(LaneDestList_t));
	  ltmp->addr = g_return.destaddr;
	  ltmp->next = pvc->destinations;
	  pvc->destinations = ltmp;
	}
	read_flag=0;
	set_var_vcc(curr_unit, varname, pvc);
	break;	
      default:
	Debug_unit(&load_unit, "Invalid variable value for %s", varname);
	break;
      }
      break;
    case STRING:
      Debug_unit(&load_unit,"Invalid string placement %s",g_return.stringgi);
      mem_free(&load_unit, g_return.stringgi);
      break;
    case ATMADDRESS:
      Debug_unit(&load_unit, "Invalid atm address placement");
      mem_free(&load_unit, g_return.atmaddress);
      break;
    case LANEDEST:
      Debug_unit(&load_unit, "Invalid lane destination placement");
      mem_free(&load_unit, g_return.destaddr);
      break;
    case INTEGER:
      Debug_unit(&load_unit, "Invalid integer placement");
      break;
    default:
      Debug_unit(&load_unit, "Invalid input");
      break;
    }
  } while (ret != END);
  if (fclose(yyin) != 0) {
    Debug_unit(&load_unit, "Cannot close file %s: %s", file, strerror(errno));
  }
}

