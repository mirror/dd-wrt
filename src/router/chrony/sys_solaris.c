/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Driver file for Solaris operating system
  */

#include "config.h"

#include "sysincl.h"

#include "privops.h"
#include "sys_solaris.h"
#include "sys_timex.h"
#include "util.h"

/* ================================================== */

void
SYS_Solaris_Initialise(void)
{
  /* The kernel allows the frequency to be set in the full range off int32_t */
  SYS_Timex_InitialiseWithFunctions(32500, 1.0 / 100, NULL, NULL, NULL,
                                    0.0, 0.0, NULL, NULL);
}

/* ================================================== */

void
SYS_Solaris_Finalise(void)
{
  SYS_Timex_Finalise();
}

/* ================================================== */

#ifdef FEAT_PRIVDROP
void
SYS_Solaris_DropRoot(uid_t uid, gid_t gid)
{
  PRV_StartHelper();
  UTI_DropRoot(uid, gid);
}
#endif
