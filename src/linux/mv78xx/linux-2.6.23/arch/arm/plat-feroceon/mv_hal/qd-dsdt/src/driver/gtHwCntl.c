#include <Copyright.h>
/********************************************************************************
* gtHwCntl.c
*
* DESCRIPTION:
*       Functions declarations for Hw accessing quarterDeck phy, internal and
*       global registers.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtHwCntl.h>
#include <gtMiiSmiIf.h>
#include <gtSem.h>


static GT_STATUS hwReadPPU(GT_QD_DEV *dev, GT_U16 *data);
static GT_STATUS hwWritePPU(GT_QD_DEV *dev, GT_U16 data);
static GT_STATUS coreReadPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    regAddr,
	OUT GT_U16   *data
);
static GT_STATUS coreWritePhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    regAddr,
	IN  GT_U16   data
);
static GT_STATUS coreReadPagedPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    pageNum,
	IN  GT_U8    regAddr,
	IN  GT_U32	 anyPage,
	OUT GT_U16   *data
);
static GT_STATUS coreWritePagedPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    pageNum,
	IN  GT_U8    regAddr,
	IN  GT_U32	 anyPage,
	IN  GT_U16   data
);

static GT_STATUS phyRegReadPPUEn (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int regAddr,
                        GT_U16* value);
static GT_STATUS phyRegWritePPUEn (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int regAddr,
                       GT_U16 value);
static GT_STATUS phyReadGlobal2Reg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
);
static GT_STATUS phyWriteGlobal2Reg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
);


/*******************************************************************************
* portToSmiMapping
*
* DESCRIPTION:
*       This function mapps port to smi address
*
* INPUTS:
*		dev - device context
*       portNum - Port number to read the register for.
*		accessType - type of register (Phy, Port, or Global)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       smiAddr    - smi address.
*
*******************************************************************************/
GT_U8 portToSmiMapping
(
    IN GT_QD_DEV *dev,
    IN GT_U8	portNum,
	IN GT_U32	accessType
)
{
	GT_U8 smiAddr;

	if(IS_IN_DEV_GROUP(dev,DEV_8PORT_SWITCH))
	{
		switch(accessType)
		{
			case PHY_ACCESS:
					if (dev->validPhyVec & (1<<portNum))
						smiAddr = PHY_REGS_START_ADDR_8PORT + portNum;
					else
						smiAddr = 0xFF;
					break;
			case PORT_ACCESS:
					if (dev->validPortVec & (1<<portNum))
						smiAddr = PORT_REGS_START_ADDR_8PORT + portNum;
					else
						smiAddr = 0xFF;
					break;
			case GLOBAL_REG_ACCESS:
					smiAddr = GLOBAL_REGS_START_ADDR_8PORT;
					break;
			default:
					smiAddr = GLOBAL_REGS_START_ADDR_8PORT + 1;
					break;
		}
	}
	else
	{
		smiAddr = dev->baseRegAddr;
		switch(accessType)
		{
			case PHY_ACCESS:
					if (dev->validPhyVec & (1<<portNum))
						smiAddr += PHY_REGS_START_ADDR + portNum;
					else
						smiAddr = 0xFF;
					break;
			case PORT_ACCESS:
					if (dev->validPortVec & (1<<portNum))
						smiAddr += PORT_REGS_START_ADDR + portNum;
					else
						smiAddr = 0xFF;
					break;
			case GLOBAL_REG_ACCESS:
					smiAddr += GLOBAL_REGS_START_ADDR;
					break;
			default:
					smiAddr += GLOBAL_REGS_START_ADDR - 1;
					break;
		}
	}

    return smiAddr;
}


/****************************************************************************/
/* Phy registers related functions.                                         */
/****************************************************************************/

