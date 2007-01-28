/*
 * $Id: stub_memaccount.c,v 1.6 2006/09/18 22:54:38 hno Exp $
 */

/* Stub function for programs not implementing statMemoryAccounted */
#include "config.h"
#include "util.h"
size_t
statMemoryAccounted(void)
{
    return -1;
}
