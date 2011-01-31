/* itf.h - IP interface registry */
 
/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef ITF_H
#define ITF_H

#include <stdint.h>

#include "table.h"

ITF *lookup_itf(int number);
ITF *lookup_itf_by_ip(uint32_t ip);
void itf_create(int number);
void itf_up(int number);
void itf_down(int number);
void itf_change(int number);

#endif
