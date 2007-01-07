/*
 * screenfilter.h:
 *
 * Copyright (c) 2002 DecisionSoft Ltd.
 * Paul Warren (pdw) Fri Oct 25 10:25:50 2002
 *
 * RCS: $Id: screenfilter.h,v 1.2 2002/11/04 12:27:35 chris Exp $
 */

#ifndef __SCREENFILTER_H_ /* include guard */
#define __SCREENFILTER_H_

#include "config.h"

#ifdef HAVE_REGCOMP

int screen_filter_set(char* s);
int screen_filter_match(char* s);

#endif

#endif /* __SCREENFILTER_H_ */
