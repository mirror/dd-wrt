#include <Copyright.h>

/********************************************************************************
* gtHwCntl.h
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

#ifndef __gtHwCntlh
#define __gtHwCntlh

#include <msApi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This macro is used to calculate the register's SMI   */
/* device address, according to the baseAddr            */
/* field in the Switch configuration struct.            */
#define CALC_SMI_DEV_ADDR(_dev, _portNum, _accessType)        \
            portToSmiMapping(_dev, _portNum, _accessType)

/* This macro calculates the mask for partial read /    */
/* write of register's data.                            */
#define CALC_MASK(fieldOffset,fieldLen,mask)        \
            if((fieldLen + fieldOffset) >= 16)      \
                mask = (0 - (1 << fieldOffset));    \
            else                                    \
                mask = (((1 << (fieldLen + fieldOffset))) - (1 << fieldOffset))

#define GT_GET_PAGE_ADDR(_regAddr) ((_regAddr<29)?22:29)

/* Start address of phy related register.               */
#define PHY_REGS_START_ADDR     0x0
#define PHY_REGS_START_ADDR_8PORT	0x0

/* Start address of ports related register.             */
#define PORT_REGS_START_ADDR    	0x8
#define PORT_REGS_START_ADDR_8PORT	0x10

/* Start address of global register.                    */
#define GLOBAL_REGS_START_ADDR  0xF
#define GLOBAL_REGS_START_ADDR_8PORT  0x1B

#define PHY_ACCESS			1
#define PORT_ACCESS			2
#define GLOBAL_REG_ACCESS	3
#define GLOBAL2_REG_ACCESS	4

#define QD_SMI_ACCESS_LOOP		1000
#define QD_SMI_TIMEOUT			2


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    OUT GT_U16    *data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    IN  GT_U16    data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    OUT GT_U16    *data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    IN  GT_U16    data
);

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
    IN  GT_QD_DEV    *dev,
    IN  GT_U8     portNum,
	IN	GT_U16		u16Data
);


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
*		anyPage - register list(vector) that are common to all pages
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
);


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
*		anyPage - Register list(vector) that are common to all pages
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
);

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
*		anyPage - Register list(vector) that are common to all pages
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
);

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
*		anyPage 	- Register list(vector) that are common to all pages
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
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    OUT GT_U16    *data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    IN  GT_U16    data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    OUT GT_U16    *data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     portNum,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    IN  GT_U16    data
);

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
);

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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    OUT GT_U16    *data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    IN  GT_U16    data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    OUT GT_U16    *data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    IN  GT_U16    data
);


/****************************************************************************/
/* Global 2 registers related functions.                                      */
/****************************************************************************/

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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    OUT GT_U16    *data
);


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
    IN  GT_U8     regAddr,
    IN  GT_U16    data
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    OUT GT_U16    *data
);


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
*
*******************************************************************************/
GT_STATUS hwSetGlobal2RegField
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     regAddr,
    IN  GT_U8     fieldOffset,
    IN  GT_U8     fieldLength,
    IN  GT_U16    data
);

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
);

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
);


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
    IN  GT_QD_DEV *dev,
    IN  GT_U8     phyAddr,
    IN  GT_U8     regAddr,
    IN  GT_U16    data
);

#ifdef __cplusplus
}
#endif
#endif /* __gtHwCntlh */
