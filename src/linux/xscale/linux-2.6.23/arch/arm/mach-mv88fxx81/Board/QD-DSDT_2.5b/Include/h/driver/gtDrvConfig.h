#include <Copyright.h>

/********************************************************************************
* gtDrvConfig.h
*
* DESCRIPTION:
*       Includes driver level configuration and initialization function.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*
*******************************************************************************/

#ifndef __gtDrvConfigh
#define __gtDrvConfigh

#include <msApi.h>
#include <gtVct.h>
#include <gtDrvSwRegs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* driverConfig
*
* DESCRIPTION:
*       This function initializes the driver level of the quarterDeck software.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success, or
*       GT_OUT_OF_CPU_MEM   - if failed to allocate CPU memory,
*       GT_FAIL             - otherwise.
*
* COMMENTS:
*       1.  This function should perform the following:
*           -   Initialize the global switch configuration structure.
*           -   Initialize Mii Interface
*           -   Set the CPU port into trailer mode (Ingress and Egress).
*
*******************************************************************************/
GT_STATUS driverConfig(IN GT_QD_DEV *dev);

/*******************************************************************************
* driverEnable
*
* DESCRIPTION:
*       This function enables the switch for full operation, after the driver
*       Config function was called.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverEnable(IN GT_QD_DEV *dev);

/*******************************************************************************
* driverIsPhyAttached
*
* DESCRIPTION:
*       This function reads and returns Phy ID (register 3) of Marvell Phy.
*
* INPUTS:
*       hwPort	- port number where the Phy is connected
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       phyId - if Marvell Phy exists
*		0	  - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U32 driverIsPhyAttached
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort
);

/*******************************************************************************
* driverGetSerdesPort
*
* DESCRIPTION:
*       This function converts port to Serdes port
*
* INPUTS:
*       hwPort	 - port number where the Phy is connected
*
* OUTPUTS:
*       hwPort	 - port number where the Phy is connected
*
* RETURNS:
*       GT_OK 	- if success
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverGetSerdesPort(GT_QD_DEV *dev, GT_U8* hwPort);


/*******************************************************************************
* driverPagedAccessStart
*
* DESCRIPTION:
*       This function stores page register and Auto Reg Selection mode if needed.
*
* INPUTS:
*       hwPort	 - port number where the Phy is connected
*		pageType - type of the page registers
*
* OUTPUTS:
*       autoOn	- GT_TRUE if Auto Reg Selection enabled, GT_FALSE otherwise.
*		pageReg - Page Register Data
*
* RETURNS:
*       GT_OK 	- if success
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverPagedAccessStart
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	IN	GT_U8		 pageType,
	OUT	GT_BOOL		 *autoOn,
	OUT	GT_U16		 *pageReg
);


/*******************************************************************************
* driverPagedAccessStop
*
* DESCRIPTION:
*       This function restores page register and Auto Reg Selection mode if needed.
*
* INPUTS:
*       hwPort	 - port number where the Phy is connected
*		pageType - type of the page registers
*       autoOn	 - GT_TRUE if Auto Reg Selection enabled, GT_FALSE otherwise.
*		pageReg  - Page Register Data
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK 	- if success
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverPagedAccessStop
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	IN	GT_U8		 pageType,
	IN	GT_BOOL		 autoOn,
	IN	GT_U16		 pageReg
);


/*******************************************************************************
* driverFindPhyInformation
*
* DESCRIPTION:
*       This function gets information of Phy connected to the given port.
*
* INPUTS:
*       hwPort	- port number where the Phy is connected
*
* OUTPUTS:
*       phyId	- Phy ID
*
* RETURNS:
*       GT_OK 	- if found Marvell Phy,
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverFindPhyInformation
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	OUT	GT_PHY_INFO	 *phyInfo
);


#ifdef __cplusplus
}
#endif

#endif /* __gtDrvConfigh */
