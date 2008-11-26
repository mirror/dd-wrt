#include <Copyright.h>
/********************************************************************************
* utils.c
*
* DESCRIPTION:
*       Collection of Utility functions
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

static GT_U16 hashMode = 0;
int testPrintEnable = 1;

void testPrint(char* format, ...)
{
	va_list argP;
	char dbgStr[1000] = "";

	if(testPrintEnable == 0)
		return;

	va_start(argP, format);

	vsprintf(dbgStr, format, argP);

	printf("%s",dbgStr);

	return;
}

int vtuEntryCmpFunc(void* buf, int a, int b)
{
    GT_VTU_ENTRY* vtuEntry = (GT_VTU_ENTRY*)buf;

	if ((vtuEntry+a)->vid > (vtuEntry+b)->vid)
		return 1;
	else if ((vtuEntry+a)->vid < (vtuEntry+b)->vid)
		return -1;
	else
		return 0;
}

int atuEntryCmpFunc(void* buf, int a, int b)
{
	GT_ATU_ENTRY *atuEntry = (GT_ATU_ENTRY *)buf;
	GT_U8* aChar,* bChar;
	int i;

	aChar = (GT_U8*)((atuEntry+a)->macAddr.arEther);
	bChar = (GT_U8*)((atuEntry+b)->macAddr.arEther);

	for (i=0; i<6; i++)
	{
		if(*aChar > *bChar)
			return 1;
		else if (*aChar < *bChar)
			return -1;
		aChar++;
		bChar++;
	}

	return 0;

}

/* insertion sort */
GT_STATUS gtSort(int list[], GT_CMP_FUNC cmpFunc, void* buf, GT_U32 len)
{
	GT_U32 index;
	int curValue, preValue;

	if (len <= 1)
		return GT_FAIL;

	preValue = list[0];

	for (index = 1; index < len; ++index)
	{
		curValue = list[index];
		if ((*cmpFunc)(buf, preValue, curValue) > 0)
		{
			/* out of order: list[index-1] > list[index] */
			GT_U32 index2;
			list[index] = preValue; /* move up the larger item first */

			/* find the insertion point for the smaller item */
			for (index2 = index - 1; index2 > 0;)
			{
				int temp_val = list[index2 - 1];
				if ((*cmpFunc)(buf, temp_val, curValue) > 0)
				{
					list[index2--] = temp_val;
					/* still out of order, move up 1 slot to make room */
				}
				else
					break;
			}
			list[index2] = curValue; /* insert the smaller item right here */
		}
		else
		{
			/* in order, advance to next element */
			preValue = curValue;
		}
	}
	return GT_OK;
}
 

GT_U16 hashFunction(char eaddr[])
{
	GT_U16 crc_reg;
	GT_U8 crc_in;
	int i,j;

	crc_reg=0;
	
	for(i=0; i<6; i++)
	{
		crc_in = eaddr[5-i];
		for(j=0; j<8; j++)
		{
			crc_reg = ((((crc_in & 1) ^ ((crc_reg>>15) & 1)) ^ 
						((crc_reg>>14) & 1)) << 15)				|	/* bit 15 */
						((crc_reg & 0x3FFC) << 1) 				|	/* bit 14:3 */
						((((crc_in & 1) ^ ((crc_reg>>15) & 1)) ^ 
						((crc_reg>>1) & 1)) << 2) 				|	/* bit 2 */
						((crc_reg & 1) << 1)					| 	/* bit 1 */
						((crc_in & 1) ^ ((crc_reg>>15) & 1));		/* bit 0 */

			crc_in >>= 1;
		}
	}

	return crc_reg;
}


