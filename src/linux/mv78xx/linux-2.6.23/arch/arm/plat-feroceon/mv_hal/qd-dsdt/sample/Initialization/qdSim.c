#include <Copyright.h>
/********************************************************************************
* qdSim.c
*
* DESCRIPTION:
*       Simulate QuaterDeck Device(88E6052)'s register map. When QuareterDeck API 
*		try to read/write a bit or bits into QuaterDeck, the simulator will redirect to
* 		its own memory place and performing the function very close to QuaterDeck.
*		For example, 
*		1) user can set/reset a certain bit of QuarterDeck registers(Phy,Port,and General registers).
*		2) user can access ATU (flush, load, purge, etc. with max MAC addresses of 32)
*		3) user can manually generate an Interrupt and test the Interrupt routine.
*		4) when user read a register, it will clear a certain register if it's a Self Clear register.
*		5) when user write a register, it will return ERROR if it's read only register.
*		 
*
* DEPENDENCIES:   QuaterDeck (88E6052) Register MAP.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include <msApi.h>
#include <qdSimRegs.h>

#define IS_BROADCAST_ADDR(_addr)                                \
            (((_addr)[0] == 0xFF) && ((_addr)[1] == 0xFF) &&    \
             ((_addr)[2] == 0xFF) && ((_addr)[3] == 0xFF) &&    \
             ((_addr)[4] == 0xFF) && ((_addr)[5] == 0xFF))

#define IS_GLOBAL_REG(_port)	((int)(_port) == qdSimDev.qdSimGlobalRegBase)
#define IS_PORT_REG(_port) (((int)(_port) >= qdSimDev.qdSimPortBase) && ((int)(_port) < qdSimDev.qdSimPortBase + qdSimDev.qdSimNumOfPorts))
#define IS_PHY_REG(_port) (((int)(_port) >= qdSimDev.qdSimPhyBase) && ((int)(_port) < qdSimDev.qdSimPhyBase + qdSimDev.qdSimNumOfPhys))

typedef struct _QD_SIM_DEV
{
	int qdSimUsed;
	unsigned int qdSimDevId;
	int qdSimNumOfPorts;
	int qdSimPortBase;
	int qdSimNumOfPhys;
	int qdSimPhyBase;
	int qdSimGlobalRegBase;
	int qdSimPortStatsClear[10];
	int qdSimStatsCapturedPort;
	int vtuSize;
	int atuSize;
} QD_SIM_DEV;

static QD_SIM_DEV qdSimDev = {0};

void qdSimRegsInit();
GT_BOOL qdSimRead (GT_QD_DEV *dev, unsigned int portNumber , unsigned int miiReg, unsigned int* value);
GT_BOOL qdSimWrite(GT_QD_DEV *dev, unsigned int portNumber , unsigned int miiReg, unsigned int value);

/*
 *	This Array will simulate the QuarterDeck Registers.
 *	To use it, qdSimRegs has to be initialized with its default values and
 *	Call qdSimRead and qdSimWrite functions.
*/
#define MAX_SMI_ADDRESS		0x20
#define MAX_REG_ADDRESS		0x20
#define MAX_ATU_ADDRESS		0x800
#define MAX_QD_VTU_ENTRIES	0x40

GT_U16 qdSimRegs[MAX_SMI_ADDRESS][MAX_REG_ADDRESS];

typedef struct _QDSIM_ATU_ENTRY
{
	GT_U16 atuData;
	GT_U16 DBNum;
	GT_U8 atuMac[6];
} QDSIM_ATU_ENTRY;

/* 
	Since QuarterDeck Simulator supports only fixed size of atu entry,
	we are going with array list not dynamic linked list.
*/
typedef struct _QDSIM_ATU_NODE
{
	QDSIM_ATU_ENTRY atuEntry;
	GT_U32 nextEntry;
} QDSIM_ATU_NODE;

typedef struct _QDSIM_ATU_LIST
{
	int atuSize;
	GT_U32 head;
} QDSIM_ATU_LIST;

QDSIM_ATU_NODE ATUNode[MAX_ATU_ADDRESS];
QDSIM_ATU_LIST ATUList;

typedef struct _QDSIM_VTU_ENTRY
{
	GT_U16 DBNum;
	GT_U16 memberTag[10];
	GT_U16 vid;
} QDSIM_VTU_ENTRY;

/* 
	Since QuarterDeck Simulator supports only fixed size of atu entry,
	we are going with array list not dynamic linked list.
*/
typedef struct _QDSIM_VTU_NODE
{
	QDSIM_VTU_ENTRY vtuEntry;
	GT_U32 nextEntry;
} QDSIM_VTU_NODE;

typedef struct _QDSIM_VTU_LIST
{
	int vtuSize;
	GT_U32 head;
} QDSIM_VTU_LIST;

QDSIM_VTU_NODE VTUNode[MAX_QD_VTU_ENTRIES];
QDSIM_VTU_LIST VTUList;

/*******************************************************************************
* qdMemSet
*
* DESCRIPTION:
*       Set a block of memory
*
* INPUTS:
*       start  - start address of memory block for setting
*       simbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * qdMemSet
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
)
{
	GT_U32 i;
	char* buf;
	
	buf = (char*)start;
		
	for(i=0; i<size; i++)
	{
		*buf++ = (char)symbol;
	}

	return start;
}

/*******************************************************************************
* qdMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * qdMemCpy
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
)
{
	GT_U32 i;
	char* buf;
	char* src;
	
	buf = (char*)destination;
	src = (char*)source;
		
	for(i=0; i<size; i++)
	{
		*buf++ = *src++;
	}

	return destination;
}

/*******************************************************************************
* qdMemCmp
*
* DESCRIPTION:
*       Compares given memories.
*
* INPUTS:
*       src1 - source 1
*       src2 - source 2
*       size - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0, if equal.
*		negative number, if src1 < src2.
*		positive number, if src1 > src2.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int qdMemCmp
(
    IN char src1[],
    IN char src2[],
    IN GT_U32 size
)
{
	GT_U32 i;
	int value;

	for(i=0; i<size; i++)
	{
		if((value = (int)(src1[i] - src2[i])) != 0)
			return value; 
	}

	return 0;
}

/*
	Compare the given ethernet addresses.
	0, if they are equal.
	Negative int, if mac2 is bigger than mac1.
	Positive int, if mac1 is bigger than mac2.
*/
int cmpEtherMac(unsigned char* mac1, unsigned char* mac2)
{
	int i, tmp;

	for(i=0; i<6; i++)
	{
		if((tmp = mac1[i] - mac2[i]) != 0)
			return tmp;
	}
	return 0;
}

