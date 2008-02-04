/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
*********************************************************************************/
/* 
 * FILENAME:    $Workfile: qdFFmii.c $ 
 * REVISION:    $Revision: 8 $ 
 * LAST UPDATE: $Modtime: 3/03/03 3:24a $ 
 * 
 * DESCRIPTION: SMI access routines for Firefox board
 *     
 */
#include "mv_platform.h"
#include "mv_os.h"
#include "mv_qd.h"

/*
 * For each platform, all we need is 
 * 1) Assigning functions into 
 * 		fgtReadMii : to read MII registers, and
 * 		fgtWriteMii : to write MII registers.
 *
 * 2) Register Interrupt (Not Defined Yet.)
*/

/* 
 *  Firefox Specific Definition
 */
#define SMI_OP_CODE_BIT_READ                    1
#define SMI_OP_CODE_BIT_WRITE                   0
#define SMI_BUSY                                1<<28
#define READ_VALID                              1<<27

#define SMI_TIMEOUT_COUNTER				1000

/*****************************************************************************
*
* GT_BOOL qdFFReadMii (GT_QD_DEV* dev, unsigned int portNumber , 
*                      unsigned int MIIReg, unsigned int* value)
*
* Description
* This function will access the MII registers and will read the value of
* the MII register , and will retrieve the value in the pointer.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* Outputs
* value - pointer to unsigned int which will receive the value.
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/
GT_BOOL ffReadMii (GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                        unsigned int* value)
{
 	GT_U32			smiReg;
	unsigned int	phyAddr;
	unsigned int	timeOut = SMI_TIMEOUT_COUNTER; /* in 100MS units */
	int	i;

	/* first check that it is not busy */
    smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);	
    if (smiReg & SMI_BUSY) 
    {
        for(i=0; i<SMI_TIMEOUT_COUNTER; i++);
        do 
		{
            smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
            if (timeOut-- < 1) 
			{
                return GT_FALSE;
    	    }			
        } while (smiReg & SMI_BUSY);
    }	
	/* not busy */
    phyAddr = portNumber;
    smiReg =  (phyAddr << 16) | (SMI_OP_CODE_BIT_READ << 26) | (MIIReg << 21) 
			| SMI_OP_CODE_BIT_READ << 26;

    gtOsGtRegWrite(GT_REG_ETHER_SMI_REG, smiReg);
    timeOut = SMI_TIMEOUT_COUNTER; /* initialize the time out var again */
    smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
    if (!(smiReg & READ_VALID)) 
    {
		for(i = 0 ; i < SMI_TIMEOUT_COUNTER; i++);
        do 
		{
            smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
            if (timeOut-- < 1 ) 
			{
                return GT_FALSE;
    	    }
        } while (!(smiReg & READ_VALID));
    }
    *value = (unsigned int)(smiReg & 0xffff);    

    return GT_TRUE;
}

/*****************************************************************************
* 
* GT_BOOL qdFFWriteMii (GT_QD_DEV* dev, unsigned int portNumber , 
*                       unsigned int MIIReg, unsigned int value)
* 
* Description
* This function will access the MII registers and will write the value
* to the MII register.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* value -the value that will be written.
* Outputs
* Returns Value
* true if success.
* false if fail to make the assignment.
* Error types (and exceptions if exist)
*/
GT_BOOL ffWriteMii (GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                       unsigned int value)
{
	GT_U32			smiReg;
	unsigned int	phyAddr;
	unsigned int	timeOut = SMI_TIMEOUT_COUNTER; /* in 100MS units */
	int	i;

	/* first check that it is not busy */	
    smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
    if (smiReg & SMI_BUSY) 
    {
   	    for(i=0; i<SMI_TIMEOUT_COUNTER; i++);
        do 
		{
            smiReg = gtOsGtRegRead(GT_REG_ETHER_SMI_REG);
            if (timeOut-- < 1) 
			{
                return GT_FALSE;
    	    }			
        } while (smiReg & SMI_BUSY);
    }
	/* not busy */
    phyAddr = portNumber;

    smiReg = 0; /* make sure no garbage value in reserved bits */
    smiReg = smiReg | (phyAddr << 16) | (SMI_OP_CODE_BIT_WRITE << 26) |
             (MIIReg << 21) | (value & 0xffff);	
    gtOsGtRegWrite(GT_REG_ETHER_SMI_REG, smiReg);	

    return GT_TRUE;
}