GT_U16 hashToBucket(GT_U16 hash, GT_U16 mode)
{
	GT_U16 bucket;

	switch (mode)
	{
		case 0:
			bucket = 
			(((hash >> 7) & 1) << 10) |		/* bit 10 */
			(((hash >> 11) & 1) << 9) |		/* bit 9 */
			(((hash >> 3 ) & 1) << 8) |		/* bit 8 */
			(((hash >> 14) & 1) << 7) |		/* bit 7 */
			(((hash >> 12) & 1) << 6) |		/* bit 6 */
			(((hash >> 10) & 1) << 5) |		/* bit 5 */
			(((hash >> 8 ) & 1) << 4) |		/* bit 4 */
			(((hash >> 6 ) & 1) << 3) |		/* bit 3 */
			(((hash >> 4 ) & 1) << 2) |		/* bit 2 */
			(((hash >> 2 ) & 1) << 1) |		/* bit 1 */
			(( hash >> 0 ) & 1); 			/* bit 0 */
			break;
		case 1:
			bucket = 
			(((hash >> 2) & 1) << 10) |		/* bit 10 */
			(((hash >> 12) & 1) << 9) |		/* bit 9 */
			(((hash >> 0 ) & 1) << 8) |		/* bit 8 */
			(((hash >> 10) & 1) << 7) |		/* bit 7 */
			(((hash >> 8 ) & 1) << 6) |		/* bit 6 */
			(((hash >> 7 ) & 1) << 5) |		/* bit 5 */
			(((hash >> 6 ) & 1) << 4) |		/* bit 4 */
			(((hash >> 5 ) & 1) << 3) |		/* bit 3 */
			(((hash >> 4 ) & 1) << 2) |		/* bit 2 */
			(((hash >> 3 ) & 1) << 1) |		/* bit 1 */
			(( hash >> 1 ) & 1); 			/* bit 0 */
			break;
		case 2:
			bucket = 
			(((hash >> 13) & 1) << 10)|		/* bit 10 */
			(((hash >> 15) & 1) << 9) |		/* bit 9 */
			(((hash >> 3 ) & 1) << 8) |		/* bit 8 */
			(((hash >> 14) & 1) << 7) |		/* bit 7 */
			(((hash >> 12) & 1) << 6) |		/* bit 6 */
			(((hash >> 11) & 1) << 5) |		/* bit 5 */
			(((hash >> 10) & 1) << 4) |		/* bit 4 */
			(((hash >> 9 ) & 1) << 3) |		/* bit 3 */
			(((hash >> 8 ) & 1) << 2) |		/* bit 2 */
			(((hash >> 7 ) & 1) << 1) |		/* bit 1 */
			(( hash >> 5 ) & 1); 			/* bit 0 */
			break;
		case 3:
			bucket = 
			(((hash >> 10) & 1) << 10)|		/* bit 10 */
			(((hash >> 8 ) & 1) << 9) |		/* bit 9 */
			(((hash >> 7 ) & 1) << 8) |		/* bit 8 */
			(((hash >> 13) & 1) << 7) |		/* bit 7 */
			(((hash >> 12) & 1) << 6) |		/* bit 6 */
			(((hash >> 10) & 1) << 5) |		/* bit 5 */
			(((hash >> 9 ) & 1) << 4) |		/* bit 4 */
			(((hash >> 6 ) & 1) << 3) |		/* bit 3 */
			(((hash >> 5 ) & 1) << 2) |		/* bit 2 */
			(((hash >> 3 ) & 1) << 1) |		/* bit 1 */
			(( hash >> 2 ) & 1); 			/* bit 0 */
			break;
		default:
			/* treat as case 0 */
			bucket = 
			(((hash >> 7) & 1) << 10) |		/* bit 10 */
			(((hash >> 11) & 1) << 9) |		/* bit 9 */
			(((hash >> 3 ) & 1) << 8) |		/* bit 8 */
			(((hash >> 14) & 1) << 7) |		/* bit 7 */
			(((hash >> 12) & 1) << 6) |		/* bit 6 */
			(((hash >> 10) & 1) << 5) |		/* bit 5 */
			(((hash >> 8 ) & 1) << 4) |		/* bit 4 */
			(((hash >> 6 ) & 1) << 3) |		/* bit 3 */
			(((hash >> 4 ) & 1) << 2) |		/* bit 2 */
			(((hash >> 2 ) & 1) << 1) |		/* bit 1 */
			(( hash >> 0 ) & 1); 			/* bit 0 */
			break;
	}
	return bucket;
}