/*******************************************************************************
* hwReadPhyReg
*
* DESCRIPTION:
*       This function reads a switch's port phy register.
*
* INPUTS:
*       portNum - Port number to read the register for.
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    regAddr,
	OUT GT_U16   *data
)
{
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	retVal = coreReadPhyReg(dev, portNum, regAddr, data);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwWritePhyReg
*
* DESCRIPTION:
*       This function writes to a switch's port phy register.
*
* INPUTS:
*       portNum - Port number to write the register for.
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWritePhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    regAddr,
	IN  GT_U16   data
)
{
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	retVal = coreWritePhyReg(dev, portNum, regAddr, data);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwGetPhyRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's port phy register.
*
* INPUTS:
*       portNum     - Port number to read the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetPhyRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	retVal = coreReadPhyReg(dev, portNum, regAddr, &tmpData);

	gtSemGive(dev,dev->multiAddrSem);

	if (retVal != GT_OK)
		return retVal;

    CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;

    DBG_INFO(("Read from phy(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

	return retVal;
}


/*******************************************************************************
* hwSetPhyRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's port phy register.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetPhyRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	retVal = coreReadPhyReg(dev, portNum, regAddr, &tmpData);

    if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

	retVal = coreWritePhyReg(dev, portNum, regAddr, tmpData);

	gtSemGive(dev,dev->multiAddrSem);
    return retVal;
}


/*******************************************************************************
* hwReadPagedPhyReg
*
* DESCRIPTION:
*       This function reads a switch's port phy register in page mode.
*
* INPUTS:
*       portNum - Port number to read the register for.
*       pageNum - Page number of the register to be read.
*       regAddr - The register's address.
*		anyPage - Any Page register vector
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadPagedPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    pageNum,
	IN  GT_U8    regAddr,
	IN  GT_U32	 anyPage,
	OUT GT_U16   *data
)
{
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	retVal = coreReadPagedPhyReg(dev,portNum,pageNum,regAddr,anyPage,data);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwWritePagedPhyReg
*
* DESCRIPTION:
*       This function writes to a switch's port phy register in page mode.
*
* INPUTS:
*       portNum - Port number to write the register for.
*       pageNum - Page number of the register to be written.
*       regAddr - The register's address.
*		anyPage - Any Page register vector
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWritePagedPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    pageNum,
	IN  GT_U8    regAddr,
	IN  GT_U32	 anyPage,
	IN  GT_U16   data
)
{
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	retVal = coreWritePagedPhyReg(dev,portNum,pageNum,regAddr,anyPage,data);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwGetPagedPhyRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's port phy register
*		in page mode.
*
* INPUTS:
*       portNum     - Port number to read the register for.
*       pageNum 	- Page number of the register to be read.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*		anyPage 	- Any Page register vector
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetPagedPhyRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    pageNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
	IN  GT_U32	 anyPage,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = coreReadPagedPhyReg(dev,portNum,pageNum,regAddr,anyPage,&tmpData);

	gtSemGive(dev,dev->multiAddrSem);

	if(retVal != GT_OK)
	{
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;

    DBG_INFO(("Read from phy(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetPagedPhyRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's port phy register
*		in page mode
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       pageNum 	- Page number of the register to be read.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*		anyPage 	- Any Page register vector
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetPagedPhyRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    pageNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
	IN  GT_U32	 anyPage,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;
	GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    if((retVal=coreReadPagedPhyReg(dev,portNum,pageNum,regAddr,anyPage,&tmpData)) != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));
    retVal = coreWritePagedPhyReg(dev,portNum,pageNum,regAddr,anyPage,tmpData);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwPhyReset
*
* DESCRIPTION:
*       This function performs softreset and waits until reset completion.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       u16Data     - data should be written into Phy control register.
*					  if this value is 0xFF, normal operation occcurs (read,
*					  update, and write back.)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS hwPhyReset
(
    IN  GT_QD_DEV	*dev,
    IN  GT_U8		portNum,
	IN	GT_U16		u16Data
)
{
    GT_U16 tmpData;
    GT_STATUS   retVal;
    GT_U32 retryCount;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

	if (u16Data == 0xFF)
	{
	    if((retVal=coreReadPhyReg(dev,portNum,0,&tmpData))
    	    != GT_OK)
	    {
    	    DBG_INFO(("Reading Register failed\n"));
			gtSemGive(dev,dev->multiAddrSem);
        	return retVal;
	    }
	}
	else
	{
		tmpData = u16Data;
	}

    /* Set the desired bits to 0. */
    tmpData |= 0x8000;

    if((retVal=coreWritePhyReg(dev,portNum,0,tmpData))
        != GT_OK)
    {
        DBG_INFO(("Writing to register failed\n"));
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
    }

    for (retryCount = 0x1000; retryCount > 0; retryCount--)
    {
        if((retVal=coreReadPhyReg(dev,portNum,0,&tmpData)) != GT_OK)
        {
            DBG_INFO(("Reading register failed\n"));
			gtSemGive(dev,dev->multiAddrSem);
            return retVal;
        }
        if ((tmpData & 0x8000) == 0)
            break;
    }

	gtSemGive(dev,dev->multiAddrSem);

    if (retryCount == 0)
    {
        DBG_INFO(("Reset bit is not cleared\n"));
        return GT_FAIL;
    }

    return GT_OK;
}

