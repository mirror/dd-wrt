/* SMCRoute script API */
#ifndef SMCROUTE_SCRIPT_H_
#define SMCROUTE_SCRIPT_H_

#include "mroute.h"

int script_init (char *script);
int script_exec (struct mroute *mroute);

#endif /* SMCROUTE_SCRIPT_H_ */
