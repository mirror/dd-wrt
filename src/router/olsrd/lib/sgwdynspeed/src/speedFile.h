#ifndef SPEEDFILE_H
#define SPEEDFILE_H

/* Plugin includes */

/* OLSRD includes */

/* System includes */
#include <stdbool.h>

bool startSpeedFile(void);
void stopSpeedFile(void);
void readSpeedFile(char * fileName);

#endif /* SPEEDFILE_H */