/****************************************************************************/
/* Per port registers related functions.                                    */
/****************************************************************************/

/*******************************************************************************
* hwReadPortReg
*
* DESCRIPTION:
*       This function reads a switch's port register.
*
* INPUTS:
*       portNum - Port number to read the register for.
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadPortReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PORT_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

    DBG_INFO(("Read from port(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
              portNum,phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}


/*******************************************************************************
* hwWritePortReg
*
* DESCRIPTION:
*       This function writes to a switch's port register.
*
* INPUTS:
*       portNum - Port number to write the register for.
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWritePortReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PORT_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

    DBG_INFO(("Write to port(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
              portNum,phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",data));

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwGetPortRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's port register.
*
* INPUTS:
*       portNum     - Port number to read the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetPortRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PORT_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

	gtSemGive(dev,dev->multiAddrSem);

	if (retVal != GT_OK)
		return retVal;

    CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    DBG_INFO(("Read from port(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetPortRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's port register.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetPortRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PORT_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

    if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);
    DBG_INFO(("Write to port(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    return retVal;
}


/*******************************************************************************
* hwSetPortRegBits
*
* DESCRIPTION:
*       This function writes to specified bits in a switch's port register.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       regAddr     - The register's address.
*       mask 		- The bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  When Data is 0x1002 and mask is 0xF00F, 0001b is written to bit[31:24]
*			and 0010b is written to bit[3:0]
*
*******************************************************************************/
GT_STATUS hwSetPortRegBits
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U16   mask,
    IN  GT_U16   data
)
{
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PORT_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

    if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= (data & mask);
    DBG_INFO(("Write to port(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("mask %d, data 0x%x.\n",mask,data));

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    return retVal;
}



/****************************************************************************/
/* Global registers related functions.                                      */
/****************************************************************************/

/*******************************************************************************
* hwReadGlobalReg
*
* DESCRIPTION:
*       This function reads a switch's global register.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadGlobalReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL_REG_ACCESS);

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

    DBG_INFO(("read from global register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}


/*******************************************************************************
* hwWriteGlobalReg
*
* DESCRIPTION:
*       This function writes to a switch's global register.
*
* INPUTS:
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWriteGlobalReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL_REG_ACCESS);

    DBG_INFO(("Write to global register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",data));

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

	return retVal;
}


/*******************************************************************************
* hwGetGlobalRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's global register.
*
* INPUTS:
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetGlobalRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    if(retVal != GT_OK)
	{
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);
    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    DBG_INFO(("Read from global register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetGlobalRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's global register.
*
* INPUTS:
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetGlobalRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

    if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);

    DBG_INFO(("Write to global register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    return retVal;
}

/*******************************************************************************
* hwReadGlobal2Reg
*
* DESCRIPTION:
*       This function reads a switch's global 2 register.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadGlobal2Reg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

    DBG_INFO(("read from global 2 register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}


/*******************************************************************************
* hwWriteGlobal2Reg
*
* DESCRIPTION:
*       This function writes to a switch's global 2 register.
*
* INPUTS:
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWriteGlobal2Reg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

    DBG_INFO(("Write to global 2 register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",data));

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

    return retVal;
}


/*******************************************************************************
* hwGetGlobal2RegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's global 2 register.
*
* INPUTS:
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetGlobal2RegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    if(retVal != GT_OK)
        return retVal;

    CALC_MASK(fieldOffset,fieldLength,mask);
    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    DBG_INFO(("Read from global 2 register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetGlobal2RegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's global 2 register.
*
* INPUTS:
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetGlobal2RegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

    if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);

    DBG_INFO(("Write to global 2 register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    return retVal;
}

/*******************************************************************************
* hwSetGlobal2RegBits
*
* DESCRIPTION:
*       This function writes to specified bits in a switch's global 2 register.
*
* INPUTS:
*       regAddr     - The register's address.
*       mask 		- The bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  When Data is 0x1002 and mask is 0xF00F, 0001b is written to bit[31:24]
*			and 0010b is written to bit[3:0]
*
*******************************************************************************/
GT_STATUS hwSetGlobal2RegBits
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U16   mask,
    IN  GT_U16   data
)
{
    GT_U16 tmpData;
	GT_STATUS   retVal;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,&tmpData);

    if(retVal != GT_OK)
	{
		gtSemGive(dev,dev->multiAddrSem);
        return retVal;
	}

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= (data & mask);

    DBG_INFO(("Write to global 2 register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("mask %d, data 0x%x.\n",mask,data));

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,tmpData);

	gtSemGive(dev,dev->multiAddrSem);

    return retVal;
}


/*******************************************************************************
* hwReadMiiReg
*
* DESCRIPTION:
*       This function reads a switch register.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadMiiReg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     phyAddr,
    IN  GT_U8     regAddr,
    OUT GT_U16    *data
)
{
    GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

 	gtSemGive(dev,dev->multiAddrSem);

    DBG_INFO(("Read from phy(0x%x) register: regAddr 0x%x, data 0x%x.\n",
              phyAddr,regAddr,*data));

    return retVal;
}


/*******************************************************************************
* hwWriteMiiReg
*
* DESCRIPTION:
*       This function writes a switch register.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWriteMiiReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    phyAddr,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_STATUS   retVal;

	gtSemTake(dev,dev->multiAddrSem,OS_WAIT_FOREVER);

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);

	gtSemGive(dev,dev->multiAddrSem);

    DBG_INFO(("Write to phy(0x%x) register: regAddr 0x%x, data 0x%x.\n",
              phyAddr,regAddr,data));

    return retVal;
}


/*******************************************************************************
* hwReadPPU
*
* DESCRIPTION:
*			This function reads PPU bit in Global Register
*
* INPUTS:
*			None.
*
* OUTPUTS:
*			data    - The read register's data.
*
* RETURNS:
*			GT_OK on success, or
*			GT_FAIL otherwise.
*
* COMMENTS:
*			This function can be used to access PHY register connected to Gigabit
*			Switch.
*			Semaphore should be acquired before this function get called.
*
*******************************************************************************/
static GT_STATUS hwReadPPU
(
	IN  GT_QD_DEV *dev,
	OUT GT_U16    *data
)
{
	GT_STATUS   retVal;
	GT_U16		tmpData;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

    retVal =  miiSmiIfReadRegister(dev,phyAddr,4,&tmpData);

    if(retVal != GT_OK)
	{
        return retVal;
	}

	*data = (tmpData >> 14) & 0x1;

	DBG_INFO(("OK.\n"));
	return GT_OK;
}

/*******************************************************************************
* hwWritePPU
*
* DESCRIPTION:
*			This function writes PPU bit in Global Register
*
* INPUTS:
*			data - The value to write into PPU bit
*
* OUTPUTS:
*			None.
*
* RETURNS:
*			GT_OK on success, or
*			GT_FAIL otherwise.
*
* COMMENTS:
*			This function can be used to access PHY register connected to Gigabit
*			Switch.
*			Semaphore should be acquired before this function get called.
*
*******************************************************************************/
static GT_STATUS hwWritePPU
(
	IN  GT_QD_DEV *dev,
	IN  GT_U16    data
)
{
	GT_STATUS   retVal;
	GT_U16		tmpData;
    GT_U8       phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

    retVal =  miiSmiIfReadRegister(dev,phyAddr,4,&tmpData);

    if(retVal != GT_OK)
	{
        return retVal;
	}

	if (data)
		tmpData |= (0x1 << 14);
	else
		tmpData &= ~(0x1 << 14);

    retVal = miiSmiIfWriteRegister(dev,phyAddr,4,tmpData);

    if(retVal != GT_OK)
	{
        return retVal;
	}

	/* busy wait - till PPU is actually disabled */
	if (data == 0) /* disable PPU */
	{
		gtDelay(250);
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;
}


static GT_STATUS coreReadPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    regAddr,
	OUT GT_U16   *data
)
{
	GT_U8       phyAddr;
	GT_STATUS   retVal, retPPU;
	GT_U16		orgPPU;
	GT_BOOL		usePPU = GT_FALSE;

	phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PHY_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if((retPPU=hwReadPPU(dev, &orgPPU)) != GT_OK)
		{
			return retPPU;
		}

		if(orgPPU)
		{
			if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS))
			{
				if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS_RES))
				{
					if(dev->revision != 0)
						usePPU = GT_TRUE;
				}
				else
				{
				 	usePPU = GT_TRUE;
				}
			}

			/* Disable PPU so that External Phy can be accessible */
			if (!usePPU)
			{
			if((retPPU=hwWritePPU(dev, 0)) != GT_OK)
			{
				return retPPU;
			}
		}
	}
	}

	if (usePPU)
	{
		retVal = phyRegReadPPUEn (dev,phyAddr,regAddr,data);
	}
	else
	{
	retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);
	}

	DBG_INFO(("Read from phy(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
				portNum,phyAddr,regAddr));

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if(orgPPU && (!usePPU))
		{
			if((retPPU=hwWritePPU(dev, orgPPU)) != GT_OK)
			{
				return retPPU;
			}
		}
	}

	return retVal;
}


