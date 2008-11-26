#include "mv_include.h"
#include "core_inter.h"

#ifdef RAID_DRIVER
#ifdef SOFTWARE_XOR

/*
 * Software XOR operations
 */
void mvXORWrite (MV_PVOID This, PMV_XOR_Request pXORReq);
void mvXORCompare (MV_PVOID This, PMV_XOR_Request pXORReq);
void mvXORDMA (MV_PVOID This, PMV_XOR_Request pXORReq);

void Core_ModuleSendXORRequest(MV_PVOID This, PMV_XOR_Request pXORReq)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;

	switch (pXORReq->Request_Type) 
	{
		case XOR_REQUEST_WRITE:
			mvXORWrite (pCore, pXORReq);
			break;
		case XOR_REQUEST_COMPARE:
			mvXORCompare (pCore, pXORReq);
			break;
		case XOR_REQUEST_DMA:
			mvXORDMA (pCore, pXORReq);
			break;
		default:
			pXORReq->Request_Status = XOR_STATUS_INVALID_REQUEST;
			break;
	}
	pXORReq->Completion( pXORReq->Cmd_Initiator, pXORReq );
}

void mvXORInit(
	PMV_SG_Entry            *pSGPtr,
	MV_PU32	                SGSizePtr,
	MV_PVOID                *pVirPtr,
	PMV_SG_Table            SGListPtr,
	MV_U8                   tableCount,
	MV_PU32                 minSizePtr)
{
	MV_U8 id;
	for ( id=0; id<tableCount; id++ ) {
		pSGPtr[id] = SGListPtr[id].Entry_Ptr;
		pVirPtr[id] = (MV_PVOID)
			( (MV_PTR_INTEGER)pSGPtr[id]->Base_Address 
			| (MV_PTR_INTEGER)pSGPtr[id]->Base_Address_High<<32 );
		SGSizePtr[id] = pSGPtr[id]->Size;
		if ( *minSizePtr > SGSizePtr[id] ) *minSizePtr=SGSizePtr[id];
	}
}

void mvXORUpdateEntry(
	PMV_SG_Entry	*pSGPtr,
	MV_PU32			SGSizePtr,
	MV_PVOID		*pVirPtr,
	MV_U32			finishSize,
	MV_U8			tableCount,
	MV_PU32			minSizePtr)
{
	MV_U8 id;
	for ( id=0; id<tableCount; id++ ) {
		if ( SGSizePtr[id] > finishSize )
			SGSizePtr[id] -= finishSize;
		else {
			pSGPtr[id]++;
			pVirPtr[id] = (MV_PVOID)
					( (MV_PTR_INTEGER)pSGPtr[id]->Base_Address 
					| (MV_PTR_INTEGER)pSGPtr[id]->Base_Address_High<<32 );
			SGSizePtr[id] = pSGPtr[id]->Size;
		}
		if ( *minSizePtr > SGSizePtr[id] ) *minSizePtr=SGSizePtr[id];
	}
}

MV_U8 mvXORByte(
	MV_PU8			*pSourceVirPtr,
	PMV_XOR_Request	pXORReq,
	MV_U8			tId
)
{
	MV_U8 xorResult, sId;

	xorResult = GF_Multiply(*pSourceVirPtr[0], pXORReq->Coef[tId][0]);
	for ( sId=1; sId<pXORReq->Source_SG_Table_Count; sId++ ) {
		xorResult = GF_Add(xorResult,
						   GF_Multiply(*pSourceVirPtr[sId], pXORReq->Coef[tId][sId]));
	}
	return xorResult;
}

#ifdef SUPPORT_XOR_DWORD
MV_U32 mvXORDWord(
	MV_PU32			*pSourceVirPtr,
	PMV_XOR_Request	pXORReq,
	MV_U8			tId
)
{
	MV_U8	sId;
	MV_U32 xorResult;

	xorResult = GF_Multiply(*pSourceVirPtr[0], pXORReq->Coef[tId][0]);
	for ( sId=1; sId<pXORReq->Source_SG_Table_Count; sId++ ) {
		xorResult = GF_Add(xorResult,
						   GF_Multiply(*pSourceVirPtr[sId], pXORReq->Coef[tId][sId]));
	}
	return xorResult;
}
#endif

