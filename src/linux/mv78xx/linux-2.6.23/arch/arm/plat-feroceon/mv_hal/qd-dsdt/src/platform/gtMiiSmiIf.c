#include <Copyright.h>
/********************************************************************************
* gtMiiSmiIf.c
*
* DESCRIPTION:
*       Includes functions prototypes for initializing and accessing the
*       MII / SMI interface.
*       This is the only file to be included from upper layers.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtHwCntl.h>
#include <gtMiiSmiIf.h>
#include <platformDeps.h>
#include <gtSem.h>

GT_BOOL qdMultiAddrRead (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int MIIReg,
                        unsigned int* value);
GT_BOOL qdMultiAddrWrite (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int MIIReg,
                       unsigned int value);
/*******************************************************************************
* miiSmiIfInit
*
* DESCRIPTION:
*       This function initializes the MII / SMI interface.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       highSmiDevAddr - Indicates whether to use the high device register
*                     addresses when accessing switch's registers (of all kinds)
*                     i.e, the devices registers range is 0x10 to 0x1F, or to
*                     use the low device register addresses (range 0x0 to 0xF).
*                       GT_TRUE     - use high addresses (0x10 to 0x1F).
*                       GT_FALSE    - use low addresses (0x0 to 0xF).
*
* RETURNS:
*       DEVICE_ID       - on success
*       0    - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U16 miiSmiIfInit
(
	IN  GT_QD_DEV    *dev,
	OUT GT_BOOL * highSmiDevAddr
)
{
	GT_STATUS status;
	GT_U16 data, data1;

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR,QD_REG_SWITCH_ID,&data)) != GT_OK)
	{
		return 0;
	}

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+1,QD_REG_SWITCH_ID,&data1)) != GT_OK)
	{
		return 0;
	}

	switch(data & 0xFF00)
	{
		case 0x0200:
		case 0x0300:
		case 0x0500:
		case 0x0600:
		case 0x1500:
		case 0xF500:
		case 0xF900:
			if (data == data1)
			{
				*highSmiDevAddr = GT_FALSE;
				return data;
			}
			break;
		default:
			break;
	}

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+0x10,QD_REG_SWITCH_ID,&data)) != GT_OK)
	{
		return 0;
	}

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+0x11,QD_REG_SWITCH_ID,&data1)) != GT_OK)
	{
		return 0;
	}

	switch(data & 0xFF00)
	{
		case 0x0200:
		case 0x0300:
		case 0x0500:
		case 0x0600:
		case 0x1500:
		case 0xF500:
		case 0xF900:
			if (data == data1)
			{
				*highSmiDevAddr = GT_TRUE;
				return data;
			}
			break;
		case 0x0800:
		case 0x1A00:
		case 0x1000:
		case 0x0900:
		case 0x0400:
		case 0x1200:
		case 0x1400:
		case 0x1600:
			if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+0xF,QD_REG_SWITCH_ID,&data1)) != GT_OK)
			{
				return 0;
			}

			if (data == data1)
			{
				*highSmiDevAddr = GT_FALSE;
				return data;
			}
			break;
		default:
			break;
	}

    return 0;
}


/*******************************************************************************
* miiSmiManualIfInit
*
* DESCRIPTION:
*       This function returns Device ID from the given base address
*
* INPUTS:
*       baseAddr - either 0x0 or 0x10. Indicates whether to use the low device 
*					register address or high device register address.
*					The device register range is from 0x0 to 0xF or from 0x10 
*					to 0x1F for 5 port switchs and from 0x0 to 0x1B for 8 port 
*					switchs.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       DEVICE_ID       - on success
*       0    - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U16 miiSmiManualIfInit
(
	IN  GT_QD_DEV   *dev,
	IN  GT_U32		baseAddr
)
{
	GT_STATUS status;
	GT_U16 data;

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+baseAddr,QD_REG_SWITCH_ID,&data)) != GT_OK)
	{
		return 0;
	}

	switch(data & 0xFF00)
	{
		case 0x0200:
		case 0x0300:
		case 0x0500:
		case 0x0600:
		case 0x1500:
		case 0xF500:
		case 0xF900:
			return data;
		default:
			break;
	}
	if(baseAddr != 0)
		return 0;

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR_8PORT+baseAddr,QD_REG_SWITCH_ID,&data)) != GT_OK)
	{
		return 0;
	}

	switch(data & 0xFF00)
	{
		case 0x0800:
		case 0x1A00:
		case 0x1000:
		case 0x0900:
		case 0x0400:
		case 0x1200:
		case 0x1400:
		case 0x1600:
			return data;
		default:
			break;
	}

    return 0;
}


/*******************************************************************************
* miiSmiIfReadRegister
*
* DESCRIPTION:
*       This function reads a register throw the SMI / MII interface, to be used
*       by upper layers.
*
* INPUTS:
*       phyAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*
* OUTPUTS:
*       data        - The register's data.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS miiSmiIfReadRegister
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U8        phyAddr,
    IN  GT_U8        regAddr,
    OUT GT_U16       *data
)
{
    unsigned int tmpData;

	if(dev->accessMode == SMI_MULTI_ADDR_MODE)
	{
     	if(qdMultiAddrRead(dev,(GT_U32)phyAddr,(GT_U32)regAddr,&tmpData) != GT_TRUE)
	    {
    	    return GT_FAIL;
	    }
	}
    else
	{
     	if(dev->fgtReadMii(dev,(GT_U32)phyAddr,(GT_U32)regAddr,&tmpData) != GT_TRUE)
	    {
    	    return GT_FAIL;
	    }
	}
    *data = (GT_U16)tmpData;
    return GT_OK;
}


/*******************************************************************************
* miiSmiIfWriteRegister
*
* DESCRIPTION:
*       This function writes to a register throw the SMI / MII interface, to be
*       used by upper layers.
*
* INPUTS:
*       phyAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*       data        - The data to be written to the register.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS miiSmiIfWriteRegister
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U8        phyAddr,
    IN  GT_U8        regAddr,
    IN  GT_U16       data
)
{
	if(dev->accessMode == SMI_MULTI_ADDR_MODE)
	{
     	if(qdMultiAddrWrite(dev,(GT_U32)phyAddr,(GT_U32)regAddr,(GT_U32)data) != GT_TRUE)
	    {
    	    return GT_FAIL;
	    }
	}
    else
	{
	    if(dev->fgtWriteMii(dev,(GT_U32)phyAddr,(GT_U32)regAddr,(GT_U32)data) != GT_TRUE)
	    {
    	    return GT_FAIL;
	    }
	}
	return GT_OK;
}


/*****************************************************************************
* qdMultiAddrRead
*
* DESCRIPTION:
*       This function reads data from a device in the secondary MII bus.
*
* INPUTS:
*       phyAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*       value       - The storage where register date to be saved.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE   - on success
*       GT_FALSE  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL qdMultiAddrRead (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int regAddr,
                        unsigned int* value)
{
	unsigned int smiReg;
	volatile unsigned int timeOut; /* in 100MS units */
	volatile int i;

	/* first check that it is not busy */
    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, &smiReg) != GT_TRUE)
    {
        return GT_FALSE;
    }
    timeOut = QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    if(smiReg & QD_SMI_BUSY) 
    {
        for(i = 0 ; i < QD_SMI_TIMEOUT ; i++);
        do 
        {
            if(timeOut-- < 1 ) 
            {
    	        return GT_FALSE;
    	    }
		    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, &smiReg) != GT_TRUE)
		    {
		        return GT_FALSE;
		    }
        } while (smiReg & QD_SMI_BUSY);
    }

    smiReg =  QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_READ << QD_SMI_OP_BIT) | 
    		(regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if(dev->fgtWriteMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, smiReg) != GT_TRUE)
    {
        return GT_FALSE;
    }
    timeOut = QD_SMI_ACCESS_LOOP; /* initialize the loop count */
    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, &smiReg) != GT_TRUE)
    {
        return GT_FALSE;
    }

    if(smiReg & QD_SMI_BUSY) 
    {
        for(i = 0 ; i < QD_SMI_TIMEOUT ; i++);
		do 
		{
            if(timeOut-- < 1 ) 
            {
    	        return GT_FALSE;
    	    }
		    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, &smiReg) != GT_TRUE)
		    {
		        return GT_FALSE;
		    }
        } while (smiReg & QD_SMI_BUSY);
	}
    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_DATA, &smiReg) != GT_TRUE)
    {
        return GT_FALSE;
    }
	*value = smiReg;
    
	return GT_TRUE;
}