static GT_STATUS coreWritePhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    regAddr,
	IN  GT_U16   data
)
{
	GT_U8   		phyAddr;
	GT_STATUS   retVal, retPPU;
	GT_U16		orgPPU;
	GT_BOOL		usePPU = GT_FALSE;

	phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PHY_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if((retPPU=hwReadPPU(dev, &orgPPU)) != GT_OK)
		{
			return retPPU;
		}

		if(orgPPU)
		{
			if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS))
			{
				if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS_RES))
				{
					if(dev->revision != 0)
						usePPU = GT_TRUE;
				}
				else
				{
				 	usePPU = GT_TRUE;
				}
			}

			/* Disable PPU so that External Phy can be accessible */
			if (!usePPU)
			{
			if((retPPU=hwWritePPU(dev, 0)) != GT_OK)
			{
				return retPPU;
			}
		}
	}
	}

	DBG_INFO(("Write to phy(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
				portNum,phyAddr,regAddr));
	DBG_INFO(("data 0x%x.\n",data));

	if (usePPU)
	{
		retVal = phyRegWritePPUEn (dev,phyAddr,regAddr,data);
	}
	else
	{
	retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if(orgPPU && (!usePPU))
		{
			if((retPPU=hwWritePPU(dev, orgPPU)) != GT_OK)
			{
				return retPPU;
			}
		}
	}

	return retVal;
}


