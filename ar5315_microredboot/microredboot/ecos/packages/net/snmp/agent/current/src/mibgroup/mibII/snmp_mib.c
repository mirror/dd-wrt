//==========================================================================
//
//      ./agent/current/src/mibgroup/mibII/snmp_mib.c
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/*
 *  SNMPv1 MIB group implementation - snmp.c
 *
 */

#include <config.h>
#include <sys/types.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include "mibincl.h"
#include "system.h"
#include "mibgroup/util_funcs.h"

#include "mibgroup/mibII/snmp_mib.h"
#include "mibgroup/mibII/sysORTable.h"


	/*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

extern int snmp_enableauthentraps;
       int old_snmp_enableauthentraps;

/*********************
 *
 *  Initialisation & common implementation functions
 *
 *********************/

/* define the structure we're going to ask the agent to register our
   information at */
struct variable2 snmp_variables[] = {
    {SNMPINPKTS, ASN_COUNTER, RONLY, var_snmp, 1, {1}},
    {SNMPOUTPKTS, ASN_COUNTER, RONLY, var_snmp, 1, {2}},
    {SNMPINBADVERSIONS, ASN_COUNTER, RONLY, var_snmp, 1, {3}},
    {SNMPINBADCOMMUNITYNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {4}},
    {SNMPINBADCOMMUNITYUSES, ASN_COUNTER, RONLY, var_snmp, 1, {5}},
    {SNMPINASNPARSEERRORS, ASN_COUNTER, RONLY, var_snmp, 1, {6}},
    {SNMPINTOOBIGS, ASN_COUNTER, RONLY, var_snmp, 1, {8}},
    {SNMPINNOSUCHNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {9}},
    {SNMPINBADVALUES, ASN_COUNTER, RONLY, var_snmp, 1, {10}},
    {SNMPINREADONLYS, ASN_COUNTER, RONLY, var_snmp, 1, {11}},
    {SNMPINGENERRS, ASN_COUNTER, RONLY, var_snmp, 1, {12}},
    {SNMPINTOTALREQVARS, ASN_COUNTER, RONLY, var_snmp, 1, {13}},
    {SNMPINTOTALSETVARS, ASN_COUNTER, RONLY, var_snmp, 1, {14}},
    {SNMPINGETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {15}},
    {SNMPINGETNEXTS, ASN_COUNTER, RONLY, var_snmp, 1, {16}},
    {SNMPINSETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {17}},
    {SNMPINGETRESPONSES, ASN_COUNTER, RONLY, var_snmp, 1, {18}},
    {SNMPINTRAPS, ASN_COUNTER, RONLY, var_snmp, 1, {19}},
    {SNMPOUTTOOBIGS, ASN_COUNTER, RONLY, var_snmp, 1, {20}},
    {SNMPOUTNOSUCHNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {21}},
    {SNMPOUTBADVALUES, ASN_COUNTER, RONLY, var_snmp, 1, {22}},
    {SNMPOUTGENERRS, ASN_COUNTER, RONLY, var_snmp, 1, {24}},
    {SNMPOUTGETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {25}},
    {SNMPOUTGETNEXTS, ASN_COUNTER, RONLY, var_snmp, 1, {26}},
    {SNMPOUTSETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {27}},
    {SNMPOUTGETRESPONSES, ASN_COUNTER, RONLY, var_snmp, 1, {28}},
    {SNMPOUTTRAPS, ASN_COUNTER, RONLY, var_snmp, 1, {29}},
    {SNMPENABLEAUTHENTRAPS, ASN_INTEGER, RWRITE, var_snmp, 1, {30}},
    {SNMPSILENTDROPS, ASN_COUNTER, RONLY, var_snmp, 1, {31}},
    {SNMPPROXYDROPS, ASN_COUNTER, RONLY, var_snmp, 1, {32}}
};

/* Define the OID pointer to the top of the mib tree that we're
   registering underneath */
