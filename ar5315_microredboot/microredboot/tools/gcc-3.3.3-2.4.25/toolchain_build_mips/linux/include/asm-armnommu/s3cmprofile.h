/***********************************************************************
 * 
 * Profiling Header
 * Copyright (c) 2002 Arcturus Networks Inc.
 *                    by MaTed <www.ArcturusNetworks.com>
 *
 ***********************************************************************/
/* s3cprofile.h */
/*
 * modification history
 * --------------------
 *
 * 	08/02/02 MaTed <www.ArcturusNetworks.com>
 *			- Initial version for uClinux - Samsung
 *			- please set TABS == 4
 *
 */


//#ifdef MPROFILE		// enable miniprofile
#undef MPROFILE
#if 0
#define ProfileArraySize 2048
typedef struct
{
  unsigned short	code;
  void *			addr;
  unsigned long		jiffie;
  unsigned long		counter;
} PROFILE_ARRAY;

#ifdef S3C2500_ETHERNET
PROFILE_ARRAY profileArray[ ProfileArraySize ];
UINT32	profileAddrIndex = 0;
#else
extern PROFILE_ARRAY profileArray[ ProfileArraySize ];
extern unsigned long	profileAddrIndex;
#endif

extern unsigned long volatile jiffies; 

#define COUNTER_REG		0xf0040014		// Counter register - runs at bus speed
#define enterCnt( profileCode )				\
({										\
   __label__ entryAddr;					\
entryAddr:								\
	profileArray[ profileAddrIndex ].code = profileCode;	\
	profileArray[ profileAddrIndex ].addr = && entryAddr;	\
	profileArray[ profileAddrIndex ].counter				\
	  = (*((unsigned long *) 0xf0040010) -*((unsigned long *) 0xf0040014));	\
	profileArray[ profileAddrIndex ].jiffie = jiffies;		\
	if (++profileAddrIndex >= ProfileArraySize)				\
	  profileAddrIndex = 0;									\
})

#define enterCntVal( profileCode, value )				\
({										\
	profileArray[ profileAddrIndex ].code = profileCode;	\
	profileArray[ profileAddrIndex ].addr = (void*) value;	\
	profileArray[ profileAddrIndex ].counter				\
	  = (*((unsigned long *) 0xf0040010) -*((unsigned long *) 0xf0040014));	\
	profileArray[ profileAddrIndex ].jiffie = jiffies;		\
	if (++profileAddrIndex >= ProfileArraySize)				\
	  profileAddrIndex = 0;									\
})
#else // MPROFILE
#define enterCnt( profileCode)
#define enterCntVal( profileCode, value)
#endif // MPROFILE