GT_U16 dbNumMap(GT_U32 bucket, GT_U32 dbNum)
{
	return (GT_U16)(bucket + dbNum);
}

GT_U16 runQDHash(GT_U8* eaddr, GT_U16 dbNum, int bSize, GT_U16* pHash, GT_U16* preBucket, GT_U16* posBucket)
{
	GT_U16 hash, bucket;

	hash = hashFunction(eaddr);
	if (pHash)
		*pHash = hash;
	bucket = hashToBucket(hash,hashMode);
	bucket &= (bSize-1);
	if (preBucket)
		*preBucket = bucket;
	bucket = dbNumMap(bucket,dbNum);
	if (posBucket)
		*posBucket = bucket;
	bucket &= (bSize-1);

	return bucket;
}

GT_U16 hashTest(unsigned int maxMacs, int maxDbNum, int bSize)
{
	char eaddr[6] = {0,0,0,0,0,0};
	char buckets[MAX_BUCKET_SIZE];
	GT_U32 i, dbNum;
	GT_U16 hash, bucket, tmpBucket;

	gtMemSet(buckets,0,MAX_BUCKET_SIZE);

	for(dbNum=0; dbNum<(GT_U32)maxDbNum; dbNum++)
	{
		MSG_PRINT(("DBNum %i:\n", dbNum));
		for(i=1; i<maxMacs; i++)
		{
			eaddr[2] = (char)((i >> 24) & 0xff);
			eaddr[3] = (char)((i >> 16) & 0xff);
			eaddr[4] = (char)((i >> 8) & 0xff);
			eaddr[5] = (char)(i & 0xff);
			tmpBucket=runQDHash((GT_U8*)eaddr, (GT_U16)dbNum, bSize, &hash, &bucket, NULL);
			buckets[tmpBucket]++;
			MSG_PRINT(("EADDR : %02x-%02x-%02x-%02x, ", eaddr[2],eaddr[3],eaddr[4],eaddr[5]));
			MSG_PRINT(("Hash : %03x, ", hash));
			MSG_PRINT(("bucket : %03x, ", bucket));
			MSG_PRINT(("bins : %02x\n", buckets[tmpBucket]-1));
		}

	}

	return 0;
}

GT_U16 hashFindEntriesInBucket(unsigned int maxMacs, int bucketNum, int bSize)
{
	char eaddr[6] = {0,0,0,0,0,0};
	char buckets[MAX_BUCKET_SIZE];
	GT_U32 i, dbNum;
	GT_U16 hash, bucket, tmpBucket;

	gtMemSet(buckets,0,MAX_BUCKET_SIZE);

	dbNum = 0;
	MSG_PRINT(("DBNum %i:\n", dbNum));
	for(i=1; i<maxMacs; i++)
	{
		eaddr[2] = (char)((i >> 24) & 0xff);
		eaddr[3] = (char)((i >> 16) & 0xff);
		eaddr[4] = (char)((i >> 8) & 0xff);
		eaddr[5] = (char)(i & 0xff);
		tmpBucket=runQDHash((GT_U8*)eaddr, (GT_U16)dbNum, bSize, &hash, &bucket, NULL);
		buckets[tmpBucket]++;
		if (tmpBucket != bucketNum)
			continue;
		MSG_PRINT(("EADDR : %02x-%02x-%02x-%02x, ", eaddr[2],eaddr[3],eaddr[4],eaddr[5]));
		MSG_PRINT(("Hash : %03x, ", hash));
		MSG_PRINT(("bucket : %03x, ", bucket));
		MSG_PRINT(("bins : %02x\n", buckets[tmpBucket]-1));
	}

	return 0;
}