/*
	entry index, if found.
	MAX_ATU_ADDRESS, otherwise.
*/
int qdSimATUFindNext(QDSIM_ATU_ENTRY* entry)
{
	int i;
	int node = ATUList.head;

	if (IS_BROADCAST_ADDR(entry->atuMac))
	{
		if(ATUList.atuSize != 0)
		{
			if (ATUNode[node].atuEntry.DBNum == entry->DBNum)
				return node;
			else
			{
				for(i=0; i<ATUList.atuSize; i++)
				{
					if(ATUNode[node].atuEntry.DBNum == entry->DBNum)
						return node;
					node = ATUNode[node].nextEntry;
				}
			}
				
		}
		return MAX_ATU_ADDRESS;
	}

	for(i=0; i<ATUList.atuSize; i++)
	{
		if(cmpEtherMac(ATUNode[node].atuEntry.atuMac,entry->atuMac) > 0)
		{
			if(ATUNode[node].atuEntry.DBNum == entry->DBNum)
				break;
		}
		node = ATUNode[node].nextEntry;
	}

	if (i == ATUList.atuSize)
		return MAX_ATU_ADDRESS;

	return node;
}

/*
	Return 1, if added successfully.
	Return 0, otherwise.
*/
GT_BOOL qdSimATUAdd(QDSIM_ATU_ENTRY* entry)
{
	int i, freeNode, preNode, node;

	preNode = node = ATUList.head;

	if (ATUList.atuSize >= MAX_ATU_ADDRESS)
		return GT_FALSE;

	/* find a free entry from our global memory. */
	for(i=0; i<MAX_ATU_ADDRESS; i++)
	{
		if(ATUNode[i].nextEntry == MAX_ATU_ADDRESS)
			break;
	}
	
	if (i==MAX_ATU_ADDRESS)
	{
		return GT_FALSE;
	}

	freeNode = i;

	/* find the smallest entry which is bigger than the given entry */
	for(i=0; i<ATUList.atuSize; i++)
	{
		if(cmpEtherMac(ATUNode[node].atuEntry.atuMac,entry->atuMac) >= 0)
			break;
		preNode = node;
		node = ATUNode[node].nextEntry;
	}	

	/* if the same Mac address is in the list and dbnum is identical, then just update and return. */
	if (i != ATUList.atuSize)
		if(cmpEtherMac(ATUNode[node].atuEntry.atuMac,entry->atuMac) == 0)
		{
			if(ATUNode[node].atuEntry.DBNum == entry->DBNum)
			{
				ATUNode[node].atuEntry.atuData = entry->atuData;
				return GT_TRUE;
			}
		}

	qdMemCpy(ATUNode[freeNode].atuEntry.atuMac, entry->atuMac, 6);
	ATUNode[freeNode].atuEntry.atuData = entry->atuData;
	ATUNode[freeNode].atuEntry.DBNum = entry->DBNum;

	/* Add it to head */
	if (i == 0)
	{
		ATUNode[freeNode].nextEntry = ATUList.head;
		ATUList.head = freeNode;
	}
	/* Add it to tail */
	else if (i == ATUList.atuSize)
	{
		ATUNode[preNode].nextEntry = freeNode;
		ATUNode[freeNode].nextEntry = ATUList.head;
	}
	/* Add it in the middle of the list */
	else
	{
		ATUNode[freeNode].nextEntry = ATUNode[preNode].nextEntry;
		ATUNode[preNode].nextEntry = freeNode;
	}
	ATUList.atuSize++;
	return GT_TRUE;
}


/*
	Return 1, if added successfully.
	Return 0, otherwise.
*/
GT_BOOL qdSimATUDel(QDSIM_ATU_ENTRY* entry)
{
	int i, preNode, node;

	preNode = node = ATUList.head;

	/* find the entry */
	for(i=0; i<ATUList.atuSize; i++)
	{
		if(cmpEtherMac(ATUNode[node].atuEntry.atuMac,entry->atuMac) == 0)
		{
			if(ATUNode[node].atuEntry.DBNum == entry->DBNum)
				break;
		}
		preNode = node;
		node = ATUNode[node].nextEntry;
	}	

	if (i == ATUList.atuSize)
	{
		/* cannot find the given entry to be deleted. */
		return GT_FALSE;
	}

	/* Delete it from head */
	if (i == 0)
	{
		ATUList.head = ATUNode[node].nextEntry;
	}
	/* Delete it in the middle of the list */
	else if (i != ATUList.atuSize-1)
	{
		ATUNode[preNode].nextEntry = ATUNode[node].nextEntry;
	}
	ATUList.atuSize--;
	ATUNode[node].nextEntry = MAX_ATU_ADDRESS;

	return GT_TRUE;
}


GT_BOOL qdSimATUFlushUnlockedEntry()
{
	int i;

	for (i=0; i<MAX_ATU_ADDRESS; i++)
	{
		if(((ATUNode[i].atuEntry.atuData & 0xF) != 0xF)	&&
			(!(ATUNode[i].atuEntry.atuMac[0] & 1)) 		&&
			(ATUNode[i].nextEntry != MAX_ATU_ADDRESS))
		{
			qdSimATUDel(&ATUNode[i].atuEntry);
		}			
	}
	return GT_TRUE;
}

GT_BOOL qdSimATUFlushInDB(int dbNum)
{
	int i;

	for (i=0; i<MAX_ATU_ADDRESS; i++)
	{
		if(ATUNode[i].atuEntry.DBNum != dbNum)
			continue;
		qdSimATUDel(&ATUNode[i].atuEntry);
	}
	return GT_TRUE;
}

GT_BOOL qdSimATUFlushUnlockedInDB(int dbNum)
{
	int i;

	for (i=0; i<MAX_ATU_ADDRESS; i++)
	{
		if(ATUNode[i].atuEntry.DBNum != dbNum)
			continue;
		
		if(((ATUNode[i].atuEntry.atuData & 0xF) != 0xF)	&&
			(!(ATUNode[i].atuEntry.atuMac[0] & 1)) 		&&
			(ATUNode[i].nextEntry != MAX_ATU_ADDRESS))
		{
			qdSimATUDel(&ATUNode[i].atuEntry);
		}			
	}
	return GT_TRUE;
}


void qdSimATUInit()
{
	int i;

	qdMemSet((char*)ATUNode, 0, sizeof(ATUNode));

	/* MAX_ATU_ADDRESS means entry i is free, otherwise, it's not free */
	for (i=0; i<MAX_ATU_ADDRESS; i++)
		ATUNode[i].nextEntry = MAX_ATU_ADDRESS;

	ATUList.atuSize = 0;	
	ATUList.head = 0;	
}

