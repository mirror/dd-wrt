/******************************************************************************
 *
 * Name:    fwoids.h
 * Project: fwcommon
 * Version: $Revision: #4 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware sepcific oids handling
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#ifndef __SK_FWOIDS_H__
#define __SK_FWOIDS_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int FwSetOid(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U32 Id,  /* OID  */
            SK_U32 Inst,
            SK_U8 *pBuf,
            unsigned int *pLen);
int FwGetOid(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U32 Id,  /* OID  */
            SK_U32 Inst,
            SK_U8 *pBuf,
            unsigned int *pLen);
int FwGet(
            SK_AC *pAC,    /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U8 *pBuf,
            unsigned int *pLen);
int FwPreSet(
			SK_AC *pAC, /* the adapter context */
			SK_IOC IoC, /* I/O context */
			SK_U8 *pBuf,
			unsigned int *pLen);
int FwPreSetOid(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U32 Id,  /* OID  */
            SK_U32 Inst,
            SK_U8 *pBuf,
            unsigned int *pLen);
int FwSet(
            SK_AC *pAC, /* the adapter context */
            SK_IOC IoC, /* I/O context */
            SK_U8 *pBuf,
            unsigned int *pLen);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWOIDS_H__ */

