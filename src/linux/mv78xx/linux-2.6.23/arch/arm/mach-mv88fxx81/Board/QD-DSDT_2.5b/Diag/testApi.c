#include <Copyright.h>
/********************************************************************************
* testApi.c
*
* DESCRIPTION:
*       API test functions
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

/*
#define TEST_DEBUG
*/
#define MAX_TEST_RUN			16

ATU_ENTRY_INFO *gAtuEntry = NULL;
int gAgeDelayTime = 0;

GT_U32 testSysCtrl(GT_QD_DEV *dev);
GT_U32 testPort(GT_QD_DEV *dev);
GT_U32 testATU(GT_QD_DEV *dev);
GT_U32 testVlan(GT_QD_DEV *dev);
GT_U32 testSTP(GT_QD_DEV *dev);
GT_U32 testPhy(GT_QD_DEV *dev);
GT_U32 testPortStatus(GT_QD_DEV *dev);
GT_U32 testQoSRule(GT_QD_DEV *dev);
GT_U32 testQoSMap(GT_QD_DEV *dev);
GT_U32 testRMON(GT_QD_DEV *dev);
GT_U32 testVTU(GT_QD_DEV *dev);
GT_U32 testSysStatus(GT_QD_DEV *dev);
GT_U32 testPortRateCtrl(GT_QD_DEV *dev);
GT_U32 testPortPAV(GT_QD_DEV *dev);
GT_U32 testInterrupt(GT_QD_DEV *dev);
GT_U32 testPCSCtrl(GT_QD_DEV *dev);
GT_U32 testPIRL(GT_QD_DEV *dev);

TEST_STRUCT testStruct[] = 
	{
	{"SysCtrl    :", testSysCtrl,    0},
	{"PortCtrl   :", testPort,       0},
	{"ATU        :", testATU,        0},
	{"VLAN       :", testVlan,       0},
	{"STP        :", testSTP,        0},
	{"PhyCtrl    :", testPhy,        0},
	{"Port Status:", testPortStatus, 0},
	{"QoS Rule   :", testQoSRule,    0},
	{"QoS Map    :", testQoSMap,     0},
	{"RMON       :", testRMON,       0},
	{"VTU        :", testVTU,        0},
	{"Sys Status :", testSysStatus,  0},
	{"Port Rate  :", testPortRateCtrl, 0},
	{"Port PAV   :", testPortPAV,    0},
	{"PCSCtrl    :", testPCSCtrl,    0},
	/*Interrupt is tested only on simulator
	{"Interrupt  :", testInterrupt,  GT_OK},
	*/
	{"PIRL       :", testPIRL,    0},
	{"", NULL,GT_OK}
	};
#if 0
TEST_STRUCT testStructG[] = 
	{
	{"SysCtrl(G)    :", testSysCtrlG,    0},
	{"PortCtrl(G)   :", testPortG,       0},
	{"ATU(G)        :", testATUG,        0},
	{"VLAN(G)       :", testVlanG,       0},
	{"STP(G)        :", testSTPG,        0},
	{"PhyCtrl(G)    :", testPhyG,        0},
	{"Port Status(G):", testPortStatusG, 0},
	{"QoS Rule(G)   :", testQoSRuleG,    0},
	{"QoS Map(G)    :", testQoSMapG,     0},
	{"RMON(G)       :", testRMONG,       0},
	{"VTU(G)        :", testVTUG,        0},
	{"Sys Status(G) :", testSysStatusG,  0},
	{"Port Rate(G)  :", testPortRateCtrlG, 0},
	{"Port PAV(G)   :", testPortPAVG,    0},
	/*Interrupt is tested only on simulator
	{"Interrupt  :", testInterrupt,  GT_OK},
	*/
	{"", NULL,GT_OK}
	};
#endif

void testDisplayStatus(GT_STATUS status)
{
	switch(status)
	{
		case GT_OK :
			MSG_PRINT(("Passed.\n"));
			break;
		case GT_FAIL :
			MSG_PRINT(("Failed.\n"));
			break;
		case GT_BAD_PARAM :
			MSG_PRINT(("Bad Parameter.\n"));
			break;
		case GT_NOT_SUPPORTED :
			MSG_PRINT(("Not Supported.\n"));
			break;
		case GT_NOT_FOUND :
			MSG_PRINT(("Not Found.\n"));
			break;
		case GT_NO_MORE :
			MSG_PRINT(("No more Item.\n"));
			break;
		case GT_NO_SUCH :
			MSG_PRINT(("No Such Item.\n"));
			break;
		default:
			MSG_PRINT(("Failed.\n"));
			break;
	}
}

void printATUEntry(GT_ATU_ENTRY *entry)
{
	MSG_PRINT(("(%02x-%02x-%02x-%02x-%02x-%02x), DBNum %i, PortVec %#x, Pri %#x, State %#x\n",
					entry->macAddr.arEther[0],
					entry->macAddr.arEther[1],
					entry->macAddr.arEther[2],
					entry->macAddr.arEther[3],
					entry->macAddr.arEther[4],
					entry->macAddr.arEther[5],
					entry->DBNum,
					entry->portVec,
					entry->prio,
					entry->entryState.mcEntryState));
}

void dumpMemory(char* startAddr, int length)
{	int i;

	printf("\n");
	while(length>16)
	{
		for(i=0; i<16; i++)
			printf("%02x ",(unsigned char)*startAddr++);
		printf("\n");
		length -= 16;
	}
	for(i=0; i<length; i++)
		printf("%02x ",(unsigned char)*startAddr++);
	printf("\n");
}

GT_STATUS testBoolType(GT_QD_DEV *dev, TEST_API* api)
{
	GT_STATUS status;
	GT_BOOL orgValue, tmpValue;
	GT_STATUS testResult = GT_OK;
	
	/* Get the Original value */
	if((status=api->getFunc.bool(dev,&orgValue)) != GT_OK)
	{
		MSG_PRINT(("Reading Bool Value Failed (%#x).\n", status));
		return status;
	}

	/* Set to 1 */
	if((status=api->setFunc.bool(dev,GT_TRUE)) != GT_OK)
	{
		MSG_PRINT(("Writing Bool Value Failed (%#x).\n", status));
		return status;
	}

	/* Get the modified value */
	if((status=api->getFunc.bool(dev,&tmpValue)) != GT_OK)
	{
		MSG_PRINT(("Reading Bool Value Failed (%#x).\n", status));
		return status;
	}

	if(tmpValue != GT_TRUE)
	{
		MSG_PRINT(("Test Value Mismatch (expected GT_TRUE).\n"));
		testResult = GT_FAIL;
	}

	/* Set to 0 */
	if((status=api->setFunc.bool(dev,GT_FALSE)) != GT_OK)
	{
		MSG_PRINT(("Writing Bool Value Failed (%#x).\n", status));
		return status;
	}

	/* Get the modified value */
	if((status=api->getFunc.bool(dev,&tmpValue)) != GT_OK)
	{
		MSG_PRINT(("Reading Bool Value Failed (%#x).\n", status));
		return status;
	}

	if(tmpValue != GT_FALSE)
	{
		MSG_PRINT(("Test Value Mismatch (expected GT_FALSE).\n"));
		testResult = GT_FAIL;
	}

	/* Set to original value */
	if((status=api->setFunc.bool(dev,orgValue)) != GT_OK)
	{
		MSG_PRINT(("Writing Bool Value Failed (%#x).\n", status));
		return status;
	}

	return testResult;		
}

GT_STATUS testU16Type(GT_QD_DEV *dev, TEST_API* api, int testLimit)
{
	GT_STATUS status;
	GT_U16 orgValue, tmpValue, i;
	GT_STATUS testResult = GT_OK;
		
	/* Get the Original value */
	if((status=api->getFunc.u16(dev,&orgValue)) != GT_OK)
	{
		MSG_PRINT(("Reading U16 Value Failed (%#x).\n", status));
		return status;
	}

	/* Set to 0 */
	for(i=0; i<(GT_U16)testLimit; i++)
	{
		if((status=api->setFunc.u16(dev,i)) != GT_OK)
		{
			MSG_PRINT(("Writing U16 Value Failed (%#x, value %i).\n", status,i));
			return status;
		}

		/* Get the modified value */
		if((status=api->getFunc.u16(dev,&tmpValue)) != GT_OK)
		{
			MSG_PRINT(("Reading U16 Value Failed (%#x, value %i).\n", status,i));
			return status;
		}

		if(tmpValue != i)
		{
			MSG_PRINT(("Test Value Mismatch (write %i, read %i).\n",i,tmpValue));
			testResult = GT_FAIL;
		}
	}

	/* Set to original value */
	if((status=api->setFunc.u16(dev,orgValue)) != GT_OK)
	{
		MSG_PRINT(("Writing U16 Value Failed (%#x, org value %i).\n", status, orgValue));
		return status;
	}

	return testResult;		
}


GT_STATUS testU32Type(GT_QD_DEV *dev, TEST_API* api, int testLimit)
{
	GT_STATUS status;
	GT_U32 orgValue, tmpValue, i;
	GT_STATUS testResult = GT_OK;
		
	/* Get the Original value */
	if((status=api->getFunc.u32(dev,&orgValue)) != GT_OK)
	{
		MSG_PRINT(("Reading U32 Value Failed (%#x).\n", status));
		return status;
	}

	/* Set to 0 */
	for(i=0; i<(GT_U32)testLimit; i++)
	{
		if((status=api->setFunc.u32(dev,i)) != GT_OK)
		{
			MSG_PRINT(("Writing U32 Value Failed (%#x, value %i).\n", status,i));
			return status;
		}

		/* Get the modified value */
		if((status=api->getFunc.u32(dev,&tmpValue)) != GT_OK)
		{
			MSG_PRINT(("Reading U32 Value Failed (%#x, value %i).\n", status,i));
			return status;
		}

		if(tmpValue != i)
		{
			MSG_PRINT(("Test Value Mismatch (write %i, read %i).\n",i,tmpValue));
			testResult = GT_FAIL;
		}
	}

	/* Set to original value */
	if((status=api->setFunc.u32(dev,orgValue)) != GT_OK)
	{
		MSG_PRINT(("Writing U32 Value Failed (%#x, org value %i).\n", status, orgValue));
		return status;
	}

	return testResult;		
}


GT_STATUS testMacType(GT_QD_DEV *dev, TEST_API* api)
{
	GT_STATUS status;
	GT_ETHERADDR orgMac, tmpMacIn, tmpMacOut;
	GT_STATUS testResult = GT_OK;
		
	/* Get the Discard Excessive state */
	if((status=api->getFunc.mac(dev,&orgMac)) != GT_OK)
	{
		MSG_PRINT(("Reading MAC Address Failed (%#x).\n", status));
		return status;
	}

	tmpMacIn.arEther[0] = 0xAA;
	tmpMacIn.arEther[1] = 0xAA;
	tmpMacIn.arEther[2] = 0xAA;
	tmpMacIn.arEther[3] = 0xAA;
	tmpMacIn.arEther[4] = 0xAA;
	tmpMacIn.arEther[5] = 0xAA;

	/* Set the Discard Excessive state */
	if((status=api->setFunc.mac(dev,&tmpMacIn)) != GT_OK)
	{
		MSG_PRINT(("Writing MAC Address Failed (%#x).\n", status));
		return status;
	}

	/* Get the Discardl Excessive state */
	if((status=api->getFunc.mac(dev,&tmpMacOut)) != GT_OK)
	{
		MSG_PRINT(("Reading MAC Address Failed (%#x).\n", status));
		return status;
	}

	if(memcmp(&tmpMacIn,&tmpMacOut,6) != 0)
	{
		MSG_PRINT(("Unexpected MAC address(%#x-%#x-%#x-%#x-%#x-%#x)\n",
					tmpMacOut.arEther[0],
					tmpMacOut.arEther[1],
					tmpMacOut.arEther[2],
					tmpMacOut.arEther[3],
					tmpMacOut.arEther[4],
					tmpMacOut.arEther[5]));

		testResult = GT_FAIL;
	}

	tmpMacIn.arEther[0] = 0x54;
	tmpMacIn.arEther[1] = 0x55;
	tmpMacIn.arEther[2] = 0x55;
	tmpMacIn.arEther[3] = 0x55;
	tmpMacIn.arEther[4] = 0x55;
	tmpMacIn.arEther[5] = 0x55;

	/* Set the Discard Excessive state */
	if((status=api->setFunc.mac(dev,&tmpMacIn)) != GT_OK)
	{
		MSG_PRINT(("Writing MAC Address Failed (%#x).\n", status));
		return status;
	}

	/* Get the Discardl Excessive state */
	if((status=api->getFunc.mac(dev,&tmpMacOut)) != GT_OK)
	{
		MSG_PRINT(("Reading MAC Address Failed (%#x).\n", status));
		return status;
	}

	if(memcmp(&tmpMacIn,&tmpMacOut,6) != 0)
	{
		MSG_PRINT(("Unexpected MAC address(%#x-%#x-%#x-%#x-%#x-%#x)\n",
					tmpMacOut.arEther[0],
					tmpMacOut.arEther[1],
					tmpMacOut.arEther[2],
					tmpMacOut.arEther[3],
					tmpMacOut.arEther[4],
					tmpMacOut.arEther[5]));

		testResult = GT_FAIL;
	}

	tmpMacIn.arEther[0] = 0x00;
	tmpMacIn.arEther[1] = 0x00;
	tmpMacIn.arEther[2] = 0x00;
	tmpMacIn.arEther[3] = 0x00;
	tmpMacIn.arEther[4] = 0x00;
	tmpMacIn.arEther[5] = 0x01;

	/* Set the Discard Excessive state */
	if((status=api->setFunc.mac(dev,&tmpMacIn)) != GT_OK)
	{
		MSG_PRINT(("Writing MAC Address Failed (%#x).\n", status));
		return status;
	}

	/* Get the Discardl Excessive state */
	if((status=api->getFunc.mac(dev,&tmpMacOut)) != GT_OK)
	{
		MSG_PRINT(("Reading MAC Address Failed (%#x).\n", status));
		return status;
	}

	if(memcmp(&tmpMacIn,&tmpMacOut,6) != 0)
	{
		MSG_PRINT(("Unexpected MAC address(%#x-%#x-%#x-%#x-%#x-%#x)\n",
					tmpMacOut.arEther[0],
					tmpMacOut.arEther[1],
					tmpMacOut.arEther[2],
					tmpMacOut.arEther[3],
					tmpMacOut.arEther[4],
					tmpMacOut.arEther[5]));

		testResult = GT_FAIL;
	}

	/* Set the Discard Excessive state with original value */
	if((status=api->setFunc.mac(dev,&orgMac)) != GT_OK)
	{
		MSG_PRINT(("Writing MAC Address Failed (%#x).\n", status));
		return status;
	}

	return testResult;		
}


GT_STATUS testPortBoolType(GT_QD_DEV *dev, TEST_API* api)
{
	GT_STATUS status;
	GT_BOOL orgValue, tmpValue;
	GT_STATUS testResult = GT_OK;
	GT_LPORT port;
	int portIndex;
		
	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		/* Get the Original value */
		if((status=api->getFunc.port_bool(dev,port,&orgValue)) != GT_OK)
		{
			MSG_PRINT(("Reading Bool Value Failed (%#x).\n", status));
			return status;
		}

		/* Set to 1 */
		if((status=api->setFunc.port_bool(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("Writing Bool Value Failed (%#x).\n", status));
			return status;
		}

		/* Get the modified value */
		if((status=api->getFunc.port_bool(dev,port,&tmpValue)) != GT_OK)
		{
			MSG_PRINT(("Reading Bool Value Failed (%#x).\n", status));
			return status;
		}

		if(tmpValue != GT_TRUE)
		{
			MSG_PRINT(("Test Value Mismatch (expected GT_TRUE).\n"));
			testResult = GT_FAIL;
		}

		/* Set to 0 */
		if((status=api->setFunc.port_bool(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("Writing Bool Value Failed (%#x).\n", status));
			return status;
		}

		/* Get the modified value */
		if((status=api->getFunc.port_bool(dev,port,&tmpValue)) != GT_OK)
		{
			MSG_PRINT(("Reading Bool Value Failed (%#x).\n", status));
			return status;
		}

		if(tmpValue != GT_FALSE)
		{
			MSG_PRINT(("Test Value Mismatch (expected GT_FALSE).\n"));
			testResult = GT_FAIL;
		}

		/* Set to original value */
		if((status=api->setFunc.port_bool(dev,port,orgValue)) != GT_OK)
		{
			MSG_PRINT(("Writing Bool Value Failed (%#x).\n", status));
			return status;
		}

		if (testResult != GT_OK)
			return testResult;

	}
	return testResult;		
}


GT_STATUS testPortU8Type(GT_QD_DEV *dev, TEST_API* api, int testLimit)
{
	GT_STATUS status;
	GT_U8 orgValue, tmpValue, i;
	GT_STATUS testResult = GT_OK;
	GT_LPORT port;
	int portIndex;
		
	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		/* Get the Original value */
		if((status=api->getFunc.port_u8(dev,port,&orgValue)) != GT_OK)
		{
			MSG_PRINT(("Reading U8 Value Failed (%#x).\n", status));
			return status;
		}

		/* Set to 0 */
		for(i=0; i<(GT_U8)testLimit; i++)
		{
			if((status=api->setFunc.port_u8(dev,port,i)) != GT_OK)
			{
				MSG_PRINT(("Writing U8 Value Failed (%#x).\n", status));
				return status;
			}

			/* Get the modified value */
			if((status=api->getFunc.port_u8(dev,port,&tmpValue)) != GT_OK)
			{
				MSG_PRINT(("Reading U8 Value Failed (%#x).\n", status));
				return status;
			}

			if(tmpValue != i)
			{
				MSG_PRINT(("U16 Value Mismatch (port %i, write %#x, read %#x).\n", portIndex,i,tmpValue));
				testResult = GT_FAIL;
			}
		}

		/* Set to original value */
		if((status=api->setFunc.port_u8(dev,port,orgValue)) != GT_OK)
		{
			MSG_PRINT(("Writing U8 Value Failed (%#x).\n", status));
			return status;
		}

		if (testResult != GT_OK)
			return testResult;

	}
	return testResult;		
}

GT_STATUS testPortU16Type(GT_QD_DEV *dev, TEST_API* api, int testLimit)
{
	GT_STATUS status;
	GT_U16 orgValue, tmpValue, i;
	GT_STATUS testResult = GT_OK;
	GT_LPORT port;
	int portIndex;
		
	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		/* Get the Original value */
		if((status=api->getFunc.port_u16(dev,port,&orgValue)) != GT_OK)
		{
			MSG_PRINT(("Reading U16 Value Failed (%#x).\n", status));
			return status;
		}

		/* Set to 0 */
		for(i=0; i<(GT_U16)testLimit; i++)
		{
			if((status=api->setFunc.port_u16(dev,port,i)) != GT_OK)
			{
				MSG_PRINT(("Writing U16 Value Failed (%#x).\n", status));
				return status;
			}

			/* Get the modified value */
			if((status=api->getFunc.port_u16(dev,port,&tmpValue)) != GT_OK)
			{
				MSG_PRINT(("Reading U16 Value Failed (%#x).\n", status));
				return status;
			}

			if(tmpValue != i)
			{
				MSG_PRINT(("U16 Value Mismatch (port %i, write %#x, read %#x).\n", portIndex,i,tmpValue));
				testResult = GT_FAIL;
				return GT_FAIL; /* MJ Temp */
			}
		}

		/* Set to original value */
		if((status=api->setFunc.port_u16(dev,port,orgValue)) != GT_OK)
		{
			MSG_PRINT(("Writing U16 Value Failed (%#x).\n", status));
			return status;
		}

		if (testResult != GT_OK)
			return testResult;

	}
	return testResult;		
}

GT_STATUS testPortU32Type(GT_QD_DEV *dev, TEST_API* api, int testLimit)
{
	GT_STATUS status;
	GT_U32 orgValue, tmpValue, i;
	GT_STATUS testResult = GT_OK;
	GT_LPORT port;
	int portIndex;
		
	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;
		
		/* Get the Original value */
		if((status=api->getFunc.port_u32(dev,port,&orgValue)) != GT_OK)
		{
			MSG_PRINT(("Reading U32 Value Failed (%#x).\n", status));
			return status;
		}

		/* Set to 0 */
		for(i=0; i<(GT_U32)testLimit; i++)
		{
			if((status=api->setFunc.port_u32(dev,port,i)) != GT_OK)
			{
				MSG_PRINT(("Writing U32 Value Failed (port%i,data%i,%#x).\n", port,i,status));
				return status;
			}

			/* Get the modified value */
			if((status=api->getFunc.port_u32(dev,port,&tmpValue)) != GT_OK)
			{
				MSG_PRINT(("Reading U32 Value Failed (%#x).\n", status));
				return status;
			}

			if(tmpValue != i)
			{
				MSG_PRINT(("U16 Value Mismatch (port %i, write %#x, read %#x).\n", port,i,tmpValue));
				testResult = GT_FAIL;
			}
		}

		/* Set to original value */
		if((status=api->setFunc.port_u32(dev,port,orgValue)) != GT_OK)
		{
			MSG_PRINT(("Writing Org Value Failed (value %i,%#x).\n", orgValue,status));
			return status;
		}

		if (testResult != GT_OK)
			return testResult;

	}
	return testResult;		
}

GT_STATUS testU32U32Type(GT_QD_DEV *dev, TEST_API* api, int indexLimit, int testLimit)
{
	GT_STATUS status;
	GT_U32 orgValue, tmpValue, i;
	GT_STATUS testResult = GT_OK;
	int index;

	for(index=0; index<indexLimit; index++)
	{
		/* Get the Original value */
		if((status=api->getFunc.port_u32(dev,index,&orgValue)) != GT_OK)
		{
			MSG_PRINT(("Reading U32 Org Value Failed (%#x).\n", status));
			return status;
		}

		/* Set to 0 */
		for(i=0; i<(GT_U32)testLimit; i++)
		{
			if((status=api->setFunc.port_u32(dev,index,i)) != GT_OK)
			{
				MSG_PRINT(("Writing U32 Value Failed (%#x) index:%i,value:%i.\n", status,index,i));
				return status;
			}

			/* Get the modified value */
			if((status=api->getFunc.port_u32(dev,index,&tmpValue)) != GT_OK)
			{
				MSG_PRINT(("Reading U32 Value Failed (%#x) index:%i,value:%i.\n", status,index,i));
				return status;
			}

			if(tmpValue != i)
			{
				MSG_PRINT(("U16 Value Mismatch (index %i, write %#x, read %#x).\n", index,i,tmpValue));
				testResult = GT_FAIL;
			}
		}

		/* Set to original value */
		if((status=api->setFunc.port_u32(dev,index,orgValue)) != GT_OK)
		{
			MSG_PRINT(("Writing U32 Org Value Failed (%#x) index:%i,value:%i.\n", status,index,orgValue));
			return status;
		}

		if (testResult != GT_OK)
			return testResult;

	}
	return testResult;		
}

GT_STATUS testTrunkPortSetup(GT_QD_DEV *dev, int portIndex)
{
	GT_STATUS status;
	GT_U32 orgTrunkId, trunkId, tmpId;
	GT_LPORT port;
	GT_BOOL orgEn, tmpEn;

	port = (GT_LPORT)portIndex;

	if((status = gprtGetTrunkPort(dev,port,&orgEn,&orgTrunkId)) != GT_OK)
	{
		MSG_PRINT(("Getting Org. TrunkPort setup failed (port%i, status:%#x).\n", port,status));
		return status;		
	}

	for(trunkId=0; trunkId<16; trunkId++)
	{
		if((status = gprtSetTrunkPort(dev,port,GT_TRUE,trunkId)) != GT_OK)
		{
			MSG_PRINT(("Setting TrunkPort setup failed (port%i,trunkId:%i,status:%#x).\n", port,trunkId,status));
			return status;		
		}

		if((status = gprtGetTrunkPort(dev,port,&tmpEn,&tmpId)) != GT_OK)
		{
			MSG_PRINT(("Getting TrunkPort setup failed (port%i, status:%#x).\n", port,status));
			return status;		
		}

		if((tmpEn != GT_TRUE) || (tmpId != trunkId))
		{
			MSG_PRINT(("TrunkPort Enable failed (ID:%i,%i).\n", trunkId,tmpId));
			return status;		
		}

	}

	if((status = gprtSetTrunkPort(dev,port,GT_FALSE,trunkId)) != GT_OK)
	{
		MSG_PRINT(("Setting TrunkPort setup failed (port%i,trunkId:%i,status:%#x).\n", port,trunkId,status));
		return status;		
	}

	if((status = gprtGetTrunkPort(dev,port,&tmpEn,&tmpId)) != GT_OK)
	{
		MSG_PRINT(("Getting TrunkPort setup failed (port%i, status:%#x).\n", port,status));
		return status;		
	}

	if(tmpEn != GT_FALSE)
	{
		MSG_PRINT(("TrunkPort Disable failed (En:%i,%i).\n", GT_FALSE,tmpEn));
		return status;		
	}

	if((status = gprtSetTrunkPort(dev,port,orgEn,orgTrunkId)) != GT_OK)
	{
		MSG_PRINT(("Setting TrunkPort setup failed (port%i,trunkId:%i,status:%#x).\n", port,trunkId,status));
		return status;		
	}

	return GT_OK;
}