void qdSimGetATUInfo(QDSIM_ATU_ENTRY* entry)
{
	entry->atuData = qdSimRegs[qdSimDev.qdSimGlobalRegBase][12];
	entry->atuMac[0] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][13] >> 8) & 0xFF;
	entry->atuMac[1] = qdSimRegs[qdSimDev.qdSimGlobalRegBase][13] & 0xFF;
	entry->atuMac[2] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][14] >> 8) & 0xFF;
	entry->atuMac[3] = qdSimRegs[qdSimDev.qdSimGlobalRegBase][14] & 0xFF;
	entry->atuMac[4] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][15] >> 8) & 0xFF;
	entry->atuMac[5] = qdSimRegs[qdSimDev.qdSimGlobalRegBase][15] & 0xFF;
	entry->DBNum = qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] & 0xF;
	return;
}

void qdSimSetATUInfo(QDSIM_ATU_ENTRY* entry)
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][12] = entry->atuData;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][13] = (entry->atuMac[0]<<8) | entry->atuMac[1];
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][14] = (entry->atuMac[2]<<8) | entry->atuMac[3];
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][15] = (entry->atuMac[4]<<8) | entry->atuMac[5];
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] &= ~0xF;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] |= (entry->DBNum & 0xF);

	return;
}

void qdSimReSetATUInfo()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] &= ~0xF;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][12] = 0;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][13] = 0xFFFF;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][14] = 0xFFFF;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][15] = 0xFFFF;

	return;
}

GT_BOOL qdSimATUOperation(unsigned int value)
{
	QDSIM_ATU_ENTRY entry;
	int	index;

	switch((value & 0x7000) >> 12)
	{
		case 1:
			/* Flush ALL */
			qdSimATUInit();
			break;
		case 2:
			/* Flush all unlocked entries */
			return qdSimATUFlushUnlockedEntry();
		case 3:
			/* Load or Purge entry */
			qdSimGetATUInfo(&entry);
			if(entry.atuData & 0xF)
				return qdSimATUAdd(&entry);
			else
				return qdSimATUDel(&entry);
			break;
		case 4:
			/* Get Next Entry */
			qdSimGetATUInfo(&entry);
			index = qdSimATUFindNext(&entry);
			if (index == MAX_ATU_ADDRESS)
			{
				qdSimReSetATUInfo();
				return GT_TRUE;
			}
			else
			{
				qdSimSetATUInfo(&ATUNode[index].atuEntry);
				return GT_TRUE;
			}
			break;
		case 5:
			/* Flush ALL in a DBNum */
			return qdSimATUFlushInDB(value & 0xF);
			break;
		case 6:
			/* Flush all unlocked entries */
			return qdSimATUFlushUnlockedInDB(value & 0xF);
		default:
			break;
	}
	return GT_TRUE;
}

/*
	VTU Related Routines
*/

/*
	entry index, if found.
	MAX_QD_VTU_ENTRIES, otherwise.
*/
int qdSimVTUFindNext(QDSIM_VTU_ENTRY* entry)
{
	int i;
	int node = VTUList.head;

	if (entry->vid == 0xFFF)
	{
		if(VTUList.vtuSize != 0)
			return node;
		else
			return MAX_QD_VTU_ENTRIES;
	}
		
	for(i=0; i<VTUList.vtuSize; i++)
	{
		if(VTUNode[node].vtuEntry.vid > entry->vid)
			break;
		node = VTUNode[node].nextEntry;
	}

	if (i == VTUList.vtuSize)
		return MAX_QD_VTU_ENTRIES;

	return node;
}

/*
	Return 1, if added successfully.
	Return 0, otherwise.
*/
GT_BOOL qdSimVTUAdd(QDSIM_VTU_ENTRY* entry)
{
	int i, freeNode, preNode, node;

	preNode = node = VTUList.head;

	if (VTUList.vtuSize >= qdSimDev.vtuSize)
		return GT_FALSE;

	/* find a free entry from our global memory. */
	for(i=0; i<MAX_QD_VTU_ENTRIES; i++)
	{
		if(VTUNode[i].nextEntry == MAX_QD_VTU_ENTRIES)
			break;
	}
	
	if (i==MAX_QD_VTU_ENTRIES)
	{
		return GT_FALSE;
	}

	freeNode = i;

	/* find the smallest entry which is bigger than the given entry */
	for(i=0; i<VTUList.vtuSize; i++)
	{
		if(VTUNode[node].vtuEntry.vid >= entry->vid)
			break;
		preNode = node;
		node = VTUNode[node].nextEntry;
	}	

	/* if the same vid is in the list, then just update and return. */
	if (i != VTUList.vtuSize)
		if(VTUNode[node].vtuEntry.vid == entry->vid)
		{
			qdMemCpy(&VTUNode[node].vtuEntry, entry, sizeof(QDSIM_VTU_ENTRY));
			return GT_TRUE;
		}

	qdMemCpy(&VTUNode[freeNode].vtuEntry, entry, sizeof(QDSIM_VTU_ENTRY));

	/* Add it to head */
	if (i == 0)
	{
		VTUNode[freeNode].nextEntry = VTUList.head;
		VTUList.head = freeNode;
	}
	/* Add it to tail */
	else if (i == VTUList.vtuSize)
	{
		VTUNode[preNode].nextEntry = freeNode;
		VTUNode[freeNode].nextEntry = VTUList.head;
	}
	/* Add it in the middle of the list */
	else
	{
		VTUNode[freeNode].nextEntry = VTUNode[preNode].nextEntry;
		VTUNode[preNode].nextEntry = freeNode;
	}
	VTUList.vtuSize++;
	return GT_TRUE;
}


/*
	Return 1, if added successfully.
	Return 0, otherwise.
*/
GT_BOOL qdSimVTUDel(QDSIM_VTU_ENTRY* entry)
{
	int i, preNode, node;

	preNode = node = VTUList.head;

	/* find the entry */
	for(i=0; i<VTUList.vtuSize; i++)
	{
		if(VTUNode[node].vtuEntry.vid == entry->vid)
			break;
		preNode = node;
		node = VTUNode[node].nextEntry;
	}	

	if (i == VTUList.vtuSize)
	{
		/* cannot find the given entry to be deleted. */
		return GT_FALSE;
	}

	/* Delete it from head */
	if (i == 0)
	{
		VTUList.head = VTUNode[node].nextEntry;
	}
	/* Delete it in the middle of the list */
	else if (i != VTUList.vtuSize-1)
	{
		VTUNode[preNode].nextEntry = VTUNode[node].nextEntry;
	}
	VTUList.vtuSize--;
	VTUNode[node].nextEntry = MAX_QD_VTU_ENTRIES;

	return GT_TRUE;
}


