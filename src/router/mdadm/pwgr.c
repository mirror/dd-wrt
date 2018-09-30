
/*
 * We cannot link a static binary with passwd/group support, so
 * just do without
 */
#include	<stdlib.h>
#include	<pwd.h>
#include	<grp.h>

struct passwd *getpwnam(const char *name)
{
	return NULL;
}
struct group *getgrnam(const char *name)
{
	return NULL;
}
