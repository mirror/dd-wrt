/*
 * Unitlist
 *
 * $Id: units.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */
#ifndef LANE_UNIT_H
#define LANE_UNIT_H

/* System includes needed for types */

/* Local includes needed for types */

/* Type definitions */
typedef void (* VoidFunc_t)(void);

typedef struct {
  const char *name;
  VoidFunc_t init0;
  VoidFunc_t init1;
  VoidFunc_t dump;
  VoidFunc_t release;
} Unit_t;

/* Global function prototypes */
/* Find unit */
const Unit_t *find_unit(const char *name);

/* Global data */
extern const Unit_t *unitlist[];
extern const unsigned int num_units;

/* Enumerate units */
#define FOR_ALL_UNITS(unit) for (unit = &unitlist[0]; *unit != NULL; unit++)
#define FOR_ALL_UNITS_REV(unit) for (unit = &unitlist[num_units-1]; unit >= &unitlist[0]; unit--)

#endif