/*
	Return 1, if added successfully.
	Return 0, otherwise.
*/
GT_BOOL qdSimVTUUpdate(QDSIM_VTU_ENTRY* entry)
{
	int i;
	int node = VTUList.head;

	/* find the entry */
	for(i=0; i<VTUList.vtuSize; i++)
	{
		if(VTUNode[node].vtuEntry.vid == entry->vid)
			break;
		node = VTUNode[node].nextEntry;
	}	

	if (i == VTUList.vtuSize)
	{
		/* cannot find the given entry to be deleted. */
		return GT_FALSE;
	}

	/* Update the found entry */
	qdMemCpy(&VTUNode[node].vtuEntry, entry, sizeof(QDSIM_VTU_ENTRY));

	return GT_TRUE;
}

void qdSimVTUInit()
{
	int i;

	qdMemSet((char*)VTUNode, 0, sizeof(VTUNode));

	/* MAX_ATU_ADDRESS means entry i is free, otherwise, it's not free */
	for (i=0; i<MAX_QD_VTU_ENTRIES; i++)
		VTUNode[i].nextEntry = MAX_QD_VTU_ENTRIES;

	VTUList.vtuSize = 0;	
	VTUList.head = 0;	
}

void qdSimGetVTUInfo(QDSIM_VTU_ENTRY* entry)
{
	entry->DBNum = qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] & 0xF;
	entry->vid = qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] & 0x1FFF;
	entry->memberTag[0] = qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] & 0x3;
	entry->memberTag[1] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] >> 4) & 0x3;
	entry->memberTag[2] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] >> 8) & 0x3;
	entry->memberTag[3] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] >> 12) & 0x3;
	entry->memberTag[4] = qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] & 0x3;
	entry->memberTag[5] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] >> 4) & 0x3;
	entry->memberTag[6] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] >> 8) & 0x3;
	entry->memberTag[7] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] >> 12) & 0x3;
	entry->memberTag[8] = qdSimRegs[qdSimDev.qdSimGlobalRegBase][9] & 0x3;
	entry->memberTag[9] = (qdSimRegs[qdSimDev.qdSimGlobalRegBase][9] >> 4) & 0x3;

	return;
}

void qdSimSetVTUInfo(QDSIM_VTU_ENTRY* entry)
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] |= entry->DBNum;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] = (entry->vid & 0xFFF) | 0x1000;

	qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] = 	entry->memberTag[0] |
						(entry->memberTag[1] << 4) |
						(entry->memberTag[2] << 8) |
						(entry->memberTag[3] << 12);

	qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] = 	entry->memberTag[4] |
						(entry->memberTag[5] << 4) |
						(entry->memberTag[6] << 8) |
						(entry->memberTag[7] << 12);

	qdSimRegs[qdSimDev.qdSimGlobalRegBase][9] = 	entry->memberTag[8] |
						(entry->memberTag[9] << 4);

	return;
}

void qdSimReSetVTUInfo()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] = 0xFFF;

	return;
}

void qdSimVTUGetViolation()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] &= ~0xFFF;
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] |= 1;	/* assume port 1 causes the violation */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] = 1;	/* assume vid 1 causes the violation */
}

void qdSimVTUResetBusy()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] &= ~0x8000;

	return;
}

GT_BOOL qdSimVTUOperation(unsigned int value)
{
	QDSIM_VTU_ENTRY entry;
	int	index;

	if(!(value & 0x8000))
		return GT_FALSE;

	qdSimVTUResetBusy();

	switch((value & 0x7000) >> 12)
	{
		case 1:
			/* Flush ALL */
			qdSimVTUInit();
			break;
		case 3:
			/* Load or Purge entry */
			qdSimGetVTUInfo(&entry);
			if(entry.vid & 0x1000)
			{
				entry.vid &= ~0x1000;
				return qdSimVTUAdd(&entry);
			}
			else
				return qdSimVTUDel(&entry);
			break;
		case 4:
			/* Get Next Entry */
			qdSimGetVTUInfo(&entry);
			entry.vid &= ~0x1000;
			index = qdSimVTUFindNext(&entry);
			if (index == MAX_QD_VTU_ENTRIES)
			{
				qdSimReSetVTUInfo();
				return GT_TRUE;
			}
			else
			{
				qdSimSetVTUInfo(&VTUNode[index].vtuEntry);
				return GT_TRUE;
			}
			break;
		case 7:
			qdSimVTUGetViolation();
			break;
		default:
			break;
	}
	return GT_TRUE;
}

void qdSimStatsInit()
{
	int i;

	for(i=0; i<qdSimDev.qdSimNumOfPorts; i++)
		qdSimDev.qdSimPortStatsClear[i] = 0;

}

GT_BOOL qdSimStatsOperation(unsigned int value)
{
	int	i;

	if(!(value & 0x8000))
		return GT_FALSE;

	qdSimRegs[qdSimDev.qdSimGlobalRegBase][29] &= ~0x8000;

	switch((value & 0x7000) >> 12)
	{
		case 1:
			/* Flush ALL */
			for(i=0; i<qdSimDev.qdSimNumOfPorts; i++)
				qdSimDev.qdSimPortStatsClear[i] = 1;
			break;
		case 2:
			/* Flush a port */
			if ((value & 0x3F) >= (unsigned int)qdSimDev.qdSimNumOfPorts)
				return GT_FALSE;
			qdSimDev.qdSimPortStatsClear[value & 0x3F] = 1;
			break;
		case 4:
			/* Read a counter */
			if(qdSimDev.qdSimPortStatsClear[qdSimDev.qdSimStatsCapturedPort] == 1)
			{
				qdSimRegs[qdSimDev.qdSimGlobalRegBase][30] = 0;
				qdSimRegs[qdSimDev.qdSimGlobalRegBase][31] = 0;
			}
			else
			{
				qdSimRegs[qdSimDev.qdSimGlobalRegBase][30] = qdSimDev.qdSimStatsCapturedPort;
				qdSimRegs[qdSimDev.qdSimGlobalRegBase][31] = value & 0x3F;
			}
			break;
		case 5:
			if ((value & 0x3F) >= (unsigned int)qdSimDev.qdSimNumOfPorts)
				return GT_FALSE;
			qdSimDev.qdSimStatsCapturedPort = value & 0x3F;
			break;
		default:
			return GT_FALSE;
	}
	return GT_TRUE;
}

#define QD_PHY_CONTROL_RW (QD_PHY_RESET|QD_PHY_LOOPBACK|QD_PHY_SPEED|QD_PHY_AUTONEGO|QD_PHY_POWER|QD_PHY_RESTART_AUTONEGO|QD_PHY_DUPLEX)
#define QD_PHY_CONTROL_RO (~QD_PHY_CONTROL_RW)

