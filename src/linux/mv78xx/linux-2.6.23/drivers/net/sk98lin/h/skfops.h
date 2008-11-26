
/******************************************************************************
 *
 * Name:    skfops.c
 * Project: Gigabit Ethernet Adapters, Common Modules
 * Version: $Revision: 1.1.2.2 $
 * Date:    $Date: 2006/09/18 11:55:54 $
 * Purpose: Kernel mode file read functions.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect
 *	(C)Copyright 2002-2003 Marvell
 *
 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SYSKONNECT
 *	The copyright notice above does not evidence any
 *	actual or intended publication of such source code.
 *
 *	This Module contains Proprietary Information of SysKonnect
 *	and should be treated as Confidential.
 *
 *	The information in this file is provided for the exclusive use of
 *	the licensees of SysKonnect.
 *	Such users have the right to use, modify, and incorporate this code
 *	into products for purposes authorized by the license agreement
 *	provided they include this notice and the associated copyright notice
 *	with any such product.
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/


SK_BOOL fw_read(	SK_AC *pAC,    /* Pointer to adapter context */
	char *name, SK_U8 **addr, SK_U32 *len );
SK_BOOL fw_file_exists(	SK_AC *pAC,    /* Pointer to adapter context */
	char *name );