GT_U32 testSysCtrlG(GT_QD_DEV *dev )
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	GT_U32 data;
	TEST_API testAPI;

	/*
	 *  PPU Setup API
	 */
	testAPI.getFunc.bool = gsysGetPPUEn;
	testAPI.setFunc.bool = gsysSetPPUEn;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("PPU Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Cascade Port Setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetCascadePort;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetCascadePort;
	if((status = testU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Cascade Port Setup API test "));
	testDisplayStatus(status);
	/*
	 *  Device Number Setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetDeviceNumber;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetDeviceNumber;
	if((status = testU32Type(dev,&testAPI,32)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Device Number Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Core Tag Type setup API
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gsysGetCoreTagType;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gsysSetCoreTagType;
	if((status = testU16Type(dev,&testAPI,64)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Core Tag Type Setup API test "));
	testDisplayStatus(status);

	/*
	 *  IngressMonitorDest setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetIngressMonitorDest;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetIngressMonitorDest;
	if((status = testU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("IngressMonitorDest Setup API test "));
	testDisplayStatus(status);

	/*
	 *  EngressMonitorDest setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetEgressMonitorDest;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetEgressMonitorDest;
	if((status = testU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("EngressMonitorDest Setup API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
			return testResults;
		default:
			break;
	}			

	/*
	 *  ARPDest setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetARPDest;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetARPDest;
	if((status = testU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ARPDest Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Rsvd2CpuEnables setup API
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gsysGetRsvd2CpuEnables;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gsysSetRsvd2CpuEnables;
	if((status = testU16Type(dev,&testAPI,0x8001)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Rsvd2CpuEnables Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Rsvd2Cpu setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetRsvd2Cpu;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetRsvd2Cpu;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Rsvd2Cpu Setup API test "));
	testDisplayStatus(status);

	/*
	 *  MGMTPri setup API
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gsysGetMGMTPri;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gsysSetMGMTPri;
	if((status = testU16Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("MGMTPri Setup API test "));
	testDisplayStatus(status);

	/*
	 *  UseDoubleTagData setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetUseDoubleTagData;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetUseDoubleTagData;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("UseDoubleTagData Setup API test "));
	testDisplayStatus(status);

	/*
	 *  PreventLoops setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetPreventLoops;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetPreventLoops;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("PreventLoops Setup API test "));
	testDisplayStatus(status);

	/*
	 *  FlowControlMessage setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetFlowControlMessage;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetFlowControlMessage;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("FlowControlMessage Setup API test "));
	testDisplayStatus(status);

	/*
	 *  gsysSetForceFlowControlPri setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetForceFlowControlPri;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetForceFlowControlPri;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ForceFlowControlPri Setup API test "));
	testDisplayStatus(status);

	/*
	 *  FcPri setup API
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gsysGetFCPri;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gsysSetFCPri;
	if((status = testU16Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("FCPri Setup API test "));
	testDisplayStatus(status);

	/*
	 *  gsysSetHashTrunk setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetHashTrunk;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetHashTrunk;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("HashTrunk Setup API test "));
	testDisplayStatus(status);

	/*
	 *  FlowCtrlDelay Setup
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gsysGetFlowCtrlDelay;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gsysSetFlowCtrlDelay;
	if((status = testU32U32Type(dev,&testAPI,3,0x2000)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("FlowCtrlDelay Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DevRoutingTable Setup
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gsysGetDevRoutingTable;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gsysSetDevRoutingTable;
	if((status = testU32U32Type(dev,&testAPI,32,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DevRoutingTable Setup API test "));
	testDisplayStatus(status);

	/*
	 *  TrunkMaskTable Setup
	 */
	switch (dev->deviceId)
	{
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
				data = 4;
				break;
		default:
				data = 8;
				break;
	}
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gsysGetTrunkMaskTable;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gsysSetTrunkMaskTable;
	if((status = testU32U32Type(dev,&testAPI,data,(1<<(dev->numOfPorts-1)))) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("TrunkMaskTable Setup API test "));
	testDisplayStatus(status);

	/*
	 *  gsysSetTrunkRouting Setup
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gsysGetTrunkRouting;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gsysSetTrunkRouting;
	if((status = testU32U32Type(dev,&testAPI,8,(1<<(dev->numOfPorts-1)))) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("TrunkRouting Setup API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6092:
		case GT_88E6095:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
			if (dev->revision < 2)
				return testResults;
			break;
		case GT_88E6131:
		case GT_88E6108:
		default:
			break;
	}			

	/*
	 *  gsysSetRateLimitMode Setup
	 */
	testAPI.getFunc.bool = (GT_API_GET_BOOL)gsysGetRateLimitMode;
	testAPI.setFunc.bool = (GT_API_SET_BOOL)gsysSetRateLimitMode;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Rate Limit Mode Setup API test "));
	testDisplayStatus(status);

	/*
	 *  gsysSetAgeInt Setup
	 */
	testAPI.getFunc.bool = gsysGetAgeInt;
	testAPI.setFunc.bool = gsysSetAgeInt;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Age Interrupt Setup API test "));
	testDisplayStatus(status);

	/*
	 *  gsysSetForceSnoopPri setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetForceSnoopPri;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetForceSnoopPri;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ForceSnoopPri Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Snoop Pri setup API
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gsysGetSnoopPri;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gsysSetSnoopPri;
	if((status = testU16Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Snoop Pri Setup API test "));
	testDisplayStatus(status);

	/*
	 *  gsysSetForceARPPri setup API
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetForceARPPri;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetForceARPPri;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ForceARPPri Setup API test "));
	testDisplayStatus(status);

	/*
	 *  ARP Pri setup API
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gsysGetARPPri;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gsysSetARPPri;
	if((status = testU16Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ARP Pri Setup API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6131:
		case GT_88E6108:
			return testResults;
		default:
			break;
	}			

	/*
	 *  Use Port Schedule API
	 */
	testAPI.getFunc.bool = gsysGetUsePortSchedule;
	testAPI.setFunc.bool = gsysSetUsePortSchedule;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Use Port Schedule API test "));
	testDisplayStatus(status);

	/*
	 *  Use Old Header API
	 */
	testAPI.getFunc.bool = gsysGetOldHader;
	testAPI.setFunc.bool = gsysSetOldHader;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Use Old Header API test "));
	testDisplayStatus(status);

	/*
	 *  Recursive Stripping Disable API
	 */
	testAPI.getFunc.bool = gsysGetRecursiveStrippingDisable;
	testAPI.setFunc.bool = gsysSetRecursiveStrippingDisable;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Recursive Stripping Disable API test "));
	testDisplayStatus(status);

	/*
	 *  CPU Port
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gsysGetCPUPort;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gsysSetCPUPort;
	if((status = testU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("CPU Port API test "));
	testDisplayStatus(status);

	return testResults;
}

GT_U32 testSysCtrl(GT_QD_DEV *dev )
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	/* Sw Reset */
	if((status=gsysSwReset(dev)) != GT_OK)
	{
		MSG_PRINT(("gsysSwReset returned Fail (%#x).\n", status));
		testResults |= 1 << status;
		return testResults;
	}
	testResult = GT_OK;

	/*
	 *  Testing Discard Excessive State API
	 */
	testAPI.getFunc.bool = gsysGetDiscardExcessive;
	testAPI.setFunc.bool = gsysSetDiscardExcessive;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DiscardExcessive API test "));
	testDisplayStatus(status);

	/*
	 *  Testing Scheduling Mose API
	 */
	testAPI.getFunc.bool = gsysGetSchedulingMode;
	testAPI.setFunc.bool = gsysSetSchedulingMode;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Scheduling Mode API test "));
	testDisplayStatus(status);


	/*
	 *  Testing Max Frame Size API
	 */
	testAPI.getFunc.bool = gsysGetMaxFrameSize;
	testAPI.setFunc.bool = gsysSetMaxFrameSize;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("MAX Frame Size API test "));
	testDisplayStatus(status);


	/*
	 *  Testing WatchDog API
	 */
	testAPI.getFunc.bool = gsysGetWatchDog;
	testAPI.setFunc.bool = gsysSetWatchDog;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("WatchDog API test "));
	testDisplayStatus(status);


	/*
	 *  Testing PerPortDuplexPauseMac API
	 */
	testAPI.getFunc.bool = gsysGetPerPortDuplexPauseMac;
	testAPI.setFunc.bool = gsysSetPerPortDuplexPauseMac;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Per Port Duplex Pause Mac API test "));
	testDisplayStatus(status);


#if 0
	/*
	 *  Retransmit Mode API
	 */
	testAPI.getFunc.bool = gsysGetRetransmitMode;
	testAPI.setFunc.bool = gsysSetRetransmitMode;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Retransmit Mode API test "));
	testDisplayStatus(status);

	/*
	 *  Limit Backoff API
	 */
	testAPI.getFunc.bool = gsysGetLimitBackoff;
	testAPI.setFunc.bool = gsysSetLimitBackoff;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Limit Backoff API test "));
	testDisplayStatus(status);

	/*
	 *  Rsv Queue's Request Priority API
	 */
	testAPI.getFunc.bool = gsysGetRsvReqPri;
	testAPI.setFunc.bool = gsysSetRsvReqPri;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Rsv Queue's Request Priority API test "));
	testDisplayStatus(status);
#endif

	/*
	 *  Testing DuplexPauseMac API
	 */
	testAPI.getFunc.mac = gsysGetDuplexPauseMac;
	testAPI.setFunc.mac = gsysSetDuplexPauseMac;
	if((status = testMacType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Duplex Pause Mac API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
			testResults |= testSysCtrlG(dev);
			break;
		default:
			break;
	}
	
#if 0
	/* Reload EEPROM value */
	if((status=gsysReLoad(dev)) != GT_OK)
	{
		MSG_PRINT(("gsysReLoad returned Fail (%#x).\n", status));
		testResults |= 1 << status;
		return testResults;
	}
#endif

	return testResults;
}

GT_U32 testPCSCtrl(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	testResult = GT_OK;

	/*
	 *  Inband Auto-Nego Bypass Setup API
	 */
	testAPI.getFunc.port_bool = gpcsGetAnBypassMode;
	testAPI.setFunc.port_bool = gpcsSetAnBypassMode;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Inband Auto-Nego Bypass Setup API test "));
	testDisplayStatus(status);

	/*
	 *  PCS Inband Auto-Nego Setup API
	 */
	testAPI.getFunc.port_bool = gpcsGetPCSAnEn;
	testAPI.setFunc.port_bool = gpcsSetPCSAnEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("PCS Inband Auto-Nego Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Link Value Setup
	 */
	testAPI.getFunc.port_bool = gpcsGetLinkValue;
	testAPI.setFunc.port_bool = gpcsSetLinkValue;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Link Value Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Forced Link Setup
	 */
	testAPI.getFunc.port_bool = gpcsGetForcedLink;
	testAPI.setFunc.port_bool = gpcsSetForcedLink;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Forced Link Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Duplex Value Setup
	 */
	testAPI.getFunc.port_bool = gpcsGetDpxValue;
	testAPI.setFunc.port_bool = gpcsSetDpxValue;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Duplex Value Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Forced Duplex Setup
	 */
	testAPI.getFunc.port_bool = gpcsGetForcedDpx;
	testAPI.setFunc.port_bool = gpcsSetForcedDpx;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Forced Duplex Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Forced Speed Setup
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gpcsGetForceSpeed;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gpcsSetForceSpeed;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Forced Speed Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Flow control Value Setup
	 */
	testAPI.getFunc.port_bool = gpcsGetFCValue;
	testAPI.setFunc.port_bool = gpcsSetFCValue;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Flow Control Value Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Forced Flow control Setup
	 */
	testAPI.getFunc.port_bool = gpcsGetForcedFC;
	testAPI.setFunc.port_bool = gpcsSetForcedFC;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Forced Flow Control Setup API test "));
	testDisplayStatus(status);

	return testResults;
}

GT_U32 testPortEnhancedFE(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;
	int portIndex;

	testResult = GT_OK;

	/*
	 *  Drop on Lock
	 */
	testAPI.getFunc.port_bool = gprtGetDropOnLock;
	testAPI.setFunc.port_bool = gprtSetDropOnLock;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Drop on Lock API test "));
	testDisplayStatus(status);

	/*
	 *  Double Tag
	 */
	testAPI.getFunc.port_bool = gprtGetDoubleTag;
	testAPI.setFunc.port_bool = gprtSetDoubleTag;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Double Tag API test "));
	testDisplayStatus(status);

	/*
	 *  Interswitch Port
	 */
	testAPI.getFunc.port_bool = gprtGetInterswitchPort;
	testAPI.setFunc.port_bool = gprtSetInterswitchPort;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Interswith port setup API test "));
	testDisplayStatus(status);

	/*
	 *  Learning Disable
	 */
	testAPI.getFunc.port_bool = gprtGetLearnDisable;
	testAPI.setFunc.port_bool = gprtSetLearnDisable;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Learning Disable API test "));
	testDisplayStatus(status);

	/*
	 *  FCS Ignore
	 */
	testAPI.getFunc.port_bool = gprtGetIgnoreFCS;
	testAPI.setFunc.port_bool = gprtSetIgnoreFCS;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("FCS Ignore API test "));
	testDisplayStatus(status);

	/*
	 *  VTU Priority Override
	 */
	testAPI.getFunc.port_bool = gprtGetVTUPriOverride;
	testAPI.setFunc.port_bool = gprtSetVTUPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("VTU Priority Override API test "));
	testDisplayStatus(status);

	/*
	 *  SA Priority Override
	 */
	testAPI.getFunc.port_bool = gprtGetSAPriOverride;
	testAPI.setFunc.port_bool = gprtSetSAPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("SA Priority Override API test "));
	testDisplayStatus(status);

	/*
	 *  DA Priority Override
	 */
	testAPI.getFunc.port_bool = gprtGetDAPriOverride;
	testAPI.setFunc.port_bool = gprtSetDAPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DA Priority Override API test "));
	testDisplayStatus(status);

	/*
	 *  CPU Port Setup
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetCPUPort;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetCPUPort;
	if((status = testPortU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("CPU Port Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Locked Port Setup
	 */
	testAPI.getFunc.port_bool = gprtGetLockedPort;
	testAPI.setFunc.port_bool = gprtSetLockedPort;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Locked Port Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Ignore Wrong Data Setup
	 */
	testAPI.getFunc.port_bool = gprtGetIgnoreWrongData;
	testAPI.setFunc.port_bool = gprtSetIgnoreWrongData;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Ignore Wrong Data Setup API test "));
	testDisplayStatus(status);

	/*
	 *  UseCoreTag Setup
	 */
	testAPI.getFunc.port_bool = gprtGetUseCoreTag;
	testAPI.setFunc.port_bool = gprtSetUseCoreTag;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("UseCoreTag Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DiscardTagged Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDiscardTagged;
	testAPI.setFunc.port_bool = gprtSetDiscardTagged;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DiscardTagged Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DiscardUntagged Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDiscardUntagged;
	testAPI.setFunc.port_bool = gprtSetDiscardUntagged;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DiscardUntagged Setup API test "));
	testDisplayStatus(status);

	/*
	 *  MapDA Setup
	 */
	testAPI.getFunc.port_bool = gprtGetMapDA;
	testAPI.setFunc.port_bool = gprtSetMapDA;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("MapDA Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DefaultForward Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDefaultForward;
	testAPI.setFunc.port_bool = gprtSetDefaultForward;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DefaultForward Setup API test "));
	testDisplayStatus(status);

	/*
	 *  EgressMonitorSource Setup
	 */
	testAPI.getFunc.port_bool = gprtGetEgressMonitorSource;
	testAPI.setFunc.port_bool = gprtSetEgressMonitorSource;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("EgressMonitorSource Setup API test "));
	testDisplayStatus(status);

	/*
	 *  IngressMonitorSource Setup
	 */
	testAPI.getFunc.port_bool = gprtGetIngressMonitorSource;
	testAPI.setFunc.port_bool = gprtSetIngressMonitorSource;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("IngressMonitorSource Setup API test "));
	testDisplayStatus(status);

	/*
	 *  MessagePort Setup
	 */
	testAPI.getFunc.port_bool = gprtGetMessagePort;
	testAPI.setFunc.port_bool = gprtSetMessagePort;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("MessagePort Setup API test "));
	testDisplayStatus(status);

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		if((status = testTrunkPortSetup(dev,portIndex)) != GT_OK)
		{
			testResult = GT_FAIL;
			testResults |= 1 << status;
			portIndex = 0xFF;
			break;		
		}
	}
	MSG_PRINT(("TrunkPort Setup API test "));
	testDisplayStatus(status);

	/*
	 *  AGE Int Setup
	 */
	testAPI.getFunc.port_bool = geventGetAgeIntEn;
	testAPI.setFunc.port_bool = geventSetAgeIntEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("AGE Interrupt Setup API test "));
	testDisplayStatus(status);

	/*
	 *  SA Filtering
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetSAFiltering;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetSAFiltering;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("SA Filtering Setup API test "));
	testDisplayStatus(status);

	/*
	 *  ARP to CPU Setup
	 */
	testAPI.getFunc.port_bool = gprtGetARPtoCPU;
	testAPI.setFunc.port_bool = gprtSetARPtoCPU;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ARP to CPU Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Egress Flood
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetEgressFlood;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetEgressFlood;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Egress Flood Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Port Scheduling
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetPortSched;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetPortSched;
	if((status = testPortU32Type(dev,&testAPI,2)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Port Scheduling Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Provider Tag
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetProviderTag;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetProviderTag;
	if((status = testPortU32Type(dev,&testAPI,0xFF)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Provider Tag Setup API test "));
	testDisplayStatus(status);

	return testResults;
}

GT_U32 testPortG(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;
	int portIndex;

	testResult = GT_OK;

	/*
	 *  Drop on Lock
	 */
	testAPI.getFunc.port_bool = gprtGetDropOnLock;
	testAPI.setFunc.port_bool = gprtSetDropOnLock;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Drop on Lock API test "));
	testDisplayStatus(status);

	/*
	 *  Double Tag
	 */
	testAPI.getFunc.port_bool = gprtGetDoubleTag;
	testAPI.setFunc.port_bool = gprtSetDoubleTag;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Double Tag API test "));
	testDisplayStatus(status);

	/*
	 *  Interswitch Port
	 */
	testAPI.getFunc.port_bool = gprtGetInterswitchPort;
	testAPI.setFunc.port_bool = gprtSetInterswitchPort;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Interswith port setup API test "));
	testDisplayStatus(status);

	/*
	 *  Learning Disable
	 */
	testAPI.getFunc.port_bool = gprtGetLearnDisable;
	testAPI.setFunc.port_bool = gprtSetLearnDisable;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Learning Disable API test "));
	testDisplayStatus(status);

	/*
	 *  FCS Ignore
	 */
	testAPI.getFunc.port_bool = gprtGetIgnoreFCS;
	testAPI.setFunc.port_bool = gprtSetIgnoreFCS;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("FCS Ignore API test "));
	testDisplayStatus(status);

	/*
	 *  VTU Priority Override
	 */
	testAPI.getFunc.port_bool = gprtGetVTUPriOverride;
	testAPI.setFunc.port_bool = gprtSetVTUPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("VTU Priority Override API test "));
	testDisplayStatus(status);

	/*
	 *  SA Priority Override
	 */
	testAPI.getFunc.port_bool = gprtGetSAPriOverride;
	testAPI.setFunc.port_bool = gprtSetSAPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("SA Priority Override API test "));
	testDisplayStatus(status);

	/*
	 *  DA Priority Override
	 */
	testAPI.getFunc.port_bool = gprtGetDAPriOverride;
	testAPI.setFunc.port_bool = gprtSetDAPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DA Priority Override API test "));
	testDisplayStatus(status);

	/*
	 *  CPU Port Setup
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetCPUPort;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetCPUPort;
	if((status = testPortU32Type(dev,&testAPI,dev->numOfPorts)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("CPU Port Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Locked Port Setup
	 */
	testAPI.getFunc.port_bool = gprtGetLockedPort;
	testAPI.setFunc.port_bool = gprtSetLockedPort;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Locked Port Setup API test "));
	testDisplayStatus(status);

	/*
	 *  Ignore Wrong Data Setup
	 */
	testAPI.getFunc.port_bool = gprtGetIgnoreWrongData;
	testAPI.setFunc.port_bool = gprtSetIgnoreWrongData;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Ignore Wrong Data Setup API test "));
	testDisplayStatus(status);

	/*
	 *  UseCoreTag Setup
	 */
	testAPI.getFunc.port_bool = gprtGetUseCoreTag;
	testAPI.setFunc.port_bool = gprtSetUseCoreTag;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("UseCoreTag Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DiscardTagged Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDiscardTagged;
	testAPI.setFunc.port_bool = gprtSetDiscardTagged;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DiscardTagged Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DiscardUntagged Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDiscardUntagged;
	testAPI.setFunc.port_bool = gprtSetDiscardUntagged;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DiscardUntagged Setup API test "));
	testDisplayStatus(status);

	/*
	 *  MapDA Setup
	 */
	testAPI.getFunc.port_bool = gprtGetMapDA;
	testAPI.setFunc.port_bool = gprtSetMapDA;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("MapDA Setup API test "));
	testDisplayStatus(status);

	/*
	 *  DefaultForward Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDefaultForward;
	testAPI.setFunc.port_bool = gprtSetDefaultForward;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DefaultForward Setup API test "));
	testDisplayStatus(status);

	/*
	 *  EgressMonitorSource Setup
	 */
	testAPI.getFunc.port_bool = gprtGetEgressMonitorSource;
	testAPI.setFunc.port_bool = gprtSetEgressMonitorSource;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("EgressMonitorSource Setup API test "));
	testDisplayStatus(status);

	/*
	 *  IngressMonitorSource Setup
	 */
	testAPI.getFunc.port_bool = gprtGetIngressMonitorSource;
	testAPI.setFunc.port_bool = gprtSetIngressMonitorSource;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("IngressMonitorSource Setup API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
			return testResults;
		default:
			break;
	}			
	
	/*
	 *  MessagePort Setup
	 */
	testAPI.getFunc.port_bool = gprtGetMessagePort;
	testAPI.setFunc.port_bool = gprtSetMessagePort;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("MessagePort Setup API test "));
	testDisplayStatus(status);

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		if((status = testTrunkPortSetup(dev,portIndex)) != GT_OK)
		{
			testResult = GT_FAIL;
			testResults |= 1 << status;
			portIndex = 0xFF;
			break;		
		}
	}
	MSG_PRINT(("TrunkPort Setup API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6092:
		case GT_88E6095:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
			if (dev->revision < 1)
				return testResults;
			break;
		case GT_88E6131:
		case GT_88E6108:
			break;
		default:
			return testResults;
	}			
	
	/*
	 *  Discard Broadcast Mode Setup
	 */
	testAPI.getFunc.port_bool = gprtGetDiscardBCastMode;
	testAPI.setFunc.port_bool = gprtSetDiscardBCastMode;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Discard BCast Setup API test "));
	testDisplayStatus(status);

	/*
	 *  FC On RateLimit Mode Setup
	 */
	testAPI.getFunc.port_bool = gprtGetFCOnRateLimitMode;
	testAPI.setFunc.port_bool = gprtSetFCOnRateLimitMode;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("FC On RateLimit Setup API test "));
	testDisplayStatus(status);

	return testResults;
}

GT_U32 testPort(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	testResult = GT_OK;

	/*
	 *  Force FlowControl
	 */
	testAPI.getFunc.port_bool = gprtGetForceFc;
	testAPI.setFunc.port_bool = gprtSetForceFc;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Force FlowControl API test "));
	testDisplayStatus(status);

	/*
	 *  Trailer Mode
	 */
	testAPI.getFunc.port_bool = gprtGetTrailerMode;
	testAPI.setFunc.port_bool = gprtSetTrailerMode;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Trailer Mode API test "));
	testDisplayStatus(status);

	/*
	 *  Ingress Mode
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetIngressMode;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetIngressMode;
	if((status = testPortU32Type(dev,&testAPI,3)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Ingress Mode API test "));
	testDisplayStatus(status);


	/*
	 *  Muticast Rate Limit
	 *  This feature is only avalable on 6021/6051/6052
	 *  It is replace with Rate Control Register in Clippership and beyond
	 */
	if( (dev->deviceId == GT_88E6021) ||
	    (dev->deviceId == GT_88E6051) ||
	    (dev->deviceId == GT_88E6052) 
	  ){

	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetMcRateLimit;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetMcRateLimit;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Multicast Rate Limit API test "));
	testDisplayStatus(status);
	};

	/*
	 *  IGMP Snoop
	 */
	testAPI.getFunc.port_bool = gprtGetIGMPSnoop;
	testAPI.setFunc.port_bool = gprtSetIGMPSnoop;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("IGMP Snoop API test "));
	testDisplayStatus(status);

	/*
	 *  Header Mode
	 */
	testAPI.getFunc.port_bool = gprtGetHeaderMode;
	testAPI.setFunc.port_bool = gprtSetHeaderMode;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Header Mode API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
			testResults |= testPortG(dev);
			break;
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
			testResults |= testPortEnhancedFE(dev);
			break;
		default:
			break;
	}			
	return testResults;
}

GT_U32 testPortRCforEnhancedFE(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	testResult = GT_OK;

	/*
	 *  VID NRL En
	 */
	testAPI.getFunc.port_bool = grcGetVidNrlEn;
	testAPI.setFunc.port_bool = grcSetVidNrlEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("VID NRL En API test "));
	testDisplayStatus(status);

	/*
	 *  SA NRL En
	 */
	testAPI.getFunc.port_bool = grcGetSaNrlEn;
	testAPI.setFunc.port_bool = grcSetSaNrlEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("SA NRL En API test "));
	testDisplayStatus(status);

	/*
	 *  DA NRL En
	 */
	testAPI.getFunc.port_bool = grcGetDaNrlEn;
	testAPI.setFunc.port_bool = grcSetDaNrlEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DA NRL En API test "));
	testDisplayStatus(status);


	/*
	 *  Egress Limit Mode
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)grcGetELimitMode;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)grcSetELimitMode;
	if((status = testPortU32Type(dev,&testAPI,3)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Egress Limit Mode API test "));
	testDisplayStatus(status);

	/*
	 *  Rsvd NRL En
	 */
	testAPI.getFunc.bool = grcGetRsvdNrlEn;
	testAPI.setFunc.bool = grcSetRsvdNrlEn;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Rsvd NRL En API test "));
	testDisplayStatus(status);

	return testResults;
}