GT_BOOL qdSimPhyControl(unsigned int portNumber , unsigned int miiReg, unsigned int value)
{

	/* reset all the Read Only bits. */
	value &= QD_PHY_CONTROL_RW;

	/* If powerDown is set, add Reset and Restart Auto bits. */
	if(value & QD_PHY_POWER)
	{
		value |= (QD_PHY_RESET|QD_PHY_RESTART_AUTONEGO);
		qdSimRegs[portNumber][miiReg] = (GT_U16)value;
		return GT_TRUE;
	}

	/* If Power Down was set, clear Reset and Restart Auto bits. */
	if(qdSimRegs[portNumber][miiReg] & QD_PHY_POWER)
	{
		value &= ~(QD_PHY_RESET|QD_PHY_RESTART_AUTONEGO);
		qdSimRegs[portNumber][miiReg] = (GT_U16)value;
		return GT_TRUE;
	}

	/* If Reset or Restart Auto set, replace with current value and clear Reset/Restart Auto. */
	if (value & (QD_PHY_RESET|QD_PHY_RESTART_AUTONEGO))
	{
		value &= ~(QD_PHY_RESET|QD_PHY_RESTART_AUTONEGO);
		qdSimRegs[portNumber][miiReg] = (GT_U16)value;
		return GT_TRUE;
	}
	else
	{
		value &= ~(QD_PHY_SPEED|QD_PHY_AUTONEGO|QD_PHY_DUPLEX);
		qdSimRegs[portNumber][miiReg] &= (QD_PHY_SPEED|QD_PHY_AUTONEGO|QD_PHY_DUPLEX);
		qdSimRegs[portNumber][miiReg] |= (GT_U16)value;
		return GT_TRUE;
	}

	return GT_TRUE;
}

void qdSimRegsInit_6021()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] = 0;	/* VTU Operation Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] = 0;	/* VTU VID Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] = 0;	/* VTU Data Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][29] = 0;	/* Stats Operation Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][30] = 0;	/* Stats Counter Register Bytes 3,2 */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][31] = 0;	/* Stats Counter Register Bytes 1,0 */
}

void qdSimRegsInit_6063()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] = 0;	/* VTU Operation Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] = 0;	/* VTU VID Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] = 0;	/* VTU Data Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] = 0;	/* VTU Data Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][29] = 0;	/* Stats Operation Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][30] = 0;	/* Stats Counter Register Bytes 3,2 */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][31] = 0;	/* Stats Counter Register Bytes 1,0 */
}

void qdSimRegsInit_6083()
{
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] = 0;	/* VTU Operation Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][6] = 0;	/* VTU VID Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][7] = 0;	/* VTU Data Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][8] = 0;	/* VTU Data Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][9] = 0;	/* VTU Data Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][29] = 0;	/* Stats Operation Register */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][30] = 0;	/* Stats Counter Register Bytes 3,2 */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][31] = 0;	/* Stats Counter Register Bytes 1,0 */
}

void qdSimRegsInit()
{
	int i;

	qdMemSet(qdSimRegs, 0xff, sizeof(qdSimRegs));
	
	/* 
		PHY Registers Setup
	*/
	for(i=0; i<qdSimDev.qdSimNumOfPhys; i++)
	{
		qdSimRegs[i][0] = 0x3100;	/* PHY Control */
		qdSimRegs[i][1] = 0x7849;	/* PHY Status */
		qdSimRegs[i][2] = 0x0141;	/* PHY Id 1 */
		qdSimRegs[i][3] = 0x0c1f;	/* PHY Id 2 */
		qdSimRegs[i][4] = 0x01e1;	/* AutoNego Ad */
		qdSimRegs[i][5] = 0;		/* Partner Ability */
		qdSimRegs[i][6] = 4;		/* AutoNego Expansion */
		qdSimRegs[i][7] = 0x2001;	/* Next Page Transmit */
		qdSimRegs[i][8] = 0;		/* Link Partner Next Page */
		qdSimRegs[i][16] = 0x130;	/* Phy Specific Control */
		qdSimRegs[i][17] = 0x40;	/* Phy Specific Status */
		qdSimRegs[i][18] = 0;		/* Phy Interrupt Enable */
		qdSimRegs[i][19] = 0x40;	/* Phy Interrupt Status */
		qdSimRegs[i][20] = 0;		/* Interrupt Port Summary */
		qdSimRegs[i][21] = 0;		/* Receive Error Counter */
		qdSimRegs[i][22] = 0xa34;	/* LED Parallel Select */
		qdSimRegs[i][23] = 0x3fc;	/* LED Stream Select */
		qdSimRegs[i][24] = 0x42bf;	/* LED Control */
	}

	/*
		Port Registers Setup
	*/
	for(i=qdSimDev.qdSimPortBase; i<qdSimDev.qdSimNumOfPorts+qdSimDev.qdSimPortBase; i++)
	{
		qdSimRegs[i][0] = 0x800;	/* Port Status */
		qdSimRegs[i][3] = (GT_U16)qdSimDev.qdSimDevId << 4;	/* Switch ID */
		qdSimRegs[i][4] = 0x7f;	/* Port Control */
		qdSimRegs[i][6] = 0x7f & (~(1 << (i-8)));	/* Port Based Vlan Map */
		qdSimRegs[i][7] = 1;		/* Default Port Vlan ID & Priority */
		qdSimRegs[i][16] = 0;		/* Rx Frame Counter */
		qdSimRegs[i][17] = 0;		/* Tx Frame Counter */
	}

	/*
		Global Registers Setup
	*/
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][0] = 0x3c01;	/* Global Status */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][1] = 0;		/* Switch Mac Addr 0 ~ 1 byte */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][2] = 0;		/* Switch Mac Addr 2 ~ 3 byte */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][3] = 0;		/* Switch Mac Addr 4 ~ 5 byte */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][4] = 0x81;	/* Global Control */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][10] = 0x1130;		/* ATU Control */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] = 0;				/* ATU Operation */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][12] = 0;				/* ATU Data */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][13] = 0;				/* ATU Mac Addr 0 ~ 1 byte */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][14] = 0;				/* ATU Mac Addr 2 ~ 3 byte */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][15] = 0;				/* ATU Mac Addr 4 ~ 5 byte */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][16] = 0;			/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][17] = 0;			/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][18] = 0x5555;	/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][19] = 0x5555;	/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][20] = 0xaaaa;	/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][21] = 0xaaaa;	/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][22] = 0xffff;	/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][23] = 0xffff;	/* IP-PRI Mapping */
	qdSimRegs[qdSimDev.qdSimGlobalRegBase][24] = 0xfa41;	/* IEEE-PRI Mapping */
			  
	switch(qdSimDev.qdSimDevId)
	{
		case GT_88E6021:
			qdSimRegsInit_6021();
			break;
		case GT_88E6063:
		case GT_FF_HG:
		case GT_FF_EG:
		case GT_FH_VPN:
			qdSimRegsInit_6063();
			break;
		case GT_88E6083:
			qdSimRegsInit_6083();
			break;
		default:
			break;
	}
}

