/* User functions for run-time dynamic loading.  libdl version */
#ifndef	_DLFCN_H
#define	_DLFCN_H 1

#include <features.h>
#include <bits/dlfcn.h>

#define RTLD_NEXT	((void *) -1l)
#define RTLD_DEFAULT	((void *) 0)

/* Structure containing information about object searched using
   `dladdr'.  */
typedef struct
{
	__const char *dli_fname;	/* File name of defining object.  */
	void *dli_fbase;		/* Load address of that object.  */
	__const char *dli_sname;	/* Name of nearest symbol.  */
	void *dli_saddr;		/* Exact value of nearest symbol.  */
} Dl_info;


#endif	/* dlfcn.h */