/* The SG Table should have the virtual address instead of the physical address. */
void mvXORWrite(MV_PVOID This, PMV_XOR_Request pXORReq)
{
	PMV_SG_Entry	pSourceSG[XOR_SOURCE_SG_COUNT];
	PMV_SG_Entry	pTargetSG[XOR_TARGET_SG_COUNT];
	MV_U32			sourceSize[XOR_SOURCE_SG_COUNT];
	MV_U32			targetSize[XOR_TARGET_SG_COUNT];
	MV_U32 i;
	MV_U8 sId,tId;									/* source index and target index. */
	MV_U32 size, remainSize, minSize;
#ifdef SUPPORT_XOR_DWORD
	MV_PU32			pSourceVir[XOR_SOURCE_SG_COUNT];
	MV_PU32			pTargetVir[XOR_TARGET_SG_COUNT];
	MV_U32			xorResult, Dword_size;
#else
	MV_PU8			pSourceVir[XOR_SOURCE_SG_COUNT];
	MV_PU8			pTargetVir[XOR_TARGET_SG_COUNT];
	MV_U8			xorResult;
#endif

	/* Initialize these two variables. */
	remainSize = pXORReq->Source_SG_Table_List[0].Byte_Count;	/* All the SG table should have same Byte_Count */
	minSize = remainSize;
	/* Initialize XOR source */
	mvXORInit(pSourceSG, sourceSize, (MV_PVOID)pSourceVir,
			  pXORReq->Source_SG_Table_List,
			  pXORReq->Source_SG_Table_Count,
			  &minSize);

	/* Initialize XOR target */
	mvXORInit(pTargetSG, targetSize, (MV_PVOID)pTargetVir,
			  pXORReq->Target_SG_Table_List,
			  pXORReq->Target_SG_Table_Count,
			  &minSize);

/*
	for ( sId=0; sId<pXORReq->Source_SG_Table_Count; sId++ ) 
	{
		pSourceSG[sId] = pXORReq->Source_SG_Table_List[sId].Entry;
		sourceSize[sId] = pSourceSG[sId]->Size;
		pSourceVir[sId] = (MV_PVOID)
			( (MV_PTR_INTEGER)pSourceSG[sId]->Base_Address 
			| (MV_PTR_INTEGER)pSourceSG[sId]->Base_Address_High<<32 );
		MV_DASSERT( remainSize==pXORReq->Source_SG_Table_List[sId].Byte_Count );
		if ( minSize>sourceSize[sId] ) minSize=sourceSize[sId];
	}

	for ( tId=0; tId<pXORReq->Target_SG_Table_Count; tId++ ) 
	{
		pTargetSG[tId] = pXORReq->Target_SG_Table_List[tId].Entry;
		targetSize[tId] = pTargetSG[tId]->Size;
		pTargetVir[tId] = (MV_PVOID)
			( (MV_PTR_INTEGER)pTargetSG[tId]->Base_Address 
			| (MV_PTR_INTEGER)pTargetSG[tId]->Base_Address_High<<32 );
		MV_DASSERT( remainSize==pXORReq->Target_SG_Table_List[tId].Byte_Count );
		if ( minSize>targetSize[tId] ) minSize=targetSize[tId];
	}
*/

	/* Navigate all the SG table, calculate the target xor value. */
	while ( remainSize>0 ) 
	{
		size = minSize;
#ifdef SUPPORT_XOR_DWORD
		MV_DASSERT( !(size%4) );
		Dword_size = size/4;
		for ( i=0; i<Dword_size; i++ ) 
#else
		for ( i=0; i<size; i++ ) 
#endif
		{
			for ( tId=0; tId<pXORReq->Target_SG_Table_Count; tId++ )
			{
#ifdef SUPPORT_XOR_DWORD
				xorResult = mvXORDWord(pSourceVir, pXORReq, tId);
#else
				xorResult = mvXORByte(pSourceVir, pXORReq, tId);
#endif

/*
				tmp = GF_Multiply(*pSourceVir[0], pXORReq->Coef[tId][0]);

				for ( sId=1; sId<pXORReq->Source_SG_Table_Count; sId++ )
				{
					tmp = GF_Add(tmp,
								GF_Multiply(*pSourceVir[sId], pXORReq->Coef[tId][sId]));
				}
				*pTargetVir[tId] = tmp;
*/
				*pTargetVir[tId] = xorResult;
				pTargetVir[tId]++;
			}

			for ( sId=0; sId<pXORReq->Source_SG_Table_Count; sId++ )
				pSourceVir[sId]++;
		}

		/* Update entry pointer, size */
		MV_DASSERT( remainSize>=size );
		remainSize -= size;
		minSize = remainSize;
		/* Update XOR source */
		mvXORUpdateEntry(pSourceSG, sourceSize, (MV_PVOID)pSourceVir,
						 size, pXORReq->Source_SG_Table_Count, &minSize);
		/* Update XOR target */
		mvXORUpdateEntry(pTargetSG, targetSize, (MV_PVOID)pTargetVir,
						 size, pXORReq->Target_SG_Table_Count, &minSize);
/*

		for ( sId=0; sId<pXORReq->Source_SG_Table_Count; sId++ )
		{
			if ( sourceSize[sId]>size )
			{
				sourceSize[sId]-=size;
			}
			else
			{
				pSourceSG[sId]++;
				pSourceVir[sId] = (MV_PVOID)
					( (MV_PTR_INTEGER)pSourceSG[sId]->Base_Address | (MV_PTR_INTEGER)pSourceSG[sId]->Base_Address_High<<32 );
				sourceSize[sId] = pSourceSG[sId]->Size;
			}
			if ( minSize>sourceSize[sId] ) minSize=sourceSize[sId];
		}

		for ( tId=0; tId<pXORReq->Target_SG_Table_Count; tId++ )
		{
			if ( targetSize[tId]>size )
			{
				targetSize[tId]-=size;
			}
			else
			{
				pTargetSG[tId]++;
				pTargetVir[tId] = (MV_PVOID)
					( (MV_PTR_INTEGER)pTargetSG[tId]->Base_Address | (MV_PTR_INTEGER)pTargetSG[tId]->Base_Address_High<<32 );
				targetSize[tId] = pTargetSG[tId]->Size;
			}
			if ( minSize>targetSize[tId] ) minSize=targetSize[tId];
		}
*/
	}

	pXORReq->Request_Status = XOR_STATUS_SUCCESS;
}

