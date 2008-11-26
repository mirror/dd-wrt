#include "mv_include.h"

MV_VOID MV_SetSenseData(
	IN PMV_Sense_Data pSense,
	IN MV_U8 SenseKey,
    IN MV_U8 AdditionalSenseCode,
	IN MV_U8 ASCQ
	)
{
	/* The caller should make sure it's a valid sense buffer. */
	MV_DASSERT( pSense!=NULL );

    MV_ZeroMemory(pSense, sizeof(MV_Sense_Data));

    pSense->Valid = 0;	//TBD: Why?
	pSense->ErrorCode = MV_SCSI_RESPONSE_CODE;
	pSense->SenseKey = SenseKey;
	pSense->AdditionalSenseCode = AdditionalSenseCode;
	pSense->AdditionalSenseCodeQualifier = ASCQ;
	pSense->AdditionalSenseLength = sizeof(MV_Sense_Data) - 8;
}

