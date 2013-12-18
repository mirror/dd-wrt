/* (c) D.Souflis dsouflis@acm.org */

#ifndef _WINSTUBDL_H
#define _WINSTUBDL_H

#ifdef WIN32
# define WIN32DLL_DEFINE __declspec( dllexport)
#else
# define WIN32DLL_DEFINE
#endif

#ifdef WIN32_DLOPEN
#include <windows.h>
#ifndef RTLD_LAZY
# define RTLD_LAZY 1
#endif
void *dlopen(const char *module, int unused);
void* dlsym(void* mo, const char *proc);
void dlclose(void* mo);
#endif

#endif
