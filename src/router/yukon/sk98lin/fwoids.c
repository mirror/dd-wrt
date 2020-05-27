/******************************************************************************
 *
 * Name:    fwoids.c
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

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/******************************************************************************
*
*   FwSetOid -
*
* Context:
*   init, pageable
*
* Returns:
*/
int FwSetOid(
	SK_AC *pAC, /* the adapter context */
	SK_IOC IoC, /* I/O context */
	SK_U32 Id,  /* OID  */
	SK_U32 Inst,
	SK_U8 *pBuf,
	unsigned int *pLen)
{
	SK_U32     RetLocVal;
	SK_U16     i;
	SK_U8      *pTmp;
	SK_U32     TmpLen = 0;
	SK_U32     RetCode = SK_ASF_PNMI_ERR_OK;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_INIT,("FwSetOid -> OID:0x%x Len:%d ***********************\n", Id, *pLen ));
	SK_DBG_DMP(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, pBuf, *pLen );

	if( pAC->FwApp.InitState != ASF_INIT_OK )  {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("FwSetOid -> Set OID denied, ASF not initialized !\n"));
		return( SK_ASF_PNMI_ERR_GENERAL );
	}

	/*  Check if the buffer used by OIDs is valid.	 */
	if(pBuf == NULL){
		 return( SK_ASF_PNMI_ERR_GENERAL );
	}

	switch( Id )  {		
		case OID_SKGE_ASF_STORE_CONFIG:
			pAC->FwApp.Mib.NewParam = 4;
			break;

		case OID_SKGE_ASF_DASH_ENA:
			if( *pLen == 1 )  {
				if(pAC->FwApp.NewOpMode == SK_GEASF_MODE_DASH)  {
					if( *pBuf != pAC->FwApp.Mib.Ena )
						pAC->FwApp.Mib.ConfigChange |= 0x01;
					pAC->FwApp.Mib.Ena = *pBuf;
				}
				else  {
					if( *pBuf != 0 )  {
						if( *pBuf != pAC->FwApp.Mib.Ena )
							pAC->FwApp.Mib.ConfigChange |= 0x01;
						pAC->FwApp.Mib.Ena = *pBuf;
					}
				}
				
				*pLen = 1;
				if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
					if( *pBuf != 0 ) 
						pAC->FwApp.NewOpMode = SK_GEASF_MODE_DASH;
				}
				else  {
					pAC->FwApp.NewOpMode = SK_GEASF_MODE_ASF;
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_DASH_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_ENA:
			if( *pLen == 1 )  {
				if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
					if(pAC->FwApp.NewOpMode == SK_GEASF_MODE_DASH_ASF)  {
						if( *pBuf != pAC->FwApp.Mib.Ena )
							pAC->FwApp.Mib.ConfigChange |= 0x01;
						pAC->FwApp.Mib.Ena = *pBuf;
					}
					else  {
						if( *pBuf != 0 )  {
							if( *pBuf != pAC->FwApp.Mib.Ena )
								pAC->FwApp.Mib.ConfigChange |= 0x01;
							pAC->FwApp.Mib.Ena = *pBuf;
						}
					}
					*pLen = 1;
					if( *pBuf != 0 )  
						pAC->FwApp.NewOpMode = SK_GEASF_MODE_DASH_ASF;
				}
				else  {
					if( *pBuf != pAC->FwApp.Mib.Ena )
						pAC->FwApp.Mib.ConfigChange |= 0x01;
					pAC->FwApp.Mib.Ena = *pBuf;
					*pLen = 1;
					pAC->FwApp.NewOpMode = SK_GEASF_MODE_ASF;
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",  pAC->FwApp.Mib.Ena ));				
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_RETRANS:
			if( *pLen == 2 )  {
				pAC->FwApp.Mib.Retrans = *( (SK_U16 *) pBuf );
				*pLen = 2;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS: %d\n",  pAC->FwApp.Mib.Retrans ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_RETRANS_INT:
			if( *pLen == 2 )  {
				pAC->FwApp.Mib.RetransInt = *( (SK_U16 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 2;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT: %d\n",  pAC->FwApp.Mib.RetransInt ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_HB_ENA:
			if( *pLen == 1 )  {
				pAC->FwApp.Mib.HbEna = *pBuf;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_ENA: %d\n",  pAC->FwApp.Mib.HbEna ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_HB_INT:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.HbInt = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT: %d\n",  pAC->FwApp.Mib.HbInt ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_WD_ENA:
			if( *pLen == 1 )  {
				pAC->FwApp.Mib.WdEna = *pBuf;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_ENA: %d\n",  pAC->FwApp.Mib.WdEna ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_WD_TIME:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.WdTime = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME: %d\n",  pAC->FwApp.Mib.WdTime ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_IP_DEST:
			if( AsfAsci2Ip( pBuf, *pLen, pAC->FwApp.Mib.IpDest )  == 1 )   {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_IP_DEST: %d.%d.%d.%d\n",
										pAC->FwApp.Mib.IpDest[0], pAC->FwApp.Mib.IpDest[1],
										pAC->FwApp.Mib.IpDest[2], pAC->FwApp.Mib.IpDest[3] ));
			}
			else
				RetCode = SK_PNMI_ERR_GENERAL;
			break;

		case OID_SKGE_ASF_MAC_DEST:
			if( AsfAsci2Mac( pBuf, *pLen, pAC->FwApp.Mib.MacDest )  == 1 )   {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_MAC_DEST: %02x:%02x:%02x:%02x:%02x:%02x\n",
										pAC->FwApp.Mib.MacDest[0], pAC->FwApp.Mib.MacDest[1],
										pAC->FwApp.Mib.MacDest[2], pAC->FwApp.Mib.MacDest[3],
										pAC->FwApp.Mib.MacDest[4], pAC->FwApp.Mib.MacDest[5] ));
#ifdef ASF_FW_ARP_RESOLVE
			/* just for FW-ARP-Resolve testing purposes */
			/* for( i=0; i<6; i++ ) */
			/*     pAC->FwApp.Mib.MacDest[i] = 0; */
#endif  /* ASF_FW_ARP_RESOLVE */

			}
			else
				RetCode = SK_PNMI_ERR_GENERAL;
			break;

		case OID_SKGE_ASF_COMMUNITY_NAME:
			for( i=0; (i<*pLen)&&(i<63); i++ )
				pAC->FwApp.Mib.CommunityName[i] = *(pBuf + i);
			pAC->FwApp.Mib.CommunityName[i] = 0;
			break;

		case OID_SKGE_ASF_RSP_ENA:
			if( *pLen == 1 )  {
				if( *pBuf != pAC->FwApp.Mib.RspEnable )
					pAC->FwApp.Mib.ConfigChange |= 0x02;

				pAC->FwApp.Mib.RspEnable = *pBuf;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RSP_ENA: %d\n",  pAC->FwApp.Mib.RspEnable ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

			case OID_SKGE_ASF_RETRANS_COUNT_MIN:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.RetransCountMin = *( (SK_U32 *) pBuf );
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MIN: %d\n",  pAC->FwApp.Mib.RetransCountMin ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_RETRANS_COUNT_MAX:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.RetransCountMax = *( (SK_U32 *) pBuf );
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MAX: %d\n",  pAC->FwApp.Mib.RetransCountMax ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_RETRANS_INT_MIN:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.RetransIntMin = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MIN: %d\n",  pAC->FwApp.Mib.RetransIntMin ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_RETRANS_INT_MAX:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.RetransIntMax = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MAX: %d\n",  pAC->FwApp.Mib.RetransIntMax ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_HB_INT_MIN:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.HbIntMin = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MIN: %d\n",  pAC->FwApp.Mib.HbIntMin ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_HB_INT_MAX:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.HbIntMax = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MAX: %d\n",  pAC->FwApp.Mib.HbIntMax ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_WD_TIME_MIN:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.WdTimeMin = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MIN: %d\n",  pAC->FwApp.Mib.WdTimeMin ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_WD_TIME_MAX:
			if( *pLen == 4 )  {
				pAC->FwApp.Mib.WdTimeMax = *( (SK_U32 *) pBuf ) * ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MAX: %d\n",  pAC->FwApp.Mib.WdTimeMax ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_KEY_OP:
			if( *pLen == (RSP_KEYLENGTH*2) )  {
				AsfHex2Array( pBuf, *pLen, pAC->FwApp.Mib.KeyOperator );
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_OP:\n >"));
				for( i=0; i<RSP_KEYLENGTH; i++ )
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->FwApp.Mib.KeyOperator[i] ));
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_KEY_ADM:
			if( *pLen == (RSP_KEYLENGTH*2) )  {
				AsfHex2Array( pBuf, *pLen, pAC->FwApp.Mib.KeyAdministrator );
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_ADM:\n >"));
				for( i=0; i<RSP_KEYLENGTH; i++ )
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->FwApp.Mib.KeyAdministrator[i] ));
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_KEY_GEN:
			if( *pLen == (RSP_KEYLENGTH*2) )  {
				AsfHex2Array( pBuf, *pLen, pAC->FwApp.Mib.KeyGenerator );
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_GEN:\n >"));
				for( i=0; i<RSP_KEYLENGTH; i++ )
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("%02x",pAC->FwApp.Mib.KeyGenerator[i] ));
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("<\n"));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_PAR_1:
			if( *pLen == 1 )  {
				pAC->FwApp.Mib.Reserved[Id-OID_SKGE_ASF_PAR_1] = *pBuf;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_PAR_X: ---\n" ));
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_FW_REMOVE:
			if( *pLen == 4 )  {
				FwRemoveFirmware(pAC, IoC);
				*pBuf= 0x66;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_FW_REMOVE: ---\n" ));
				*pLen = 4;
			}
			else
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			break;

		case OID_SKGE_ASF_DASH_IP_INFO:
			if(*pLen >= SK_GEASF_IP_INFO_SIZE){
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_IP_INFO\n"));
				/*  we must only to read the info which is in the buffer. */
				SK_MEMCPY(&pAC->FwApp.IpInfoBuff[0], pBuf, SK_GEASF_IP_INFO_SIZE);
			}
			else{
				*pLen = SK_GEASF_IP_INFO_SIZE;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;
			
		case OID_SKGE_ASF_DASH_IP_INFO_EXT:
			if(*pLen >= sizeof(TAsfDashIpInfoExt)){
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_IP_INFO_EXT\n"));
				/*  we must only to read the info which is in the buffer. */
				if( SK_MEMCMP(&pAC->FwApp.Mib.IpInfoExt, pBuf, sizeof(TAsfDashIpInfoExt)) != 0 )  {
					pAC->FwApp.Mib.ConfigChange |= 0x01;
					SK_MEMCPY(&pAC->FwApp.Mib.IpInfoExt, pBuf, sizeof(TAsfDashIpInfoExt));
				}
			}
			else{
				*pLen = sizeof(TAsfDashIpInfoExt);
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_DASH_USER_DAT:
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_USER_DAT\n"));
			pTmp = pBuf;
			TmpLen = *pLen;

			/*   delete old user list */
			for( i=0; i<DASH_USER_DATA_MAX_USER; i++ )  {
				pAC->FwApp.Mib.User[i].Priv = 0;
				pAC->FwApp.Mib.User[i].NameLength = 0;
				pAC->FwApp.Mib.User[i].PwdLength = 0;
			}

			i = 0;
			while( TmpLen > 0 )  {
				/*   name  */
				pAC->FwApp.Mib.User[i].NameLength = *(pTmp++);
				if( (pAC->FwApp.Mib.User[i].NameLength > DASH_USER_DATA_MAX_NAME_LEN) ||
					(pAC->FwApp.Mib.User[i].NameLength < 1)  )
					break;
				SK_MEMSET(pAC->FwApp.Mib.User[i].Name, 0, DASH_USER_DATA_MAX_NAME_LEN );
				SK_STRNCPY(pAC->FwApp.Mib.User[i].Name, pTmp, pAC->FwApp.Mib.User[i].NameLength);
				pTmp += pAC->FwApp.Mib.User[i].NameLength;
				TmpLen -= (pAC->FwApp.Mib.User[i].NameLength+1);
				/*   password */
				pAC->FwApp.Mib.User[i].PwdLength = *(pTmp++);
				if( (pAC->FwApp.Mib.User[i].PwdLength > DASH_USER_DATA_MAX_NAME_LEN) ||
					(pAC->FwApp.Mib.User[i].PwdLength < 1)  )
					break;
				SK_MEMSET(pAC->FwApp.Mib.User[i].Pwd, 0, DASH_USER_DATA_MAX_PWD_LEN );
				SK_STRNCPY(pAC->FwApp.Mib.User[i].Pwd, pTmp, pAC->FwApp.Mib.User[i].PwdLength);
				pTmp += pAC->FwApp.Mib.User[i].PwdLength;
				TmpLen -= (pAC->FwApp.Mib.User[i].PwdLength+1);
				/*   privilege */
				pTmp++; /*   skip length field of prvileges (allways 1) */
				pAC->FwApp.Mib.User[i].Priv = *(pTmp++);
				TmpLen -= 2;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    User %d -> %s %s %c (pLen:%d)\n",
					i,
					pAC->FwApp.Mib.User[i].Name,
					pAC->FwApp.Mib.User[i].Pwd,
					pAC->FwApp.Mib.User[i].Priv,
					TmpLen));
				i++;
				if( i >= DASH_USER_DATA_MAX_USER )
					break;
			}

			if( TmpLen > 0 )  {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_USER_DAT -> ERROR\n"));
				/*   ERROR  */
			}
			break;

		case OID_SKGE_ASF_CHECK_SPI:
		case OID_SKGE_ASF_IP_SOURCE:
			*pLen = 0;
			break;

		case OID_SKGE_ASF_FWVER_OID:
			/*  fall through            */
		case OID_SKGE_ASF_ACPI_OID:
			/*  fall through */
		case OID_SKGE_ASF_SMBUS_OID:
			/*  these OIDs are read only - they cannot be set */
			*pLen = 0;
			break;

		default:
			*pLen = 0;
			RetCode = SK_ASF_PNMI_ERR_NOT_SUPPORTED;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("FwSetOid -> OID not supported \n" ));
			break;
	}

	if( RetCode != SK_ASF_PNMI_ERR_OK )  { /*   No bytes used */
		if( RetCode == SK_PNMI_ERR_TOO_SHORT )  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("FwSetOid ->  Error Length: %d \n", *pLen ));
		}
		else  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("FwSetOid -> Error ???\n"));
		}
		*pLen = 0;
	}
	else
		pAC->FwApp.Mib.NewParam = 2;  /*   Trigger SEPROM Update */

	return(RetCode);
}

