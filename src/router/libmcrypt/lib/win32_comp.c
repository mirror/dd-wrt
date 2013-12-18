/* (c) D.Souflis dsouflis@acm.org */

#include "libdefs.h"
#include "win32_comp.h"

#ifdef WIN32
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
#endif

#ifdef WIN32_DLOPEN

void *dlopen(const char *module, int unused)
{
	return (void *) LoadLibrary(module);
}

void *dlsym(void *mo, const char *proc)
{
	return (void *) GetProcAddress((HMODULE) mo, proc);
}

void dlclose(void *mo)
{
	(void) FreeLibrary((HMODULE) mo);
}

#endif