/*****************************************************************************
* qdMultiAddrWrite
*
* DESCRIPTION:
*       This function writes data to the device in the secondary MII bus.
*
* INPUTS:
*       phyAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*       value       - The data to be written into the register.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE   - on success
*       GT_FALSE  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/

GT_BOOL qdMultiAddrWrite (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int regAddr,
                       unsigned int value)
{
	unsigned int smiReg;
	volatile unsigned int timeOut; /* in 100MS units */
	volatile int i;

	/* first check that it is not busy */
    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, &smiReg) != GT_TRUE)
    {
        return GT_FALSE;
    }
    timeOut = QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    if(smiReg & QD_SMI_BUSY) 
    {
        for(i = 0 ; i < QD_SMI_TIMEOUT ; i++);
        do 
        {
            if(timeOut-- < 1 ) 
            {
    	        return GT_FALSE;
    	    }
		    if(dev->fgtReadMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, &smiReg) != GT_TRUE)
		    {
		        return GT_FALSE;
		    }
        } while (smiReg & QD_SMI_BUSY);
    }

    if(dev->fgtWriteMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_DATA, value) != GT_TRUE)
    {
        return GT_FALSE;
    }
    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE << QD_SMI_OP_BIT) | 
			(regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if(dev->fgtWriteMii(dev,(GT_U32)dev->phyAddr,(GT_U32)QD_REG_SMI_COMMAND, smiReg) != GT_TRUE)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