GT_BOOL qdSimRead_6052(unsigned int portNumber , unsigned int miiReg, unsigned int* value)
{
	*value = (unsigned int) qdSimRegs[portNumber][miiReg];

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					qdSimRegs[portNumber][miiReg] &= ~0xF;
					if(qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG])
						qdSimRegs[portNumber][miiReg] |= 0x2;

					break;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
			case QD_REG_GLOBAL_CONTROL:
			case QD_REG_ATU_CONTROL:
			case QD_REG_ATU_OPERATION:
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
					break;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
			case QD_REG_SWITCH_ID:
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
					break;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					break;
			case QD_PHY_INT_ENABLE_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
					qdSimRegs[portNumber][miiReg] = 0;
					qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG] &= ~(1<<portNumber);
					break;
			case QD_PHY_INT_PORT_SUMMARY_REG:
					*value = (unsigned int) qdSimRegs[0][miiReg];
					break;
		}
	}

	return GT_TRUE;
}

GT_BOOL qdSimRead_6021(unsigned int portNumber , unsigned int miiReg, unsigned int* value)
{
	*value = (unsigned int) qdSimRegs[portNumber][miiReg];

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					qdSimRegs[portNumber][miiReg] &= ~0x7F;
					if(qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG])
						qdSimRegs[portNumber][miiReg] |= 0x2;

					break;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
			case QD_REG_VTU_OPERATION:
			case QD_REG_VTU_VID_REG:
			case QD_REG_VTU_DATA1_REG:
			case QD_REG_VTU_DATA2_REG:
			case QD_REG_GLOBAL_CONTROL:
			case QD_REG_ATU_CONTROL:
			case QD_REG_ATU_OPERATION:
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
			case QD_REG_STATS_OPERATION:
			case QD_REG_STATS_COUNTER3_2:
			case QD_REG_STATS_COUNTER1_0:
					break;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
			case QD_REG_SWITCH_ID:
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
			case QD_REG_RATE_CTRL:
			case QD_REG_PAV:
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
			case QD_REG_Q_COUNTER:
					break;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					break;
			case QD_PHY_INT_ENABLE_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
					qdSimRegs[portNumber][miiReg] = 0;
					qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG] &= ~(1<<portNumber);
					break;
			case QD_PHY_INT_PORT_SUMMARY_REG:
					*value = (unsigned int) qdSimRegs[0][miiReg];
					break;
		}
	}

	return GT_TRUE;
}

GT_BOOL qdSimRead_6063(unsigned int portNumber , unsigned int miiReg, unsigned int* value)
{
	*value = (unsigned int) qdSimRegs[portNumber][miiReg];

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					qdSimRegs[portNumber][miiReg] &= ~0x7F;
					if(qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG])
						qdSimRegs[portNumber][miiReg] |= 0x2;

					break;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
			case QD_REG_VTU_OPERATION:
			case QD_REG_VTU_VID_REG:
			case QD_REG_VTU_DATA1_REG:
			case QD_REG_VTU_DATA2_REG:
			case QD_REG_GLOBAL_CONTROL:
			case QD_REG_ATU_CONTROL:
			case QD_REG_ATU_OPERATION:
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
			case QD_REG_STATS_OPERATION:
			case QD_REG_STATS_COUNTER3_2:
			case QD_REG_STATS_COUNTER1_0:
					break;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
			case QD_REG_SWITCH_ID:
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
			case QD_REG_RATE_CTRL:
			case QD_REG_PAV:
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
			case QD_REG_Q_COUNTER:
					break;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					break;
			case QD_PHY_INT_ENABLE_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
					qdSimRegs[portNumber][miiReg] = 0;
					qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG] &= ~(1<<portNumber);
					break;
			case QD_PHY_INT_PORT_SUMMARY_REG:
					*value = (unsigned int) qdSimRegs[0][miiReg];
					break;
		}
	}

	return GT_TRUE;
}


GT_BOOL qdSimRead_6083(unsigned int portNumber , unsigned int miiReg, unsigned int* value)
{
	*value = (unsigned int) qdSimRegs[portNumber][miiReg];

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					qdSimRegs[portNumber][miiReg] &= ~0x7F;
					if(qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG])
						qdSimRegs[portNumber][miiReg] |= 0x2;

					break;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
			case QD_REG_VTU_OPERATION:
			case QD_REG_VTU_VID_REG:
			case QD_REG_VTU_DATA1_REG:
			case QD_REG_VTU_DATA2_REG:
			case QD_REG_GLOBAL_CONTROL:
			case QD_REG_ATU_CONTROL:
			case QD_REG_ATU_OPERATION:
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
			case QD_REG_STATS_OPERATION:
			case QD_REG_STATS_COUNTER3_2:
			case QD_REG_STATS_COUNTER1_0:
					break;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
			case QD_REG_SWITCH_ID:
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
			case QD_REG_RATE_CTRL:
			case QD_REG_PAV:
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
			case QD_REG_Q_COUNTER:
					break;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					break;
			case QD_PHY_INT_ENABLE_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
					qdSimRegs[portNumber][miiReg] = 0;
					qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG] &= ~(1<<portNumber);
					break;
			case QD_PHY_INT_PORT_SUMMARY_REG:
					*value = (unsigned int) qdSimRegs[0][miiReg];
					break;
		}
	}

	return GT_TRUE;
}

GT_BOOL qdSimRead (GT_QD_DEV *dev,unsigned int portNumber , unsigned int miiReg, unsigned int* value)
{
	if (portNumber >= MAX_SMI_ADDRESS)
		portNumber -= MAX_SMI_ADDRESS;

	if ((portNumber >= MAX_SMI_ADDRESS) || (miiReg >= MAX_REG_ADDRESS))
		return GT_FALSE;

	switch(qdSimDev.qdSimDevId)
	{
		case GT_88E6051:
		case GT_88E6052:
			return qdSimRead_6052(portNumber, miiReg, value);
		case GT_88E6021:
			return qdSimRead_6021(portNumber, miiReg, value);
		case GT_88E6063:
		case GT_FF_HG:
		case GT_FF_EG:
		case GT_FH_VPN:
			return qdSimRead_6063(portNumber, miiReg, value);
		case GT_88E6083:
			return qdSimRead_6083(portNumber, miiReg, value);
		default:
			break;
	}

	return GT_TRUE;
}