/******************************************************************************
*
*   FwGetOid -
*
* Context:
*   init, pageable
*
* Returns:
*/
int FwGetOid(
	SK_AC *pAC, /* the adapter context */
	SK_IOC IoC, /* I/O context */
	SK_U32 Id,  /* OID  */
	SK_U32 Inst,
	SK_U8 *pBuf,
	unsigned int *pLen)
{
	SK_I8   RetLocVal, *pTmp;
	SK_U32  TmpVal32;
	SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;
	SK_U16   i;

	SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_INIT,("FwGetOid -> OID:0x%x Len:%d\n", Id, *pLen ));
	
	if( pAC->FwApp.InitState != ASF_INIT_OK )  {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("FwGetOid -> Get OID denied, ASF not initialized !\n"));
		if( Id != OID_SKGE_ASF_CAP && Id != OID_SKGE_ASF_CHECK_SPI)
			return( SK_ASF_PNMI_ERR_GENERAL );
	}

	switch( Id )  {
		case OID_SKGE_ASF_DASH_CAP:
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    (1) OID_SKGE_ASF_DASH_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
			if( *pLen >= 4 )  {
				TmpVal32 = 0;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    (2) OID_SKGE_ASF_DASH_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
				if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )
					TmpVal32 |= BIT_0;  /*   DASH capable */
				if( pAC->FwApp.InitState == ASF_INIT_OK )
					TmpVal32 |= BIT_1;  /*   Init OK */
				if( pAC->FwApp.Mib.Ena )
					TmpVal32 |= BIT_2;  /*   enable */
				*( (SK_U32 *) pBuf ) = TmpVal32;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_CAP:
			if( *pLen >= 4 )  {
				TmpVal32 = 0;
				if( (pAC->FwApp.InitState != ASF_INIT_UNDEFINED) &&
					(pAC->FwApp.InitState != ASF_INIT_ERROR_CHIP_ID) )
					TmpVal32 |= BIT_0;  /*   ASF capable */
				if( pAC->FwApp.InitState == ASF_INIT_OK )
					TmpVal32 |= BIT_1;  /*   ASF Init OK */
				if( pAC->FwApp.Mib.Ena )
					TmpVal32 |= BIT_2;  /*   ASF enable */
				*( (SK_U32 *) pBuf ) = TmpVal32;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_DASH_BUSY:
			if( *pLen >= 1 )  {
				if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
					/*   Indication for configuration in progress */
					if( pAC->FwApp.NewOpMode != pAC->FwApp.OpMode )  {
						*( (SK_U8 *) pBuf ) = 1;
					}
					else  {
						*( (SK_U8 *) pBuf ) = 0;
					}
					*pLen = 1;
				}
				else  {
					*( (SK_U8 *) pBuf ) = 0;
					*pLen = 1;
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_DASH_BUSY: %d\n",    *( (SK_U8 *) pBuf )));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_DASH_ENA:
			if( *pLen >= 1 )  {
				if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
					if( (pAC->FwApp.NewOpMode == SK_GEASF_MODE_DASH) &&
						(pAC->FwApp.OpMode == SK_GEASF_MODE_DASH) )  {
						*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Ena;
					}
					else  {
						*( (SK_U8 *) pBuf ) = 0;
					}

#ifdef WORKING_INDICATION_IN_GUI
					/*   Indication for configuration in progress */
					if( pAC->FwApp.NewOpMode != pAC->FwApp.OpMode )  {
						*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Ena;
					}
#endif  /* WORKING_INDICATION_IN_GUI */
					*pLen = 1;
				}
				else  {
					*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Ena;
					*pLen = 1;
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_ENA: %d\n",    *( (SK_U8 *) pBuf )));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_ENA:
			if( *pLen >= 1 )  {
				if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
					if( (pAC->FwApp.NewOpMode == SK_GEASF_MODE_DASH_ASF) &&
						(pAC->FwApp.OpMode == SK_GEASF_MODE_DASH_ASF ) ) {
						*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Ena;
					}
					else  {
						*( (SK_U8 *) pBuf ) = 0;
					}
					
#ifdef WORKING_INDICATION_IN_GUI
					/*   Indication for configuration in progress */
					if( pAC->FwApp.NewOpMode != pAC->FwApp.OpMode )  {
						*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Ena;
					}
#endif /* WORKING_INDICATION_IN_GUI */
					*pLen = 1;
				}
				else  {
					*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Ena;
					*pLen = 1;
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_DASH_ENA: %d\n",  pAC->FwApp.Mib.Ena ));
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_DASH_ENA: %d\n",    *( (SK_U8 *) pBuf )));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RETRANS:
			if( *pLen >= 2 )  {
				*( (SK_U16 *) pBuf ) = pAC->FwApp.Mib.Retrans;
				*pLen = 2;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS: %d\n",  pAC->FwApp.Mib.Retrans ));
			}
			else  {
				*pLen = 2;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RETRANS_INT:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.RetransInt / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT: %d\n",  pAC->FwApp.Mib.RetransInt ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_HB_ENA:
			if( *pLen >= 1 )  {
				*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.HbEna;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_ENA: %d\n",  pAC->FwApp.Mib.HbEna ));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_HB_INT:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.HbInt / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT: %d\n",  pAC->FwApp.Mib.HbInt ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_WD_ENA:
			if( *pLen >= 1 )  {
				*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.WdEna;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_ENA: %d\n",  pAC->FwApp.Mib.WdEna ));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_WD_TIME:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.WdTime / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME: %d\n",  pAC->FwApp.Mib.WdTime ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_IP_SOURCE:
			if( *pLen >= 16 )  {
				AsfIp2Asci( pBuf+1, pLen, pAC->FwApp.Mib.IpSource );
				*pBuf = (SK_U8) *pLen;
				(*pLen)++;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_IP_SOURCE: %d.%d.%d.%d\n",
										pAC->FwApp.Mib.IpSource[0], pAC->FwApp.Mib.IpSource[1],
										pAC->FwApp.Mib.IpSource[2], pAC->FwApp.Mib.IpSource[3] ));
			}
			else  {
				*pLen = 16;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_IP_DEST:
			if( *pLen >= 16 )  {
				AsfIp2Asci( pBuf+1, pLen, pAC->FwApp.Mib.IpDest );
				*pBuf = (SK_U8) *pLen;
				(*pLen)++;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_IP_DEST: %d.%d.%d.%d\n",
										pAC->FwApp.Mib.IpDest[0], pAC->FwApp.Mib.IpDest[1],
										pAC->FwApp.Mib.IpDest[2], pAC->FwApp.Mib.IpDest[3] ));
			}
			else  {
				*pLen = 16;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_MAC_DEST:
			if( *pLen >= 18 )  {
				AsfMac2Asci( pBuf+1, pLen, pAC->FwApp.Mib.MacDest );
				*pBuf = (SK_U8) *pLen;
				(*pLen)++;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_MAC_DEST: %02x:%02x:%02x:%02x:%02x:%02x\n",
										pAC->FwApp.Mib.MacDest[0], pAC->FwApp.Mib.MacDest[1],
										pAC->FwApp.Mib.MacDest[2], pAC->FwApp.Mib.MacDest[3],
										pAC->FwApp.Mib.MacDest[4], pAC->FwApp.Mib.MacDest[5] ));
			}
			else  {
				*pLen = 18;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_MAC_SOURCE:
			if( *pLen >= 18 )  {
				AsfMac2Asci( pBuf+1, pLen, pAC->FwApp.Mib.MacSource );
				*pBuf = (SK_U8) *pLen;
				(*pLen)++;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_MAC_SOURCE: %02x:%02x:%02x:%02x:%02x:%02x\n",
										pAC->FwApp.Mib.MacSource[0], pAC->FwApp.Mib.MacSource[1],
										pAC->FwApp.Mib.MacSource[2], pAC->FwApp.Mib.MacSource[3],
										pAC->FwApp.Mib.MacSource[4], pAC->FwApp.Mib.MacSource[5] ));
			}
			else  {
				*pLen = 18;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_COMMUNITY_NAME:
			if( *pLen >= 64 )  {
				for( *pLen=0; *pLen<63; (*pLen)++  )  {
					*(pBuf + *pLen + 1 ) = pAC->FwApp.Mib.CommunityName[*pLen];

					if( pAC->FwApp.Mib.CommunityName[*pLen] != 0 )  {
						*(pBuf + *pLen + 1 ) = pAC->FwApp.Mib.CommunityName[*pLen];
					}
					else  {
						break;
					}
				}
				*pBuf = (SK_U8) *pLen;
				(*pLen)++;
			}
			else  {
				*pLen = 64;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RSP_ENA:
			if( *pLen >= 1 )  {
				*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.RspEnable;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RSP_ENA: %d\n",  pAC->FwApp.Mib.RspEnable ));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RETRANS_COUNT_MIN:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.RetransCountMin;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MIN: %d\n",  pAC->FwApp.Mib.RetransCountMin ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RETRANS_COUNT_MAX:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.RetransCountMax;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_COUNT_MAX: %d\n",  pAC->FwApp.Mib.RetransCountMax ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RETRANS_INT_MIN:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.RetransIntMin / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MIN: %d\n",  pAC->FwApp.Mib.RetransIntMin ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_RETRANS_INT_MAX:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.RetransIntMax / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_RETRANS_INT_MAX: %d\n",  pAC->FwApp.Mib.RetransIntMax ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_HB_INT_MIN:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.HbIntMin / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MIN: %d\n",  pAC->FwApp.Mib.HbIntMin ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_HB_INT_MAX:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.HbIntMax / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_HB_INT_MAX: %d\n",  pAC->FwApp.Mib.HbIntMax ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_WD_TIME_MIN:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.WdTimeMin / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MIN: %d\n",  pAC->FwApp.Mib.WdTimeMin ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_WD_TIME_MAX:
			if( *pLen >= 4 )  {
				*( (SK_U32 *) pBuf ) = pAC->FwApp.Mib.WdTimeMax / ASF_GUI_TSF;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_WD_TIME_MAX: %d\n",  pAC->FwApp.Mib.WdTimeMax ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_GUID:
			if( *pLen >= 33 )  {
				AsfArray2Hex( pBuf+1, 16, pAC->FwApp.Mib.Guid );
				*pBuf = 32;
				*pLen = 33;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_GUID\n" ));
			}
			else  {
				*pLen = 33;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_KEY_OP:
			if( *pLen >= 41 )  {
				AsfArray2Hex( pBuf+1, 40, pAC->FwApp.Mib.KeyOperator );
				*pBuf = (SK_U8) 40;
				*pLen = 41;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_OP:\n"));
			}
			else  {
				*pLen = 41;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_KEY_ADM:
			if( *pLen >= 41 )  {
				AsfArray2Hex( pBuf+1, 40, pAC->FwApp.Mib.KeyAdministrator );
				*pBuf = (SK_U8) 40;
				*pLen = 41;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_ADM:\n"));
			}
			else  {
				*pLen = 41;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_KEY_GEN:
			if( *pLen >= 41 )  {
				AsfArray2Hex( pBuf+1, 40, pAC->FwApp.Mib.KeyGenerator );
				*pBuf = (SK_U8) 40;
				*pLen = 41;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_KEY_GEN:\n"));
			}
			else  {
				*pLen = 41;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_PAR_1:
			if( *pLen >= 1 )  {
				*( (SK_U8 *) pBuf ) = pAC->FwApp.Mib.Reserved[Id-OID_SKGE_ASF_PAR_1];
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_PAR_X: --\n"));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_FWVER_OID:
			/*  returns the firmware revision to GUI */
			if (*pLen >= ASF_FWVER_MAXBUFFLENGTH) {
				for (i=0; i < ASF_FWVER_MAXBUFFLENGTH; i++ ) {
					/*  maybe we should lock the access to FwVersionString against reading/writing at the same time? */
					*((SK_U8 *)pBuf+i) = pAC->FwApp.FwVersionString[i];
				}
				*pLen = ASF_FWVER_MAXBUFFLENGTH;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_FWVER_OID: %d\n", *pLen));
			}
			else {
				/*  set the right length */
				*pLen   = ASF_FWVER_MAXBUFFLENGTH;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_ACPI_OID:
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    (1)OID_SKGE_ASF_ACPI_OID: %d\n", *pLen));

			/*  returns ACPI/ASF table (ASF_ACPI_MAXBUFFLENGTH bytes) to GUI */
			if ( (pAC->FwApp.Mib.Acpi.length > 0) && (pAC->FwApp.Mib.Acpi.length <= ASF_ACPI_MAXBUFFLENGTH) ) {
				if (*pLen >= pAC->FwApp.Mib.Acpi.length) {
					/*  there is enough space in buffer for reporting ACPI buffer */
					for (i=0; i < pAC->FwApp.Mib.Acpi.length; i++) {
						/*  maybe we should lock the access to Acpi.buffer against reading/writing at the same time? */
						*((SK_U8 *)pBuf+i) = pAC->FwApp.Mib.Acpi.buffer[i];
					}
					*pLen = pAC->FwApp.Mib.Acpi.length;
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    (2)OID_SKGE_ASF_ACPI_OID: %d\n", *pLen));
				}
				else {
					/*  there is not enough space in buffer to report ACPI buffer */
					/*   *pLen   = ASF_ACPI_MAXBUFFLENGTH; */

					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
						("    OID_SKGE_ASF_ACPI_OID: ERROR -> buffer too short %d\n",*pLen));
					*pLen   = pAC->FwApp.Mib.Acpi.length;
					RetCode = SK_PNMI_ERR_TOO_SHORT;
				}
			}
			else {
				/*  no buffer to report */
				*pLen = 0;
				RetCode = SK_PNMI_ERR_GENERAL;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
					("    OID_SKGE_ASF_ACPI_OID: ERROR -> Length:%d\n",pAC->FwApp.Mib.Acpi.length));
			}
			break;

		case OID_SKGE_ASF_SMBUS_OID:
			/*  set a request flag, that smbus info must be updated */
			pAC->FwApp.Mib.SMBus.UpdateReq = 1;
			/*  returns SMBus table GUI */
			if ( (pAC->FwApp.Mib.SMBus.length > 0) && (pAC->FwApp.Mib.SMBus.length <= ASF_SMBUS_MAXBUFFLENGTH) ) {
				if (*pLen >= pAC->FwApp.Mib.SMBus.length) {
					/*  there is enough space in buffer for reporting ACPI buffer */
					for (i=0; i < pAC->FwApp.Mib.SMBus.length; i++) {
						/*  maybe we should lock the access to SMBus.buffer against reading/writing at the same time? */
						*((SK_U8 *)pBuf+i) = pAC->FwApp.Mib.SMBus.buffer[i];
					}
					*pLen = pAC->FwApp.Mib.SMBus.length;
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_SMBUS_OID: %d\n", *pLen));
				}
				else {
					/*  there is not enough space in buffer to report SMBus buffer */
					*pLen   = ASF_SMBUS_MAXBUFFLENGTH;
					RetCode = SK_PNMI_ERR_TOO_SHORT;
				}
			}
			else {
				/*  no buffer to report */
				*pLen = 0;
			}
			break;
		case OID_SKGE_ASF_CHECK_SPI:
			if( *pLen >= 4 )  {
				*((SK_U8 *)pBuf) = pAC->FwApp.PxeSPI;
				*pLen = 4;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,
							("    OID_SKGE_ASF_CHECK_SPI: %d\n",
							pAC->FwApp.PxeSPI ));
			}
			else  {
				*pLen = 4;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_DASH_MAX_USER:
			if( *pLen >= 1 )  {
				*( (SK_U8 *) pBuf ) = DASH_USER_DATA_MAX_USER;
				*pLen = 1;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_MAX_USER: %d\n",    *( (SK_U8 *) pBuf )));
			}
			else  {
				*pLen = 1;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_DASH_IP_INFO:
			if(*pLen >= SK_GEASF_IP_INFO_SIZE){
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_IP_INFO\n"));
				/*  we must only to read the info which is in the buffer. */
				SK_MEMCPY(&pAC->FwApp.IpInfoBuff[0], pBuf, SK_GEASF_IP_INFO_SIZE);
			}
			else{
				*pLen = SK_GEASF_IP_INFO_SIZE;
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;


		case OID_SKGE_ASF_DASH_IP_INFO_EXT:
			if(*pLen >= sizeof(TAsfDashIpInfoExt)){
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_IP_INFO_EXT\n"));
				/*  we must only read the info that is in the buffer. */
				SK_MEMCPY( pBuf, &pAC->FwApp.Mib.IpInfoExt, sizeof(TAsfDashIpInfoExt) );
			}
			else{
				*pLen = sizeof(TAsfDashIpInfoExt);
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_DASH_SPECIAL_ACTION:
			if( *pLen >= sizeof(TAsfDashSpecialAction) )  {
				PAsfDashSpecialAction pAction = (PAsfDashSpecialAction) pBuf;

				if( (pAction->Header.Revision 	== ASF_DASH_SPECIAL_ACTION_VERSION) &&
					(pAction->Header.Type 		== ASF_DASH_SPECIAL_ACTION_OBJECT_TYPE) &&
					(pAction->Header.Size 		== sizeof(TAsfDashSpecialAction)) )  {

					pAction->Header.Revision 	= ASF_DASH_SPECIAL_ACTION_VERSION;
					pAction->Header.Type		= ASF_DASH_SPECIAL_ACTION_OBJECT_TYPE;
					pAction->Header.Size		= sizeof(TAsfDashSpecialAction);
					pAction->bNeedDriverReload 	= 0;  /*  SSL: don't reload driver  */
					if( pAC->FwApp.SslPatchedFile > 0 ) {
						pAC->FwApp.SslPatchedFile--;
						pAction->bDeleteSslCertificate= 1;  /*  SSL: delete SSL certificates */
					} else
						pAction->bDeleteSslCertificate= 0;
					*pLen = sizeof(TAsfDashSpecialAction);
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_SPECIAL_ACTION: %d\n",    *( (SK_U8 *) pBuf )));
				}
			}
			else  {
				*pLen = sizeof(TAsfDashSpecialAction);
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;
			
		case OID_SKGE_ASF_DASH_USER_DAT:
			if (*pLen >= (DASH_USER_DATA_MAX_USER*sizeof(STR_DASH_USER_DAT)) ) {
				pTmp = (SK_U8 *)pBuf;
				*pLen = 0;
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    OID_SKGE_ASF_DASH_USER_DAT\n"));
				for( i=0; i<DASH_USER_DATA_MAX_USER; i++ )  {
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL,("    User %d: %s %s %c\n",
						i,
						pAC->FwApp.Mib.User[i].Name,
						pAC->FwApp.Mib.User[i].Pwd,
						pAC->FwApp.Mib.User[i].Priv));
					/*   name */
					*(pTmp++) = pAC->FwApp.Mib.User[i].NameLength;
					SK_STRNCPY(pTmp, pAC->FwApp.Mib.User[i].Name, pAC->FwApp.Mib.User[i].NameLength);
					pTmp += pAC->FwApp.Mib.User[i].NameLength;
					/*   password */
					*(pTmp++) = pAC->FwApp.Mib.User[i].PwdLength;
					SK_STRNCPY(pTmp, pAC->FwApp.Mib.User[i].Pwd, pAC->FwApp.Mib.User[i].PwdLength);
					pTmp += pAC->FwApp.Mib.User[i].PwdLength;
					/*   priv */
					*(pTmp++) = 1;
					*(pTmp++) = pAC->FwApp.Mib.User[i].Priv;
					*pLen = (SK_U32) (pTmp - (SK_U8 *)pBuf);
				}
			}
			else {
				/*  set the right length */
				*pLen   = DASH_USER_DATA_MAX_USER*sizeof(STR_DASH_USER_DAT);
				RetCode = SK_PNMI_ERR_TOO_SHORT;
			}
			break;

		case OID_SKGE_ASF_FW_REMOVE:
		case OID_SKGE_ASF_HB_CAP:
			/*  oid not used */
			*pLen = 0;
			break;

		default:
			*pLen   = 0;
			RetCode = SK_ASF_PNMI_ERR_NOT_SUPPORTED;
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("FwGetOid -> OID not supported\n"));
			break;
	}

	if( RetCode != SK_ASF_PNMI_ERR_OK )  { /*   No bytes used */
		if( RetCode == SK_PNMI_ERR_TOO_SHORT )  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("\n*** FwGetOid ->  Error Length: %d \n", *pLen ));
		}
		else  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL | SK_DBGCAT_ERR,("\n*** FwGetOid -> Error ???\n"));
		}
	}
	else  {
		SK_DBG_DMP(pAC, SK_DBGMOD_ASF, SK_DBGCAT_CTRL, pBuf, *pLen );
	}

	return(RetCode);
}

/******************************************************************************
*
*   FwGet -
*
* Context:
*   init, pageable
*
* Returns:
*/
int FwGet(
	SK_AC *pAC,    /* the adapter context */
	SK_IOC IoC, /* I/O context */
	SK_U8 *pBuf,
	unsigned int *pLen)
{
	SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

	return(RetCode) ;
}

/******************************************************************************
*
*   FwPreSet -
*
* Context:
*   init, pageable
*
* Returns:
*/
int FwPreSet(
	SK_AC *pAC, /* the adapter context */
	SK_IOC IoC, /* I/O context */
	SK_U8 *pBuf,
	unsigned int *pLen)
{
	/* preset the Return code to error */
	SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

	return(RetCode);
}

/******************************************************************************
*
*   FwPreSetOid -
*
* Context:
*   init, pageable
*
* Returns:
*/
int FwPreSetOid(
	SK_AC *pAC, /* the adapter context */
	SK_IOC IoC, /* I/O context */
	SK_U32 Id,  /* OID  */
	SK_U32 Inst,
	SK_U8 *pBuf,
	unsigned int *pLen)
{
	SK_U32 RetCode = SK_ASF_PNMI_ERR_OK;

	switch( Id )  {
		case OID_SKGE_ASF_ENA:
			break;

		default:
			*pLen = 0;
			RetCode = SK_ASF_PNMI_ERR_NOT_SUPPORTED;
			break;
	}

	return(RetCode);
}

/******************************************************************************
*
*   FwSet -
*
* Context:
*   init, pageable
*
* Returns:
*/
int FwSet(
	SK_AC *pAC, /* the adapter context */
	SK_IOC IoC, /* I/O context */
	SK_U8 *pBuf,
	unsigned int *pLen)
{
	SK_U32  RetCode = SK_ASF_PNMI_ERR_OK;

	return(RetCode);
}
