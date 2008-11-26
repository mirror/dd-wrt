#if !defined(FIS_H)
#define FIS_H

/* SATA FIS: Register-Host to Device*/
typedef struct _SATA_FIS_REG_H2D
{
	MV_U8	FIS_Type;
#ifdef __MV_BIG_ENDIAN_BITFIELD__
	MV_U8	C : 1;
	MV_U8	Reserved0 : 3;
	MV_U8	PM_Port : 4;
#else
	MV_U8	PM_Port : 4;
	MV_U8	Reserved0 : 3;
	MV_U8	C : 1;
#endif /* __MV_BIG_ENDIAN_BITFIELD__ */
	MV_U8	Command;
	MV_U8	Features;

	MV_U8	LBA_Low;
	MV_U8	LBA_Mid;
	MV_U8	LBA_High;
	MV_U8	Device;

	MV_U8	LBA_Low_Exp;
	MV_U8	LBA_Mid_Exp;
	MV_U8	LBA_High_Exp;
	MV_U8	Features_Exp;

	MV_U8	Sector_Count;
	MV_U8	Sector_Count_Exp;
	MV_U8	Reserved1;
	MV_U8	Control;

	MV_U8	Reserved2[4];
} SATA_FIS_REG_H2D, *PSATA_FIS_REG_H2D;

/* FIS type definition */
#define SATA_FIS_TYPE_REG_H2D			0x27	/* Register FIS - Host to Device */
#define SATA_FIS_TYPE_REG_D2H			0x34	/* Register FIS - Device to Host */

#define SATA_FIS_TYPE_DMA_ACTIVATE		0x39	/* DMA Activate FIS - Device to Host */
#define SATA_FIS_TYPE_DMA_SETUP			0x41	/* DMA Setup FIS - Bi-directional */

#define SATA_FIS_TYPE_DATA				0x46	/* Data FIS - Bi-directional */
#define SATA_FIS_TYPE_BIST_ACTIVATE		0x58	/* BIST Activate FIS - Bi-directional */
#define SATA_FIS_TYPE_PIO_SETUP			0x5F	/* PIO Setup FIS - Device to Host */
#define SATA_FIS_TYPE_SET_DEVICE_BITS	0xA1	/* Set Device Bits FIS - Device to Host */

#endif /* FIS_H */

