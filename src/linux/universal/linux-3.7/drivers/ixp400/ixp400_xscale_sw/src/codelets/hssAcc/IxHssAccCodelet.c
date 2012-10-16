/**
 * @file IxHssAccCodelet.c
 *
 * @date 26 Mar 2002
 *
 * @brief This file contains the main routine for the HSS Access Codelet.
 *
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
 * @sa IxHssAccCodelet.h
 */

/*
 * Put the system defined include files required.
 */

#ifdef __vxworks
#include <end.h>    /* END drivers */
#include <endLib.h> /* END drivers */
#include <logLib.h>
#include <ioLib.h>
#endif

/*
 * Put the user defined include files required.
 */
#include "IxHssAcc.h"
#include "IxNpeDl.h"
#include "IxNpeMh.h"
#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxFeatureCtrl.h"
#include "IxHssAccCodelet.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#define gets ixSerGets
#endif

/*
 * #defines and macros used in this file.
 */

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* Service Flag */
extern BOOL packetisedFlag ;
extern BOOL channelisedFlag ;

/*
 * Function prototypes.
 */
IX_STATUS
ixHssAccCodeletServiceMain (IxHssAccCodeletPortMode portMode);

/*
 * Function definition: ixHssAccCodeletMain
 */
PUBLIC IX_STATUS
ixHssAccCodeletMain (IxHssAccCodeletOperation operationType,
                     IxHssAccCodeletPortMode portMode,
                     IxHssAccCodeletVerifyMode verifyMode)
{
    /* Setup codelet services */
    switch (operationType)
    {
      case IX_HSSACC_CODELET_PKT_CHAN_SERV :
           packetisedFlag = TRUE ;
           channelisedFlag = TRUE ;
           break ;

      case IX_HSSACC_CODELET_PKT_SERV_ONLY :     
           packetisedFlag = TRUE ;
           break ;

      case IX_HSSACC_CODELET_CHAN_SERV_ONLY :
           channelisedFlag = TRUE ;
           break ;

      default:
	  printf ("Invalid operationType option! \n");
	  printf ("Please choose one of the following options: \n");
	  printf ("%u : Packetised Service Only.\n",
		  IX_HSSACC_CODELET_PKT_SERV_ONLY);
	  printf ("%u : Channelised Service Only. \n", 
		  IX_HSSACC_CODELET_CHAN_SERV_ONLY);
	  printf ("%u : Packetised & Channelised Services.\n", 
		  IX_HSSACC_CODELET_PKT_CHAN_SERV);

        return IX_FAIL;
    }

    /*  Check for the Silicon type */
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead ()||
        IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X == ixFeatureCtrlDeviceRead ())
    {
    /* Check HSS Port Mode */
        if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY > portMode) || 
        (IX_HSSACC_CODELET_DUAL_PORTS < portMode))
        {
	    printf ("Invalid portMode option! \n");
            printf ("Please choose one of the following options: \n");
            printf ("%u : HSS Port 0 Only.\n", 
		    IX_HSSACC_CODELET_HSS_PORT_0_ONLY);
            printf ("%u : HSS Port 1 Only.\n", 
		    IX_HSSACC_CODELET_HSS_PORT_1_ONLY);
            printf ("%u : Both HSS Port 0 and 1. \n", 
		    IX_HSSACC_CODELET_DUAL_PORTS);

            return IX_FAIL;
        }
    }
    else 
    {
        if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
        {
            /* Check HSS Port Mode */
            if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY > portMode) || 
            (IX_HSSACC_CODELET_HSS_PORT_0_ONLY < portMode))
            {
	        printf ("Invalid portMode option! \n");
                printf ("Please choose the following option for IXP 43X: \n");
                printf ("%u : HSS Port 0 Only.\n", 
		        IX_HSSACC_CODELET_HSS_PORT_0_ONLY);
     
                return IX_FAIL;
            }
        }
    }

    /* Check Verify Mode */
    if ((IX_HSSACC_CODELET_VERIFY_ON != verifyMode) &&
        (IX_HSSACC_CODELET_VERIFY_OFF != verifyMode))
    { 
 	printf ("Invalid verifyMode option! \n");
        printf ("Please choose one of the following options: \n");
        printf ("%u : Codelet verifies received traffic.\n", 
		IX_HSSACC_CODELET_VERIFY_ON);
        printf ("%u : Codelet does not verify received traffic.\n", 
		IX_HSSACC_CODELET_VERIFY_OFF);

        return IX_FAIL;     
    }
    else 
    {
        /*
         * Enable verification in both packatised and channelised services. 
         */
        ixHssAccCodeletPacketisedVerifySet ((verifyMode == IX_HSSACC_CODELET_VERIFY_ON ? TRUE : FALSE));
        ixHssAccCodeletChannelisedVerifySet((verifyMode == IX_HSSACC_CODELET_VERIFY_ON ? TRUE : FALSE));
    }
   
    /* The codelet only supports PHY Loopback. Disable codelet loopback  */
    ixHssAccCodeletHssLoopbackSet (TRUE);
    ixHssAccCodeletCodeletLoopbackSet (FALSE);
 
    /* Initialize npeDl, npeMh, QMgr and hssAcc */
    if (IX_SUCCESS != ixHssAccCodeletInit())
    {
        printf("ixHssAccCodeletInit() fails ! Exit \n");
        return IX_FAIL;
    }

    /* Start HSS Services */
    ixHssAccCodeletServiceStart(portMode);

    return IX_SUCCESS ;
}