GT_BOOL qdSimWrite_6052 (unsigned int portNumber , unsigned int miiReg, unsigned int value)
{
	GT_BOOL status;

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					/* readonly register */
					return GT_FALSE;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
					break;
			case QD_REG_GLOBAL_CONTROL:
					if(value & 0x200)
					{
						/* Reload EEPROM values */
						qdSimRegsInit();
						qdSimRegs[portNumber][QD_REG_GLOBAL_STATUS] |= 0x1;
						return GT_TRUE;
					}
					break;
			case QD_REG_ATU_CONTROL:
					value &= ~0x8000;
					break;
			case QD_REG_ATU_OPERATION:
					status = qdSimATUOperation(value);
					return status;
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
					break;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
			case QD_REG_SWITCH_ID:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
					break;
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
					/* readonly registers */
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					return qdSimPhyControl(portNumber,miiReg,value);
			case QD_PHY_INT_ENABLE_REG:
			case QD_PHY_AUTONEGO_AD_REG:
			case QD_PHY_NEXTPAGE_TX_REG:
			case QD_PHY_SPEC_CONTROL_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
			case QD_PHY_INT_PORT_SUMMARY_REG:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else
		return GT_FALSE;

	qdSimRegs[portNumber][miiReg] = (GT_U16)value;
	return GT_TRUE;
}

GT_BOOL qdSimWrite_6021 (unsigned int portNumber , unsigned int miiReg, unsigned int value)
{
	GT_BOOL status;

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					/* readonly register */
					return GT_FALSE;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
					break;
			case QD_REG_VTU_OPERATION:
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] &= ~0xF;
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] |= (value & 0xF);
					status = qdSimVTUOperation(value);
					return status;
			case QD_REG_VTU_VID_REG:
			case QD_REG_VTU_DATA1_REG:
			case QD_REG_VTU_DATA2_REG:
					break;
			case QD_REG_GLOBAL_CONTROL:
					if(value & 0x200)
					{
						/* Reload EEPROM values */
						qdSimRegsInit();
						qdSimRegs[portNumber][QD_REG_GLOBAL_STATUS] |= 0x1;
						return GT_TRUE;
					}
					break;
			case QD_REG_ATU_CONTROL:
					value &= ~0x8000;
					break;
			case QD_REG_ATU_OPERATION:
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] &= ~0xF;
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] |= (value & 0xF);
					status = qdSimATUOperation(value);
					return status;
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
					break;
			case QD_REG_STATS_OPERATION:
					status = qdSimStatsOperation(value);
					return status;
			case QD_REG_STATS_COUNTER3_2:
			case QD_REG_STATS_COUNTER1_0:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
					if(portNumber > 9)
					{
						qdSimRegs[portNumber][miiReg] &= ~QD_PORT_STATUS_DUPLEX;
						qdSimRegs[portNumber][miiReg] |= (value & QD_PORT_STATUS_DUPLEX);
						return GT_TRUE;
					}
			case QD_REG_SWITCH_ID:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
					break;
			case QD_REG_RATE_CTRL:
			case QD_REG_PAV:
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_Q_COUNTER:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					return qdSimPhyControl(portNumber,miiReg,value);
			case QD_PHY_INT_ENABLE_REG:
			case QD_PHY_AUTONEGO_AD_REG:
			case QD_PHY_NEXTPAGE_TX_REG:
			case QD_PHY_SPEC_CONTROL_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
			case QD_PHY_INT_PORT_SUMMARY_REG:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else
		return GT_FALSE;

	qdSimRegs[portNumber][miiReg] = (GT_U16)value;
	return GT_TRUE;
}

GT_BOOL qdSimWrite_6063 (unsigned int portNumber , unsigned int miiReg, unsigned int value)
{
	GT_BOOL status;

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					/* readonly register */
					return GT_FALSE;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
					break;
			case QD_REG_VTU_OPERATION:
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] &= ~0xF;
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] |= (value & 0xF);
					status = qdSimVTUOperation(value);
					return status;
			case QD_REG_VTU_VID_REG:
			case QD_REG_VTU_DATA1_REG:
			case QD_REG_VTU_DATA2_REG:
					break;
			case QD_REG_GLOBAL_CONTROL:
					if(value & 0x200)
					{
						/* Reload EEPROM values */
						qdSimRegsInit();
						qdSimRegs[portNumber][QD_REG_GLOBAL_STATUS] |= 0x1;
						return GT_TRUE;
					}
					break;
			case QD_REG_ATU_CONTROL:
					value &= ~0x8000;
					break;
			case QD_REG_ATU_OPERATION:
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] &= ~0xF;
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] |= (value & 0xF);
					status = qdSimATUOperation(value);
					return status;
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
					break;
			case QD_REG_STATS_OPERATION:
					status = qdSimStatsOperation(value);
					return status;
			case QD_REG_STATS_COUNTER3_2:
			case QD_REG_STATS_COUNTER1_0:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
					if(portNumber > 12)
					{
						qdSimRegs[portNumber][miiReg] &= ~QD_PORT_STATUS_DUPLEX;
						qdSimRegs[portNumber][miiReg] |= (value & QD_PORT_STATUS_DUPLEX);
						return GT_TRUE;
					}
			case QD_REG_SWITCH_ID:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
			case QD_REG_RATE_CTRL:
			case QD_REG_PAV:
					break;
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_Q_COUNTER:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					return qdSimPhyControl(portNumber,miiReg,value);
			case QD_PHY_INT_ENABLE_REG:
			case QD_PHY_AUTONEGO_AD_REG:
			case QD_PHY_NEXTPAGE_TX_REG:
			case QD_PHY_SPEC_CONTROL_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
			case QD_PHY_INT_PORT_SUMMARY_REG:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else
		return GT_FALSE;

	qdSimRegs[portNumber][miiReg] = (GT_U16)value;
	return GT_TRUE;
}

