/*
 * Configuration file loader
 *
 * $Id: load.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */
#ifndef LANE_LOAD_H
#define LANE_LOAD_H

/* System includes needed for types */

/* Local includes needed for types */
#include "units.h"
#include "lane.h"

/* Type definitions */
typedef enum {
  BL_FALSE=0, BL_TRUE
} Bool_t;

/* Global function prototypes */
/* Get or initialize variable */
int get_var_int(const Unit_t *unit, const char *varname);
const char *get_var_str(const Unit_t *unit, const char *varname);
Bool_t get_var_bool(const Unit_t *unit, const char *varname);
const AtmAddr_t *get_var_addr(const Unit_t *unit, const char *varname);
const InitPvc_t *get_var_vcc(const Unit_t *unit, const char *varname);

/* Set or initialize variable */
void set_var_int(const Unit_t *unit, const char *varname, int intval);
void set_var_str(const Unit_t *unit, const char *varname, const char *strval);
void set_var_bool(const Unit_t *unit, const char *varname, Bool_t boolval);
void set_var_addr(const Unit_t *unit, const char *varname, const AtmAddr_t *addr);
void set_var_vcc(const Unit_t *unit, const char *varname, const InitPvc_t *vcc);
/* Dump variable definitions, NULL == all */
void dump_vars(const Unit_t *unit);

/* Global data */
#define DEFAULT_CFG_FILE ".lanevars"

extern const Unit_t load_unit;
#endif