static GT_STATUS coreReadPagedPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    pageNum,
	IN  GT_U8    regAddr,
	IN  GT_U32	 anyPage,
	OUT GT_U16   *data
)
{
	GT_U8       phyAddr,pageAddr;
	GT_STATUS   retVal, retPPU;
	GT_U16		orgPPU, tmpData, orgPage;
	GT_BOOL		usePPU = GT_FALSE;

	phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PHY_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if((retPPU=hwReadPPU(dev, &orgPPU)) != GT_OK)
		{
			return retPPU;
		}

		if(orgPPU)
		{
			if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS))
			{
				if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS_RES))
				{
					if(dev->revision != 0)
						usePPU = GT_TRUE;
				}
				else
				{
				 	usePPU = GT_TRUE;
				}
			}

			/* Disable PPU so that External Phy can be accessible */
			if (!usePPU)
			{
			if((retPPU=hwWritePPU(dev, 0)) != GT_OK)
			{
				return retPPU;
			}
		}
	}
	}

	if(anyPage & (1 << regAddr))
	{
		if (usePPU)
		{
			retVal = phyRegReadPPUEn (dev,phyAddr,regAddr,data);
		}
		else
		{
		retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);
		}
		DBG_INFO(("Read from phy(%d) register: smiAddr 0x%x, pageNum 0x%x, regAddr 0x%x\n",
					portNum,phyAddr,pageNum,regAddr));
	}
	else
	{
	    pageAddr = GT_GET_PAGE_ADDR(regAddr);

		if (usePPU)
		{
			retVal = phyRegReadPPUEn (dev,phyAddr,regAddr,&orgPage);
		}
		else
		{
			retVal = miiSmiIfReadRegister(dev,phyAddr,pageAddr,&orgPage);
		}

		if (retVal != GT_OK)
		{
			DBG_INFO(("Reading page register failed\n"));
			return retVal;
		}

		if(pageAddr == 22)
			tmpData = orgPage & 0xFF00;
		else
			tmpData = orgPage & 0xFFC0;
		tmpData |= pageNum;

		if (usePPU)
		{
			if((retVal = phyRegWritePPUEn(dev,phyAddr,pageAddr,tmpData)) == GT_OK)
			{
				retVal = phyRegReadPPUEn (dev,phyAddr,regAddr,data);

				DBG_INFO(("Read from phy(%d) register: smiAddr 0x%x, pageNum 0x%x, regAddr 0x%x\n",
							portNum,phyAddr,pageNum,regAddr));
			}
		}
		else
		{
		if((retVal = miiSmiIfWriteRegister(dev,phyAddr,pageAddr,tmpData)) == GT_OK)
		{
			retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

			DBG_INFO(("Read from phy(%d) register: smiAddr 0x%x, pageNum 0x%x, regAddr 0x%x\n",
						portNum,phyAddr,pageNum,regAddr));
		}
	}
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if(orgPPU && (!usePPU))
		{
			if((retPPU=hwWritePPU(dev, orgPPU)) != GT_OK)
			{
				return retPPU;
			}
		}
	}

	return retVal;

}