void displayHash(char* eaddr, GT_U16 dbNum, GT_U32 bSize)
{
	GT_U16 hash, posBucket, preBucket;

	posBucket=runQDHash(eaddr, dbNum, bSize, &hash, &preBucket, NULL);
	MSG_PRINT(("EADDR : %02x-%02x-%02x-%02x, ", eaddr[2],eaddr[3],eaddr[4],eaddr[5]));
	MSG_PRINT(("Hash : %03x, ", hash));
	MSG_PRINT(("bucket : %03x, ", preBucket));
	MSG_PRINT(("bucket(db) : %03x, ", posBucket));
	MSG_PRINT(("bSize : %03x\n", bSize));
}

GT_U16 hashWrap(int bSize, int entry)
{
	char eaddr[6] = {0,0,0,0,0,0};
	char buckets[MAX_BUCKET_SIZE];
	GT_U32 i, maxMacs;
	GT_U16 hash, bucket, tmpBucket, preBucket;
	int wrapping = 0;
	maxMacs =0xFFFFFFFF;
	gtMemSet(buckets,0,MAX_BUCKET_SIZE);

	MSG_PRINT(("Wrapped Entry :\n"));
	for(i=1; i<maxMacs; i++)
	{
		eaddr[2] = (char)((i >> 24) & 0xff);
		eaddr[3] = (char)((i >> 16) & 0xff);
		eaddr[4] = (char)((i >> 8) & 0xff);
		eaddr[5] = (char)(i & 0xff);
		tmpBucket = runQDHash(eaddr, 15, bSize, &hash, &preBucket, &bucket);
		buckets[tmpBucket]++;
		if(bucket != tmpBucket)
		{
			wrapping++;
			MSG_PRINT(("EADDR : %02x-%02x-%02x-%02x, ", eaddr[2],eaddr[3],eaddr[4],eaddr[5]));
			MSG_PRINT(("Hash : %03x, ", hash));
			MSG_PRINT(("bucket : %03x, ", preBucket));
			MSG_PRINT(("bucket(db) : %03x, ", tmpBucket));
			MSG_PRINT(("bins : %02x\n", buckets[tmpBucket]-1));
		}
		if (wrapping >= entry)
			break;
	}

	return wrapping;
}