GT_BOOL qdSimWrite_6083 (unsigned int portNumber , unsigned int miiReg, unsigned int value)
{
	GT_BOOL status;

	if (IS_GLOBAL_REG(portNumber))	/* Global register */
	{
		switch(miiReg)
		{
			case QD_REG_GLOBAL_STATUS:
					/* readonly register */
					return GT_FALSE;
			case QD_REG_MACADDR_01:
			case QD_REG_MACADDR_23:
			case QD_REG_MACADDR_45:
					break;
			case QD_REG_VTU_OPERATION:
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] &= ~0xF;
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][5] |= (value & 0xF);
					status = qdSimVTUOperation(value);
					return status;
			case QD_REG_VTU_VID_REG:
			case QD_REG_VTU_DATA1_REG:
			case QD_REG_VTU_DATA2_REG:
			case QD_REG_VTU_DATA3_REG:
					break;
			case QD_REG_GLOBAL_CONTROL:
					if(value & 0x200)
					{
						/* Reload EEPROM values */
						qdSimRegsInit();
						qdSimRegs[portNumber][QD_REG_GLOBAL_STATUS] |= 0x1;
						return GT_TRUE;
					}
					break;
			case QD_REG_ATU_CONTROL:
					value &= ~0x8000;
					break;
			case QD_REG_ATU_OPERATION:
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] &= ~0xF;
					qdSimRegs[qdSimDev.qdSimGlobalRegBase][11] |= (value & 0xF);
					status = qdSimATUOperation(value);
					return status;
			case QD_REG_ATU_DATA_REG:
			case QD_REG_ATU_MAC_01:
			case QD_REG_ATU_MAC_23:
			case QD_REG_ATU_MAC_45:
			case QD_REG_IP_PRI_REG0:
			case QD_REG_IP_PRI_REG1:
			case QD_REG_IP_PRI_REG2:
			case QD_REG_IP_PRI_REG3:
			case QD_REG_IP_PRI_REG4:
			case QD_REG_IP_PRI_REG5:
			case QD_REG_IP_PRI_REG6:
			case QD_REG_IP_PRI_REG7:
			case QD_REG_IEEE_PRI:
					break;
			case QD_REG_STATS_OPERATION:
					status = qdSimStatsOperation(value);
					return status;
			case QD_REG_STATS_COUNTER3_2:
			case QD_REG_STATS_COUNTER1_0:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PORT_REG(portNumber))	/* Port registers */
	{
		switch(miiReg)
		{
			case QD_REG_PORT_STATUS:
					if(portNumber > 12)
					{
						qdSimRegs[portNumber][miiReg] &= ~QD_PORT_STATUS_DUPLEX;
						qdSimRegs[portNumber][miiReg] |= (value & QD_PORT_STATUS_DUPLEX);
						return GT_TRUE;
					}
			case QD_REG_SWITCH_ID:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_PORT_CONTROL:
			case QD_REG_PORT_VLAN_MAP:
			case QD_REG_PVID:
			case QD_REG_RATE_CTRL:
			case QD_REG_PAV:
					break;
			case QD_REG_RXCOUNTER:
			case QD_REG_TXCOUNTER:
					/* readonly registers */
					return GT_FALSE;
			case QD_REG_Q_COUNTER:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else if(IS_PHY_REG(portNumber))	/* phy registers */
	{
		switch(miiReg)
		{
			case QD_PHY_CONTROL_REG:
					return qdSimPhyControl(portNumber,miiReg,value);
			case QD_PHY_INT_ENABLE_REG:
			case QD_PHY_AUTONEGO_AD_REG:
			case QD_PHY_NEXTPAGE_TX_REG:
			case QD_PHY_SPEC_CONTROL_REG:
					break;
			case QD_PHY_INT_STATUS_REG:
			case QD_PHY_INT_PORT_SUMMARY_REG:
					return GT_FALSE;
			default:
					return GT_FALSE;
		}
	}
	else
		return GT_FALSE;

	qdSimRegs[portNumber][miiReg] = (GT_U16)value;
	return GT_TRUE;
}


GT_BOOL qdSimWrite (GT_QD_DEV *dev,unsigned int portNumber , unsigned int miiReg, unsigned int value)
{
	if (portNumber >= MAX_SMI_ADDRESS)
		portNumber -= MAX_SMI_ADDRESS;

	if ((portNumber >= MAX_SMI_ADDRESS) || (miiReg >= MAX_REG_ADDRESS))
		return GT_FALSE;

	switch(qdSimDev.qdSimDevId)
	{
		case GT_88E6051:
		case GT_88E6052:
			return qdSimWrite_6052(portNumber, miiReg, value);
		case GT_88E6021:
			return qdSimWrite_6021(portNumber, miiReg, value);
		case GT_88E6063:
		case GT_FF_HG:
		case GT_FF_EG:
		case GT_FH_VPN:
			return qdSimWrite_6063(portNumber, miiReg, value);
		case GT_88E6083:
			return qdSimWrite_6083(portNumber, miiReg, value);

		default:
			break;
	}

	return GT_TRUE;
}

GT_STATUS qdSimSetPhyInt(unsigned int portNumber, unsigned short u16Data)
{
	if(!qdSimDev.qdSimUsed)
		return GT_FAIL;

	qdSimRegs[portNumber][QD_PHY_INT_STATUS_REG] = u16Data;
	if(u16Data)
		qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG] |= (1<<portNumber);
	else
		qdSimRegs[0][QD_PHY_INT_PORT_SUMMARY_REG] &= ~(1<<portNumber);
	
	qdSimRegs[MAX_SMI_ADDRESS-1][QD_REG_GLOBAL_STATUS] |= 0x2;
	return GT_OK;
}

GT_STATUS qdSimSetGlobalInt(unsigned short u16Data)
{
	if(!qdSimDev.qdSimUsed)
		return GT_FAIL;

	qdSimRegs[MAX_SMI_ADDRESS-1][QD_REG_GLOBAL_STATUS] |= (u16Data & 0xF);
	return GT_OK;
}


void qdSimInit(GT_DEVICE devId, int baseAddr)
{
	qdSimDev.qdSimUsed = 1;

	qdSimDev.qdSimDevId = devId;
	qdSimDev.vtuSize = 0;

	qdSimDev.qdSimPhyBase = baseAddr;
	qdSimDev.qdSimPortBase = baseAddr + 0x8;
	qdSimDev.qdSimGlobalRegBase = baseAddr + 0xF;

	switch(devId)
	{
		case GT_88E6021:
			qdSimDev.vtuSize = 16;
			qdSimDev.qdSimNumOfPhys = 2;
			qdSimDev.qdSimNumOfPorts = 3;
			break;
		case GT_88E6051:
			qdSimDev.qdSimNumOfPhys = 5;
			qdSimDev.qdSimNumOfPorts = 6;
			break;
		case GT_88E6063:
		case GT_FH_VPN:
			qdSimDev.vtuSize = 64;
		case GT_88E6052:
		case GT_FF_HG:
		case GT_FF_EG:
			qdSimDev.qdSimNumOfPhys = 5;
			qdSimDev.qdSimNumOfPorts = 7;
			break;
		case GT_88E6083:
			qdSimDev.vtuSize = 64;
			qdSimDev.qdSimNumOfPhys = 8;
			qdSimDev.qdSimNumOfPorts = 10;
			qdSimDev.qdSimPhyBase = 0;
			qdSimDev.qdSimPortBase = 0x10;
			qdSimDev.qdSimGlobalRegBase = 0x1b;
			break;
		default:
			qdSimDev.vtuSize = 64;
			qdSimDev.qdSimDevId = GT_88E6063;
			qdSimDev.qdSimNumOfPhys = 5;
			qdSimDev.qdSimNumOfPorts = 7;
			break;
	}

	qdSimATUInit();
	qdSimVTUInit();
	qdSimRegsInit();

	return;
}