/*
 * Function definition: ixHssAccCodeletServiceMain
 */
IX_STATUS
ixHssAccCodeletServiceMain (IxHssAccCodeletPortMode portMode)
{
    IxHssAccHdlcPort hdlcPortId;
    UINT32 i;
 
    /**********************/
    /* CONFIGURE HSS PORT */
    /**********************/

    if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
        (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
    {
        ixHssAccCodeletConfigure (IX_HSSACC_HSS_PORT_0);
    }

    if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
        (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
    {
        ixHssAccCodeletConfigure (IX_HSSACC_HSS_PORT_1);
    }

    /******************/
    /* START SERVICES */
    /******************/

    if (TRUE == packetisedFlag)
    {

        if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* start the Packetised Service for all clients */
            for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX;
                 hdlcPortId++)
            {
                ixHssAccCodeletPacketisedServiceStart (IX_HSSACC_HSS_PORT_0, hdlcPortId);
            }
	}

        if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* start the Packetised Service for all clients */
            for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX;
                 hdlcPortId++)
            {
                ixHssAccCodeletPacketisedServiceStart (IX_HSSACC_HSS_PORT_1, hdlcPortId);
            }
	}
    }

    if (TRUE == channelisedFlag)
    { 
        if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* start the Channelised Service for the single client */
            ixHssAccCodeletChannelisedServiceConfigure (IX_HSSACC_HSS_PORT_0);
	}

        if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* start the Channelised Service for the single client */
            ixHssAccCodeletChannelisedServiceConfigure (IX_HSSACC_HSS_PORT_1);
	}

        if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* start the Channelised Service for the single client */
            ixHssAccCodeletChannelisedServiceStart (IX_HSSACC_HSS_PORT_0);
	}

        if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* start the Channelised Service for the single client */
            ixHssAccCodeletChannelisedServiceStart (IX_HSSACC_HSS_PORT_1);
	}
    }
 
    /*********************/
    /* EXERCISE SERVICES */
    /*********************/

    for (i = 0; i < (IX_HSSACC_CODELET_DURATION_IN_MS/200); i++)
    {
        if (TRUE == packetisedFlag)
        {
            if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
                (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
            {
                /* let each packetised client exercise the Packetised Service */
                for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX;
                     hdlcPortId++)
                {
                    ixHssAccCodeletPacketisedServiceRun (IX_HSSACC_HSS_PORT_0, hdlcPortId);
                }
	    }

            if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
                (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
            {
                /* let each packetised client exercise the Packetised Service */
                for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX;
                     hdlcPortId++)
                {
                    ixHssAccCodeletPacketisedServiceRun (IX_HSSACC_HSS_PORT_1, hdlcPortId);
                }
	    }
        }

        if (TRUE == channelisedFlag)
        {
            if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
                (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
            {
                /* let the channelised client exercise the Channelised Service */
                ixHssAccCodeletChannelisedServiceRun (IX_HSSACC_HSS_PORT_0);
	    }

            if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
                (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
            {
                /* let the channelised client exercise the Channelised Service */
                ixHssAccCodeletChannelisedServiceRun (IX_HSSACC_HSS_PORT_1);
	    }
        }

        /* sleep for 200ms */
        ixOsalSleep (200);
    } /* for (i ... */

    /*****************/
    /* STOP SERVICES */
    /*****************/

    if (TRUE == packetisedFlag)
    {
        if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* stop the Packetised Service for all clients */
            for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX;
                 hdlcPortId++)
            {
                ixHssAccCodeletPacketisedServiceStop (IX_HSSACC_HSS_PORT_0, hdlcPortId);
            }
	}

        if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        {
            /* stop the Packetised Service for all clients */
            for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX;
                 hdlcPortId++)
            {
                ixHssAccCodeletPacketisedServiceStop (IX_HSSACC_HSS_PORT_1, hdlcPortId);
            }
	}
    }

    if (TRUE == channelisedFlag)
    {
        if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        { 
            /* stop the Channelised Service for the single client */
            ixHssAccCodeletChannelisedServiceStop (IX_HSSACC_HSS_PORT_0);
	}

        if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
            (IX_HSSACC_CODELET_DUAL_PORTS == portMode))
        { 
            /* stop the Channelised Service for the single client */
            ixHssAccCodeletChannelisedServiceStop (IX_HSSACC_HSS_PORT_1);
	}
    } 

    if ((IX_HSSACC_CODELET_HSS_PORT_0_ONLY == portMode) ||
        (IX_HSSACC_CODELET_DUAL_PORTS == portMode)) 
    { 
        ixHssAccCodeletShow(IX_HSSACC_HSS_PORT_0);
    }

    if ((IX_HSSACC_CODELET_HSS_PORT_1_ONLY == portMode) ||
        (IX_HSSACC_CODELET_DUAL_PORTS == portMode)) 
    { 
        ixHssAccCodeletShow(IX_HSSACC_HSS_PORT_1);
    }

    return IX_SUCCESS;
}

