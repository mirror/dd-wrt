#ifndef _PUD_CONFIGTOOLS_H_
#define _PUD_CONFIGTOOLS_H_

/* Plugin includes */

/* OLSR includes */
#include "olsr_types.h"

/* System includes */
#include <stdbool.h>

bool readBool(const char * parameterName, const char * str, bool * dst);

bool readUC(const char * parameterName, const char * str, unsigned char * dst);

bool readUS(const char * parameterName, const char * str, unsigned short * dst);

bool readULL(const char * parameterName, const char * str, unsigned long long * dst, int base);

bool readDouble(const char * parameterName, const char * str, double * dst);

bool readIPAddress(const char * parameterName, const char * ipStr, unsigned short portDefault,
		union olsr_sockaddr * dst, bool * dstSet);

#endif /* PUD_CONFIGTOOLS_H_ */