GT_U32 testPortRateCtrl(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, tmpResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;
	GT_LPORT port;
	int portIndex;
	GT_BURST_RATE bLimit, rbLimit, obLimit;
	GT_BURST_SIZE bSize, rbSize, obSize;
	GT_U32 RateCtrl[5];

	testResult = GT_OK;

	/*
	 *  Ingress Rate Limit Mode
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)grcGetLimitMode;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)grcSetLimitMode;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Ingress Rate Limit Mode API test "));
	testDisplayStatus(status);


	/*
	 *  Priority 3 Frames Rate Limit
	 */
	testAPI.getFunc.port_bool = grcGetPri3Rate;
	testAPI.setFunc.port_bool = grcSetPri3Rate;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Priority 3 Frames Rate Limit API test "));
	testDisplayStatus(status);

	/*
	 *  Priority 2 Frames Rate Limit
	 */
	testAPI.getFunc.port_bool = grcGetPri2Rate;
	testAPI.setFunc.port_bool = grcSetPri2Rate;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Priority 2 Frames Rate Limit API test "));
	testDisplayStatus(status);

	/*
	 *  Priority 1 Frames Rate Limit
	 */
	testAPI.getFunc.port_bool = grcGetPri1Rate;
	testAPI.setFunc.port_bool = grcSetPri1Rate;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Priority 1 Frames Rate Limit API test "));
	testDisplayStatus(status);

	/*
	 *  Priority 0 Frames Rate Limit
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)grcGetPri0Rate;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)grcSetPri0Rate;
	if((status = testPortU32Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Priority 0 Frames Rate Limit API test "));
	testDisplayStatus(status);

	/*
	 *  Included Bytes in Rate Control API
	 */
	tmpResult = GT_OK;
	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		GT_BOOL limitMGMT, countIFG, countPre;
		GT_BOOL orgLimitMGMT, orgCountIFG, orgCountPre;
		GT_BOOL tmpLimitMGMT, tmpCountIFG, tmpCountPre;

		port = portIndex;
		
		if((status = grcGetBytesCount(dev,port,&orgLimitMGMT,&orgCountIFG,&orgCountPre)) != GT_OK)
		{
			MSG_PRINT(("grcSetBytesCount returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			tmpResult = status;
			break;
		}

		limitMGMT = GT_TRUE;
		countIFG = GT_FALSE;
		countPre = GT_TRUE;

		if((status = grcSetBytesCount(dev,port,limitMGMT,countIFG,countPre)) != GT_OK)
		{
			MSG_PRINT(("grcSetBytesCount returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = grcGetBytesCount(dev,port,&tmpLimitMGMT,&tmpCountIFG,&tmpCountPre))
			!= GT_OK)
		{
			MSG_PRINT(("grcGetBytesCount returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if ((limitMGMT != tmpLimitMGMT) ||
			(countIFG != tmpCountIFG) ||
			(countPre != tmpCountPre))
		{
			MSG_PRINT(("grcGetBytesCount returned unexpected value(s)\n"));
			MSG_PRINT(("Expecting: limitMgmg %i, countIFG %i, countPre %i\n",
						limitMGMT,countIFG,countPre));
			MSG_PRINT(("Returned : limitMgmg %i, countIFG %i, countPre %i\n",
						tmpLimitMGMT,tmpCountIFG,tmpCountPre));
			tmpResult = GT_FAIL;
			testResults |= 1 << tmpResult;
		}

		limitMGMT = GT_FALSE;
		countIFG = GT_TRUE;
		countPre = GT_FALSE;

		if((status = grcSetBytesCount(dev,port,limitMGMT,countIFG,countPre)) != GT_OK)
		{
			MSG_PRINT(("grcSetBytesCount returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = grcGetBytesCount(dev,port,&tmpLimitMGMT,&tmpCountIFG,&tmpCountPre))
			!= GT_OK)
		{
			MSG_PRINT(("grcGetBytesCount returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if ((limitMGMT != tmpLimitMGMT) ||
			(countIFG != tmpCountIFG) ||
			(countPre != tmpCountPre))
		{
			MSG_PRINT(("grcGetBytesCount returned unexpected value(s)\n"));
			MSG_PRINT(("Expecting: limitMgmg %i, countIFG %i, countPre %i\n",
						limitMGMT,countIFG,countPre));
			MSG_PRINT(("Returned : limitMgmg %i, countIFG %i, countPre %i\n",
						tmpLimitMGMT,tmpCountIFG,tmpCountPre));
			tmpResult = GT_FAIL;
			testResults |= 1 << tmpResult;
		}

		if((status = grcSetBytesCount(dev,port,orgLimitMGMT,orgCountIFG,orgCountPre))
			!= GT_OK)
		{
			MSG_PRINT(("grcSetBytesCount returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	}
	if(tmpResult != GT_OK)
	{
		MSG_PRINT(("Count Bytes API test Failed.\n"));
		testResult = tmpResult;
		testResults |= 1 << testResult;
	}
	else
	{
		MSG_PRINT(("Count Bytes API test Passed.\n"));
	}

	/*
	 *  Egress Rate Limit
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)grcGetEgressRate;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)grcSetEgressRate;
	if((status = testPortU32Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Egress Rate Limit API test "));
	testDisplayStatus(status);

	switch(dev->deviceId)
	{
		case GT_88E6092:
		case GT_88E6095:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
			if (dev->revision < 1)
				return testResults;
			break;
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
			testResults |= testPortRCforEnhancedFE(dev);
			break;
		case GT_88E6131:
		case GT_88E6108:
			break;
		default:
			return testResults;
	}			

	/*
	 *  Burst Size based Rate Limit API
	 */
	RateCtrl[0] = 0x7FFFF; /* No_Limit, 64k ~ 256M */
	RateCtrl[1] = 0x7FFFD; /* No_Limit, 128k ~ 256M */
	RateCtrl[2] = 0x7FFF9; /* No_Limit, 256k ~ 256M */
	RateCtrl[3] = 0x7FFE1; /* No_Limit, 512k ~ 256M */
	RateCtrl[4] = 0;
	if(grcGetBurstRate(dev,0,&rbSize,&rbLimit) == GT_NOT_SUPPORTED)
		testResult = GT_NOT_SUPPORTED;
	else
	{
		testResult = GT_OK;

		for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
		{
			if((status=grcGetBurstRate(dev,portIndex,&obSize,&obLimit)) != GT_OK)
			{
				MSG_PRINT(("Get Burst Rate returned wrong (bsize %i, rate %i)\n",obSize,obLimit));
				testResult = GT_FAIL;
				testResults |= 1 << status;
				break;
			}

			for(bSize=GT_BURST_SIZE_12K; bSize<=GT_BURST_SIZE_96K+1; bSize++)
			{
				for(bLimit=GT_BURST_NO_LIMIT; bLimit<=GT_BURST_256M+1; bLimit++)
				{
					if((status=grcSetBurstRate(dev,portIndex,bSize,bLimit)) != GT_OK)
					{
						if(RateCtrl[bSize] & (1<<bLimit))
						{
							MSG_PRINT(("Burst Rate Control returned wrong (bsize %i, rate %i)\n",bSize,bLimit));
							testResult = GT_FAIL;
							testResults |= 1 << GT_FAIL;
						}
					}
					else
					{
						if (!(RateCtrl[bSize] & (1<<bLimit)))
						{
							MSG_PRINT(("Burst Rate Control returned GT_OK (bsize %i, rate %i)\n",bSize,bLimit));
							testResult = GT_FAIL;
							testResults |= 1 << GT_FAIL;
							continue;
						}

						if((status=grcGetBurstRate(dev,portIndex,&rbSize,&rbLimit)) != GT_OK)
						{
							MSG_PRINT(("Get Burst Rate returned wrong (bsize %i, rate %i)\n",bSize,bLimit));
							testResult = GT_FAIL;
							testResults |= 1 << status;
						}

						if ((bSize != rbSize) || (bLimit != rbLimit))
						{
							MSG_PRINT(("Burst Rate returned value not consistant (bsize %i %i, rate %i %i)\n",
										bSize,rbSize,bLimit,rbLimit));
							testResult = GT_FAIL;
							testResults |= 1 << GT_FAIL;
						}
					}
				}
			}

			if((status=grcSetBurstRate(dev,portIndex,obSize,obLimit)) != GT_OK)
			{
				MSG_PRINT(("Set Burst Rate returned wrong (bsize %i, rate %i)\n",obSize,obLimit));
				testResult = GT_FAIL;
				testResults |= 1 << status;
				break;
			}
		}
	}

	MSG_PRINT(("Burst Rate Limit API test "));
	testDisplayStatus(testResult);

	/*
	 *  TCP/IP Burst Rate Limit API
	 */
	RateCtrl[0] = 0x7FF; /* No_Limit, 64k ~ 1500k */
	if(grcGetTCPBurstRate(dev,0,&rbLimit) == GT_NOT_SUPPORTED)
		testResult = GT_NOT_SUPPORTED;
	else
	{
		testResult = GT_OK;

		for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
		{
			if((status=grcGetTCPBurstRate(dev,portIndex,&obLimit)) != GT_OK)
			{
				MSG_PRINT(("Get TCP Burst Rate returned wrong (rate %i)\n",obLimit));
				testResult = GT_FAIL;
				testResults |= 1 << status;
				break;
			}

			for(bLimit=GT_BURST_NO_LIMIT; bLimit<=GT_BURST_256M; bLimit++)
			{
				if(grcSetTCPBurstRate(dev,portIndex,bLimit) != GT_OK)
				{
					if(RateCtrl[0] & (1<<bLimit))
					{
						MSG_PRINT(("TCP Burst Rate Control returned wrong (rate %i)\n",bLimit));
						testResult = GT_FAIL;
						testResults |= 1 << GT_FAIL;
					}
				}
				else
				{
					if((status=grcGetTCPBurstRate(dev,portIndex,&rbLimit)) != GT_OK)
					{
						MSG_PRINT(("Get TCP Burst Rate returned wrong (rate %i)\n",bLimit));
						testResult = GT_FAIL;
						testResults |= 1 << status;
					}

					if (bLimit != rbLimit)
					{
						MSG_PRINT(("TCP Burst Rate returned value not consistant (rate %i %i)\n",
									bLimit,rbLimit));
						testResult = GT_FAIL;
						testResults |= 1 << GT_FAIL;
					}
				}
			}		

			if((status=grcSetTCPBurstRate(dev,portIndex,obLimit)) != GT_OK)
			{
				MSG_PRINT(("Get TCP Burst Rate returned wrong (rate %i)\n",obLimit));
				testResult = GT_FAIL;
				testResults |= 1 << status;
				break;
			}

		}
	}
	MSG_PRINT(("TCP Burst Rate Limit API test "));
	testDisplayStatus(testResult);

	return testResults;
}

GT_U32 testPortPAV(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	testResult = GT_OK;

	/*
	 *  Port Association Vector API
	 */
	testAPI.getFunc.port_u16 = gpavGetPAV;
	testAPI.setFunc.port_u16 = gpavSetPAV;
	if((status = testPortU16Type(dev,&testAPI,(1<<dev->numOfPorts)-1)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Port Association Vector API test "));
	testDisplayStatus(status);

	/*
	 *  Ingress Monitor
	 */
	testAPI.getFunc.port_bool = gpavGetIngressMonitor;
	testAPI.setFunc.port_bool = gpavSetIngressMonitor;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Ingress Monitor API test "));
	testDisplayStatus(status);

	return testResults;
}


GT_U32 testATUSetup(GT_QD_DEV *dev )
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	/*
	 *  Learn2All Setup API
	 */
	testAPI.getFunc.bool = gfdbGetLearn2All;
	testAPI.setFunc.bool = gfdbSetLearn2All;
	if((status = testBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Learn2All Setup API test "));
	testDisplayStatus(status);

	return testResults;
}


GT_STATUS testDisplayATUList(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_ATU_ENTRY tmpMacEntry;
	GT_U32 dbNum,i;
	int entries;

	MSG_PRINT(("ATU List:\n"));

	switch(dev->deviceId)
	{
		case GT_88E6051:
		case GT_88E6052:
		case GT_FF_HG:
		case GT_FF_EG:
			dbNum = 1;
			break;
		case GT_88E6021:
		case GT_88E6061:
		case GT_88E6063:
		case GT_FH_VPN:
		case GT_88E6083:
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
			dbNum = 16;
			break;
		case GT_88E6065:
			dbNum = 64;
			break;
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
			dbNum = 256;
			break;
		default:
			dbNum = 1;
			break;
	}

	for(i=0; i<dbNum; i++)
	{
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		tmpMacEntry.DBNum = (GT_U8)i;
		entries = 0;
		MSG_PRINT(("DB %i :\n",i));
		while(1)
		{
			/* Get the sorted list of MAC Table. */
			if((status = gfdbGetAtuEntryNext(dev,&tmpMacEntry)) != GT_OK)
			{
				break;
			}
			entries++;
			printATUEntry(&tmpMacEntry);

		}
		MSG_PRINT(("DB %i : entry %i\n",i,entries));

	}
	return GT_OK;
}

GT_U16 testDisplayAtuDbNumList(GT_QD_DEV *dev,GT_U32 dbNum)
{
	GT_STATUS status;
	GT_ATU_ENTRY tmpMacEntry;
	GT_U16 entries = 0;

	MSG_PRINT(("ATU List for DBNum %i:\n", dbNum));

	memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
	tmpMacEntry.DBNum = (GT_U8)dbNum;

	while(1)
	{
		/* Get the sorted list of MAC Table. */
		if((status = gfdbGetAtuEntryNext(dev,&tmpMacEntry)) != GT_OK)
		{
			break;
		}

		entries++;
		printATUEntry(&tmpMacEntry);
	}
	return entries;
}

/*******************************************************************************
* testATUDBNum
*
* DESCRIPTION:
*       Testing ATU related APIs.
*		(SetAtuSize, SetAgingTimeout, GetAgingTimeRange, LearnEnable, AddEntry,
*		DeleteEntry, GetFirst, GetNext, FindEntry, Flush, GetDynamicCount)
*		
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on fail
*
* COMMENTS:
*		1. Setup ATU Size.
*		2. Disable AGING.
*		3. Disable Learning.
*		4. Flush all the ATU Entries.
*		5. Create ATU Entries (random, with arg).
*		6. Keep the Sorted list of the entries.
*		7. Write the entries into the device.
*		8. Check the Dynamic Counter.
*		9. Get the First Entry and Check if it's correct.
*		10.Delete the First entry.
*		11.Get the First Entry and check if it's correct.
*		12.Get the next entry and check if it's correct.
*		13.Find the middle entry and check if it's correct.
*		14.Find the last entry and check if it's correct.
*		15.Find the next entry of the last entry.(It should fail.)
*		16.Find the deleted entry.(It should fail.)
*		17.Repeat 9 ~ 16 for each DBNum
*		18.If the device supports MOVE, 
*			18.1 Move all the entries in DB (maxDbNum-1) to port 1 and verify
*			18.2 Move all the entries to port 0 and verify
*		19.Check Dynamic count.
*		20.Flush All in a DB
*
*******************************************************************************/
GT_STATUS testAtuDbNum(GT_QD_DEV *dev,int arg, GT_U32 entrySize, GT_U32 maxDbNum, GT_U8 sameMacs, GT_U8 atuSize)
{
	GT_STATUS status;
	GT_STATUS testResult = GT_OK;
	GT_U32 u32Data1, u32Data2, dbNum;
	TEST_ATU_ENTRY *macEntry;
	int *macList[256];
	GT_ATU_ENTRY tmpMacEntry;
	int i, j, dynamicMacs;
	GT_BOOL found;
	GT_BOOL sapphire, aging15, moveAllowed;
	GT_LPORT port, portDest;
	GT_STATUS secResult;
	volatile int timer;

	if(entrySize < 4)
		entrySize = 4;

	if(entrySize > TEST_MAC_ENTRIES)
		entrySize = TEST_MAC_ENTRIES;

	if (entrySize*maxDbNum > (256 << atuSize))
	{
		entrySize = (256 << atuSize) / maxDbNum;
	}

	macEntry = NULL;
	for(dbNum=0; dbNum < maxDbNum; dbNum++)
		macList[dbNum] = NULL;
	
	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
			sapphire = GT_TRUE;
			aging15 = GT_TRUE;
			moveAllowed = GT_TRUE;
			break;
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
			moveAllowed = GT_TRUE;
			sapphire = GT_FALSE;
			aging15 = GT_TRUE;
			break;
		default:
			moveAllowed = GT_FALSE;
			sapphire = GT_FALSE;
			aging15 = GT_FALSE;
			break;
	}			
	/* Set ATU Size will cause ATU reset and SW reset, so call before any other setup. */
	MSG_PRINT(("Setting ATU Size : %i\n",256<<atuSize));
	if((status = gfdbSetAtuSize(dev,atuSize)) != GT_OK)
	{
		if(sapphire != GT_TRUE)
		{
			MSG_PRINT(("gfdbSetAtuSize returned "));
			testDisplayStatus(status);
			goto errorExit;
		}
	}

	/* Aging Time Range is between 16 and 4080 */
	MSG_PRINT(("Getting Aging Time Range... \n"));
	if((status = gfdbGetAgingTimeRange(dev,&u32Data1, &u32Data2)) != GT_OK)
	{
		MSG_PRINT(("gfdbAgingTimeRange returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	if((u32Data2 != 16) || (u32Data1 != 4080))
	{
		if(aging15 != GT_TRUE)
		{
			MSG_PRINT(("AgingTimeRange is between %d and %d.\n",u32Data1,u32Data2));
			testResult = GT_FAIL;
		}
	}

	/* Disable Aging */
	MSG_PRINT(("Disable Aging Timeout... \n"));
	if((status = gfdbSetAgingTimeout(dev,0)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAgingTimeout returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	/* Disable Learning */
	MSG_PRINT(("Disable Learning... \n"));
	if((status = gfdbLearnEnable(dev,GT_FALSE)) != GT_OK)
	{
		MSG_PRINT(("gfdbLearnEnable returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	/* Flush all addresses from the ATU table. */
	MSG_PRINT(("Flush out all the entries in the ATU Table ... \n"));
	if((status = gfdbFlush(dev,GT_FLUSH_ALL)) != GT_OK)
	{
		MSG_PRINT(("gfdbFlush returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	/* Get Atu Dynamic Count, which should be 0, since we flush them all. */
	if((status = gfdbGetAtuDynamicCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	MSG_PRINT(("Atu Dynamic Count : %d.\n", u32Data1));

	if(u32Data1)
	{
		testResult = GT_FAIL;
	}
	
	/* Now ATU table is clean. Play with our own MAC entries */	
	MSG_PRINT(("Setup Testing Table... \n"));

	macEntry = (TEST_ATU_ENTRY*)malloc(maxDbNum*sizeof(TEST_ATU_ENTRY));
	if(macEntry == NULL)
	{
		MSG_PRINT(("Failed to allocate MAC Entries. \n"));
		goto errorExit;
	}

	memset(macEntry,0,sizeof(macEntry));

	for(dbNum=0; dbNum < maxDbNum; dbNum++)
	{
		macList[dbNum] = (int*)malloc(TEST_MAC_ENTRIES*sizeof(int));
		if(macList[dbNum] == NULL)
		{
			MSG_PRINT(("Failed to allocate MAC Entries. \n"));
			goto errorExit;
		}
	}

	MSG_PRINT(("Creating ATU List... \n"));
	dynamicMacs = createATUList(dev,macEntry,entrySize,(GT_U16)maxDbNum,sameMacs,64 << atuSize);

	if (dynamicMacs > entrySize*maxDbNum)
	{
		MSG_PRINT(("Cannot create ATU List for testing... \n"));
		goto errorExit;
	}

	for(i=0; i<entrySize; i++)
	{
		for(dbNum=0; dbNum < maxDbNum; dbNum++)
		{
			macList[dbNum][i] = i;
		}
	}	

	/* Get Sorted List for the arg, 0 or 1. */

	MSG_PRINT(("Sorting the created ATU List... \n"));
	for(dbNum=0; dbNum < maxDbNum; dbNum++)
		gtSort(macList[dbNum], atuEntryCmpFunc, (void*)macEntry[dbNum].atuEntry, entrySize);

	MSG_PRINT(("Writing ATU Entries... \n"));
	for (dbNum=0; dbNum<maxDbNum; dbNum++)
	{

		for(i=0; i<entrySize; i++)
		{
			switch (arg)
			{
				case 0: /* ascending order */
					j = macList[dbNum][i];
					break;
				case 1: /* descending order */
					j = macList[dbNum][entrySize - 1 - i];
					break;
				default:
					j = i;
					break;
			}

			if((status = gfdbAddMacEntry(dev,&macEntry[dbNum].atuEntry[j])) != GT_OK)
			{
				MSG_PRINT(("gfdbAddMacEntry returned "));
				testDisplayStatus(status);
				dumpMemory((char*)&macEntry[dbNum].atuEntry[j], sizeof(GT_ATU_ENTRY));
				MSG_PRINT(("dbNum %i, entry %i\n",dbNum,j));
				goto errorExit;
			}

#ifdef TEST_DEBUG
			printATUEntry(&macEntry[dbNum].atuEntry[j]);
			
			memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
			tmpMacEntry.DBNum = (GT_U8)dbNum;

			if((status = gfdbGetAtuEntryFirst(dev,&tmpMacEntry)) != GT_OK)
			{
				MSG_PRINT(("gfdbGetAtuEntryFirst returned "));
				testDisplayStatus(status);
				MSG_PRINT(("Expected entry:"));
				dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][0]], sizeof(GT_ATU_ENTRY));
				testDisplayATUList(dev);
				goto errorExit;
			}

			if(memcmp(&tmpMacEntry, &macEntry[dbNum].atuEntry[macList[dbNum][0]], sizeof(GT_ATU_ENTRY)))
			{
				MSG_PRINT(("gfdbGetAtuEntryFirst returned wrong entry."));
				dumpMemory((char*)&tmpMacEntry, sizeof(GT_ATU_ENTRY));
				MSG_PRINT(("Expected entry:"));
				dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][0]], sizeof(GT_ATU_ENTRY));
		
				testDisplayATUList(dev);
				goto errorExit;
			}
#endif
		}	
	}

	timer = gAgeDelayTime;
	while(timer>0)
		timer--;

#ifdef TEST_DEBUG
	testDisplayATUList(dev);
#endif
	/* 
		Now we have entrySize*16 entries in the table. 
	*/

	/* Get Atu Dynamic Count, which should be dynamicMacs. */
	if((status = gfdbGetAtuDynamicCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	MSG_PRINT(("Dynamic Macs in the table : %d\n", dynamicMacs));
	if (u32Data1 != dynamicMacs)
	{
		MSG_PRINT(("Atu Dynamic Count returned %d.(should be %d)\n", u32Data1,dynamicMacs));
		testResult = GT_FAIL;
	}

	/* Get First Entry in the Table, which should be macEntry[0]. */

	for (dbNum=0; dbNum<maxDbNum; dbNum++)
	{
		MSG_PRINT(("Running ATU Test for DBNum %i\n", dbNum));

		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		tmpMacEntry.DBNum = (GT_U8)dbNum;

		if((status = gfdbGetAtuEntryFirst(dev,&tmpMacEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbGetAtuEntryFirst returned "));
			testDisplayStatus(status);
			goto errorExit;
		}

		if(memcmp(&tmpMacEntry, &macEntry[dbNum].atuEntry[macList[dbNum][0]], sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbGetAtuEntryFirst returned wrong entry."));
			dumpMemory((char*)&tmpMacEntry, sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expected entry:"));
			dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][0]], sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Getting First Entry is passed. \n"));

		/* Delete the first entry for each DBNum */
		MSG_PRINT(("Delete the First Entry\n"));

		if((status = gfdbDelAtuEntry(dev,&macEntry[dbNum].atuEntry[macList[dbNum][0]])) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned "));
			testDisplayStatus(status);
			goto errorExit;
		}

		if(!(macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[0] & 0x1) && 
			(macEntry[dbNum].atuEntry[macList[dbNum][0]].entryState.ucEntryState == GT_UC_DYNAMIC))
				dynamicMacs--;

		/* Get First Entry in the Table, which should be macEntry[1]. */

		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		tmpMacEntry.DBNum = (GT_U8)dbNum;

		if((status = gfdbGetAtuEntryFirst(dev,&tmpMacEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbGetAtuEntryFirst returned "));
			testDisplayStatus(status);
			goto errorExit;
		}

		if(memcmp(&tmpMacEntry, &macEntry[dbNum].atuEntry[macList[dbNum][1]], sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbGetAtuEntryFirst returned wrong entry:"));
			dumpMemory((char*)&tmpMacEntry, sizeof(GT_ATU_ENTRY));
		
			MSG_PRINT(("Expecting entry."));
			dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][1]], sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Getting First Entry is passed. \n"));

		if((status = gfdbGetAtuEntryNext(dev,&tmpMacEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbGetAtuEntryNext returned "));
			testDisplayStatus(status);
			goto errorExit;
		}

		if(memcmp(&tmpMacEntry, &macEntry[dbNum].atuEntry[macList[dbNum][2]], sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbGetAtuEntryNext returned wrong entry."));
			dumpMemory((char*)&tmpMacEntry, sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expected entry:"));
			dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][2]], sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Getting Next Entry is passed. \n"));

		/* Find Mac Entry (use macEntry[TEST_MAC_ENTRIES/2]) */
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		i = entrySize/2;
		tmpMacEntry.macAddr.arEther[0] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[5];

		tmpMacEntry.DBNum = (GT_U8)dbNum;

		if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned "));
			testDisplayStatus(status);
			goto errorExit;
		}

		if (found == GT_FALSE)
		{
			MSG_PRINT(("Cannot find the middle entry.\n"));
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Successfully Found the middle Mac Entry (PASS). \n"));

		if(memcmp(&tmpMacEntry, &macEntry[dbNum].atuEntry[macList[dbNum][i]], sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned wrong entry."));
			dumpMemory((char*)&tmpMacEntry, sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expected entry:"));
			dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][i]], sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Found Entry is valid (PASS). \n"));

		/* Find Mac Entry (use macEntry[TEST_MAC_ENTRIES-1]) */
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		i = entrySize-1;
		tmpMacEntry.macAddr.arEther[0] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[5];

		tmpMacEntry.DBNum = (GT_U8)dbNum;

		if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned "));
			testDisplayStatus(status);
			goto errorExit;
		}

		if (found == GT_FALSE)
		{
			MSG_PRINT(("Cannot find the last entry.\n"));
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Successfully Found the last Mac Entry (PASS). \n"));

		if(memcmp(&tmpMacEntry, &macEntry[dbNum].atuEntry[macList[dbNum][i]], sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned wrong entry."));
			dumpMemory((char*)&tmpMacEntry, sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expected entry:"));
			dumpMemory((char*)&macEntry[dbNum].atuEntry[macList[dbNum][i]], sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Found Entry is valid (PASS). \n"));

		/* Error Checking */

		/* Now tmpMacEntry is pointing to the last entry. So, GetNext should return error */
		i = entrySize-1;
		tmpMacEntry.macAddr.arEther[0] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[5];

		tmpMacEntry.DBNum = (GT_U8)dbNum;

		if((status = gfdbGetAtuEntryNext(dev,&tmpMacEntry)) == GT_OK)
		{
			MSG_PRINT(("gfdbGetAtuEntryNext should return fail.\n"));
			printATUEntry(&tmpMacEntry);
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("Getting Next Entry from the last entry returned %i (PASS).\n", status));

		/* macEntry[0] has been deleted. So, finding the entry should return not found */

		if((status = gfdbFindAtuMacEntry(dev,&macEntry[dbNum].atuEntry[macList[dbNum][0]],&found)) == GT_OK)
		{
			if (found == GT_TRUE)
			{
				MSG_PRINT(("gfdbFindAtuMacEntry should not be found.(%#x-%#x-%#x-%#x-%#x-%#x)\n",
						macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[0],
						macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[1],
						macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[2],
						macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[3],
						macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[4],
						macEntry[dbNum].atuEntry[macList[dbNum][0]].macAddr.arEther[5]));
				testResult = GT_FAIL;
			}
			else
				MSG_PRINT(("Finding invalid entry returned not found (PASS).\n"));

		}
		else
		{
			MSG_PRINT(("Finding invalid entry returned not OK (PASS).\n"));
		}
	}
	/* If the device supports MOVE, 
	 *		Move all the entries in DB 0 to port 1 and verify
	 *		Move all the entries to port 0 and verify
	*/

	if (moveAllowed)
	{
		secResult = GT_OK;
		
		/* move all the entries in DB (maxDbNum-1) to port 1 */
		portDest = 1;
		dbNum = maxDbNum-1;

		MSG_PRINT(("Moving entries to Port 1... (in DB %i) \n", dbNum));
		
		for(port=0; port<dev->numOfPorts; port++)
		{
			if(port == portDest)
				continue;

			if((status = gfdbMoveInDB(dev,GT_MOVE_ALL,(GT_U8)dbNum,port,portDest)) != GT_OK)
			{
				MSG_PRINT(("gfdbMove returned "));
				testDisplayStatus(status);
				if (status == GT_NOT_SUPPORTED)
					break;
				goto errorExit;
			}
		}

		/* verify Move, First Entry has been deleted from previous test */
		for(i=1; i<entrySize; i++)
		{
			if (status == GT_NOT_SUPPORTED)
				break;

			memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
			tmpMacEntry.macAddr.arEther[0] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[0];
			tmpMacEntry.macAddr.arEther[1] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[1];
			tmpMacEntry.macAddr.arEther[2] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[2];
			tmpMacEntry.macAddr.arEther[3] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[3];
			tmpMacEntry.macAddr.arEther[4] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[4];
			tmpMacEntry.macAddr.arEther[5] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[5];

			tmpMacEntry.DBNum = (GT_U8)dbNum;

			if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
			{
				MSG_PRINT(("gfdbFindAtuMacEntry returned "));
				testDisplayStatus(status);
				goto errorExit;
			}
			
			if (found == GT_FALSE)
			{
				MSG_PRINT(("Cannot find the last entry.\n"));
				secResult = GT_FAIL;
				testResult = GT_FAIL;
			}
			/* verify if the port is portDest */
			if ((tmpMacEntry.portVec & (1 << portDest)) == 0)
			{
				MSG_PRINT(("Move to port %i failed (portVec %#x, dbnum %i, orgVec %#x).\n",
							portDest,tmpMacEntry.portVec,dbNum,
							macEntry[dbNum].atuEntry[macList[dbNum][i]].portVec));
				MSG_PRINT(("Entry : \n"));
				printATUEntry(&tmpMacEntry);
				secResult = GT_FAIL;
				testResult = GT_FAIL;
			}
		}

		if(secResult == GT_FAIL)
		{
			MSG_PRINT(("Moving entry Failed.\n"));
		}
		else
		{
			MSG_PRINT(("Moving entry Passed.\n"));                          
		}
		
		/* move all the entries to port 0 */
		portDest = 0;
		secResult = GT_OK;
		MSG_PRINT(("Moving entries to Port 0...\n"));                          
		for(port=0; port<dev->numOfPorts; port++)
		{
			if(port == portDest)
				continue;

			if((status = gfdbMove(dev,GT_MOVE_ALL,port,portDest)) != GT_OK)
			{
				MSG_PRINT(("gfdbMove returned "));
				testDisplayStatus(status);
				if (status == GT_NOT_SUPPORTED)
					break;
				goto errorExit;
			}
		}

		/* verify Move. First Entry has been deleted from previous test */
		for(i=1; i<entrySize; i++)
		{
			if (status == GT_NOT_SUPPORTED)
				break;

			for (dbNum=0; dbNum<maxDbNum; dbNum++)
			{
				memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
				tmpMacEntry.macAddr.arEther[0] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[0];
				tmpMacEntry.macAddr.arEther[1] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[1];
				tmpMacEntry.macAddr.arEther[2] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[2];
				tmpMacEntry.macAddr.arEther[3] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[3];
				tmpMacEntry.macAddr.arEther[4] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[4];
				tmpMacEntry.macAddr.arEther[5] = macEntry[dbNum].atuEntry[macList[dbNum][i]].macAddr.arEther[5];

				tmpMacEntry.DBNum = (GT_U8)dbNum;

				if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
				{
					MSG_PRINT(("gfdbFindAtuMacEntry returned "));
					testDisplayStatus(status);
					goto errorExit;
				}
			
				if (found == GT_FALSE)
				{
					MSG_PRINT(("Cannot find the last entry.\n"));
					secResult = GT_FAIL;
					testResult = GT_FAIL;
				}
				/* verify if the port is portDest */
				if ((tmpMacEntry.portVec & (1 << portDest)) == 0)
				{
					MSG_PRINT(("Move to port %i failed (portVec %#x, dbnum %i, orgVec %#x).\n",
								portDest,tmpMacEntry.portVec,dbNum,
								macEntry[dbNum].atuEntry[macList[dbNum][i]].portVec));
					MSG_PRINT(("Entry :\n"));
					printATUEntry(&tmpMacEntry);
					secResult = GT_FAIL;
					testResult = GT_FAIL;
				}
			}
		}

		if(secResult == GT_FAIL)
		{
			MSG_PRINT(("Moving entry Failed.\n"));
		}
		else
		{
			MSG_PRINT(("Moving entry Passed.\n"));                          
		}

	}


	MSG_PRINT(("Checking Dynamic Count... \n"));
	/* Get Atu Dynamic Count, which should be dynamicMacs. */
	if((status = gfdbGetAtuDynamicCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	if (u32Data1 != dynamicMacs)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned %d (should be %d).\n",u32Data1,dynamicMacs));
		testResult = GT_FAIL;
	}
	else
		MSG_PRINT(("Dynamic Entries : %d (PASS).\n",u32Data1));

	/* Flush all non-static addresses from the ATU table. */
	MSG_PRINT(("Flush out all the Dynamic Entries...\n"));
	for (dbNum=0; dbNum<maxDbNum; dbNum++)
	{
		if((status = gfdbFlushInDB(dev,GT_FLUSH_ALL_UNBLK,dbNum)) != GT_OK)
		{
			MSG_PRINT(("gfdbFlushInDB returned "));
			testDisplayStatus(status);
			if (status == GT_NOT_SUPPORTED)
				break;
			goto errorExit;
		}
	}

	if (dbNum != maxDbNum)
	{
		MSG_PRINT(("Call gfdbFlush \n"));
		if((status = gfdbFlush(dev,GT_FLUSH_ALL_UNBLK)) != GT_OK)
		{
			MSG_PRINT(("gfdbFlush returned "));
			testDisplayStatus(status);
			goto errorExit;
		}
	}

	/* Get Atu Dynamic Count, which should be 0. */
	if((status = gfdbGetAtuDynamicCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned "));
		testDisplayStatus(status);
		goto errorExit;
	}

	if (u32Data1)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned %d (should be 0).\n",u32Data1));
		testResult = GT_FAIL;
	}
	else
		MSG_PRINT(("Dynamic Entries : %d (PASS).\n",u32Data1));

	/* Flush all addresses from the ATU table. */
	MSG_PRINT(("Flush out all the Entries...\n"));
	for (dbNum=0; dbNum<maxDbNum; dbNum++)
	{
		if((status = gfdbFlushInDB(dev,GT_FLUSH_ALL,dbNum)) != GT_OK)
		{
			MSG_PRINT(("gfdbFlushInDB returned "));
			testDisplayStatus(status);
			if (status == GT_NOT_SUPPORTED)
				break;
			goto errorExit;
		}
	}

	if (dbNum != maxDbNum)
	{
		MSG_PRINT(("Call gfdbFlush \n"));
		if((status = gfdbFlush(dev,GT_FLUSH_ALL_UNBLK)) != GT_OK)
		{
			MSG_PRINT(("gfdbFlush returned "));
			testDisplayStatus(status);
			goto errorExit;
		}
	}

	return testResult;

errorExit:

	for(dbNum=0; dbNum < maxDbNum; dbNum++)
		if(macList[dbNum])
			free(macList[dbNum]);
	if(macEntry)
		free(macEntry);
	return status;
}

GT_STATUS testFillUpAtu(GT_QD_DEV *dev,ATU_ENTRY_INFO *atuEntry, GT_U8 atuSize, GT_U32 dbNum, GT_U16 first2Bytes, GT_ATU_UC_STATE state)
{
	char buckets[MAX_BUCKET_SIZE];
	GT_U16 binSize,bSize;
	GT_U16 hash, bucket, tmpBucket, preBucket;
	GT_U32 maxMacs,entry,i,addr;
	char eaddr[6];

	if(atuSize >= 5)
		return GT_BAD_PARAM;

	maxMacs = 256 << atuSize;
	bSize = 64 << atuSize;
	binSize = 4;

	gtMemSet(buckets,0,MAX_BUCKET_SIZE);

	i = entry = 0;

	while(1)
	{
		if (i == 0xFFFFFFFF)
		{
			MSG_PRINT(("32bit is not enough.\n"));
			return GT_FAIL;
		}

		i++;

		if ((i & 0xFFFFFF) == 0)
		{
			MSG_PRINT(("loop %#x : entry %#x\n", i,entry));
			dumpMemory(buckets,bSize);
		}

		*(GT_U16*)eaddr = first2Bytes;
		eaddr[2] = (i >> 24) & 0xff;
		eaddr[3] = (i >> 16) & 0xff;
		eaddr[4] = (i >> 8) & 0xff;
		eaddr[5] = i & 0xff;
		bucket = runQDHash(eaddr, dbNum, bSize, &hash, &preBucket, &tmpBucket);
		if(buckets[bucket] == binSize)
			continue;
		addr = bucket*binSize + buckets[bucket];
		buckets[bucket]++;
		memcpy(atuEntry[addr].atuEntry.macAddr.arEther,eaddr,6);
		atuEntry[addr].atuEntry.entryState.ucEntryState = state;
		atuEntry[addr].atuEntry.portVec = 1;
		atuEntry[addr].atuEntry.prio = 0;
		atuEntry[addr].atuEntry.DBNum = (GT_U8)dbNum;
		atuEntry[addr].hash = hash;
		atuEntry[addr].bucket = bucket;
#if 0
		MSG_PRINT(("EADDR : %02x-%02x-%02x-%02x, ", eaddr[2],eaddr[3],eaddr[4],eaddr[5]));
		MSG_PRINT(("Hash : %03x, ", hash));
		MSG_PRINT(("bucket : %03x, ", preBucket));
		MSG_PRINT(("bucket(db) : %03x, ", bucket));
		MSG_PRINT(("bins : %02x\n", buckets[bucket]-1));
#endif
		entry++;
		if (entry >= maxMacs)
		{
			MSG_PRINT(("loop %#x\n", i));
			break;
		}
		
	}

	return GT_OK;
}

/*
	1. Set ATU Size.
	2. Disable AGING.
	3. Disable Learning.
	4. Flush all the ATU entries.
	5. Check Dynamic Counts.
	6. Create Filled ATU Table in system memory with EntryState = 0x7.
	7. Write the table into the device.
	8. Compare ATU Entries.
	9. Create Filled ATU Table in system memory with EntryState = 0x7.
	10.Write the table into the device.
	11.Compare ATU Entries. (Only First Entry in each bucket got replaced.)
	12.Create Filled ATU Table in system memory with EntryState = 0xF.
	13.Write the table into the device.
	14.Compare ATU Entries.
	15.Write the table which was created in step 9.
	16.Make it sure that no entry is written.
*/
GT_STATUS testFilledATU(GT_QD_DEV *dev,GT_U8 atuSize, GT_U32 dbNum)
{
	GT_STATUS testResult, status;
	ATU_ENTRY_INFO *atuEntry;
	ATU_ENTRY_INFO *tmpAtuEntry;
	ATU_ENTRY_INFO *tmpSingleAtuEntry;
	GT_ATU_ENTRY tmpMacEntry;
	GT_U16 maxMacs, i;
	GT_BOOL found;
	GT_U32 u32Data;

	if(atuSize > 5)
		return GT_FAIL;

	testResult = GT_OK;
	maxMacs = 256 << atuSize;

	if (gAtuEntry == NULL)
		gAtuEntry = (ATU_ENTRY_INFO *)malloc(sizeof(ATU_ENTRY_INFO)*4096);

	atuEntry = gAtuEntry;

	gtMemSet(atuEntry,0,sizeof(ATU_ENTRY_INFO)*maxMacs);

	MSG_PRINT(("Setting ATU Size : %i\n",256<<atuSize));
	if((status = gfdbSetAtuSize(dev,atuSize)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAtuSize returned "));
		testDisplayStatus(status);
		if (status != GT_NOT_SUPPORTED)
			return status;
	}

	MSG_PRINT(("Disable Aging Timeout... \n"));
	if((status = gfdbSetAgingTimeout(dev,0)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAgingTimeout returned "));
		testDisplayStatus(status);
		return status;
	}

	/* Disable Learning */
	MSG_PRINT(("Disable Learning... \n"));
	if((status = gfdbLearnEnable(dev,GT_FALSE)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAtuSize returned "));
		testDisplayStatus(status);
		return status;
	}

	/* Flush all addresses from the ATU table. */
	MSG_PRINT(("Flush out all the entries in the ATU Table ... \n"));
	if((status = gfdbFlush(dev,GT_FLUSH_ALL)) != GT_OK)
	{
		MSG_PRINT(("gfdbFlush returned "));
		testDisplayStatus(status);
		return status;
	}

	/* Get Atu Dynamic Count, which should be 0, since we flush them all. */
	if((status = gfdbGetAtuDynamicCount(dev,&u32Data)) != GT_OK)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned "));
		testDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Atu Dynamic Count : %d.\n", u32Data));

	/*
	 *	Entry State 0x7
	 */

	MSG_PRINT(("Getting ATU List(%i).\n",maxMacs));
	if((status=testFillUpAtu(dev,atuEntry,atuSize,dbNum,0,GT_UC_DYNAMIC)) != GT_OK)
	{
		MSG_PRINT(("testFillUpAtu returned "));
		testDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Writing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		if((status = gfdbAddMacEntry(dev,&atuEntry[i].atuEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned "));
			testDisplayStatus(status);
			return status;
		}
	}

	MSG_PRINT(("Comparing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		tmpMacEntry.macAddr.arEther[0] = atuEntry[i].atuEntry.macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = atuEntry[i].atuEntry.macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = atuEntry[i].atuEntry.macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = atuEntry[i].atuEntry.macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = atuEntry[i].atuEntry.macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = atuEntry[i].atuEntry.macAddr.arEther[5];

		tmpMacEntry.DBNum = atuEntry[i].atuEntry.DBNum;

		if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned "));
			testDisplayStatus(status);
			MSG_PRINT(("Entry to find : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));
			return status;
		}

		if (found == GT_FALSE)
		{
			MSG_PRINT(("Cannot find the Entry : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));

			testResult = GT_FAIL;
			return testResult;
		}

		if(memcmp(&tmpMacEntry, &atuEntry[i].atuEntry, sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned wrong entry.\n"));
			dumpMemory((char*)&tmpMacEntry,sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expecting:\n"));
			dumpMemory((char*)&atuEntry[i].atuEntry,sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
		{	
			if(((i & 0x3F) == 0) && (i != 0))
				MSG_PRINT(("Compared %i ATU Entries.\n",i));
		}
	}

	/*
	 *	Entry State 0x7
	 */

	if((tmpAtuEntry = (ATU_ENTRY_INFO *)malloc(sizeof(ATU_ENTRY_INFO)*maxMacs)) == NULL)
		return GT_FAIL;
	gtMemSet(tmpAtuEntry,0,sizeof(ATU_ENTRY_INFO)*maxMacs);

	MSG_PRINT(("Getting ATU List(%i).\n",maxMacs));
	if((status=testFillUpAtu(dev,tmpAtuEntry,atuSize,dbNum,0xA0A0,GT_UC_DYNAMIC)) != GT_OK)
	{
		MSG_PRINT(("testFillUpAtu returned "));
		testDisplayStatus(status);
		free(tmpAtuEntry);
		return status;
	}
	MSG_PRINT(("Writing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		if((status = gfdbAddMacEntry(dev,&tmpAtuEntry[i].atuEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned "));
			testDisplayStatus(status);
			free(tmpAtuEntry);
			return status;
		}
	}

	MSG_PRINT(("Comparing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		if ((i%4) == 0)
			tmpSingleAtuEntry = &tmpAtuEntry[i+3];
		else
			tmpSingleAtuEntry = &atuEntry[i];

		tmpMacEntry.macAddr.arEther[0] = tmpSingleAtuEntry->atuEntry.macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = tmpSingleAtuEntry->atuEntry.macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = tmpSingleAtuEntry->atuEntry.macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = tmpSingleAtuEntry->atuEntry.macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = tmpSingleAtuEntry->atuEntry.macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = tmpSingleAtuEntry->atuEntry.macAddr.arEther[5];
		tmpMacEntry.DBNum = tmpSingleAtuEntry->atuEntry.DBNum;

		if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned "));
			testDisplayStatus(status);
			MSG_PRINT(("Entry to find : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));
			free(tmpAtuEntry);
			return status;
		}

		if (found == GT_FALSE)
		{
			MSG_PRINT(("Cannot find the Entry : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));

			free(tmpAtuEntry);
			testResult = GT_FAIL;
			return testResult;
		}

		if(memcmp(&tmpMacEntry, &(tmpSingleAtuEntry->atuEntry), sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned wrong entry.\n"));
			dumpMemory((char*)&tmpMacEntry,sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expecting:\n"));
			dumpMemory((char*)&(tmpSingleAtuEntry->atuEntry),sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
		{	
			if(((i & 0x3F) == 0) && (i != 0))
				MSG_PRINT(("Compared %i ATU Entries.\n",i));
		}
	}

	/*
	 *	Entry State 0xF
	 */

	MSG_PRINT(("Getting ATU List(%i).\n",maxMacs));
	gtMemSet(atuEntry,0,sizeof(ATU_ENTRY_INFO)*maxMacs);
	if((status=testFillUpAtu(dev,atuEntry,atuSize,dbNum,0xAA00,GT_UC_STATIC)) != GT_OK)
	{
		MSG_PRINT(("testFillUpAtu returned "));
		testDisplayStatus(status);
		free(tmpAtuEntry);
		return status;
	}

	MSG_PRINT(("Writing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		if((status = gfdbAddMacEntry(dev,&atuEntry[i].atuEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned "));
			testDisplayStatus(status);
			free(tmpAtuEntry);
			return status;
		}
	}

	MSG_PRINT(("Comparing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		tmpMacEntry.macAddr.arEther[0] = atuEntry[i].atuEntry.macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = atuEntry[i].atuEntry.macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = atuEntry[i].atuEntry.macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = atuEntry[i].atuEntry.macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = atuEntry[i].atuEntry.macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = atuEntry[i].atuEntry.macAddr.arEther[5];

		tmpMacEntry.DBNum = atuEntry[i].atuEntry.DBNum;

		if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned "));
			testDisplayStatus(status);
			MSG_PRINT(("Entry to find : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));
			free(tmpAtuEntry);
			return status;
		}

		if (found == GT_FALSE)
		{
			MSG_PRINT(("Cannot find the Entry : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));

			testResult = GT_FAIL;
			free(tmpAtuEntry);
			return testResult;
		}

		if(memcmp(&tmpMacEntry, &atuEntry[i].atuEntry, sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned wrong entry.\n"));
			dumpMemory((char*)&tmpMacEntry,sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expecting:\n"));
			dumpMemory((char*)&atuEntry[i].atuEntry,sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
		{	
			if(((i & 0x3F) == 0) && (i != 0))
				MSG_PRINT(("Compared %i ATU Entries.\n",i));
		}
	}

	MSG_PRINT(("Writing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		if((status = gfdbAddMacEntry(dev,&tmpAtuEntry[i].atuEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned "));
			testDisplayStatus(status);
			free(tmpAtuEntry);
			return status;
		}
	}

	MSG_PRINT(("Comparing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));
		tmpMacEntry.macAddr.arEther[0] = atuEntry[i].atuEntry.macAddr.arEther[0];
		tmpMacEntry.macAddr.arEther[1] = atuEntry[i].atuEntry.macAddr.arEther[1];
		tmpMacEntry.macAddr.arEther[2] = atuEntry[i].atuEntry.macAddr.arEther[2];
		tmpMacEntry.macAddr.arEther[3] = atuEntry[i].atuEntry.macAddr.arEther[3];
		tmpMacEntry.macAddr.arEther[4] = atuEntry[i].atuEntry.macAddr.arEther[4];
		tmpMacEntry.macAddr.arEther[5] = atuEntry[i].atuEntry.macAddr.arEther[5];

		tmpMacEntry.DBNum = atuEntry[i].atuEntry.DBNum;

		if((status = gfdbFindAtuMacEntry(dev,&tmpMacEntry,&found)) != GT_OK)
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned "));
			testDisplayStatus(status);
			MSG_PRINT(("Entry to find : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));
			free(tmpAtuEntry);
			return status;
		}

		if (found == GT_FALSE)
		{
			MSG_PRINT(("Cannot find the Entry : (%#x-%#x-%#x-%#x-%#x-%#x)\n",
						tmpMacEntry.macAddr.arEther[0],
						tmpMacEntry.macAddr.arEther[1],
						tmpMacEntry.macAddr.arEther[2],
						tmpMacEntry.macAddr.arEther[3],
						tmpMacEntry.macAddr.arEther[4],
						tmpMacEntry.macAddr.arEther[5]));

			testResult = GT_FAIL;
			free(tmpAtuEntry);
			return testResult;
		}

		if(memcmp(&tmpMacEntry, &atuEntry[i].atuEntry, sizeof(GT_ATU_ENTRY)))
		{
			MSG_PRINT(("gfdbFindAtuMacEntry returned wrong entry.\n"));
			dumpMemory((char*)&tmpMacEntry,sizeof(GT_ATU_ENTRY));
			MSG_PRINT(("Expecting:\n"));
			dumpMemory((char*)&atuEntry[i].atuEntry,sizeof(GT_ATU_ENTRY));

			testResult = GT_FAIL;
		}
		else
		{	
			if(((i & 0x3F) == 0) && (i != 0))
				MSG_PRINT(("Compared %i ATU Entries.\n",i));
		}
	}
	
	/* Flush all addresses from the ATU table. */
	free(tmpAtuEntry);
	MSG_PRINT(("Flush out all the entries in the ATU Table ... \n"));
	if((status = gfdbFlush(dev,GT_FLUSH_ALL)) != GT_OK)
	{
		MSG_PRINT(("gfdbFlush returned "));
		testDisplayStatus(status);
		return status;
	}

	if(testResult == GT_OK)
		MSG_PRINT(("PASSED with Atu Size %i\n", 256<<atuSize));
	else
		MSG_PRINT(("FAILED with Atu Size %i\n", 256<<atuSize));

	return testResult;
}

GT_U32 testATU(GT_QD_DEV *dev)
{
	GT_STATUS testResult, status;
	GT_U32 testResults = 0;
	int arg, atuSize, sameMacs, dbNum, atuStart, atuEnd;
	GT_BOOL dbNumSupport = GT_FALSE;

	testResult = GT_OK;

	switch(dev->deviceId)
	{
		case GT_88E6051:
		case GT_FF_EG:
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_512;
			atuEnd = ATU_SIZE_4096;
			break;

		case GT_88E6021:
		case GT_88E6060:
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
			/* dbNum test is not performed at this time */
			dbNumSupport = GT_TRUE;
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_256;
			atuEnd = ATU_SIZE_2048;
			break;

		case GT_88E6052:
		case GT_FF_HG:
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_512;
			atuEnd = ATU_SIZE_2048;
			break;

		case GT_88E6063:
		case GT_FH_VPN:
		case GT_88E6083:
			/* dbNum test is not performed at this time */
			dbNumSupport = GT_TRUE;
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_512;
			atuEnd = ATU_SIZE_2048;
			break;
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
			/* dbNum test is not performed at this time */
			dbNumSupport = GT_TRUE;
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_4096;
			atuEnd = ATU_SIZE_4096;
			break;
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
			/* dbNum test is not performed at this time */
			dbNumSupport = GT_TRUE;
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_4096;
			atuEnd = ATU_SIZE_8192;
			break;
		default:
			MSG_PRINT(("Cannot run ATU test.(Unknown device)\n"));
			return GT_FAIL;
	}

	switch (dev->deviceId)
	{
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
				if(testATUSetup(dev) != GT_OK)
					return GT_FAIL;
		default:
				break;
	}

	for(arg=0; arg<1; arg++)
	{
		for(atuSize=atuStart; atuSize<=atuEnd; atuSize++)
		{
			if(dbNumSupport == GT_TRUE)
			{
				dbNum = (64<<atuSize)/TEST_MAC_ENTRIES;
				if (dbNum > 16)
					dbNum = 16;
			}
			else
				dbNum = 1;

			for(sameMacs=0;sameMacs<=dbNum;sameMacs+=4)
			{
				MSG_PRINT(("Running ATU Test : arg %i, macEntries %i, dbNum %i, atuSize %i\n",
							arg, TEST_MAC_ENTRIES, dbNum, 256 << atuSize));
				if((status=testAtuDbNum(dev,arg,TEST_MAC_ENTRIES,dbNum,sameMacs,atuSize)) != GT_OK)
				{
					MSG_PRINT(("ATU Test Fail(%d), arg %i,dbNum %i,sameMacs %i,atuSize %i\n", 
								status,arg,dbNum,sameMacs,256<<atuSize));
					testResult = GT_FAIL;
					testResults |= 1 << status;
					break;
				}
				else
				{
					MSG_PRINT(("ATU Test Pass with arg %i\n", arg));
				}
				MSG_PRINT((" \n"));
			}
			if (testResult != GT_OK)
				break;
		}
		if (testResult != GT_OK)
			break;
	}

#if 0
	MSG_PRINT(("Exercising Full ATU Table...\n"));
	for(atuSize=atuStart; atuSize<=atuEnd; atuSize++)
	{
		if((status = testFilledATU(dev,atuSize,0)) != GT_OK)
		{
			testResults |= 1 << status;
			testResult = GT_FAIL;
		}
		if(dbNumSupport == GT_TRUE)
		{
		  if((status = testFilledATU(dev,atuSize,15)) != GT_OK)
		  {
			testResults |= 1 << status;
			testResult = GT_FAIL;
		  }
		}
	}
#endif

	/* Sw Reset */
	if((status=gsysSwReset(dev)) != GT_OK)
	{
		MSG_PRINT(("gsysSwReset returned Fail (%#x).\n", status));
		testResults |= 1 << status;
		return testResults;
	}

	return testResults;
}


GT_STATUS testATUStress(GT_QD_DEV *dev)
{
	GT_STATUS testResult, status;
	int arg, atuSize, sameMacs, dbNum, maxDbNum, atuStart, atuEnd;
	GT_BOOL dbNumSupport = GT_FALSE;

	testResult = GT_OK;
	maxDbNum = 16;

	switch(dev->deviceId)
	{
		case GT_88E6051:
		case GT_FF_EG:
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_512;
			atuEnd = ATU_SIZE_4096;
			break;

		case GT_88E6021:
		case GT_88E6060:
		case GT_88E6031:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6061:
		case GT_88E6065:
			dbNumSupport = GT_TRUE;
			atuStart = ATU_SIZE_256;
			atuEnd = ATU_SIZE_2048;
			break;

		case GT_88E6052:
		case GT_FF_HG:
			dbNumSupport = GT_FALSE;
			atuStart = ATU_SIZE_512;
			atuEnd = ATU_SIZE_2048;
			break;

		case GT_88E6063:
		case GT_FH_VPN:
		case GT_88E6083:
			dbNumSupport = GT_TRUE;
			atuStart = ATU_SIZE_512;
			atuEnd = ATU_SIZE_2048;
			break;
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
			dbNumSupport = GT_TRUE;
			atuStart = ATU_SIZE_4096;
			atuEnd = ATU_SIZE_4096;
			break;
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
			dbNumSupport = GT_TRUE;
			maxDbNum = 256;
			atuStart = ATU_SIZE_4096;
			atuEnd = ATU_SIZE_4096;
			break;
		case GT_88E6131:
		case GT_88E6108:
			dbNumSupport = GT_TRUE;
			maxDbNum = 256;
			atuStart = ATU_SIZE_1024;
			atuEnd = ATU_SIZE_1024;
			break;
		default:
			MSG_PRINT(("Cannot run ATU test.(Unknown device)\n"));
			return GT_FAIL;
	}

	for(arg=0; arg<3; arg++)
	{
		for(atuSize=atuStart; atuSize<=atuEnd; atuSize++)
		{
			if(dbNumSupport == GT_TRUE)
			{
				if(atuStart == atuEnd)
				{
					dbNum = maxDbNum;
				}
				else
				{
					dbNum = (64<<atuSize)/TEST_MAC_ENTRIES;
					if (dbNum > maxDbNum)
						dbNum = maxDbNum;
				}
			}
			else
				dbNum = 1;

			for(sameMacs=0;sameMacs<=4;sameMacs+=2)
			{
				MSG_PRINT(("Running ATU Test: arg %i,macEntries %i,dbNum %i,atuSize %i,sameMac %i\n",
							arg, TEST_MAC_ENTRIES, dbNum, 256 << atuSize, sameMacs));
				if((status=testAtuDbNum(dev,arg,TEST_MAC_ENTRIES,dbNum,sameMacs,atuSize)) != GT_OK)
				{
					MSG_PRINT(("ATU Test Fail(%d), arg %i,dbNum %i,sameMacs %i,atuSize %i\n", 
								status,arg,dbNum,sameMacs,256<< atuSize));
					testResult = GT_FAIL;
					break;
				}
				else
				{
					MSG_PRINT(("ATU Test Pass with arg %i\n", arg));
				}
				MSG_PRINT((" \n"));
			}
			if (testResult != GT_OK)
				break;

		}
		if (testResult != GT_OK)
			break;
	}

	MSG_PRINT(("Exercising Full ATU Table...\n"));
	for(atuSize=atuStart; atuSize<=atuEnd; atuSize++)
	{
		if((status = testFilledATU(dev,atuSize,0)) != GT_OK)
			testResult = GT_FAIL;
		if((status = testFilledATU(dev,atuSize,maxDbNum-1)) != GT_OK)
			testResult = GT_FAIL;
	}

	/* Sw Reset */
	if((status=gsysSwReset(dev)) != GT_OK)
	{
		MSG_PRINT(("gsysSwReset returned Fail (%#x).\n", status));
		return status;
	}

	return testResult;
}


/*******************************************************************************
* testVlan
*
* DESCRIPTION:
*       Testing Vlan related APIs. (Set/Get)
*		(EgressMode, VlanTunnel, PortVlanPorts, PortUserPriLsb, and PortVid access)
*		
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on fail
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U32 testVlan(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, mapTest;
	GT_U32 testResults = 0;
	TEST_API testAPI;
	GT_LPORT port;
	GT_LPORT portList[MAX_SWITCH_PORTS];
	GT_LPORT tmpPortList[MAX_SWITCH_PORTS];
	GT_LPORT orgPortList[MAX_SWITCH_PORTS];
	GT_U8 i, portCount, orgCount, tmpCount;
	int portIndex;

	testResult = GT_OK;

	/*
	 *  Egress Mode
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gprtGetEgressMode;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gprtSetEgressMode;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Egress Mode API test "));
	testDisplayStatus(status);

	/*
	 *  Vlan Tunnel
	 */
	testAPI.getFunc.port_bool = gprtGetVlanTunnel;
	testAPI.setFunc.port_bool = gprtSetVlanTunnel;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Vlan Tunnel API test "));
	testDisplayStatus(status);

	/*
	 *  user priority (VPT) LSB bit
	 */
	testAPI.getFunc.port_bool = gvlnGetPortUserPriLsb;
	testAPI.setFunc.port_bool = gvlnSetPortUserPriLsb;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("user priority (VPT) LSB bit API test "));
	testDisplayStatus(status);

	/*
	 *  Port VID
	 */
	testAPI.getFunc.port_u16 = gvlnGetPortVid;
	testAPI.setFunc.port_u16 = gvlnSetPortVid;
	if((status = testPortU16Type(dev,&testAPI,7)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Port VID API test "));
	testDisplayStatus(status);

	/* Port Vlan Mapping */
	mapTest = GT_OK;

	portCount = (4 < (dev->numOfPorts-1))?4:(dev->numOfPorts-1);

	for(portIndex=0; portIndex<portCount; portIndex++)
		portList[portIndex] = portIndex;

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		if((status = gvlnGetPortVlanPorts(dev,port,orgPortList,&orgCount)) != GT_OK)
		{
			MSG_PRINT(("gvlnGetPortVlanPorts returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gvlnSetPortVlanPorts(dev,port,portList,portCount)) != GT_OK)
		{
			MSG_PRINT(("gvlnSetPortVlanPorts returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gvlnGetPortVlanPorts(dev,port,tmpPortList,&tmpCount)) != GT_OK)
		{
			MSG_PRINT(("gvlnGetPortVlanPorts returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if (portCount != tmpCount)
		{
			MSG_PRINT(("gvlnGetPortVlanPorts returned wrong portCount(%i:%i).\n",portCount,tmpCount));
			mapTest = GT_FAIL;
			testResults |= 1 << mapTest;
		}
		
		for(i=0; i<portCount; i++)
		{
			if(tmpPortList[i] != portList[i])
			{
				MSG_PRINT(("Returned wrong portList(i %d, port %d, should be port %d).\n"
							,i,tmpPortList[i],portList[i]));
				mapTest = GT_FAIL;
				testResults |= 1 << mapTest;
			}
		}

		if((status = gvlnSetPortVlanPorts(dev,port,orgPortList,orgCount)) != GT_OK)
		{
			MSG_PRINT(("gvlnSetPortVlanPorts returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	}
	if(mapTest != GT_OK)
	{
		MSG_PRINT(("VLAN MAP API Test Fail.\n"));
		testResult = mapTest;
	}
	else
		MSG_PRINT(("VLAN MAP API Test Pass.\n"));

#ifdef DEBUG_FEATURE 
	/*
	 *  Port DBNum
	 */
	testAPI.getFunc.port_u8 = gvlnGetPortVlanDBNum;
	testAPI.setFunc.port_u8 = gvlnSetPortVlanDBNum;
	if((status = testPortU8Type(dev,&testAPI,16)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Port DBNum API test "));
	testDisplayStatus(status);
#endif

	/*
	 *  Port Vlan 802.1Q Mode
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gvlnGetPortVlanDot1qMode;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gvlnSetPortVlanDot1qMode;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Port 802.1Q Mode API test "));
	testDisplayStatus(status);

	/*
	 *  Force Default VID
	 */
	testAPI.getFunc.port_bool = gvlnGetPortVlanForceDefaultVID;
	testAPI.setFunc.port_bool = gvlnSetPortVlanForceDefaultVID;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Force Default VID API test "));
	testDisplayStatus(status);

	/*
	 *  Force MAP
	 */
	testAPI.getFunc.port_bool = gvlnGetForceMap;
	testAPI.setFunc.port_bool = gvlnSetForceMap;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Force MAP API test "));
	testDisplayStatus(status);

	return testResults;

}

/*******************************************************************************
* testSTP
*
* DESCRIPTION:
*       Test STP(Spanning Tree Protocol) related APIs
*		
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on fail
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U32 testSTP(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, sectionResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	testResult = sectionResult = GT_OK;

	/*
	 *	STP Port State
	 */
	testAPI.getFunc.port_u32 = (GT_API_GET_PORT_U32)gstpGetPortState;
	testAPI.setFunc.port_u32 = (GT_API_SET_PORT_U32)gstpSetPortState;
	if((status = testPortU32Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("STP Port State API test "));
	testDisplayStatus(status);

	return testResults;
}


GT_U32 testPhy1(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	GT_LPORT port;
	GT_U16 u16Data, tmpData;
	GT_PHY_AUTO_MODE mode;
	int portIndex, nPhys;

	testResult = GT_OK;
	switch(dev->numOfPorts)
	{
		case 3:
			nPhys = 2;
			break;
		default:
			nPhys = 5;
			break;
	}

	for(portIndex=0; portIndex<nPhys; portIndex++)
	{
		port = portIndex;
		
		if(port == dev->cpuPortNum)
			continue;

		/* Test Reset API */
		MSG_PRINT(("Reset Phy (port %i).\n",port));
		if((status = gprtPhyReset(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gprtPhyReset returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gprtGetPhyReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(!(u16Data & 0x1000))
		{
			MSG_PRINT(("gprtPhyReset failed.\n"));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Reset, Phy (port %i) Reg 0 : %#x.\n",port,u16Data));


		/* 
		 *  Set AutoNego disable, LoopBack enable, 100Mbps, Duplex On,
		 */
		if((status = gprtPortAutoNegEnable(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortAutoNegEnable returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortLoopback(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortLoopback returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortSpeed(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortSpeed returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortDuplexMode(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortDuplexMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gprtGetPhyReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(0x6100 != (u16Data & 0x6100))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x6100).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
		{
			if(u16Data & 0x1000)
			{
				MSG_PRINT(("Set Failed (%#x, should be 0x6100).\n", u16Data));
				testResults |= 1 << GT_FAIL;
				testResult = GT_FAIL;
			}
			else
				MSG_PRINT(("After Loopback and 100Full, Phy (port %i) Reg 0 : %#x.\n",port,u16Data));
		}

		/* 
		 *  Set 10Mbps, Half Duplex.
		 */
		if((status = gprtSetPortLoopback(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortLoopback returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortSpeed(dev,port,PHY_SPEED_10_MBPS)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortSpeed returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortDuplexMode(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortDuplexMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gprtGetPhyReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(u16Data != 0x0000)
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x0000).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After 10Half, Phy (port %i) Reg 0 : %#x.\n",port,u16Data));

		/* 
		 *  Set Power Down
		 */
		if((status = gprtPortPowerDown(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortPowerDown returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* Power Down bit should be set. */
		if(!(u16Data & 0x0800))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x0800).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After PowerDown, Phy (port %i) Reg 0 : %#x.\n",port,u16Data));

		/* 
		 *  Set Power Up
		 */
		if((status = gprtPortPowerDown(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortPowerDown returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After power up, Power Down bit should be cleared.*/
		if(u16Data &= 0x0800)
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x0000).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Power back up, Phy (port %i) Reg 0 : %#x.\n",port,u16Data));

		/* 
		 *  Set Autonego and Restart AutoNego.
		 */
		if((status = gprtPortAutoNegEnable(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortAutoNegEnable returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtPortRestartAutoNeg(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gprtPortRestartAutoNeg returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(!(u16Data & 0x1000))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x1000).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Auto, Phy (port %i) Reg 0 : %#x.\n",port,u16Data));


		/*
		 *	Enable PAUSE
		*/
		if((status = gprtSetPause(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPause returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,4,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(!(u16Data & 0x400))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x400).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Pause set, Phy (port %i) Reg 4 : %#x.\n",port,u16Data));

		/*
		 *	Disable PAUSE
		*/
		if((status = gprtSetPause(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPause returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gprtGetPhyReg(dev,port,4,&u16Data) != GT_OK)
		{
			MSG_PRINT(("gprtGetPhyReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(u16Data & 0x400)
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x00).\n", u16Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Pause reset, Phy (port %i) Reg 4 : %#x.\n",port,u16Data));

		/*
		 *	Disable PAUSE
		*/
		for(mode=SPEED_AUTO_DUPLEX_AUTO;mode<=SPEED_AUTO_DUPLEX_HALF;mode++)
		{
			switch(mode)
			{
				case SPEED_1000_DUPLEX_AUTO:
				case SPEED_1000_DUPLEX_FULL:
				case SPEED_1000_DUPLEX_HALF:
						continue;
				default:
						break;
			}

			if((status = gprtSetPortAutoMode(dev,port,mode)) != GT_OK)
			{
				MSG_PRINT(("gprtSetPortAutoMode returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			/* Autonego should be enabled. */
	    	if(gprtGetPhyReg(dev,port,0,&u16Data) != GT_OK)
			{
				MSG_PRINT(("gprtGetPhyReg returned Fail.\n"));
				testResults |= 1 << GT_FAIL;
				return testResults;
			}

			/* After reset AutoNego should be enabled. */
			if(!(u16Data & 0x1000))
			{
				MSG_PRINT(("Set Failed (%#x, should be 0x1000).\n", u16Data));
				testResults |= 1 << GT_FAIL;
				testResult = GT_FAIL;
			}
			else
			{
		    	if(gprtGetPhyReg(dev,port,4,&u16Data) != GT_OK)
				{
					MSG_PRINT(("gprtGetPhyReg returned Fail.\n"));
					testResults |= 1 << GT_FAIL;
					return testResults;
				}

				switch(mode)
				{
					case SPEED_AUTO_DUPLEX_AUTO:
							tmpData = 0x1e0;
							break;
					case SPEED_100_DUPLEX_AUTO:
							tmpData = 0x180;
							break;
					case SPEED_10_DUPLEX_AUTO:
							tmpData = 0x060;
							break;
					case SPEED_AUTO_DUPLEX_FULL:
							tmpData = 0x140;
							break;
					case SPEED_AUTO_DUPLEX_HALF:
							tmpData = 0x0a0;
							break;
					default:
							tmpData = 0;
							break;
				}

				if((u16Data & 0x1e0) != tmpData)
				{
					MSG_PRINT(("Set AutoMode(%i) Failed (%#x, should be %#x).\n", mode,u16Data,tmpData));
					testResult = GT_FAIL;
					testResults |= 1 << GT_FAIL;
				}
				else
					MSG_PRINT(("After Mode(%d) set, Phy (port %i) Reg 4 : %#x.\n",mode,port,u16Data));

			}
		}


	}
	return testResults;
}

GT_U32 testPhy2(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	GT_LPORT port;
	GT_U32 u32Data, tmpData;
	GT_PHY_AUTO_MODE mode;
	int portIndex, nPhys;

	testResult = GT_OK;
	switch(dev->numOfPorts)
	{
		case 3:
			nPhys = 2;
			break;
		default:
			nPhys = 5;
			break;
	}

	for(portIndex=0; portIndex<nPhys; portIndex++)
	{
		port = portIndex;
		
		if(port == dev->cpuPortNum)
			continue;

		/* Test Reset API */
		MSG_PRINT(("Reset Phy (port %i).\n",port));
		if((status = gprtPhyReset(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gprtPhyReset returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev, dev->baseRegAddr+port,0,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(!(u32Data & 0x1000))
		{
			MSG_PRINT(("gprtPhyReset failed.\n"));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Reset, Phy (port %i) Reg 0 : %#x.\n",port,u32Data));


		/* 
		 *  Set AutoNego disable, LoopBack enable, 100Mbps, Duplex On,
		 */
		if((status = gprtPortAutoNegEnable(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortAutoNegEnable returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortLoopback(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortLoopback returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortSpeed(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortSpeed returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortDuplexMode(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortDuplexMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev, dev->baseRegAddr+port,0,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(0x6100 != (u32Data & 0x6100))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x6100).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
		{
			if(u32Data & 0x1000)
			{
				MSG_PRINT(("Set Failed (%#x, should be 0x6100).\n", u32Data));
				testResults |= 1 << GT_FAIL;
				testResult = GT_FAIL;
			}
			else
				MSG_PRINT(("After Loopback and 100Full, Phy (port %i) Reg 0 : %#x.\n",port,u32Data));
		}

		/* 
		 *  Set 10Mbps, Half Duplex.
		 */
		if((status = gprtSetPortLoopback(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortLoopback returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortSpeed(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortSpeed returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtSetPortDuplexMode(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortDuplexMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev,dev->baseRegAddr+port,0,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(u32Data != 0x0000)
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x0000).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After 10Half, Phy (port %i) Reg 0 : %#x.\n",port,u32Data));

		/* 
		 *  Set Power Down
		 */
		if((status = gprtPortPowerDown(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortPowerDown returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev,dev->baseRegAddr+port,0,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* Power Down bit should be set. */
		if(!(u32Data & 0x0800))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x0800).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After PowerDown, Phy (port %i) Reg 0 : %#x.\n",port,u32Data));

		/* 
		 *  Set Power Up
		 */
		if((status = gprtPortPowerDown(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortPowerDown returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev,dev->baseRegAddr+port,0,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After power up, Power Down bit should be cleared.*/
		if(u32Data &= 0x0800)
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x0000).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Power back up, Phy (port %i) Reg 0 : %#x.\n",port,u32Data));

		/* 
		 *  Set Autonego and Restart AutoNego.
		 */
		if((status = gprtPortAutoNegEnable(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtPortAutoNegEnable returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gprtPortRestartAutoNeg(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gprtPortRestartAutoNeg returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev,dev->baseRegAddr+port,0,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(!(u32Data & 0x1000))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x1000).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Auto, Phy (port %i) Reg 0 : %#x.\n",port,u32Data));


		/*
		 *	Enable PAUSE
		*/
		if((status = gprtSetPause(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPause returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev,dev->baseRegAddr+port,4,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(!(u32Data & 0x400))
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x400).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Pause set, Phy (port %i) Reg 4 : %#x.\n",port,u32Data));

		/*
		 *	Disable PAUSE
		*/
		if((status = gprtSetPause(dev,port,GT_FALSE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPause returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

	    if(gsysReadMiiReg(dev,dev->baseRegAddr+port,4,&u32Data) != GT_OK)
		{
			MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
			testResults |= 1 << GT_FAIL;
			return testResults;
		}

		/* After reset AutoNego should be enabled. */
		if(u32Data & 0x400)
		{
			MSG_PRINT(("Set Failed (%#x, should be 0x00).\n", u32Data));
			testResults |= 1 << GT_FAIL;
			testResult = GT_FAIL;
		}
		else
			MSG_PRINT(("After Pause reset, Phy (port %i) Reg 4 : %#x.\n",port,u32Data));

		/*
		 *	Disable PAUSE
		*/
		for(mode=SPEED_AUTO_DUPLEX_AUTO;mode<=SPEED_AUTO_DUPLEX_HALF;mode++)
		{
			if((status = gprtSetPortAutoMode(dev,port,mode)) != GT_OK)
			{
				MSG_PRINT(("gprtSetPortAutoMode returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			/* Autonego should be enabled. */
	    	if(gsysReadMiiReg(dev,dev->baseRegAddr+port,0,&u32Data) != GT_OK)
			{
				MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
				testResults |= 1 << GT_FAIL;
				return testResults;
			}

			/* After reset AutoNego should be enabled. */
			if(!(u32Data & 0x1000))
			{
				MSG_PRINT(("Set Failed (%#x, should be 0x1000).\n", u32Data));
				testResults |= 1 << GT_FAIL;
				testResult = GT_FAIL;
			}
			else
			{
		    	if(gsysReadMiiReg(dev,dev->baseRegAddr+port,4,&u32Data) != GT_OK)
				{
					MSG_PRINT(("gsysReadMiiReg returned Fail.\n"));
					testResults |= 1 << GT_FAIL;
					return testResults;
				}

				switch(mode)
				{
					case SPEED_AUTO_DUPLEX_AUTO:
							tmpData = 0x1e0;
							break;
					case SPEED_100_DUPLEX_AUTO:
							tmpData = 0x180;
							break;
					case SPEED_10_DUPLEX_AUTO:
							tmpData = 0x060;
							break;
					case SPEED_AUTO_DUPLEX_FULL:
							tmpData = 0x140;
							break;
					case SPEED_AUTO_DUPLEX_HALF:
							tmpData = 0x0a0;
							break;
					default:
							tmpData = 0;
							break;
				}

				if((u32Data & 0x1e0) != tmpData)
				{
					MSG_PRINT(("Set AutoMode(%i) Failed (%#x, should be %#x).\n", mode,u32Data,tmpData));
					testResult = GT_FAIL;
					testResults |= 1 << GT_FAIL;
				}
				else
					MSG_PRINT(("After Mode(%d) set, Phy (port %i) Reg 4 : %#x.\n",mode,port,u32Data));

			}
		}


	}
	return testResults;
}

GT_U32 testPhy(GT_QD_DEV *dev)
{
	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
			MSG_PRINT(("Not Implemented.\n"));
			break;
		default:
			return testPhy1(dev);	
	}
	return 0;
}

GT_STATUS readStatistics(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_LPORT port;
	GT_PORT_STAT portStat;

	MSG_PRINT(("Current Port Statistics\n"));
	for (port=0; port<dev->numOfPorts; port++)
	{
		if((status = gprtGetPortCtr(dev,port,&portStat)) != GT_OK)
		{
			MSG_PRINT(("gprtGetPortCtr returned fail.\n"));
			return status;
		}

		MSG_PRINT(("Port %i : Rx %i, Tx %i.\n",port,portStat.rxCtr,portStat.txCtr));
	}

	MSG_PRINT(("After Clear Port Statistics\n"));
	for (port=0; port<dev->numOfPorts; port++)
	{
		if((status = gprtClearAllCtr(dev)) != GT_OK)
		{
			MSG_PRINT(("gprtClearAllCtr returned fail.\n"));
			return status;
		}

		if((status = gprtGetPortCtr(dev,port,&portStat)) != GT_OK)
		{
			MSG_PRINT(("gprtGetPortCtr returned fail.\n"));
			return status;
		}

		MSG_PRINT(("Port %i : Rx %i, Tx %i.\n",port,portStat.rxCtr,portStat.txCtr));
	}
	return GT_OK;
}

GT_STATUS testGoodPkt(GT_QD_DEV *dev)
{
	GT_STATUS status;

	if((status = gprtSetCtrMode(dev,GT_CTR_ALL)) != GT_OK)
	{
		MSG_PRINT(("gprtSetCtrMode returned fail.\n"));
		return status;
	}
	return GT_OK;
}

GT_STATUS testBadPkt(GT_QD_DEV *dev)
{
	GT_STATUS status;

	if((status = gprtSetCtrMode(dev,GT_CTR_ERRORS)) != GT_OK)
	{
		MSG_PRINT(("gprtSetCtrMode returned fail.\n"));
		return status;
	}
	return GT_OK;
}


GT_U32 testPortStatus(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	GT_LPORT port;
	GT_BOOL mode;
	GT_U32	u32Mode;
	GT_U16	u16Mode;
	int portIndex;

	testResult = GT_OK;

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;
		
		MSG_PRINT(("Port %i :\n",port));

		if((status = gprtGetPartnerLinkPause(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetPartnerLinkPause returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("His Link Pause : %i    ",(int)mode));

		if((status = gprtGetSelfLinkPause(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetSelfLinkPause returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("My Link Pause  : %i\n",(int)mode));

		if((status = gprtGetLinkState(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetLinkState returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Link Status    : %i    ",(int)mode));

		if((status = gprtGetResolve(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetResolve returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Resolve        : %i\n",(int)mode));

		if((status = gprtGetPortMode(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetPortMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Port Mode      : %i    ",(int)mode));

		if((status = gprtGetPhyMode(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetPhyMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Phy Mode       : %i\n",(int)mode));

		if((status = gprtGetSpeed(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetSpeed returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Port Speed     : %i    ",(int)mode));

		if((status = gprtGetDuplex(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetDuplex returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Port Duplex    : %i\n",(int)mode));

		MSG_PRINT(("Setting Port Duplex : %i\n",1-(int)mode));
		if((status = gprtSetDuplex(dev,port,1-mode)) != GT_OK)
		{
			MSG_PRINT(("gprtSetDuplex returned "));
			testDisplayStatus(status);
			switch(dev->deviceId)
			{
				case GT_88E6021:
					if(port != dev->cpuPortNum)
						break;
					else
					{
						testResults |= 1 << status;
						if(status == GT_FAIL)
						return testResults;
					}
				case GT_88E6051:
				case GT_88E6052:
				case GT_FF_HG:
				case GT_FF_EG:
					break;
				case GT_88E6063:
				case GT_FH_VPN:
					if(port < 5)
						break;
					else
					{
						testResults |= 1 << status;
						if(status == GT_FAIL)
						return testResults;
					}
					break;
				default:
					break;
			}

		}
		else
		{
			GT_BOOL tmpMode;
			if((status = gprtGetDuplex(dev,port,&tmpMode)) != GT_OK)
			{
				MSG_PRINT(("gprtGetDuplex returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}
			if(tmpMode != (1-mode))
			{
				MSG_PRINT(("Setting Port Duplex Failed (current mode %i)\n",tmpMode));
				testResults |= 1 << GT_FAIL;
				testResult = GT_FAIL;
			}
			else
				MSG_PRINT(("Setting Port Duplex Passed.\n"));

			MSG_PRINT(("Setting Port Duplex : %i\n",mode));
			if((status = gprtSetDuplex(dev,port,mode)) != GT_OK)
			{
				MSG_PRINT(("gprtSetDuplex returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				if(status == GT_FAIL)
					return testResults;
			}

			if((status = gprtGetDuplex(dev,port,&tmpMode)) != GT_OK)
			{
				MSG_PRINT(("gprtGetDuplex returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}
			if(tmpMode != mode)
			{
				MSG_PRINT(("Setting Port Duplex Failed (current mode %i)\n",tmpMode));
				testResult = GT_FAIL;
				testResults |= 1 << GT_FAIL;
			}
			else
				MSG_PRINT(("Setting Port Duplex Passed.\n"));

		}

		if((status = gprtGetTxPaused(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetTxPaused returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("TxPaused       : %i    ",(int)mode));

		if((status = gprtGetFlowCtrl(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetFlowCtrl returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("FlowCtrl       : %i\n",(int)mode));

		if((status = gprtGetPxMode(dev,port,&u32Mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetPxMode returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("PxMode         : %i    ",(int)u32Mode));

		if((status = gprtGetMiiInterface(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetMiiInterface returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Mii Interface  : %i\n",(int)mode));

		if((status = gprtGetHdFlowDis(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetHdFlowDis returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("HD Flow Dis.   : %i    ",(int)mode));

		if((status = gprtGetFdFlowDis(dev,port,&mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetFdFlowDis returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("FD Flow Dis.   : %i\n",(int)mode));

		if((status = gprtGetOutQSize(dev,port,&u16Mode)) != GT_OK)
		{
			MSG_PRINT(("gprtGetOutQSize returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
		MSG_PRINT(("Out Q Size     : %i\n",(int)u16Mode));

	}

	return testResults;
}

GT_U32 testQoSRule(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	TEST_API testAPI;

	testResult = GT_OK;

	/*
	 *  Priority Map Rule (IEEE if Both IEEE and IP)
	 */
	testAPI.getFunc.port_bool = gqosGetPrioMapRule;
	testAPI.setFunc.port_bool = gqosSetPrioMapRule;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Priority Map Rule API test "));
	testDisplayStatus(status);

	/*
	 *  IP Priority Map Rule (use IP)
	 */
	testAPI.getFunc.port_bool = gqosGetIpPrioMapEn;
	testAPI.setFunc.port_bool = gqosIpPrioMapEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("IP Priority Map Rule API test "));
	testDisplayStatus(status);

	/*
	 *  IEEE Priority Map Rule (use IEEE Tag)
	 */
	testAPI.getFunc.port_bool = gqosGetUserPrioMapEn;
	testAPI.setFunc.port_bool = gqosUserPrioMapEn;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("IEEE Priority Map Rule API test "));
	testDisplayStatus(status);

	/*
	 *  VID FPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetVIDFPriOverride;
	testAPI.setFunc.port_bool = gqosSetVIDFPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("VID FPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  SA FPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetSAFPriOverride;
	testAPI.setFunc.port_bool = gqosSetSAFPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("SA FPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  DA FPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetDAFPriOverride;
	testAPI.setFunc.port_bool = gqosSetDAFPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DA FPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  VID QPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetVIDQPriOverride;
	testAPI.setFunc.port_bool = gqosSetVIDQPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("VID QPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  SA QPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetSAQPriOverride;
	testAPI.setFunc.port_bool = gqosSetSAQPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("SA QPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  DA QPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetDAQPriOverride;
	testAPI.setFunc.port_bool = gqosSetDAQPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("DA QPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  ARP QPri Override
	 */
	testAPI.getFunc.port_bool = gqosGetARPQPriOverride;
	testAPI.setFunc.port_bool = gqosSetARPQPriOverride;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ARP QPri Override API test "));
	testDisplayStatus(status);

	/*
	 *  Force QPri
	 */
	testAPI.getFunc.port_bool = gqosGetForceQPri;
	testAPI.setFunc.port_bool = gqosSetForceQPri;
	if((status = testPortBoolType(dev,&testAPI)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Force QPri API test "));
	testDisplayStatus(status);

	/*
	 *  QPri Value
	 */
	testAPI.getFunc.port_u8 = gqosGetQPriValue;
	testAPI.setFunc.port_u8 = gqosSetQPriValue;
	if((status = testPortU8Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("QPri Value API test "));
	testDisplayStatus(status);

	/*
	 *  Default FPri Value
	 */
	testAPI.getFunc.port_u8 = gqosGetDefFPri;
	testAPI.setFunc.port_u8 = gqosSetDefFPri;
	if((status = testPortU8Type(dev,&testAPI,8)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Default FPri Value API test "));
	testDisplayStatus(status);

	/*
	 *  ARP QPri Value
	 */
	testAPI.getFunc.u16 = (GT_API_GET_U16)gqosGetArpQPri;
	testAPI.setFunc.u16 = (GT_API_SET_U16)gqosSetArpQPri;
	if((status = testU16Type(dev,&testAPI,4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("ARP QPri Value API test "));
	testDisplayStatus(status);


	return testResults;
}

GT_U32 testQoSMapG(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, sectionResult;
	GT_U32 testResults = 0;
	GT_U8 priority, remapped, tmpPri;
	GT_LPORT port;
	int portIndex;

	testResult = sectionResult = GT_OK;

	for(priority=0; priority<8; priority++)
	{
		remapped = 7 - priority;

		for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
		{
			port = portIndex;
					
			if((status = gqosSetTagRemap(dev,portIndex, priority, remapped)) != GT_OK)
			{
				MSG_PRINT(("gqosSetTagRemap returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			tmpPri = 8;
			if((status = gqosGetTagRemap(dev,portIndex, priority, &tmpPri)) != GT_OK)
			{
				MSG_PRINT(("gqosGetTagRemap returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			if (tmpPri != remapped)
			{
				MSG_PRINT(("QoS Remapping setup problem (pri:%#x,remap:%#x,port).\n",priority, remapped, port));
				sectionResult = GT_FAIL;
				testResults |= 1 << GT_FAIL;
			}
		}
	}

	if(sectionResult == GT_OK)
	{
		MSG_PRINT(("QoS Remapping setup Pass.\n"));
	}
	else
	{
		MSG_PRINT(("Qos Remapping setup Fail.\n"));
		testResult = sectionResult;
		sectionResult = GT_OK;
	}

	return testResults;
}

GT_U32 testQoSMap(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, sectionResult;
	GT_U32 testResults = 0;
	GT_U8 priority, trClass;
	TEST_API testAPI;

	testResult = sectionResult = GT_OK;

	/*
	 *  Default Traffic Class Map
	 */
	testAPI.getFunc.port_u8 = gcosGetPortDefaultTc;
	testAPI.setFunc.port_u8 = gcosSetPortDefaultTc;
	if((status = testPortU8Type(dev,&testAPI, 4)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Default Traffic Class Setup "));
	testDisplayStatus(status);

	for(priority=0; priority<8; priority++)
	{
		if((status = gcosSetUserPrio2Tc(dev,priority,(priority&0x3))) != GT_OK)
		{
			MSG_PRINT(("gcosSetUserPrio2Tc returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gcosGetUserPrio2Tc(dev,priority,&trClass)) != GT_OK)
		{
			MSG_PRINT(("gcosSetUserPrio2Tc returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if (trClass != (priority&0x3))
		{
			MSG_PRINT(("IEEE traffic Class setup problem (tr:%#x,pr:%#x).\n",trClass,priority));
			sectionResult = GT_FAIL;
			testResults |= 1 << GT_FAIL;
		}
	}

	if(sectionResult == GT_OK)
	{
		MSG_PRINT(("IEEE traffic Class setup Pass.\n"));
	}
	else
	{
		MSG_PRINT(("IEEE traffic Class setup Fail.\n"));
		testResult = sectionResult;
		sectionResult = GT_OK;
	}

	for(priority=0; priority<0x40; priority++)
	{
		if((status = gcosSetDscp2Tc(dev,priority,(priority&0x3))) != GT_OK)
		{
			MSG_PRINT(("gcosSetDscp2Tc returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gcosGetDscp2Tc(dev,priority,&trClass)) != GT_OK)
		{
			MSG_PRINT(("gcosGetDscp2Tc returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if (trClass != (priority&0x3))
		{
			MSG_PRINT(("IP traffic Class setup problem (tr:%#x,pr:%#x).\n",trClass,priority));
			sectionResult = GT_FAIL;
			testResults |= 1 << GT_FAIL;
		}
	}
	if(sectionResult == GT_OK)
	{
		MSG_PRINT(("IP traffic Class setup Pass.\n"));
	}
	else
	{
		MSG_PRINT(("IP traffic Class setup Fail.\n"));
		testResult = sectionResult;
		sectionResult = GT_OK;
	}

	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6065:
			testResults |= testQoSMapG(dev);
			break;
		default:
			break;
	}			
	return testResults;
}


GT_U32 testInterrupt(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult;
	GT_U32 testResults = 0;
	GT_LPORT port;
	GT_U16 data, portIntCause, phyIntCause, tmpData;
	int portIndex, phyCount;

	MSG_PRINT(("\ntesting Interrupt Handler : \n"));

	testResult = GT_OK;

	MSG_PRINT(("\nSetting up Interrupt Test...\n"));

	/* Enable QuarterDeck interrupt for ATUFull, ATUDone, PHYInt, and EEInt */
	data = GT_ATU_FULL|GT_ATU_DONE|GT_PHY_INTERRUPT|GT_EE_INTERRUPT;
	if((status = eventSetActive(dev,data)) != GT_OK)
	{
		MSG_PRINT(("eventSetActive returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	/* 
	 Enable Phy interrupt for Link Status Change, Speed Change,
	 *	AutoNego Complete, and Duplex Changed for all phys.
	*/
	data = GT_SPEED_CHANGED|GT_DUPLEX_CHANGED|GT_AUTO_NEG_COMPLETED|GT_LINK_STATUS_CHANGED;

	switch(dev->deviceId)
	{
		case GT_88E6021:
			phyCount = 2;
			break;
		case GT_88E6051:
			phyCount = 4;
			break;
		case GT_88E6052:
		case GT_88E6063:
		case GT_FF_HG:
		case GT_FF_EG:
		case GT_FH_VPN:
			phyCount = 5;
			break;
		default:
			MSG_PRINT(("Unknown DEVICE. Assuming 88E6052.\n"));
			phyCount = 5;
			break;
	}

	for(portIndex=0; portIndex<phyCount; portIndex++)
	{
		port = portIndex;

		if((status = gprtPhyIntEnable(dev,port,data)) != GT_OK)
		{
			MSG_PRINT(("gprtPhyIntEnable returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}
	}

	/* Set QuarterDeck interrupt ATUFull, PHYInt, and EEInt */
	portIntCause = GT_ATU_FULL|GT_PHY_INTERRUPT|GT_EE_INTERRUPT;
	if(qdSimSetGlobalInt(portIntCause) != GT_OK)
	{
		MSG_PRINT(("QD Simulator is not used.\n"));
		testResults |= 1 << GT_NOT_SUPPORTED;
		return testResults;
	}

	/* 
	 * 	Set PHY Interrupt Link Status Change, Speed Change, and AutoNego
	 * 	Complete for Port 0 and Port 4.
	*/
	phyIntCause = GT_SPEED_CHANGED|GT_AUTO_NEG_COMPLETED|GT_LINK_STATUS_CHANGED;
	if(qdSimSetPhyInt(0,phyIntCause) != GT_OK)
	{
		MSG_PRINT(("QD Simulator is not used.\n"));
		testResults |= 1 << GT_NOT_SUPPORTED;
		return testResults;
	}

	if(qdSimSetPhyInt(4,phyIntCause) != GT_OK)
	{
		MSG_PRINT(("QD Simulator is not used.\n"));
		testResults |= 1 << GT_NOT_SUPPORTED;
		return testResults;
	}

	MSG_PRINT(("Calling QD Interrupt Handler with portInt %#x, phyInt %#x\n",portIntCause,phyIntCause));

	/*
	 *	Call QuarterDeck Interrupt Handler.
	*/
	if(eventGetIntStatus(dev,&tmpData) != GT_TRUE)
	{
		MSG_PRINT(("qdIntHander returned GT_FALSE.\n"));
		testResults |= 1 << GT_FAIL;
		return testResults;
	}

	if(tmpData != portIntCause)
	{
		MSG_PRINT(("qdIntHander returned wrong intCause(%#x).\n",tmpData));
		testResults |= 1 << GT_FAIL;
		testResult = GT_FAIL;
	}
	else
		MSG_PRINT(("QD Interrupt Handler returned intCause(%#x).\n",tmpData));


	/*
	 *	Call gprtGetPhyIntPortSummary to get Port Int Summary
	*/
	if((status = gprtGetPhyIntPortSummary(dev,&tmpData)) != GT_OK)
	{
		MSG_PRINT(("gprtGetPhyIntPortSummary returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	if(tmpData != 0x11)	/* port 0 and port 4 should be set. */
	{
		MSG_PRINT(("gprtGetPhyIntPortSummary returned wrong summary(%#x).\n",tmpData));
		testResult = GT_FAIL;
		testResults |= 1 << GT_FAIL;
	}
	else
		MSG_PRINT(("Port Summary returned %#x.\n",tmpData));

	/*
	 *	Call gprtGetPhyIntStatus to get intCause
	*/
	if((status = gprtGetPhyIntStatus(dev,0,&tmpData)) != GT_OK)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	if(tmpData != phyIntCause)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned wrong phyIntCause(%#x).\n",tmpData));
		testResult = GT_FAIL;
		testResults |= 1 << GT_FAIL;
	}
	else
		MSG_PRINT(("PHY Int Status(port 0) returned %#x.\n",tmpData));
		
	/*
	 *	Call gprtGetPhyIntStatus to get intCause
	*/
	if((status = gprtGetPhyIntStatus(dev,4,&tmpData)) != GT_OK)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	if(tmpData != phyIntCause)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned wrong phyIntCause(%#x).\n",tmpData));
		testResult = GT_FAIL;
		testResults |= 1 << GT_FAIL;
	}
	else
		MSG_PRINT(("PHY Int Status(port 4) returned %#x.\n",tmpData));

	/* Set QuarterDeck interrupt ATUFull, PHYInt, and EEInt */
	portIntCause = GT_PHY_INTERRUPT;
	if(qdSimSetGlobalInt(portIntCause) != GT_OK)
	{
		MSG_PRINT(("QD Simulator is not used.\n"));
		testResults |= 1 << GT_NOT_SUPPORTED;
		return testResults;
	}

	/* 
	 * 	Set PHY Interrupt Link Status Change, Speed Change, and AutoNego
	 * 	Complete for Port 0 and Port 4.
	*/
	phyIntCause = GT_SPEED_CHANGED|GT_DUPLEX_CHANGED;
	if(qdSimSetPhyInt(0,phyIntCause) != GT_OK)
	{
		MSG_PRINT(("QD Simulator is not used.\n"));
		testResults |= 1 << GT_NOT_SUPPORTED;
		return testResults;
	}

	if(qdSimSetPhyInt(3,phyIntCause) != GT_OK)
	{
		MSG_PRINT(("QD Simulator is not used.\n"));
		testResults |= 1 << GT_NOT_SUPPORTED;
		return testResults;
	}

	MSG_PRINT(("\nCalling QD Interrupt Handler with portInt %#x, phyInt %#x\n",portIntCause,phyIntCause));

	/*
	 *	Call QuarterDeck Interrupt Handler.
	*/
	if(eventGetIntStatus(dev,&tmpData) != GT_TRUE)
	{
		MSG_PRINT(("qdIntHander returned GT_FALSE.\n"));
		testResults |= 1 << GT_FAIL;
		return testResults;
	}

	if(tmpData != portIntCause)
	{
		MSG_PRINT(("qdIntHander returned wrong intCause(%#x).\n",tmpData));
		testResult = GT_FAIL;
		testResults |= 1 << GT_FAIL;
	}
	else
		MSG_PRINT(("QD Interrupt Handler returned intCause(%#x).\n",tmpData));


	/*
	 *	Call gprtGetPhyIntPortSummary to get Port Int Summary
	*/
	if((status = gprtGetPhyIntPortSummary(dev,&tmpData)) != GT_OK)
	{
		MSG_PRINT(("gprtGetPhyIntPortSummary returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	if(tmpData != 0x9)	/* port 0 and port 3 should be set. */
	{
		MSG_PRINT(("gprtGetPhyIntPortSummary returned wrong summary(%#x).\n",tmpData));
		testResult = GT_FAIL;
		testResults |= 1 << GT_FAIL;
	}
	else
		MSG_PRINT(("Port Summary returned %#x.\n",tmpData));

	/*
	 *	Call gprtGetPhyIntStatus to get intCause
	*/
	if((status = gprtGetPhyIntStatus(dev,0,&tmpData)) != GT_OK)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	if(tmpData != phyIntCause)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned wrong phyIntCause(%#x).\n",tmpData));
		testResults |= 1 << GT_FAIL;
		testResult = GT_FAIL;
	}
	else
		MSG_PRINT(("PHY Int Status(port 0) returned %#x.\n",tmpData));
	
	/*
	 *	Call gprtGetPhyIntStatus to get intCause
	*/
	if((status = gprtGetPhyIntStatus(dev,3,&tmpData)) != GT_OK)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	if(tmpData != phyIntCause)
	{
		MSG_PRINT(("gprtGetPhyIntStatus returned wrong phyIntCause(%#x).\n",tmpData));
		testResult = GT_FAIL;
		testResults |= 1 << GT_FAIL;
	}
	else
		MSG_PRINT(("PHY Int Status(port 3) returned %#x.\n",tmpData));

	return testResults;
}

void testDisplayCounter(GT_STATS_COUNTER_SET *statsCounter)
{
	MSG_PRINT(("InUnicasts    %08i    ", statsCounter->InUnicasts));
	MSG_PRINT(("InBroadcasts  %08i   \n", statsCounter->InBroadcasts));
	MSG_PRINT(("InPause       %08i    ", statsCounter->InPause));
	MSG_PRINT(("InMulticasts  %08i   \n", statsCounter->InMulticasts));
	MSG_PRINT(("InFCSErr      %08i    ", statsCounter->InFCSErr));
	MSG_PRINT(("AlignErr      %08i   \n", statsCounter->AlignErr));
	MSG_PRINT(("InGoodOctets  %08i    ", statsCounter->InGoodOctets));
	MSG_PRINT(("InBadOctets   %08i   \n", statsCounter->InBadOctets));
	MSG_PRINT(("Undersize     %08i    ", statsCounter->Undersize));
	MSG_PRINT(("Fragments     %08i   \n", statsCounter->Fragments));
	MSG_PRINT(("In64Octets    %08i    ", statsCounter->In64Octets));
	MSG_PRINT(("In127Octets   %08i   \n", statsCounter->In127Octets));
	MSG_PRINT(("In255Octets   %08i    ", statsCounter->In255Octets));
	MSG_PRINT(("In511Octets   %08i   \n", statsCounter->In511Octets));
	MSG_PRINT(("In1023Octets  %08i    ", statsCounter->In1023Octets));
	MSG_PRINT(("InMaxOctets   %08i   \n", statsCounter->InMaxOctets));
	MSG_PRINT(("Jabber        %08i    ", statsCounter->Jabber));
	MSG_PRINT(("Oversize      %08i   \n", statsCounter->Oversize));
	MSG_PRINT(("InDiscards    %08i    ", statsCounter->InDiscards));
	MSG_PRINT(("Filtered      %08i   \n", statsCounter->Filtered));
	MSG_PRINT(("OutUnicasts   %08i    ", statsCounter->OutUnicasts));
	MSG_PRINT(("OutBroadcasts %08i   \n", statsCounter->OutBroadcasts));
	MSG_PRINT(("OutPause      %08i    ", statsCounter->OutPause));
	MSG_PRINT(("OutMulticasts %08i   \n", statsCounter->OutMulticasts));
	MSG_PRINT(("OutFCSErr     %08i    ", statsCounter->OutFCSErr));
	MSG_PRINT(("OutGoodOctets %08i   \n", statsCounter->OutGoodOctets));
	MSG_PRINT(("Out64Octets   %08i    ", statsCounter->Out64Octets));
	MSG_PRINT(("Out127Octets  %08i   \n", statsCounter->Out127Octets));
	MSG_PRINT(("Out255Octets  %08i    ", statsCounter->Out255Octets));
	MSG_PRINT(("Out511Octets  %08i   \n", statsCounter->Out511Octets));
	MSG_PRINT(("Out1023Octets %08i    ", statsCounter->Out1023Octets));
	MSG_PRINT(("OutMaxOctets  %08i   \n", statsCounter->OutMaxOctets));
	MSG_PRINT(("Collisions    %08i    ", statsCounter->Collisions));
	MSG_PRINT(("Late          %08i   \n", statsCounter->Late));
	MSG_PRINT(("Excessive     %08i    ", statsCounter->Excessive));
	MSG_PRINT(("Multiple      %08i   \n", statsCounter->Multiple));
	MSG_PRINT(("Single        %08i    ", statsCounter->Single));
	MSG_PRINT(("Deferred      %08i   \n", statsCounter->Deferred));
	MSG_PRINT(("OutDiscards   %08i   \n", statsCounter->OutDiscards));
}

GT_STATUS testDisplayRMONCounter(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
	GT_STATS_COUNTER_SET	statsCounterSet;

	MSG_PRINT(("Getting counters for port %i.\n", port));
		
	if((status = gstatsGetPortAllCounters(dev,port,&statsCounterSet)) != GT_OK)
	{
		MSG_PRINT(("gstatsGetPortAllCounters returned "));
		testDisplayStatus(status);
		return status;
	}

	testDisplayCounter(&statsCounterSet);
	return GT_OK;
}

GT_U32 testRMON1(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, tmpResult;
	GT_U32 testResults = 0;
	GT_STATS_COUNTERS	counter;
	GT_STATS_COUNTER_SET	statsCounterSet;
	GT_U32	statsData;
	GT_LPORT port;	
	int portIndex;

	testResult=GT_OK;

	MSG_PRINT(("\ntesting RMON Counter :\n"));
	MSG_PRINT(("RMON testing assumes no more activities in the device.\n"));

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		tmpResult = GT_OK;
		port = portIndex;

		MSG_PRINT(("Getting all counters for port %i.\n", port));
		
		if((status = gstatsGetPortAllCounters(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		MSG_PRINT(("Getting individual counter for port %i.\n", port));

		for(counter=STATS_InUnicasts; counter<=STATS_OutDiscards; counter++)
		{
			if((status = gstatsGetPortCounter(dev,port, counter, &statsData)) != GT_OK)
			{
				MSG_PRINT(("gstatsGetPortCounter returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			if (statsData != *((GT_U32*)&statsCounterSet + counter))
			{
				MSG_PRINT(("gstatsGetPortCounter(%i) mismatches gstatsGetPortAllCounter (%i : %i).\n",
							counter,statsData,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}

		}

		if(tmpResult == GT_OK)
		{
			MSG_PRINT(("Comparing counters: (PASS)\n"));
		}
		else
		{
			MSG_PRINT(("Comparing counters: (FAIL)\n"));
		}

		if (!(port % 2))
			continue;

		MSG_PRINT(("Flushing the counter for port %i\n",port));

		tmpResult = GT_OK;

		if((status = gstatsFlushPort(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gstatsFlushPort returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gstatsGetPortAllCounters(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		for(counter=STATS_InUnicasts; counter<=STATS_OutDiscards; counter++)
		{
			if (*((GT_U32*)&statsCounterSet + counter) != 0)
			{
				MSG_PRINT(("gstatsFlushPort(%i) failed (counter : %i).\n",
							counter,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}
		}

		if(tmpResult == GT_OK)
		{
			MSG_PRINT(("Flush: (PASS)\n"));
		}
		else
		{
			MSG_PRINT(("Flush: (FAIL)\n"));
		}

	}

	/* Now RMON counters of the ports with even numbers are flushed */
	MSG_PRINT(("Flushing the counters for all port\n"));
	tmpResult = GT_OK;
	if((status = gstatsFlushAll(dev)) != GT_OK)
	{
		MSG_PRINT(("gstatsFlushAll returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		if((status = gstatsGetPortAllCounters(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		for(counter=STATS_InUnicasts; counter<=STATS_OutDiscards; counter++)
		{
			if (*((GT_U32*)&statsCounterSet + counter) != 0)
			{
				MSG_PRINT(("gstatsFlushPort(%i) failed (stats : %i).\n",
							counter,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}
		}

	}			

	if(tmpResult == GT_OK)
	{
		MSG_PRINT(("Flush ALL: (PASS)\n"));
	}
	else
	{
		MSG_PRINT(("Flush ALL: (FAIL)\n"));
	}

	return testResults;
}

void testDisplayCounter2(GT_STATS_COUNTER_SET2 *statsCounter)
{
	MSG_PRINT(("InGoodOctetsHi  %08i    ", statsCounter->InGoodOctetsHi));
	MSG_PRINT(("InGoodOctetsLo  %08i   \n", statsCounter->InGoodOctetsLo));
	MSG_PRINT(("InBadOctets     %08i    ", statsCounter->InBadOctets));
	MSG_PRINT(("OutDiscards     %08i   \n", statsCounter->OutDiscards));
	MSG_PRINT(("InGoodFrames    %08i    ", statsCounter->InGoodFrames));
	MSG_PRINT(("InBadFrames     %08i   \n", statsCounter->InBadFrames));
	MSG_PRINT(("InBroadcasts    %08i    ", statsCounter->InBroadcasts));
	MSG_PRINT(("InMulticasts    %08i   \n", statsCounter->InMulticasts));
	MSG_PRINT(("64Octets        %08i    ", statsCounter->Octets64));
	MSG_PRINT(("127Octets       %08i   \n", statsCounter->Octets127));
	MSG_PRINT(("255Octets       %08i    ", statsCounter->Octets255));
	MSG_PRINT(("511Octets       %08i   \n", statsCounter->Octets511));
	MSG_PRINT(("1023Octets      %08i    ", statsCounter->Octets1023));
	MSG_PRINT(("MaxOctets       %08i   \n", statsCounter->OctetsMax));
	MSG_PRINT(("OutOctetsHi     %08i    ", statsCounter->OutOctetsHi));
	MSG_PRINT(("OutOctetsLo     %08i   \n", statsCounter->OutOctetsLo));
	MSG_PRINT(("OutFrames       %08i    ", statsCounter->OutFrames));
	MSG_PRINT(("Excessive       %08i   \n", statsCounter->Excessive));
	MSG_PRINT(("OutMulticasts   %08i    ", statsCounter->OutMulticasts));
	MSG_PRINT(("OutBroadcasts   %08i    ", statsCounter->OutBroadcasts));
	MSG_PRINT(("InBadMACCtrl    %08i    ", statsCounter->InBadMACCtrl));
	MSG_PRINT(("OutPause        %08i   \n", statsCounter->OutPause));
	MSG_PRINT(("InPause         %08i    ", statsCounter->InPause));
	MSG_PRINT(("InDiscards      %08i   \n", statsCounter->InDiscards));
	MSG_PRINT(("Undersize       %08i    ", statsCounter->Undersize));
	MSG_PRINT(("Fragments       %08i   \n", statsCounter->Fragments));
	MSG_PRINT(("Oversize        %08i    ", statsCounter->Oversize));
	MSG_PRINT(("Jabber          %08i   \n", statsCounter->Jabber));
	MSG_PRINT(("MACRcvErr       %08i    ", statsCounter->MACRcvErr));
	MSG_PRINT(("InFCSErr        %08i   \n", statsCounter->InFCSErr));
	MSG_PRINT(("Collisions      %08i    ", statsCounter->Collisions));
	MSG_PRINT(("Late            %08i   \n", statsCounter->Late));
}

GT_STATUS testDisplayRMONCounter2(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
	GT_STATS_COUNTER_SET2 statsCounterSet;

	MSG_PRINT(("Getting counters for port %i.\n", port));
		
	if((status = gstatsGetPortAllCounters2(dev,port,&statsCounterSet)) != GT_OK)
	{
		MSG_PRINT(("gstatsGetPortAllCounters2 returned "));
		testDisplayStatus(status);
		return status;
	}

	testDisplayCounter2(&statsCounterSet);
	return GT_OK;
}

GT_U32 testRMON2(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, tmpResult;
	GT_U32 testResults = 0;
	GT_STATS_COUNTERS2 counter;
	GT_STATS_COUNTER_SET2 statsCounterSet;
	GT_U32	statsData;
	GT_LPORT port;	
	int portIndex;
	TEST_API testAPI;

	testResult=GT_OK;

	/*
	 *	Histogram Mode Setup
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gstatsGetHistogramMode;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gstatsSetHistogramMode;
	if((status = testU32Type(dev,&testAPI,3)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Histogram Mode Setup API test "));
	testDisplayStatus(status);
	MSG_PRINT(("\ntesting RMON Counter :\n"));
	MSG_PRINT(("RMON testing assumes no more activities in the device.\n"));

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		tmpResult = GT_OK;
		port = portIndex;

		MSG_PRINT(("Getting all counters for port %i.\n", port));
		
		if((status = gstatsGetPortAllCounters2(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters2 returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		MSG_PRINT(("Getting individual counter for port %i.\n", port));

		for(counter=STATS2_InGoodOctetsHi; counter<=STATS2_Late; counter++)
		{
			if((status = gstatsGetPortCounter2(dev,port, counter, &statsData)) != GT_OK)
			{
				MSG_PRINT(("gstatsGetPortCounter2 returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			if (statsData != *((GT_U32*)&statsCounterSet + counter))
			{
				MSG_PRINT(("gstatsGetPortCounter2(%i) mismatches gstatsGetPortAllCounter2 (%i : %i).\n",
							counter,statsData,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}

		}

		if(tmpResult == GT_OK)
		{
			MSG_PRINT(("Comparing counters: (PASS)\n"));
		}
		else
		{
			MSG_PRINT(("Comparing counters: (FAIL)\n"));
		}

		if (!(port % 2))
			continue;

		MSG_PRINT(("Flushing the counter for port %i\n",port));

		tmpResult = GT_OK;

		if((status = gstatsFlushPort(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gstatsFlushPort returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gstatsGetPortAllCounters2(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		for(counter=STATS2_InGoodOctetsHi; counter<=STATS2_Late; counter++)
		{
			if (*((GT_U32*)&statsCounterSet + counter) != 0)
			{
				MSG_PRINT(("gstatsFlushPort(%i) failed (counter : %i).\n",
							counter,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}
		}

		if(tmpResult == GT_OK)
		{
			MSG_PRINT(("Flush: (PASS)\n"));
		}
		else
		{
			MSG_PRINT(("Flush: (FAIL)\n"));
		}

	}

	/* Now RMON counters of the ports with even numbers are flushed */
	MSG_PRINT(("Flushing the counters for all port\n"));
	tmpResult = GT_OK;
	if((status = gstatsFlushAll(dev)) != GT_OK)
	{
		MSG_PRINT(("gstatsFlushAll returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		if((status = gstatsGetPortAllCounters2(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		for(counter=STATS2_InGoodOctetsHi; counter<=STATS2_Late; counter++)
		{
			if (*((GT_U32*)&statsCounterSet + counter) != 0)
			{
				MSG_PRINT(("gstatsFlushPort(%i) failed (stats : %i).\n",
							counter,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}
		}

	}			

	if(tmpResult == GT_OK)
	{
		MSG_PRINT(("Flush ALL: (PASS)\n"));
	}
	else
	{
		MSG_PRINT(("Flush ALL: (FAIL)\n"));
	}

	return testResults;
}

void testDisplayCounter3(GT_STATS_COUNTER_SET3 *statsCounter)
{
	MSG_PRINT(("InGoodOctetsLo  %08i    ", statsCounter->InGoodOctetsLo));
	MSG_PRINT(("InGoodOctetsHi  %08i   \n", statsCounter->InGoodOctetsHi));
	MSG_PRINT(("InBadOctets     %08i    ", statsCounter->InBadOctets));
	MSG_PRINT(("OutFCSErr       %08i   \n", statsCounter->OutFCSErr));
	MSG_PRINT(("InUnicasts      %08i    ", statsCounter->InUnicasts));
	MSG_PRINT(("Deferred        %08i   \n", statsCounter->Deferred));
	MSG_PRINT(("InBroadcasts    %08i    ", statsCounter->InBroadcasts));
	MSG_PRINT(("InMulticasts    %08i   \n", statsCounter->InMulticasts));
	MSG_PRINT(("64Octets        %08i    ", statsCounter->Octets64));
	MSG_PRINT(("127Octets       %08i   \n", statsCounter->Octets127));
	MSG_PRINT(("255Octets       %08i    ", statsCounter->Octets255));
	MSG_PRINT(("511Octets       %08i   \n", statsCounter->Octets511));
	MSG_PRINT(("1023Octets      %08i    ", statsCounter->Octets1023));
	MSG_PRINT(("MaxOctets       %08i   \n", statsCounter->OctetsMax));
	MSG_PRINT(("OutOctetsLo     %08i    ", statsCounter->OutOctetsLo));
	MSG_PRINT(("OutOctetsHi     %08i   \n", statsCounter->OutOctetsHi));
	MSG_PRINT(("OutUnicasts     %08i    ", statsCounter->OutUnicasts));
	MSG_PRINT(("Excessive       %08i   \n", statsCounter->Excessive));
	MSG_PRINT(("OutMulticasts   %08i    ", statsCounter->OutMulticasts));
	MSG_PRINT(("OutBroadcasts   %08i   \n", statsCounter->OutBroadcasts));
	MSG_PRINT(("Single          %08i    ", statsCounter->Single));
	MSG_PRINT(("OutPause        %08i   \n", statsCounter->OutPause));
	MSG_PRINT(("InPause         %08i    ", statsCounter->InPause));
	MSG_PRINT(("Multiple        %08i   \n", statsCounter->Multiple));
	MSG_PRINT(("Undersize       %08i    ", statsCounter->Undersize));
	MSG_PRINT(("Fragments       %08i   \n", statsCounter->Fragments));
	MSG_PRINT(("Oversize        %08i    ", statsCounter->Oversize));
	MSG_PRINT(("Jabber          %08i   \n", statsCounter->Jabber));
	MSG_PRINT(("InMACRcvErr     %08i    ", statsCounter->InMACRcvErr));
	MSG_PRINT(("InFCSErr        %08i   \n", statsCounter->InFCSErr));
	MSG_PRINT(("Collisions      %08i    ", statsCounter->Collisions));
	MSG_PRINT(("Late            %08i   \n", statsCounter->Late));
}

GT_STATUS testDisplayRMONCounter3(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
	GT_STATS_COUNTER_SET3 statsCounterSet;

	MSG_PRINT(("Getting counters for port %i.\n", port));
		
	if((status = gstatsGetPortAllCounters3(dev,port,&statsCounterSet)) != GT_OK)
	{
		MSG_PRINT(("gstatsGetPortAllCounters3 returned "));
		testDisplayStatus(status);
		return status;
	}

	testDisplayCounter3(&statsCounterSet);
	return GT_OK;
}

GT_U32 testRMON3(GT_QD_DEV *dev)
{
	GT_STATUS status, testResult, tmpResult;
	GT_U32 testResults = 0;
	GT_STATS_COUNTERS3 counter;
	GT_STATS_COUNTER_SET3 statsCounterSet;
	GT_U32	statsData;
	GT_LPORT port;	
	int portIndex;
	TEST_API testAPI;

	testResult=GT_OK;

	/*
	 *	Histogram Mode Setup
	 */
	testAPI.getFunc.u32 = (GT_API_GET_U32)gstatsGetHistogramMode;
	testAPI.setFunc.u32 = (GT_API_SET_U32)gstatsSetHistogramMode;
	if((status = testU32Type(dev,&testAPI,3)) != GT_OK)
	{
		testResult = GT_FAIL;
		testResults |= 1 << status;
	}
	MSG_PRINT(("Histogram Mode Setup API test "));
	testDisplayStatus(status);
	MSG_PRINT(("\ntesting RMON Counter :\n"));
	MSG_PRINT(("RMON testing assumes no more activities in the device.\n"));

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		tmpResult = GT_OK;
		port = portIndex;

		MSG_PRINT(("Getting all counters for port %i.\n", port));
		
		if((status = gstatsGetPortAllCounters3(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters3 returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		MSG_PRINT(("Getting individual counter for port %i.\n", port));

		for(counter=STATS3_InGoodOctetsLo; counter<=STATS3_Late; counter++)
		{
			if((status = gstatsGetPortCounter3(dev,port, counter, &statsData)) != GT_OK)
			{
				MSG_PRINT(("gstatsGetPortCounter3 returned "));
				testDisplayStatus(status);
				testResults |= 1 << status;
				return testResults;
			}

			if (statsData != *((GT_U32*)&statsCounterSet + counter))
			{
				MSG_PRINT(("gstatsGetPortCounter3(%i) mismatches gstatsGetPortAllCounter3 (%i : %i).\n",
							counter,statsData,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}

		}

		if(tmpResult == GT_OK)
		{
			MSG_PRINT(("Comparing counters: (PASS)\n"));
		}
		else
		{
			MSG_PRINT(("Comparing counters: (FAIL)\n"));
		}

		if (!(port % 2))
			continue;

		MSG_PRINT(("Flushing the counter for port %i\n",port));

		tmpResult = GT_OK;

		if((status = gstatsFlushPort(dev,port)) != GT_OK)
		{
			MSG_PRINT(("gstatsFlushPort returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		if((status = gstatsGetPortAllCounters3(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		for(counter=STATS3_InGoodOctetsLo; counter<=STATS3_Late; counter++)
		{
			if (*((GT_U32*)&statsCounterSet + counter) != 0)
			{
				MSG_PRINT(("gstatsFlushPort(%i) failed (counter : %i).\n",
							counter,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}
		}

		if(tmpResult == GT_OK)
		{
			MSG_PRINT(("Flush: (PASS)\n"));
		}
		else
		{
			MSG_PRINT(("Flush: (FAIL)\n"));
		}

	}

	/* Now RMON counters of the ports with even numbers are flushed */
	MSG_PRINT(("Flushing the counters for all port\n"));
	tmpResult = GT_OK;
	if((status = gstatsFlushAll(dev)) != GT_OK)
	{
		MSG_PRINT(("gstatsFlushAll returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		return testResults;
	}

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		if((status = gstatsGetPortAllCounters3(dev,port,&statsCounterSet)) != GT_OK)
		{
			MSG_PRINT(("gstatsGetPortAllCounters returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			return testResults;
		}

		for(counter=STATS3_InGoodOctetsLo; counter<=STATS3_Late; counter++)
		{
			if (*((GT_U32*)&statsCounterSet + counter) != 0)
			{
				MSG_PRINT(("gstatsFlushPort(%i) failed (stats : %i).\n",
							counter,*((GT_U32*)&statsCounterSet + counter)));
				testResult = GT_FAIL;
				tmpResult = GT_FAIL;
				testResults |= 1 << tmpResult;
				continue;
			}
		}

	}			

	if(tmpResult == GT_OK)
	{
		MSG_PRINT(("Flush ALL: (PASS)\n"));
	}
	else
	{
		MSG_PRINT(("Flush ALL: (FAIL)\n"));
	}

	return testResults;
}

GT_U32 testRMON(GT_QD_DEV *dev)
{
	GT_U32 testResults;

	switch(dev->deviceId)
	{
		case GT_88E6153:
		case GT_88E6183:
			testResults = testRMON2(dev);
			break;
		case GT_88E6093:
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6065:
			testResults = testRMON3(dev);
			break;
		default:
			testResults = testRMON1(dev);
			break;
	}

	return testResults;
}

void testDisplayVTUEntry(GT_QD_DEV *dev, GT_VTU_ENTRY *vtuEntry)
{
	GT_LPORT port;	
	int portIndex;

	MSG_PRINT(("DBNum:%i, VID:%i, ",vtuEntry->DBNum,vtuEntry->vid));
	MSG_PRINT(("Tag: "));

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		MSG_PRINT(("%#x ",port,vtuEntry->vtuData.memberTagP[port]));
	}
	MSG_PRINT(("\n"));

	MSG_PRINT(("%i,%i,%i,%i,%i,%i,%i\n",
				vtuEntry->vidPriOverride,
				vtuEntry->vidPriority,
				vtuEntry->vidExInfo.useVIDFPri,
				vtuEntry->vidExInfo.vidFPri,
				vtuEntry->vidExInfo.useVIDQPri,
				vtuEntry->vidExInfo.vidQPri,
				vtuEntry->vidExInfo.vidNRateLimit
				));

}

void testDisplayVTUList(GT_QD_DEV *dev)
{
	GT_STATUS status;
    GT_VTU_ENTRY vtuEntry;

	memset(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.vid = 0xfff;
	if((status = gvtuGetEntryFirst(dev,&vtuEntry)) != GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryCount returned "));
		testDisplayStatus(status);
		return;
	}

	testDisplayVTUEntry(dev,&vtuEntry);

	while(1)
	{
		if((status = gvtuGetEntryNext(dev,&vtuEntry)) != GT_OK)
		{
			break;
		}
		testDisplayVTUEntry(dev,&vtuEntry);
	}

}

GT_STATUS testWriteVTU(GT_QD_DEV *dev, GT_VTU_ENTRY *vtuEntry, GT_U32 vid, GT_U32 DBNum, GT_U32 portVec)
{
	int i;

	vtuEntry->vid = vid & 0xFFF;
	vtuEntry->DBNum = DBNum & 0xF;
	vtuEntry->vidPriOverride = 0;
	vtuEntry->vidPriority = 0;

	for(i=0; i<16; i++)
	{
		if((portVec>>i) & 0x1)
			vtuEntry->vtuData.memberTagP[i] = 3;
		else
			vtuEntry->vtuData.memberTagP[i] = 0;
			
	}

	return GT_OK;
}

/*
	VTU APIs Test Scenario:
	1) Flush VTU entries and check Entry Count.
	2) Create a list of 16 VID entries based on arg param.
		(descending, ascending, and random order)
	3) Add a Entry.
	4) check Entry Count, call EntryFirst and EntryNext.
	5) Add 16/64/4096 more Entries.(Last entry should be failed to be added)
	6) Delete 2 valid entries and 1 invalid entry
	7) Check Entry Count, and try to find a valid entry and deleted entry.
*/

GT_STATUS testVTUCtrl(GT_QD_DEV *dev,int arg)
{
#define MAX_VTU_ENTRIES	4096
	GT_STATUS status, testResult, tmpResult;
    GT_VTU_ENTRY vtuEntry[MAX_VTU_ENTRIES+1];
    GT_VTU_ENTRY tmpVtuEntry;
	GT_U32 u32Data1, u32Data2, u32Data3, maxDbNum, priType;
	GT_BOOL found;
	int i, j, portIndex;
	GT_LPORT port;
	int maxVtuEntries;
	GT_BOOL supportMaxEntry = GT_FALSE;

	testResult = GT_OK;
	maxDbNum = 1;

	switch(dev->deviceId)
	{
		case GT_88E6021:
			maxVtuEntries = 16;
			maxDbNum = 1;
			priType =  0;	/* no VID Priority override support */
			break;
		case GT_88E6063:
		case GT_FH_VPN:
		case GT_88E6083:
			maxVtuEntries = 64;
			maxDbNum = 16;
			priType =  0;	/* no VID Priority override support */
			break;
		case GT_88E6153:
		case GT_88E6183:
		case GT_88E6093:
			maxVtuEntries = 64;
			maxDbNum = 16;
			supportMaxEntry = GT_TRUE;
			priType =  1;	/* VID Priority override support */
			break;
		case GT_88E6031:
		case GT_88E6061:
			maxVtuEntries = 16;
			maxDbNum = 16;
			priType =  2;	/* Extended VID Priority override support */
			break;
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6065:
			maxVtuEntries = 64;
			maxDbNum = 64;
			priType =  3;	/* Extended VID Priority override support */
			break;
		case GT_88E6095:
		case GT_88E6092:
		case GT_88E6152:
		case GT_88E6155:
		case GT_88E6182:
		case GT_88E6185:
		case GT_88E6131:
		case GT_88E6108:
			maxVtuEntries = 64;
			maxDbNum = 256;
			supportMaxEntry = GT_TRUE;
			priType =  1;	/* VID Priority override support */
			break;
		default:
			maxVtuEntries = 16;
			priType =  0;	/* no VID Priority override support */
			break;
	}

	/* 1) Flush VTU entries and check Entry Count */
	MSG_PRINT(("Flushing VTU entries: "));
	if((status = gvtuFlush(dev)) != GT_OK)
	{
		MSG_PRINT(("gvtuFlush returned "));
		testDisplayStatus(status);
		return status;
	}

	if((status = gvtuGetEntryCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryCount returned "));
		testDisplayStatus(status);
		return status;
	}

	if(u32Data1 != 0)
	{
		MSG_PRINT(("\nEntryCount %i (Failed, should be 0)\n",u32Data1));
		testResult = GT_FAIL;
	}
	else
	{
		MSG_PRINT(("(PASS)\n"));
	}

	/*
		2) Create a list of 16 VID entries based on arg param.
			(descending, ascending, and random order)
	*/
	MSG_PRINT(("Creating a list of %i VID\n",maxVtuEntries+1));
	srand((unsigned)time(NULL));
	switch (arg)
	{
		case 0: /* Ascending order */
			/* check if it supports MX_VTU_ENTRIES */
			if (maxVtuEntries == MAX_VTU_ENTRIES)
			{
				for(i=0; i<maxVtuEntries+1; i++)
				{
					memset(&vtuEntry[i],0,sizeof(GT_VTU_ENTRY));
					vtuEntry[i].DBNum = 1;
					vtuEntry[i].vid = i;
					for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
					{
						port = portIndex;

						vtuEntry[i].vtuData.memberTagP[port] = (vtuEntry[i].vid+port)%4;
					}
					switch(priType)
					{
						case 0:
							break;
						case 1:
							vtuEntry[i].vidPriOverride = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							if(vtuEntry[i].vidPriOverride == GT_TRUE)
								vtuEntry[i].vidPriority = (GT_U8)(rand() & 0x7);
							break;
						case 2:
							vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							break;
						case 3:
							vtuEntry[i].vidExInfo.useVIDFPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							if (vtuEntry[i].vidExInfo.useVIDFPri == GT_TRUE)
								vtuEntry[i].vidExInfo.vidFPri = (GT_U8)(rand() & 0x7);
							vtuEntry[i].vidExInfo.useVIDQPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							if (vtuEntry[i].vidExInfo.useVIDQPri == GT_TRUE)
								vtuEntry[i].vidExInfo.vidQPri = (GT_U8)(rand() & 0x3);
							vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							break;
						default:
							break;
					}
				}
				break;
			}

			u32Data1 = (rand()%1024) + 1;
			u32Data2 = (rand()%32) + 1;
			for(i=0; i<maxVtuEntries+1; i++)
			{
				memset(&vtuEntry[i],0,sizeof(GT_VTU_ENTRY));
				vtuEntry[i].DBNum = 1;
				vtuEntry[i].vid = u32Data1 + u32Data2 * i;
				for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
				{
					port = portIndex;

					vtuEntry[i].vtuData.memberTagP[port] = (vtuEntry[i].vid+port)%4;
				}
				switch(priType)
				{
					case 0:
						break;
					case 1:
						vtuEntry[i].vidPriOverride = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if(vtuEntry[i].vidPriOverride == GT_TRUE)
							vtuEntry[i].vidPriority = (GT_U8)(rand() & 0x7);
						break;
					case 2:
						vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						break;
					case 3:
						vtuEntry[i].vidExInfo.useVIDFPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if (vtuEntry[i].vidExInfo.useVIDFPri == GT_TRUE)
							vtuEntry[i].vidExInfo.vidFPri = (GT_U8)(rand() & 0x7);
						vtuEntry[i].vidExInfo.useVIDQPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if (vtuEntry[i].vidExInfo.useVIDQPri == GT_TRUE)
							vtuEntry[i].vidExInfo.vidQPri = (GT_U8)(rand() & 0x3);
						vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						break;
					default:
						break;
				}
			}
			break;

		case 1: /* Descending order */
			/* check if it supports MX_VTU_ENTRIES */
			if (maxVtuEntries == MAX_VTU_ENTRIES)
			{
				for(i=0; i<maxVtuEntries; i++)
				{
					memset(&vtuEntry[i],0,sizeof(GT_VTU_ENTRY));
					vtuEntry[i].DBNum = 2;
					vtuEntry[i].vid = maxVtuEntries - i;
					for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
					{
						port = portIndex;

						vtuEntry[i].vtuData.memberTagP[port] = (vtuEntry[i].vid+port)%4;
					}
					switch(priType)
					{
						case 0:
							break;
						case 1:
							vtuEntry[i].vidPriOverride = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							if(vtuEntry[i].vidPriOverride == GT_TRUE)
								vtuEntry[i].vidPriority = (GT_U8)(rand() & 0x7);
							break;
						case 2:
							vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							break;
						case 3:
							vtuEntry[i].vidExInfo.useVIDFPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							if (vtuEntry[i].vidExInfo.useVIDFPri == GT_TRUE)
								vtuEntry[i].vidExInfo.vidFPri = (GT_U8)(rand() & 0x7);
							vtuEntry[i].vidExInfo.useVIDQPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							if (vtuEntry[i].vidExInfo.useVIDQPri == GT_TRUE)
								vtuEntry[i].vidExInfo.vidQPri = (GT_U8)(rand() & 0x3);
							vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
							break;
						default:
							break;
					}
				}
				memset(&vtuEntry[i],0,sizeof(GT_VTU_ENTRY));
				vtuEntry[i].DBNum = 2;
				vtuEntry[i].vid = 5;	/* choose any value. this entry is only for checking error. */
				for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
				{
					port = portIndex;

					vtuEntry[i].vtuData.memberTagP[port] = (vtuEntry[i].vid+port)%4;
				}

				break;
			
			}
			u32Data1 = (rand()%1024) + 1;
			u32Data2 = (rand()%32) + 1;
			for(i=0; i<maxVtuEntries+1; i++)
			{
				memset(&vtuEntry[maxVtuEntries-i],0,sizeof(GT_VTU_ENTRY));
				vtuEntry[maxVtuEntries-i].DBNum = 2;
				vtuEntry[maxVtuEntries-i].vid = u32Data1 + u32Data2 * i;
				for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
				{
					port = portIndex;

					vtuEntry[maxVtuEntries-i].vtuData.memberTagP[port] = (vtuEntry[maxVtuEntries-i].vid+port)%4;
				}
				switch(priType)
				{
					case 0:
						break;
					case 1:
						vtuEntry[i].vidPriOverride = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if(vtuEntry[i].vidPriOverride == GT_TRUE)
							vtuEntry[i].vidPriority = (GT_U8)(rand() & 0x7);
						break;
					case 2:
						vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						break;
					case 3:
						vtuEntry[i].vidExInfo.useVIDFPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if (vtuEntry[i].vidExInfo.useVIDFPri == GT_TRUE)
							vtuEntry[i].vidExInfo.vidFPri = (GT_U8)(rand() & 0x7);
						vtuEntry[i].vidExInfo.useVIDQPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if (vtuEntry[i].vidExInfo.useVIDQPri == GT_TRUE)
							vtuEntry[i].vidExInfo.vidQPri = (GT_U8)(rand() & 0x3);
						vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						break;
					default:
						break;
				}
			}
			break;

		default: /* random order */
			for(i=0; i<maxVtuEntries+1; i++)
			{
				memset(&vtuEntry[i],0,sizeof(GT_VTU_ENTRY));
				vtuEntry[i].DBNum = (maxDbNum > i)?(maxDbNum-i)%maxDbNum:(i-maxDbNum)%maxDbNum;
				vtuEntry[i].vid = (rand() & 0xF) | ((rand() & 0xF) << 4) | ((rand() & 0xF) << 8);
				for(j=0; j<i; j++)
				{
					if(vtuEntry[j].vid == vtuEntry[i].vid)
					{
						vtuEntry[i].vid = (rand() & 0xF) | ((rand() & 0xF) << 4) | ((rand() & 0xF) << 8);
						j = -1;
					}
				}

				for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
				{
					port = portIndex;

					vtuEntry[i].vtuData.memberTagP[port] = (vtuEntry[i].vid+port)%4;
				}
				switch(priType)
				{
					case 0:
						break;
					case 1:
						vtuEntry[i].vidPriOverride = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if(vtuEntry[i].vidPriOverride == GT_TRUE)
							vtuEntry[i].vidPriority = (GT_U8)(rand() & 0x7);
						break;
					case 2:
						vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						break;
					case 3:
						vtuEntry[i].vidExInfo.useVIDFPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if (vtuEntry[i].vidExInfo.useVIDFPri == GT_TRUE)
							vtuEntry[i].vidExInfo.vidFPri = (GT_U8)(rand() & 0x7);
						vtuEntry[i].vidExInfo.useVIDQPri = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						if (vtuEntry[i].vidExInfo.useVIDQPri == GT_TRUE)
							vtuEntry[i].vidExInfo.vidQPri = (GT_U8)(rand() & 0x3);
						vtuEntry[i].vidExInfo.vidNRateLimit = (rand() & 0x1)?GT_TRUE:GT_FALSE;
						break;
					default:
						break;
				}
			}
			break;

	}			 

#if 0	/* display VTU entries */
	for(i=0; i<maxVtuEntries+1; i++)
		testDisplayVTUEntry(dev,&vtuEntry[i]);
#endif

	/*
		3) Add a Entry.
	*/
	MSG_PRINT(("Adding a Entry: "));
	if((status = gvtuAddEntry(dev,&vtuEntry[0])) != GT_OK)
	{
		MSG_PRINT(("gvtuAddEntry returned "));
		testDisplayStatus(status);
		return status;
	}

	/*
		4) check Entry Count, call EntryFirst and EntryNext
	*/
	if((status = gvtuGetEntryCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryCount returned "));
		testDisplayStatus(status);
		return status;
	}

	if(u32Data1 != 1)
	{
		MSG_PRINT(("\nEntryCount %i (Failed, should be 1)\n",u32Data1));
		testResult = GT_FAIL;
	}
	else
	{
		MSG_PRINT(("(PASS)\n"));
	}

	MSG_PRINT(("Getting the first Entry: "));
	memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = 0xfff;
	if((status = gvtuGetEntryFirst(dev,&tmpVtuEntry)) != GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryCount returned "));
		testDisplayStatus(status);
		return status;
	}

	if(memcmp(&tmpVtuEntry,&vtuEntry[0],sizeof(GT_VTU_ENTRY)) != 0)
	{
		MSG_PRINT(("Unexpected VTU entry\n"));
		testDisplayVTUEntry(dev,&tmpVtuEntry);
		testResult = GT_FAIL;
	}
	else
	{
		MSG_PRINT(("(PASS)\n"));
	}

	if((status = gvtuGetEntryNext(dev,&tmpVtuEntry)) == GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryNext should returned "));
		testDisplayStatus(status);
		return status;
	}

	/*
		5) Add 16 more Entries.(Last entry should be failed to be added)
	*/
	MSG_PRINT(("Adding %i VTU entries: ",maxVtuEntries+1));
	for(i=1; i<maxVtuEntries; i++)
	{
		if((status = gvtuAddEntry(dev,&vtuEntry[i])) != GT_OK)
		{
			MSG_PRINT(("gvtuAddEntry returned "));
			testDisplayStatus(status);
			MSG_PRINT(("Failed VID : %i\n",vtuEntry[i].vid));
			MSG_PRINT(("Number of Entries should be in VTU : %i\n",i));
			return status;
		}
	}

	if(supportMaxEntry != GT_TRUE)
	{
		if((status = gvtuAddEntry(dev,&vtuEntry[maxVtuEntries])) == GT_OK)
		{
			MSG_PRINT(("gvtuAddEntry should return "));
			testDisplayStatus(GT_FAIL);
			for(i=0; i<maxVtuEntries+1; i++)
				testDisplayVTUEntry(dev,&vtuEntry[i]);
			return GT_FAIL;
		}
	}
	MSG_PRINT(("(PASS)\n"));

	/*
		6) Delete 3 valid entries and 1 invalid entry
	*/
	MSG_PRINT(("Deleting entries: "));
	u32Data1 = 0;
	u32Data2 = 12;
	u32Data3 = maxVtuEntries-1;
	if((status = gvtuDelEntry(dev,&vtuEntry[u32Data1])) != GT_OK)
	{
		MSG_PRINT(("gvtuDelEntry returned "));
		testDisplayStatus(status);
		return status;
	}
	if((status = gvtuDelEntry(dev,&vtuEntry[u32Data2])) != GT_OK)
	{
		MSG_PRINT(("gvtuDelEntry returned "));
		testDisplayStatus(status);
		return status;
	}
	if((status = gvtuDelEntry(dev,&vtuEntry[u32Data3])) != GT_OK)
	{
		MSG_PRINT(("gvtuDelEntry returned "));
		testDisplayStatus(status);
		return status;
	}
#if 0
	if((status = gvtuDelEntry(dev,&vtuEntry[maxVtuEntries])) == GT_OK)
	{
		MSG_PRINT(("gvtuDelEntry should not return "));
		testDisplayStatus(status);
		return status;
	}
#endif
	/*
		7) Check Entry Count, and try to find a valid entry and deleted entry.
	*/

	MSG_PRINT(("Checking Entry count: "));
	if((status = gvtuGetEntryCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryCount returned "));
		testDisplayStatus(status);
		return status;
	}

	if(u32Data1 != maxVtuEntries-3)
	{
		MSG_PRINT(("EntryCount %i (Failed, should be %i)\n",u32Data1,maxVtuEntries-3));
		testResult = GT_FAIL;
	}
	else
	{
		MSG_PRINT(("(PASS)\n"));
	}

	MSG_PRINT(("Finding entries: "));

	tmpResult = GT_OK;

	memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vtuEntry[2].vid;
	if((status = gvtuFindVidEntry(dev,&tmpVtuEntry, &found)) != GT_OK)
	{
		MSG_PRINT(("gvtuFindVidEntry returned "));
		testDisplayStatus(status);
		return status;
	}

	if (found != GT_TRUE)
	{
		MSG_PRINT(("gvtuFindVidEntry returned OK with Found not true\n"));
		testResult = GT_FAIL;
		tmpResult = GT_FAIL;
	}

	if(memcmp(&tmpVtuEntry,&vtuEntry[2],sizeof(GT_VTU_ENTRY)) != 0)
	{
		MSG_PRINT(("Unexpected VTU entry (%i)\n",2));
		testDisplayVTUEntry(dev,&tmpVtuEntry);
		testResult = GT_FAIL;
		tmpResult = GT_FAIL;
	}

	memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vtuEntry[14].vid;
	if((status = gvtuFindVidEntry(dev,&tmpVtuEntry, &found)) != GT_OK)
	{
		MSG_PRINT(("gvtuFindVidEntry returned "));
		testDisplayStatus(status);
		return status;
	}

	if (found != GT_TRUE)
	{
		MSG_PRINT(("gvtuFindVidEntry returned OK with Found not true\n"));
		testResult = GT_FAIL;
		tmpResult = GT_FAIL;
	}

	if(memcmp(&tmpVtuEntry,&vtuEntry[14],sizeof(GT_VTU_ENTRY)) != 0)
	{
		MSG_PRINT(("Unexpected VTU entry (%i)\n", 14));
		testDisplayVTUEntry(dev,&tmpVtuEntry);
		testResult = GT_FAIL;
		tmpResult = GT_FAIL;
	}

	memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vtuEntry[4].vid;
	if((status = gvtuFindVidEntry(dev,&tmpVtuEntry, &found)) != GT_OK)
	{
		MSG_PRINT(("gvtuFindVidEntry returned "));
		testDisplayStatus(status);
		return status;
	}

	if (found != GT_TRUE)
	{
		MSG_PRINT(("gvtuFindVidEntry returned OK with Found not true\n"));
		testResult = GT_FAIL;
		tmpResult = GT_FAIL;
	}

	if(memcmp(&tmpVtuEntry,&vtuEntry[4],sizeof(GT_VTU_ENTRY)) != 0)
	{
		MSG_PRINT(("Unexpected VTU entry (%i)\n", 4));
		testDisplayVTUEntry(dev,&tmpVtuEntry);
		testResult = GT_FAIL;
		tmpResult = GT_FAIL;
	}

	/* try to find deleted entry */
	MSG_PRINT(("Find Deleted Entry... "));
	memset(&tmpVtuEntry,0,sizeof(GT_VTU_ENTRY));
	tmpVtuEntry.vid = vtuEntry[u32Data2].vid;
	if((status = gvtuFindVidEntry(dev,&tmpVtuEntry, &found)) == GT_OK)
	{
		if(found == GT_TRUE)
		{
			MSG_PRINT(("gvtuFindVidEntry found a deleted entry.\n"));
			testDisplayVTUEntry(dev,&tmpVtuEntry);
			testResult = GT_FAIL;
			tmpResult = GT_FAIL;
		}
	}

	if(tmpResult == GT_OK)
		MSG_PRINT(("(PASS)\n"));

	return testResult;
}

GT_U32 testVTU(GT_QD_DEV *dev)
{
	GT_STATUS testResult, status;
	GT_U32 testResults = 0;
	int arg;

	testResult = GT_OK;

	for(arg=0; arg<5; arg++)
	{
		if((status=testVTUCtrl(dev,arg)) != GT_OK)
		{
			MSG_PRINT(("VTU Test Fail(%d) with arg %i\n", status,arg));
			testResults |= 1 << status;
			testResult = GT_FAIL;
		}
		else
		{
			MSG_PRINT(("VTU Test Pass with arg %i\n", arg));
		}
		MSG_PRINT((" \n"));
	}

	return testResults;
}

GT_U32 testSysStatus(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_U32 testResults = 0;
	GT_BOOL mode;
	GT_U16	data;
	GT_U32	u32data;

	MSG_PRINT(("Get Switch Mode\n"));

	if((status = gsysGetSW_Mode(dev,&mode)) != GT_OK)
	{
		MSG_PRINT(("gsysGetSW_Mode returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("Switch Mode : %i\n",(int)mode));

	MSG_PRINT(("Get Init Ready\n"));

	if((status = gsysGetInitReady(dev,&mode)) != GT_OK)
	{
		MSG_PRINT(("gsysGetInitReady returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("Init Ready : %i\n",(int)mode));

	MSG_PRINT(("Get Free Q Size\n"));

	if((status = gsysGetFreeQSize(dev,&data)) != GT_OK)
	{
		MSG_PRINT(("gsysGetFreeQSize returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("Free QSize : %i\n",(int)data));

	if((status = gsysGetPPUState(dev,(GT_PPU_STATE*)&u32data)) != GT_OK)
	{
		MSG_PRINT(("gsysGetPPUState returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("PPU State  : %i\n",(int)u32data));

#ifdef DEBUG_FEATURE
	MSG_PRINT(("Get QC Pointer Collision\n"));

	if((status = gsysGetPtrCollision(dev,&mode)) != GT_OK)
	{
		MSG_PRINT(("gsysGetPtrCollision returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("QC Pointer Collision : %i\n",(int)mode));

	MSG_PRINT(("Get Dest. PortVector Corrupt\n"));

	if((status = gsysGetDpvCorrupt(dev,&mode)) != GT_OK)
	{
		MSG_PRINT(("gsysGetDpvCorrupt returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("Dest. PortVector Corrupt : %i\n",(int)mode));

	MSG_PRINT(("Get Missing Pointer Error\n"));

	if((status = gsysGetMissingPointers(dev,&mode)) != GT_OK)
	{
		MSG_PRINT(("gsysGetMissingPointers returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
		if (status == GT_FAIL)
			return testResults;
	}
	MSG_PRINT(("Missing Pointer Error : %i\n",(int)mode));
#endif

	return testResults;
}

GT_U32 fillupPIRLData(GT_QD_DEV *dev, GT_PIRL_DATA *pdata, GT_U32 *vec)
{
	GT_U32	data;
	GT_LPORT port;
	GT_BOOL	restrict;

	switch (dev->deviceId)
	{
		case GT_88E6031:
		case GT_88E6061:
			restrict = GT_TRUE;
			break;
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6065:
			restrict = GT_FALSE;
			break;
		default:
			return GT_NOT_SUPPORTED;
	}

	/* Ingress Rate */
	data = rand() % 200001;
	if(data == 0)
		data = 64;
	else if(data < 1000)
		data = data - (data % 64);
	else if(data < 100000)
		data = data - (data % 1000);
	else
		data = data - (data % 10000);

	pdata->ingressRate = data;

	pdata->accountQConf = (rand() & 0x1)?GT_TRUE:GT_FALSE;
	pdata->accountFiltered = (rand() & 0x1)?GT_TRUE:GT_FALSE;
	pdata->ebsLimitAction = (rand() & 0x1);
	if (restrict)
	{
		pdata->bktRateType = 0;
		pdata->bktTypeMask = (rand() & 0xF);
	}
	else
	{
		pdata->bktRateType = (rand() & 0x1);
		if (pdata->bktRateType == 0)
			pdata->bktTypeMask = (rand() & 0x7F);
		else
			pdata->bktTypeMask = 0;
	}

	pdata->byteTobeCounted = (rand() % 3);

	*vec = rand() & ((1<<dev->numOfPorts) - 1);
	if(*vec == 0)
		*vec = 0x7;

	if(pdata->ebsLimitAction == 0)
	{
		return GT_OK;
	}

	for(port=0; port<dev->numOfPorts; port++)
	{
		if(*vec & (1 << port))
			pdata->fcDeassertMode[port] = port & 0x1;
	}

	return GT_OK;
}

GT_U32 testPIRL(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_U32 testResults = 0;
	GT_U32	i, portVec[12], tmpPortVec, pirlSize;
	GT_PIRL_DATA	pirlData[12], tmpPirlData;
	
	srand((unsigned)time(NULL));

	memset(&pirlData[0],0,sizeof(GT_PIRL_DATA));

	switch (dev->deviceId)
	{
		case GT_88E6031:
			pirlSize = 3;
			break;
		case GT_88E6061:
		case GT_88E6035:
			pirlSize = 6;
			break;
		case GT_88E6055:
		case GT_88E6065:
			pirlSize = 12;
			break;
		default:
			return 1 << GT_NOT_SUPPORTED;
	}


	MSG_PRINT(("Try Invalid PIRL API call\n"));

	/* try PIRL APIs without Activating it */

	pirlData[0].ingressRate = 64;

	if((status = gpirlUpdateParam(dev, 0, &pirlData[0])) == GT_OK)
	{
		MSG_PRINT(("gpirlUpdateParam returned "));
		testDisplayStatus(status);
		testResults |= 1 << GT_FAIL;
	}
	if (status == GT_NOT_SUPPORTED)
		return status;

	portVec[0] = 0x7;

	if((status = gpirlUpdatePortVec(dev, 0, portVec[0])) == GT_OK)
	{
		MSG_PRINT(("gpirlUpdateParam returned "));
		testDisplayStatus(status);
		testResults |= 1 << GT_FAIL;
	}

	if (testResults)
	{
		MSG_PRINT(("Invalid PIRL API call failed\n"));
	}
	else
	{
		MSG_PRINT(("Invalid PIRL API call successed\n"));
	}


	/* Activate PIRL */
	MSG_PRINT(("Filling up the PIRL Data...\n"));
	for(i=0; i<pirlSize; i++)
	{
		memset(&pirlData[i],0,sizeof(GT_PIRL_DATA));
		fillupPIRLData(dev, &pirlData[i], &portVec[i]);
	}		

	MSG_PRINT(("Activate Pirl...\n"));
	for(i=0; i<pirlSize; i++)
	{
		if((status = gpirlActivate(dev,i,portVec[i],&pirlData[i])) != GT_OK)
		{
			MSG_PRINT(("gpirlActivate returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
		}
		if (status == GT_FAIL)
			return testResults;
	}	

	MSG_PRINT(("Comparing...\n"));
	for(i=0; i<pirlSize; i++)
	{
		memset(&tmpPirlData,0,sizeof(GT_PIRL_DATA));
		if((status = gpirlReadParam(dev,i,&tmpPirlData)) != GT_OK)
		{
			MSG_PRINT(("gpirlReadParam returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
			continue;
		}

		if(memcmp(&tmpPirlData, &pirlData[i], sizeof(GT_PIRL_DATA)))
		{
			MSG_PRINT(("gpirlReadParam returned wrong entry (unit %i).",i));
			dumpMemory((char*)&tmpPirlData, sizeof(GT_PIRL_DATA));
			MSG_PRINT(("Expected entry:"));
			dumpMemory((char*)&pirlData[i], sizeof(GT_PIRL_DATA));
			testResults |= 1 << GT_FAIL;
		}

		if((status = gpirlReadPortVec(dev,i,&tmpPortVec)) != GT_OK)
		{
			MSG_PRINT(("gpirlReadParam returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
		}

		if(tmpPortVec != portVec[i])
		{
			MSG_PRINT(("gpirlReadPortVec returned wrong data (unit %i) : %#x.\n",i,tmpPortVec));
			MSG_PRINT(("Expected vector: %#x\n",portVec[i]));
			testResults |= 1 << GT_FAIL;
		}
	}

	i = 1;	/* IRL Unit number to be played with */

	MSG_PRINT(("Update Unit %i's IRL Data.\n",i));

	memset(&pirlData[i],0,sizeof(GT_PIRL_DATA));
	fillupPIRLData(dev, &pirlData[i], &portVec[i]);
	
	if((status = gpirlUpdateParam(dev,i,&pirlData[i])) != GT_OK)
	{
		MSG_PRINT(("gpirlUpdateParam returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
	}
	if (status == GT_FAIL)
		return testResults;
	
	if((status = gpirlUpdatePortVec(dev,i,portVec[i])) != GT_OK)
	{
		MSG_PRINT(("gpirlUpdatePortVec returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
	}
	if (status == GT_FAIL)
		return testResults;
	
	MSG_PRINT(("Comparing Updated data...\n"));

	memset(&tmpPirlData,0,sizeof(GT_PIRL_DATA));
	if((status = gpirlReadParam(dev,i,&tmpPirlData)) != GT_OK)
	{
		MSG_PRINT(("gpirlReadParam returned "));
		testDisplayStatus(status);
		testResults |= 1 << status;
	}

	if(memcmp(&tmpPirlData, &pirlData[i], sizeof(GT_PIRL_DATA)))
	{
		MSG_PRINT(("gpirlReadParam returned wrong entry (unit %i).",i));
		dumpMemory((char*)&tmpPirlData, sizeof(GT_PIRL_DATA));
		MSG_PRINT(("Expected entry:"));
		dumpMemory((char*)&pirlData[i], sizeof(GT_PIRL_DATA));
		testResults |= 1 << GT_FAIL;
	}

	if((status = gpirlReadPortVec(dev,i,&tmpPortVec)) != GT_OK)
	{
		MSG_PRINT(("gpirlReadParam returned \n"));
		testDisplayStatus(status);
		testResults |= 1 << status;
	}

	if(tmpPortVec != portVec[i])
	{
		MSG_PRINT(("gpirlReadPortVec returned wrong data (unit %i) : %#x.\n",i,tmpPortVec));
		MSG_PRINT(("Expected vector: %#x\n",portVec[i]));
		testResults |= 1 << GT_FAIL;
	}

	MSG_PRINT(("Deactivate Pirl...\n"));
	for(i=0; i<pirlSize; i++)
	{
		if((status = gpirlDeactivate(dev,i)) != GT_OK)
		{
			MSG_PRINT(("gpirlDeactivate returned "));
			testDisplayStatus(status);
			testResults |= 1 << status;
		}
		if (status == GT_FAIL)
			return testResults;
	}	

	return testResults;
}


GT_STATUS testPartialAll(GT_QD_DEV *dev, GT_U32 tests)
{
	int i,j;
	GT_STATUS testResult = GT_OK;
	GT_U32	testResults[32];
	
	i = 0;
	while(1)
	{
		if (testStruct[i].testFunc == NULL)
			break;

		if (!(tests & (1<<i)))
		{
			i++;
			continue;
		}

		testResults[i] = testStruct[i].testFunc(dev);
		i++;
	}

	printf("\nTest Result\n");
	i = 0;
	while(1)
	{
		if (testStruct[i].testFunc == NULL)
			break;

		if (!(tests & (1<<i)))
		{
			i++;
			continue;
		}

		printf("%s ",testStruct[i].strTest);
		
		if(testResults[i] == 0)
		{
		 	printf("PASS\n");
			i++;
			continue;
		}
		for(j=0;j<32;j++)
		{
			if(testResults[i] & (1<<j))
			{
				switch(j)
				{
					case GT_OK:
						printf("PASS ");
						break;
					case GT_FAIL :
						printf("one or more Failure ");
						break;
					case GT_NOT_SUPPORTED :
						printf("one or more Not Supported ");
						break;
					default:
						printf("one or more FAIL ");
						break;
				}
			}
		}
		printf("\n");
		i++;
	}

	return testResult;
}


GT_STATUS testAll(GT_QD_DEV *dev)
{
	int i,j;
	GT_STATUS testResult = GT_OK;
	
	i = 0;
	while(1)
	{
		if (testStruct[i].testFunc == NULL)
			break;

		MSG_PRINT(("\nTesting %s\n",testStruct[i].strTest));
		
		testStruct[i].testResults = testStruct[i].testFunc(dev);
		i++;
	}

	MSG_PRINT(("\nTest Result\n"));
	i = 0;
	while(1)
	{
		if (testStruct[i].testFunc == NULL)
			break;

		MSG_PRINT(("%s ",testStruct[i].strTest));
		
		if(testStruct[i].testResults == 0)
		{
		 	MSG_PRINT(("PASS\n"));
			i++;
			continue;
		}
		for(j=0;j<32;j++)
		{
			if(testStruct[i].testResults & (1<<j))
			{
				switch(j)
				{
					case GT_OK:
						MSG_PRINT(("PASS "));
						break;
					case GT_FAIL :
						MSG_PRINT(("one or more Failure "));
						break;
					case GT_NOT_SUPPORTED :
						MSG_PRINT(("one or more Not Supported "));
						break;
					default:
						MSG_PRINT(("one or more FAIL "));
						break;
				}
			}
		}
		MSG_PRINT(("\n"));
		i++;
	}

	return testResult;
}


/*
 * Start Packet Generator.
 * Input:
 *      pktload - enum GT_PG_PAYLOAD (GT_PG_PAYLOAD_RANDOM or GT_PG_PAYLOAD_5AA5)
 *      length  - enum GT_PG_LENGTH  (GT_PG_LENGTH_64 or GT_PG_LENGTH_1514)
 *      tx      - enum GT_PG_TX      (GT_PG_TX_NORMAL or GT_PG_TX_ERROR)
*/
GT_STATUS testStartPktGen
(
    GT_QD_DEV      *dev,
    GT_LPORT       port,
    GT_PG_PAYLOAD  payload,
    GT_PG_LENGTH   length,
    GT_PG_TX       tx
)
{
    GT_STATUS status;
    GT_PG     pktInfo;

    if (dev == 0)
    {
        MSG_PRINT(("GT driver is not initialized\n"));
        return GT_FAIL;
    }

    MSG_PRINT(("Start Packet Generator for port %i\n",(int)port));

    pktInfo.payload = payload; /* Pseudo-random, 5AA55AA5... */
    pktInfo.length = length;   /* 64 bytes, 1514 bytes */
    pktInfo.tx = tx;           /* normal packet, error packet */

    /*
     *	Start Packet Generator
    */
    if((status = gprtSetPktGenEnable(dev,port,GT_TRUE,&pktInfo)) != GT_OK)
    {
        MSG_PRINT(("gprtSetPktGenEnable return Failed\n"));
        return status;
    }

    return GT_OK;
}


/*
 * Stop Packet Generator.
 */
GT_STATUS testStopPktGen(GT_QD_DEV *dev,GT_LPORT port)
{
    GT_STATUS status;

    if (dev == 0)
    {
        MSG_PRINT(("GT driver is not initialized\n"));
        return GT_FAIL;
    }

    MSG_PRINT(("Stopping Packet Generator for port %i\n",(int)port));

    /*
     *	Start Packet Generator
    */
    if((status = gprtSetPktGenEnable(dev,port,GT_FALSE,NULL)) != GT_OK)
    {
        MSG_PRINT(("gprtSetPktGenEnable return Failed\n"));
        return status;
    }

    return GT_OK;
}

void testDisplayCableTestResult
(
	GT_TEST_STATUS *cableStatus, 
	GT_CABLE_LEN *cableLen
)
{
	switch(*cableStatus)
	{
		case GT_TEST_FAIL:
			MSG_PRINT(("Cable Test Failed\n"));
			break;
		case GT_NORMAL_CABLE:
			MSG_PRINT(("Cable Test Passed. No problem found.\n"));
			switch(cableLen->normCableLen)
			{
				case GT_LESS_THAN_50M:
					MSG_PRINT(("Cable Length is less than 50M.\n"));
					break;
				case GT_50M_80M:
					MSG_PRINT(("Cable Length is between 50M and 80M.\n"));
					break;
				case GT_80M_110M:
					MSG_PRINT(("Cable Length is between 80M and 110M.\n"));
					break;
				case GT_110M_140M:
					MSG_PRINT(("Cable Length is between 110M and 140M.\n"));
					break;
				case GT_MORE_THAN_140:
					MSG_PRINT(("Cable Length is over 140M.\n"));
					break;
				default:
					MSG_PRINT(("Cable Length is unknown.\n"));
					break;
			}
			break;
		case GT_OPEN_CABLE:
			MSG_PRINT(("Cable Test Passed. Cable is open.\n"));
			MSG_PRINT(("Approximatly %i meters from the tested port.\n",cableLen->errCableLen));
			break;
		case GT_SHORT_CABLE:
			MSG_PRINT(("Cable Test Passed. Cable is short.\n"));
			MSG_PRINT(("Approximatly %i meters from the tested port.\n",cableLen->errCableLen));
			break;
		default:
			MSG_PRINT(("Unknown Test Result.\n"));
			break;
	}
}

GT_STATUS dumpATUInfo(ATU_ENTRY_INFO *atuInfo, int entry)
{
	int i;

	for(i=0; i<entry; i++)
	{
		printATUEntry(&atuInfo->atuEntry);
		atuInfo++;
	}
	return GT_OK;
}

GT_STATUS testWriteATU(GT_QD_DEV *dev,GT_U8 atuSize,GT_U8 dbNum,GT_U32 entryState,GT_U32 macHiAddr,GT_U32 entries)
{
	GT_STATUS status;
	ATU_ENTRY_INFO *atuEntry;
	GT_U16 maxMacs, i;

	if(atuSize >= 5)
		return GT_FAIL;

	if(entries == 0)
		maxMacs = 256 << atuSize;
	else
		maxMacs = entries;

	if (gAtuEntry == NULL)
		gAtuEntry = (ATU_ENTRY_INFO *)malloc(sizeof(ATU_ENTRY_INFO)*4096);

	atuEntry = gAtuEntry;

	gtMemSet(atuEntry,0,sizeof(ATU_ENTRY_INFO)*maxMacs);

	MSG_PRINT(("Getting ATU List(%i).\n",maxMacs));
	if((status=testFillUpAtu(dev,atuEntry,atuSize,dbNum,(GT_U16)macHiAddr,entryState)) != GT_OK)
	{
		MSG_PRINT(("testFillUpAtu returned "));
		testDisplayStatus(status);
		return status;
	}

	MSG_PRINT(("Writing ATU List(%i).\n",maxMacs));
	for(i=0; i<maxMacs; i++)
	{
		if((status = gfdbAddMacEntry(dev,&atuEntry[i].atuEntry)) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned "));
			testDisplayStatus(status);
			return status;
		}
	}

	return GT_OK;
}

GT_STATUS runRWDir(GT_QD_DEV *dev, int port, int reg, int iter, int startV, int endV, int timeout)
{
	int i;
	GT_U32 data, tmpData;
	volatile int d;

	for (i=0; i<iter; i++)
	{
		for(data = startV; data <= endV; data++)
		{
			gsysWriteMiiReg(dev,port,reg,data);
	        for(d = 0 ; d <timeout  ; d++);
			gsysReadMiiReg(dev,port,reg,&tmpData);
			if(data != tmpData)
			{
				MSG_PRINT(("Data Mismatch : iter %i, wrote %#x, read %#x \n",i,data,tmpData));
				return GT_FAIL;
			}
		}
		
	}
	MSG_PRINT(("Success\n"));
	return GT_OK;
}

GT_STATUS testDisplayTrunkRouting(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_U32 route, id;

	printf("Trunk Routing Table\n");

	for (id=0; id<16; id++)
	{
		if((status = gsysGetTrunkRouting(dev,id,&route)) != GT_OK)
		{
			MSG_PRINT(("gsysSetTrunkRouting return Failed\n"));
			return status;
		}
		if(!route)
			continue;
		printf("ID %i : %#x\n",(int)id,(int)route);
	}
	return GT_OK;
}

GT_STATUS testDisplayTrunkMask(GT_QD_DEV *dev)
{
	GT_STATUS status;
	int i;
	GT_U32 mask;

	printf("Trunk Mask Table\n");

	for (i=0; i<8; i++)
	{
		if((status = gsysGetTrunkMaskTable(dev,i,&mask)) != GT_OK)
		{
			MSG_PRINT(("gsysSetTrunkMaskTable return Failed\n"));
			return status;
		}
		printf("%i : %#x\n",i+1,(int)mask);
	}
	return GT_OK;
}

GT_STATUS qdStatusShow(GT_QD_DEV *dev)
{
	GT_U32 regBaseAddr,u32Data;
	GT_LPORT port;
	int portIndex;

	regBaseAddr	= dev->baseRegAddr;

	MSG_PRINT(("Switch Status (offset 0): "));
	gsysReadMiiReg(dev,regBaseAddr+0xF,0,&u32Data);
	MSG_PRINT(("%#04x\n",u32Data & 0xFFFF));
	
	MSG_PRINT(("VTU Status (offset 5)   : "));
	gsysReadMiiReg(dev,regBaseAddr+0xF,5,&u32Data);
	MSG_PRINT(("%#04x\n",u32Data & 0xFFFF));
	
	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;
		
		MSG_PRINT(("Port %i Status (offset 0): ", port));
		gsysReadMiiReg(dev,regBaseAddr+0x8+port,0,&u32Data);
		MSG_PRINT(("%#04x\n",u32Data & 0xFFFF));
	}
	return GT_OK;
}
GT_STATUS testHelp()
{
	MSG_PRINT(("qdStart - to Initialize QuarterDeck driver\n"));
	MSG_PRINT(("testSysCtrl - to test System Control API\n"));
	MSG_PRINT(("testPort - to test Port Control API\n"));
	MSG_PRINT(("testATU - to test ATU related API\n"));
	MSG_PRINT(("testRMON - to test RMON related API\n"));
	MSG_PRINT(("testVTU - to test VTU related API\n"));
	MSG_PRINT(("testSysStatus - to test System Status related API\n"));
	MSG_PRINT(("testVlan - to test Vlan related API\n"));
	MSG_PRINT(("testSTP - to test STP related API\n"));
	MSG_PRINT(("testPhy - to test Phy related API\n"));
	MSG_PRINT(("testPortStatus - to test Port Status related API\n"));
	MSG_PRINT(("testQosRule - to test QoS Rule related API\n"));
	MSG_PRINT(("testQosMap - to test QoS Map related API\n"));
	MSG_PRINT(("testPortRateCtrl - to test Port Rate Control API\n"));
	MSG_PRINT(("testPortPAV - to test Port Association Vector API\n"));
	MSG_PRINT(("testInterrupt - to test Interrupt related API\n"));
	MSG_PRINT(("testAll - to run all the test specified above.\n"));
	MSG_PRINT(("\n"));
	MSG_PRINT(("vctTest - to run Virtual Cable Test on a given port.\n"));
	MSG_PRINT(("defaultVlan - to setup Vlan for firewall app\n"));
	MSG_PRINT(("readStatistics - to read statistics\n"));
	MSG_PRINT(("testGoodPkt - to setup Good Packet only mode\n"));
	MSG_PRINT(("testBadPkt - to setup Bad Packet only mode\n"));
	MSG_PRINT(("testDisplayATUList - to display ATU list in the device\n"));
	MSG_PRINT(("testDisplayRMONCounter - to display RMON counter of a port\n"));
	MSG_PRINT(("testDisplayVTUList - to display VTU list in the device\n"));
	MSG_PRINT(("qdStatusShow - to display the status of the device\n"));

	return GT_OK;
}