/*
 * Function definition: ixHssAccCodeletServiceStart
 */
void
ixHssAccCodeletServiceStart (IxHssAccCodeletPortMode portMode)
{
    /* Display the settings of the codelet */
    printf ("HSS Acc Codelet tries to start with following settings : \n");

    if (IX_HSSACC_CODELET_DUAL_PORTS == portMode)
    {
	printf ("\t>HSS Port 0 and HSS Port 1.\n");
    }
    else
    {
	printf ("\t>HSS Port %d Only.\n", 
		(portMode==IX_HSSACC_CODELET_HSS_PORT_0_ONLY ? 0: 1));
    }
    printf("\t>Packetised Service. [%s]\n",
	   (TRUE == packetisedFlag ? "On" : "Off"));
    printf("\t>Channelised Service.[%s]\n",
	   (TRUE == channelisedFlag ? "On" : "Off"));

    printf ("HSS Acc Codelet is being started and executed. \n");
    printf ("Codelet will run for %u ms\n\n", 
	    IX_HSSACC_CODELET_DURATION_IN_MS);
  
    ixHssAccCodeletServiceMain(portMode);
}

/*
 * Function definition: ixHssAccCodeletInit
 */
IX_STATUS
ixHssAccCodeletInit (void)
{
    IxQMgrDispatcherFuncPtr dispatcherFunc ; 

    printf ("Setting the debug level & redirecting output\n");

    /* set the debug level */
#ifdef __vxworks
    logInit (ioTaskStdGet (0,1), 1);
    logFdSet (ioTaskStdGet (0,1));
#endif    
    ixOsalLogLevelSet (IX_OSAL_LOG_LVL_ERROR); /* IX_OSAL_LOG_LVL_ALL, 
						  IX_OSAL_LOG_LVL_MESSAGE, ... */

#ifdef __vxworks
    /* When the ixe drivers are running, the codelets
    * cannot run.
    */
    if (endFindByName ("ixe", 0) != NULL)
    {
	printf("FAIL : Driver ixe0 detected\n");
	return IX_FAIL;
    }
    if (endFindByName ("ixe", 1) != NULL)
    {
	printf("FAIL : Driver ixe1 detected\n");
	return IX_FAIL;
    }
#endif

    /*  Check for the Silicon type */
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead ()||
        IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
    {
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)==
                                        IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {
             printf("Exiting: IxHssAccCodeletInit() NPE A component Does not exist \n");
	     return IX_FAIL;
        }   
    }
    
    /*Initialize and Start NPE-A */
    printf ("Initializing and starting NPE-A ...");
        
    /*  Check for the Silicon type */
    if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead ()||
        IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X == ixFeatureCtrlDeviceRead ())
    {
        if (IX_SUCCESS != ixNpeDlNpeInitAndStart (IX_NPEDL_NPEIMAGE_NPEA_HSS_2_PORT))
        {
            printf ("Error initialising and starting NPE A!\n");
            return (IX_FAIL);
        }
    }
    else 
    {
        if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead ())
        {
            if (IX_SUCCESS != ixNpeDlNpeInitAndStart (IX_NPEDL_NPEIMAGE_NPEA_HSS_TSLOT_SWITCH))
            {
            printf ("Error initialising and starting NPE A!\n");
            return (IX_FAIL);
            }
        }
    }
   
    printf (" successful.\n");

    /* initialise the IxNpeMh component */
    printf ("Initializing NpeMh Component ...");
    if (IX_SUCCESS != ixNpeMhInitialize (IX_NPEMH_NPEINTERRUPTS_YES))
    {
        printf ("ixNpeMhInitialize failed\n");
        return (IX_FAIL);
    }
    printf (" successful.\n");

    /* initialise the QMgr component */
    printf ("Initializing QMgr Component ...");
    if (IX_SUCCESS != ixQMgrInit())
    {
        printf ("ixQMgrInit failed\n");
        return (IX_FAIL);
    }
    printf (" successful.\n");

    printf ("Bind Queue Dispatcher to interrupt vector ...");
    ixQMgrDispatcherLoopGet(&dispatcherFunc);

    /* Hook the QM QUELOW dispatcher to interrupt */
    if (IX_SUCCESS != ixOsalIrqBind(IX_OSAL_IXP400_QM1_IRQ_LVL, /* vector */
                                    (IxOsalVoidFnVoidPtr)dispatcherFunc, /* routine */
                                    (void *) IX_QMGR_QUELOW_GROUP)) /* parameter */
    {
        printf ("ixOsalIrqBind failed to bind to QM1 interrupt\n");
        return (IX_FAIL);
    }
    printf (" successful.\n");

    /* initialise the HssAcc component */
    printf("Intializing HssAcc Component ...");
    if (IX_SUCCESS != ixHssAccInit())
    {
        printf ("ixHssAccInit failed\n");
        return IX_FAIL;
    }
    printf (" successful.\n");

    return IX_SUCCESS;
}