static GT_STATUS coreWritePagedPhyReg
(
	IN GT_QD_DEV *dev,
	IN  GT_U8    portNum,
	IN  GT_U8    pageNum,
	IN  GT_U8    regAddr,
	IN  GT_U32	 anyPage,
	IN  GT_U16   data
)
{
	GT_U8   		phyAddr,pageAddr;
	GT_STATUS   retVal, retPPU;
	GT_U16		orgPPU, tmpData, orgPage;
	GT_BOOL		usePPU = GT_FALSE;

	phyAddr = CALC_SMI_DEV_ADDR(dev, portNum, PHY_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if((retPPU=hwReadPPU(dev, &orgPPU)) != GT_OK)
		{
			return retPPU;
		}

		if(orgPPU)
		{
			if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS))
			{
				if(IS_IN_DEV_GROUP(dev,DEV_PPU_PHY_ACCESS_RES))
				{
					if(dev->revision != 0)
						usePPU = GT_TRUE;
				}
				else
				{
				 	usePPU = GT_TRUE;
				}
			}

			/* Disable PPU so that External Phy can be accessible */
			if (!usePPU)
			{
			if((retPPU=hwWritePPU(dev, 0)) != GT_OK)
			{
				return retPPU;
			}
		}
	}
	}

	DBG_INFO(("Write to phy(%d) register: smiAddr 0x%x, pageNum 0x%x, regAddr 0x%x\n",
				portNum,phyAddr,pageNum,regAddr));
	DBG_INFO(("data 0x%x.\n",data));

	if(anyPage & (1 << regAddr))
	{
		if (usePPU)
		{
			retVal = phyRegWritePPUEn (dev,phyAddr,regAddr,data);
		}
		else
		{
		retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);
	}
	}
	else
	{
	    pageAddr = GT_GET_PAGE_ADDR(regAddr);

		if (usePPU)
		{
			retVal = phyRegReadPPUEn (dev,phyAddr,regAddr,&orgPage);
		}
		else
		{
			retVal = miiSmiIfReadRegister(dev,phyAddr,pageAddr,&orgPage);
		}

		if (retVal != GT_OK)
		{
			DBG_INFO(("Reading page register failed\n"));
			return retVal;
		}

		if(pageAddr == 22)
			tmpData = orgPage & 0xFF00;
		else
			tmpData = orgPage & 0xFFC0;
		tmpData |= pageNum;

		if (usePPU)
		{
			if((retVal = phyRegWritePPUEn(dev,phyAddr,pageAddr,tmpData)) == GT_OK)
			{
				retVal = phyRegWritePPUEn(dev,phyAddr,regAddr,data);
			}
		}
		else
		{
		if((retVal = miiSmiIfWriteRegister(dev,phyAddr,pageAddr,tmpData)) == GT_OK)
		{
			retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);
		}
	}
	}

	if(IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY))
	{
		if(orgPPU && (!usePPU))
		{
			if((retPPU=hwWritePPU(dev, orgPPU)) != GT_OK)
			{
				return retPPU;
			}
		}
	}

	return retVal;
}