//TBD: consolidate compare and write
void mvXORCompare (MV_PVOID This, PMV_XOR_Request pXORReq)
{
	PMV_SG_Entry	pSourceSG[XOR_SOURCE_SG_COUNT];
	MV_U32			sourceSize[XOR_SOURCE_SG_COUNT];
	MV_U32			totalSize, remainSize, minSize, size, i;
	MV_U8			sId;
#ifdef SUPPORT_XOR_DWORD
	MV_PU32			pSourceVir[XOR_SOURCE_SG_COUNT];
	MV_U32			xorResult, Dword_size;
#else
	MV_PU8			pSourceVir[XOR_SOURCE_SG_COUNT];
	MV_U8			xorResult;
#endif

	/* All the SG table should have same Byte_Count */
	totalSize = remainSize = minSize = pXORReq->Source_SG_Table_List[0].Byte_Count;
	mvXORInit(pSourceSG, sourceSize, (MV_PVOID)pSourceVir,
			  pXORReq->Source_SG_Table_List,
			  pXORReq->Source_SG_Table_Count,
			  &minSize);
	while ( remainSize>0 ) {
		size = minSize;
#ifdef SUPPORT_XOR_DWORD
		MV_DASSERT( !(size%4) );
		Dword_size = size/4;
		for ( i=0; i<Dword_size; i++ ) {
			xorResult = mvXORDWord(pSourceVir, pXORReq, 0);
#else
		for ( i=0; i<size; i++ ) {
			xorResult = mvXORByte(pSourceVir, pXORReq, 0);
#endif
			if (xorResult != 0)	{
				pXORReq->Request_Status = XOR_STATUS_ERROR;
#ifdef SUPPORT_XOR_DWORD
				pXORReq->Error_Offset = totalSize - remainSize + i*4;
#else
				pXORReq->Error_Offset = totalSize - remainSize + i;
#endif
				return;
			}
			for ( sId=0; sId<pXORReq->Source_SG_Table_Count; sId++ )
				pSourceVir[sId]++;
		}

		/* Update entry pointer, size */
		MV_DASSERT( remainSize>=size );
		remainSize -= size;
		minSize = remainSize;
		mvXORUpdateEntry(pSourceSG, sourceSize, (MV_PVOID)pSourceVir,
						 size, pXORReq->Source_SG_Table_Count, &minSize);
	}
}

void mvXORDMA (MV_PVOID This, PMV_XOR_Request pXORReq)
{
	MV_ASSERT( MV_FALSE );
}

#endif	/* SOFTWARE_XOR */
#endif	/* RAID_DRIVER */