#ifdef __wince
int readNumber(void)
{
    char line[256];
    gets(line);
    return atoi(line);
}

int wmain(int argc, WCHAR **argv)
{
    int hssAccCodeletOperationType;
    int hssAccCodeletPortMode;
    int hssAccCodeletVerifyMode;
    int hssAccCodeletNrRead;
    BOOL hssAccCodeletWrongNumber = TRUE;

    printf("\n");
    printf("******************** HSS Acc Codelet *************************\n");

    while(hssAccCodeletWrongNumber)
    {
        printf("  Choose type of service you want to execute:\n");
        printf("  1. Packetised service only\n");
        printf("  2. Channelised service only\n");
        printf("  3. Packetised and channelised service\n");
        printf("100. Exit hssAcc Codelet\n");
        printf("\n");
        printf("Enter number: ");
        hssAccCodeletNrRead = readNumber();

        switch(hssAccCodeletNrRead)
        {
            case 1:
                hssAccCodeletOperationType = IX_HSSACC_CODELET_PKT_SERV_ONLY;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 2:
                hssAccCodeletOperationType = IX_HSSACC_CODELET_CHAN_SERV_ONLY;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 3:
                hssAccCodeletOperationType = IX_HSSACC_CODELET_PKT_CHAN_SERV;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 100:
                return IX_SUCCESS;
                break;
            default:
                printf("Wrong number chosen: %d!\n", 
		       hssAccCodeletNrRead);
                break;
        }
    }

    hssAccCodeletWrongNumber = TRUE;
    
    while(hssAccCodeletWrongNumber)
    {
        printf("  Choose which port(s) you want to execute:\n");
        printf("  1. HSS Port 0 Only\n");
        printf("  2. HSS Port 1 Only\n");
        printf("  3. Both HSS Port 0 & 1\n");
        printf("100. Exit hssAcc Codelet\n");
        printf("\n");
        printf("Enter number: ");
        hssAccCodeletNrRead = readNumber();

        switch(hssAccCodeletNrRead)
        {
            case 1:
                hssAccCodeletPortMode = IX_HSSACC_CODELET_HSS_PORT_0_ONLY;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 2:
                hssAccCodeletPortMode = IX_HSSACC_CODELET_HSS_PORT_1_ONLY;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 3:
                hssAccCodeletPortMode = IX_HSSACC_CODELET_DUAL_PORTS;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 100:
                return IX_SUCCESS;
                break;
            default:
                printf("Wrong number chosen: %d!\n", 
		       hssAccCodeletNrRead);
                break;
        }
    }

    hssAccCodeletWrongNumber = TRUE;

    while(hssAccCodeletWrongNumber)
    {
        printf("  Choose if codelet should or shouldn't verify traffic:\n");
        printf("  1. Codelet verifies traffic received\n");
        printf("  2. Codelet does not verify traffic received\n");
        printf("100. Exit hssAcc Codelet\n");
        printf("\n");
        printf("Enter number: ");
        hssAccCodeletNrRead = readNumber();

        switch(hssAccCodeletNrRead)
        {
            case 1:
                hssAccCodeletVerifyMode = IX_HSSACC_CODELET_VERIFY_ON;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 2:
                hssAccCodeletVerifyMode = IX_HSSACC_CODELET_VERIFY_OFF;
                hssAccCodeletWrongNumber = FALSE;
                break;
            case 100:
                return IX_SUCCESS;
                break;
            default:
                printf("Wrong number chosen: %d!\n", 
		       hssAccCodeletNrRead);
                break;
        }
    }

    hssAccCodeletWrongNumber = TRUE;
    
    ixHssAccCodeletMain(hssAccCodeletOperationType,
                        hssAccCodeletPortMode,
                        hssAccCodeletVerifyMode);

    hssAccCodeletNrRead = readNumber();
    
    return IX_SUCCESS;
}
#endif /* ifdef __wince */



