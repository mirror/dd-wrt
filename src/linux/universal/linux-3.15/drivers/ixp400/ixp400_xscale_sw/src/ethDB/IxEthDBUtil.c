/**
 * @file ethUtil.c
 *
 * @brief Utility functions
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */


#include "IxFeatureCtrl.h"
#include "IxEthDB_p.h"
#include "IxNpeDl.h"

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBSingleEthNpeCheck(IxEthDBPortId portID)
{
    UINT8 functionalityId;
    UINT32 npeId = IX_ETHNPE_PHYSICAL_ID_TO_NODE(portID);
   
    if(IX_ETH_DB_ETH_NPE_ENABLED == ixEthDBEthNPEEnabled[npeId])
    {
       /* Eth NPE is enabled in the IXDP4XX*/
       return IX_ETH_DB_SUCCESS; 
    }
    else
    {
	if((IX_ETH_DB_ETH_NPE_DISABLED == ixEthDBEthNPEEnabled[npeId])
	    || (npeId > IX_NPEDL_NPEID_MAX))
	{
            /* Eth NPE is disabled in the IXDP4XX*/
	    return IX_ETH_DB_FAIL;
        }
    }

    if (IX_SUCCESS != ixNpeDlLoadedImageFunctionalityGet(npeId, &functionalityId))
    {
        return IX_ETH_DB_FAIL;
    }
    else
    {

	/* If it is IXP42X or IXP46X, check NPE B coprocessor (IX_FEATURECTRL_ETH0) 
	   for ethernet enable or disable*/ 
	if((IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X == ixFeatureCtrlDeviceRead ()) ||
	    (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead ()))
	{
            switch(npeId)
            {
                case IX_NPEDL_NPEID_NPEA:
                    if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                        ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA_ETH) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                        (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED)))
                {
                    ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                    return IX_ETH_DB_FAIL;
                }
                break;

              	case IX_NPEDL_NPEID_NPEB:
                    if ( (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED) )
                    {
                        ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                        return IX_ETH_DB_FAIL;
                    }
                    if (portID == 0)
                    {
                        if( ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                            IX_FEATURE_CTRL_COMPONENT_DISABLED)
                    	{
                            ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                            return IX_ETH_DB_FAIL;
                    	}
                    }
                    else /* ports 1-3 */
               	    {
                        if( ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB_ETH) ==
                            IX_FEATURE_CTRL_COMPONENT_DISABLED)
                    	{
                            ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                            return IX_ETH_DB_FAIL;
                    	}

                    }
               	    break;

              	case IX_NPEDL_NPEID_NPEC:
                    if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                        ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                        (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED)))
                    {
                        ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                        return IX_ETH_DB_FAIL;
                    }
                    break;

              	default: /* invalid NPE */
                    return IX_ETH_DB_FAIL;
            } /*end of switch*/
	} /* end of IXP42X || IXP46X */
	/* If it is IXP43X, check NPE C coprocessor (IX_FEATURECTRL_ETH1) 
	 * for ethernet enable or disable
         */ 
	if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
	{
            switch(npeId)
            {
                case IX_NPEDL_NPEID_NPEA: 
               	if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
               	     IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
               	     ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA_ETH) ==
              	     IX_FEATURE_CTRL_COMPONENT_DISABLED) || 
               	     (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
                     	     IX_FEATURE_CTRL_COMPONENT_DISABLED)))
               	{
                    ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                    return IX_ETH_DB_FAIL;
               	}
               	break;

                case IX_NPEDL_NPEID_NPEC: 
               	if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC) ==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
               	    ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED) ))
                {
                    ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_DISABLED;
                    return IX_ETH_DB_FAIL;
                }
                break;

              	default: /* invalid NPE */
                    return IX_ETH_DB_FAIL;
            } /* switch */
        } /* if IXP43X */

        /* Eth NPE is enabled in the IXDP4XX platform*/
        ixEthDBEthNPEEnabled[npeId] = IX_ETH_DB_ETH_NPE_ENABLED;

        return IX_ETH_DB_SUCCESS;
    }
}

#ifdef _DIAB_TOOL
__asm volatile void countLeadingZeros (UINT32 shift, UINT32 value)
{
%reg shift, value;
    clz shift, value;
}
#endif /* #ifdef _DIAB_TOOL */


IX_ETH_DB_PUBLIC
BOOL ixEthDBCheckSingleBitValue(UINT32 value)
{
#if ((CPU!=SIMSPARCSOLARIS) && (CPU!=SIMLINUX) && !defined (__wince))
    UINT32 shift;
    
    /* use the count-leading-zeros XScale instruction */
#ifdef _DIAB_TOOL
    countLeadingZeros(shift, value);
#else
    __asm__ ("clz %0, %1\n" : "=r" (shift) : "r" (value));
#endif /* #ifdef _DIAB_TOOL */
    
    return ((value << shift) == 0x80000000UL);
    
#else
	
    while (value != 0)
    {
        if (value == 1) return TRUE;
        else if ((value & 1) == 1) return FALSE;

        value >>= 1;
    }
    
    return FALSE;

#endif
}

const char *mac2string(const unsigned char *mac)
{
  static char str[19];
  
  if (mac == NULL)
  {
    return NULL;
  }  

  sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return str;
}
