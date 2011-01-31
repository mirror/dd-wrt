/* common.h - Common definitions */
 
/* Written 1995,1996 by Werner Almesberger, EPFL-LRC */
 

#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>


#define alloc_t(t) ((t *) alloc(sizeof(t)))


extern int debug;
extern int dump;


void *alloc(size_t size);
char *stralloc(const char *str);
void die(const char *fmt,...);

#endif