oid snmp_variables_oid[] = { SNMP_OID_MIB2,11 };
#ifdef USING_MIBII_SYSTEM_MIB_MODULE
extern oid system_module_oid[];
extern int system_module_oid_len;
extern int system_module_count;
#endif

void
init_snmp_mib(void) {
  /* register ourselves with the agent to handle our mib tree */
  REGISTER_MIB("mibII/snmp", snmp_variables, variable2, snmp_variables_oid);

#ifdef USING_MIBII_SYSTEM_MIB_MODULE
  if ( ++system_module_count == 3 )
	REGISTER_SYSOR_TABLE( system_module_oid, system_module_oid_len,
		"The MIB module for SNMPv2 entities");
#endif
}

/*
  header_snmp(...
  Arguments:
  vp	  IN      - pointer to variable entry that points here
  name    IN/OUT  - IN/name requested, OUT/name found
  length  IN/OUT  - length of IN/OUT oid's 
  exact   IN      - TRUE if an exact match was requested
  var_len OUT     - length of variable or 0 if function returned
  write_method
  
*/

	/*********************
	 *
	 *  System specific implementation functions
	 *	(actually common!)
	 *
	 *********************/


u_char *
var_snmp(struct variable *vp,
	 oid *name,
	 size_t *length,
	 int exact,
	 size_t *var_len,
	 WriteMethod **write_method)
{
  static long long_ret;

  *write_method = 0;         /* assume it isnt writable for the time being */
  *var_len = sizeof(long_ret); /* assume an integer and change later if not */

  if (header_generic(vp, name, length, exact, var_len, write_method)
      == MATCH_FAILED)
    return NULL;

    /* this is where we do the value assignments for the mib results. */
  if (vp->magic == SNMPENABLEAUTHENTRAPS) {
    *write_method = write_snmp;
    long_return = snmp_enableauthentraps;
    return (u_char *) &long_return;
  } else if ( (vp->magic >= 1)
       && (vp->magic <= (STAT_SNMP_STATS_END - STAT_SNMP_STATS_START + 1)) ) {
    long_ret = snmp_get_statistic(vp->magic + STAT_SNMP_STATS_START - 1);
    return (unsigned char *) &long_ret;
  }
  return NULL;
}

/*
 * only for snmpEnableAuthenTraps:
 */

int
write_snmp (int action,
	    u_char *var_val,
	    u_char var_val_type,
	    size_t var_val_len,
	    u_char *statP,
	    oid *name,
	    size_t name_len)
{
    long intval = *((long *) var_val);

    switch ( action ) {
	case RESERVE1:			/* Check values for acceptability */
	    if (var_val_type != ASN_INTEGER){
	        DEBUGMSGTL(("mibII/snmp_mib", "%x not integer type", var_val_type));
		return SNMP_ERR_WRONGTYPE;
	    }
	
	    if (intval != 1 && intval != 2) {
	        DEBUGMSGTL(("mibII/snmp_mib", "not valid %x\n", intval));
		return SNMP_ERR_WRONGVALUE;
	    }
	    break;

	case RESERVE2:			/* Allocate memory and similar resources */

		/* Using static variables, so nothing needs to be done */
	    break;

	case ACTION:			/* Perform the SET action (if reversible) */

		/* Save the old value, in case of UNDO */
	    old_snmp_enableauthentraps = snmp_enableauthentraps;
	    snmp_enableauthentraps = intval;	
	    break;

	case UNDO:			/* Reverse the SET action and free resources */

	    snmp_enableauthentraps = old_snmp_enableauthentraps;
	    break;

	case COMMIT:			/* Confirm the SET, performing any irreversible actions,
						and free resources */
	    /* save_into_conffile ("authentraps:", intval == 1 ? "yes" : "no"); */
	    break;

	case FREE:			/* Free any resources allocated */
	    break;
    }
    return SNMP_ERR_NOERROR;
}

/*********************
 *
 *  Internal implementation functions
 *
 *********************/
