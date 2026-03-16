/* $Id: seteuid.c,v 1.7 2010/09/26 13:26:59 karls Exp $ */

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#include "osdep.h"

int
setegid(gid_t egid)
{
   return setresgid(-1, egid, -1);
}

int
seteuid(uid_t euid)
{
   return setreuid(-1, euid);
}