/*
	This routine will create ATU Entry List.
	Input
		entrySize - entry size for each dbNum
		dbNumSize - number of DBNums
		sameMacs  - how many same MAC addresses are in the ATU database.
				 	Each of the same addresses are beloging to different DBNum.
*/
GT_U16 createATUList(GT_QD_DEV *dev,TEST_ATU_ENTRY entry[], GT_U16 entrySize, GT_U16 dbNumSize, GT_U16 sameMacs, GT_U16 bSize)
{
	GT_U16 i;
	char* buckets;
	GT_U16 dynamicMacs = 0;
	GT_U16 bucket,dbNum,binSize;
	GT_BOOL	exPrio, fqPri;

	if(dbNumSize == 0)
		return entrySize+1;
	if(entrySize < sameMacs)
		return entrySize+1;

	buckets = (char*)malloc(MAX_BUCKET_SIZE);
	if(buckets == NULL)
	{
		printf("No more available memories\n");
		return -1;
	}
	gtMemSet(buckets,0,MAX_BUCKET_SIZE);

#ifndef TEST_DEBUG
	srand((unsigned)time(NULL));
#else
	srand((unsigned)1);
#endif

	binSize = 4;
	switch(dev->deviceId)
	{
		case GT_88E6131:
		case GT_88E6108:
			hashMode = 1;
			exPrio = GT_FALSE;
			fqPri = GT_FALSE;
			break;
		case GT_88E6031:
		case GT_88E6061:
			hashMode = 1;
			exPrio = GT_TRUE;
			fqPri = GT_FALSE;
			break;
		case GT_88E6035:
		case GT_88E6055:
		case GT_88E6065:
			hashMode = 1;
			exPrio = GT_TRUE;
			fqPri = GT_TRUE;
			break;
		default:
			exPrio = GT_FALSE;
			fqPri = GT_FALSE;
			hashMode = 0;
			break;
	}			

	for(i=0; i<entrySize; i++)
	{
		if(sameMacs)
		{
			memset(&entry[0].atuEntry[i],0,sizeof(GT_ATU_ENTRY));
			
			*(GT_U16*)entry[0].atuEntry[i].macAddr.arEther = (GT_U16)rand();
			*(GT_U16*)(entry[0].atuEntry[i].macAddr.arEther+2) = (GT_U16)(rand() & 0xFFFF);

			do 
			{
				GT_U16 tmpBucket[256];
				*(GT_U16*)(entry[0].atuEntry[i].macAddr.arEther+4) = (GT_U16)(rand() & 0xFFFF);

				for (dbNum=0; dbNum<dbNumSize; dbNum++)
				{
					/* In current implementation, each dbNum will located in different bucket. */

					tmpBucket[dbNum] = runQDHash(entry[0].atuEntry[i].macAddr.arEther, dbNum, bSize, NULL,NULL,NULL);
					if (buckets[tmpBucket[dbNum]] >= binSize)
					{
						/* if bucket is full, find another entry. */
						break;
					}
					else
					{
						continue;
					}
				}

				if (dbNum == dbNumSize)
				{
					/* we found a entry which meets our requirement. */
					entry[0].atuEntry[i].portVec = (GT_U32)(rand() & 0x3FF) % (1<<dev->numOfPorts);
					if(entry[0].atuEntry[i].portVec == 0)
						entry[0].atuEntry[i].portVec = 0x3;
					if(!exPrio)
					{
						entry[0].atuEntry[i].prio = (GT_U8)(rand() & 0x3);
					}
					else
					{
						entry[0].atuEntry[i].exPrio.macQPri = (GT_U8)(rand() & 0x3);
						if(fqPri)
						{
							entry[0].atuEntry[i].exPrio.macFPri = (GT_U8)(rand() & 0x7);
							entry[0].atuEntry[i].exPrio.useMacFPri = GT_TRUE;
						}
					}

					if(entry[0].atuEntry[i].macAddr.arEther[0] & 0x1)
					{
						entry[0].atuEntry[i].entryState.mcEntryState = GT_MC_STATIC;
					}
					else
					{
						entry[0].atuEntry[i].entryState.ucEntryState = GT_UC_DYNAMIC;
						dynamicMacs+=dbNumSize;
					}

					buckets[tmpBucket[0]]++;
					for (dbNum=1; dbNum<dbNumSize; dbNum++)
					{
						buckets[tmpBucket[dbNum]]++;
						memcpy(&entry[dbNum].atuEntry[i],&entry[0].atuEntry[i],sizeof(GT_ATU_ENTRY));
						entry[dbNum].atuEntry[i].DBNum = (GT_U8)dbNum;
					}
#ifdef TEST_DEBUG
					MSG_PRINT(("MAC : %02x-%02x-%02x-%02x-%02x-%02x, ",
									entry[dbNum].atuEntry[i].macAddr.arEther[0],
									entry[dbNum].atuEntry[i].macAddr.arEther[1],
									entry[dbNum].atuEntry[i].macAddr.arEther[2],
									entry[dbNum].atuEntry[i].macAddr.arEther[3],
									entry[dbNum].atuEntry[i].macAddr.arEther[4],
									entry[dbNum].atuEntry[i].macAddr.arEther[5] ));
					MSG_PRINT(("dbNum 0 ~ %x, bucket %03x ~ %03x\n", dbNumSize, tmpBucket[0], tmpBucket[dbNumSize-1]));
#endif
					break;	/* we are done with current dbNum. so exit the while loop */
				}
			} while (1);

			sameMacs--;
		}
		else
		{
			for(dbNum=0; dbNum<dbNumSize; dbNum++)
			{
				memset(&entry[dbNum].atuEntry[i],0,sizeof(GT_ATU_ENTRY));
			
				*(GT_U16*)entry[dbNum].atuEntry[i].macAddr.arEther = (GT_U16)rand();
				*(GT_U16*)(entry[dbNum].atuEntry[i].macAddr.arEther+2) = (GT_U16)(rand() & 0xFFFF);

				do 
				{
					*(GT_U16*)(entry[dbNum].atuEntry[i].macAddr.arEther+4) = (GT_U16)(rand() & 0xFFFF);

					bucket=runQDHash(entry[dbNum].atuEntry[i].macAddr.arEther, dbNum, bSize, NULL,NULL,NULL);
					if (buckets[bucket] >= binSize)
						continue;
					else
					{
#ifdef TEST_DEBUG
						MSG_PRINT(("MAC : %02x-%02x-%02x-%02x-%02x-%02x, ",
									entry[dbNum].atuEntry[i].macAddr.arEther[0],
									entry[dbNum].atuEntry[i].macAddr.arEther[1],
									entry[dbNum].atuEntry[i].macAddr.arEther[2],
									entry[dbNum].atuEntry[i].macAddr.arEther[3],
									entry[dbNum].atuEntry[i].macAddr.arEther[4],
									entry[dbNum].atuEntry[i].macAddr.arEther[5] ));
						MSG_PRINT(("dbNum : %04x, ", dbNum));
						MSG_PRINT(("bucket : %03x, bins : %02x\n", bucket,buckets[bucket]));
#endif
						buckets[bucket]++;
						break;
					}

				} while (1);

				entry[dbNum].atuEntry[i].portVec = (GT_U32)(rand() & 0x3FF) % (1<<dev->numOfPorts);
				if(entry[dbNum].atuEntry[i].portVec == 0)
					entry[dbNum].atuEntry[i].portVec = 0x3;
				if(!exPrio)
				{
					entry[dbNum].atuEntry[i].prio = (GT_U8)(rand() & 0x3);
				}
				else
				{
					entry[dbNum].atuEntry[i].exPrio.macQPri = (GT_U8)(rand() & 0x3);
					if(fqPri)
					{
						entry[dbNum].atuEntry[i].exPrio.macFPri = (GT_U8)(rand() & 0x7);
						entry[dbNum].atuEntry[i].exPrio.useMacFPri = GT_TRUE;
					}
				}
				entry[dbNum].atuEntry[i].DBNum = (GT_U8)dbNum;

				if(entry[dbNum].atuEntry[i].macAddr.arEther[0] & 0x1)
				{
					entry[dbNum].atuEntry[i].entryState.mcEntryState = GT_MC_STATIC;
				}
				else
				{
					entry[dbNum].atuEntry[i].entryState.ucEntryState = GT_UC_DYNAMIC;
					dynamicMacs++;
				}
			}
		}
	}

	free(buckets);

	return dynamicMacs;
}