/*****************************************************************************
* phyRegReadPPUEn
*
* DESCRIPTION:
*       This function reads phy register data when PPU is enabled.
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
static GT_STATUS phyRegReadPPUEn (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int regAddr,
                        unsigned short* value)
{
	volatile unsigned int timeOut; /* in 100MS units */
	volatile int i;
	GT_U16 smiReg;

	DBG_INFO(("Read Phy register while PPU Enabled\n"));

	/* first check that it is not busy */
    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
    {
		DBG_INFO(("Reading Phy register Failed\n"));
        return GT_FAIL;
    }
    timeOut = QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    if(smiReg & QD_SMI_BUSY)
    {
        for(i = 0 ; i < QD_SMI_TIMEOUT ; i++);
        do
        {
            if(timeOut-- < 1 )
            {
				DBG_INFO(("Reading Phy register Timed Out\n"));
    	        return GT_FAIL;
    	    }
		    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
		    {
				DBG_INFO(("Reading Phy register Failed\n"));
		        return GT_FAIL;
		    }
        } while (smiReg & QD_SMI_BUSY);
    }

    smiReg =  QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_READ << QD_SMI_OP_BIT) |
    		(regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if(phyWriteGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, smiReg) != GT_OK)
    {
        return GT_FAIL;
    }
    timeOut = QD_SMI_ACCESS_LOOP; /* initialize the loop count */
    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
    {
        return GT_FAIL;
    }

    if(smiReg & QD_SMI_BUSY)
    {
        for(i = 0 ; i < QD_SMI_TIMEOUT ; i++);
		do
		{
            if(timeOut-- < 1 )
            {
				DBG_INFO(("Reading Phy register Timed Out\n"));
    	        return GT_FALSE;
    	    }
		    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
		    {
				DBG_INFO(("Reading Phy register Failed\n"));
		        return GT_FAIL;
		    }
        } while (smiReg & QD_SMI_BUSY);
	}
    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_DATA, &smiReg) != GT_OK)
    {
		DBG_INFO(("Reading Phy register Failed\n"));
        return GT_FAIL;
    }
	*value = (unsigned short)smiReg;

	return GT_OK;
}

/*****************************************************************************
* phyRegWritePPUEn
*
* DESCRIPTION:
*       This function writes data to a phy register when PPU is enabled.
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

static GT_STATUS phyRegWritePPUEn (GT_QD_DEV* dev, unsigned int phyAddr , unsigned int regAddr,
                       unsigned short value)
{
	volatile unsigned int timeOut; /* in 100MS units */
	volatile int i;
	GT_U16 smiReg;

	DBG_INFO(("Writing Phy register while PPU Enabled\n"));

	/* first check that it is not busy */
    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
    {
		DBG_INFO(("Reading Phy register Failed\n"));
        return GT_FAIL;
    }
    timeOut = QD_SMI_ACCESS_LOOP; /* initialize the loop count */

    if(smiReg & QD_SMI_BUSY)
    {
        for(i = 0 ; i < QD_SMI_TIMEOUT ; i++);
        do
        {
            if(timeOut-- < 1 )
            {
				DBG_INFO(("Writing Phy register Timed Out\n"));
    	        return GT_FALSE;
    	    }
		    if(phyReadGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, &smiReg) != GT_OK)
		    {
				DBG_INFO(("Writing Phy register Failed\n"));
		        return GT_FAIL;
		    }
        } while (smiReg & QD_SMI_BUSY);
    }

    if(phyWriteGlobal2Reg(dev,QD_REG_SMI_PHY_DATA, value) != GT_OK)
    {
		DBG_INFO(("Writing Phy Data register Failed\n"));
        return GT_FAIL;
    }
    smiReg = QD_SMI_BUSY | (phyAddr << QD_SMI_DEV_ADDR_BIT) | (QD_SMI_WRITE << QD_SMI_OP_BIT) |
			(regAddr << QD_SMI_REG_ADDR_BIT) | (QD_SMI_CLAUSE22 << QD_SMI_MODE_BIT);

    if(phyWriteGlobal2Reg(dev,QD_REG_SMI_PHY_CMD, smiReg) != GT_OK)
    {
		DBG_INFO(("Writing Phy Command register Failed\n"));
        return GT_FAIL;
    }

    return GT_OK;
}


static GT_STATUS phyReadGlobal2Reg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

    DBG_INFO(("read from global 2 register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}


static GT_STATUS phyWriteGlobal2Reg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev, 0, GLOBAL2_REG_ACCESS);
	if (phyAddr == 0xFF)
	{
		return GT_BAD_PARAM;
	}

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);

    return retVal;
}