GT_STATUS testFixedAtu(GT_QD_DEV *dev,GT_U8 dbNum,GT_U8 atuSize)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry[16];
	int i;
	GT_U32 u32Data1;

	/* Set ATU Size will cause ATU reset and SW reset, so call before any other setup. */
	MSG_PRINT(("Setting ATU Size\n"));
	if((status = gfdbSetAtuSize(dev,atuSize)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAtuSize returned fail.\n"));
		return status;
	}

	/* Disable Aging */
	MSG_PRINT(("Disable Aging Timeout... \n"));
	if((status = gfdbSetAgingTimeout(dev,0)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAgingTimeout returned fail.\n"));
		return status;
	}

	/* Disable Learning */
	MSG_PRINT(("Disable Learning... \n"));
	if((status = gfdbLearnEnable(dev,GT_FALSE)) != GT_OK)
	{
		MSG_PRINT(("gfdbSetAtuSize returned fail.\n"));
		return status;
	}

	/* Flush all addresses from the ATU table. */
	MSG_PRINT(("Flush out all the entries in the ATU Table ... \n"));
	if((status = gfdbFlush(dev,GT_FLUSH_ALL)) != GT_OK)
	{
		MSG_PRINT(("gfdbFlush returned fail.\n"));
		return status;
	}

	/* Get Atu Dynamic Count, which should be 0, since we flush them all. */
	if((status = gfdbGetAtuDynamicCount(dev,&u32Data1)) != GT_OK)
	{
		MSG_PRINT(("gfdbGetAtuDynamicCount returned fail.\n"));
		return status;
	}

	MSG_PRINT(("Atu Dynamic Count : %d.\n", u32Data1));

	/* Now ATU table is clean. Play with our own MAC entries */	
	MSG_PRINT(("Setup Testing Table... \n"));

	gtMemSet(macEntry,0,sizeof(macEntry));

	/* bucket 0xee */
	*(GT_U16*)&macEntry[0].macAddr.arEther[4] = 0xd1b;
	macEntry[0].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[0].DBNum = dbNum;

	*(GT_U16*)&macEntry[1].macAddr.arEther[4] = 0xd1e;
	macEntry[1].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[1].DBNum = dbNum;

	*(GT_U16*)&macEntry[2].macAddr.arEther[4] = 0xddb;
	macEntry[2].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[2].DBNum = dbNum;

	*(GT_U16*)&macEntry[3].macAddr.arEther[4] = 0xdde;
	macEntry[3].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[3].DBNum = dbNum;

	/* bucket 0xef */
	*(GT_U16*)&macEntry[4].macAddr.arEther[4] = 0xd1a;
	macEntry[4].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[4].DBNum = dbNum;

	*(GT_U16*)&macEntry[5].macAddr.arEther[4] = 0xd1f;
	macEntry[5].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[5].DBNum = dbNum;

	*(GT_U16*)&macEntry[6].macAddr.arEther[4] = 0xdda;
	macEntry[6].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[6].DBNum = dbNum;

	*(GT_U16*)&macEntry[7].macAddr.arEther[4] = 0xddf;
	macEntry[7].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[7].DBNum = dbNum;

	/* bucket 0xf0 */
	*(GT_U16*)&macEntry[8].macAddr.arEther[4] = 0x440;
	macEntry[8].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[8].DBNum = dbNum;

	*(GT_U16*)&macEntry[9].macAddr.arEther[4] = 0x485;
	macEntry[9].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[9].DBNum = dbNum;

	*(GT_U16*)&macEntry[10].macAddr.arEther[4] = 0x1123;
	macEntry[10].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[10].DBNum = dbNum;

	*(GT_U16*)&macEntry[11].macAddr.arEther[4] = 0x1223;
	macEntry[11].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[11].DBNum = dbNum;

	/* bucket 0xf0 */
	*(GT_U16*)&macEntry[12].macAddr.arEther[4] = 0x441;
	macEntry[12].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[12].DBNum = dbNum;

	*(GT_U16*)&macEntry[13].macAddr.arEther[4] = 0x484;
	macEntry[13].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[13].DBNum = dbNum;

	*(GT_U16*)&macEntry[14].macAddr.arEther[4] = 0xb41;
	macEntry[14].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[14].DBNum = dbNum;

	*(GT_U16*)&macEntry[15].macAddr.arEther[4] = 0x444;
	macEntry[15].entryState.ucEntryState = GT_UC_DYNAMIC;
	macEntry[15].DBNum = dbNum;

	
	for(i=0; i<16; i++)
	{
		displayHash(macEntry[i].macAddr.arEther,dbNum,64<<atuSize);
		if((status = gfdbAddMacEntry(dev,&macEntry[i])) != GT_OK)
		{
			MSG_PRINT(("gfdbAddMacEntry returned fail.\n"));
			return status;
		}
	}

	testDisplayATUList(dev);
	return GT_OK;

}

