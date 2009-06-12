#ifndef _STD_H_
#define _STD_H_

#ifndef BYTE 
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef LWORD
#define LWORD unsigned long
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef VLWORD
#define VLWORD volatile unsigned long
#endif

#ifndef BOOLEAN
#define BOOLEAN BYTE
#endif

#ifndef BOOL
#define BOOL BYTE
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL	((void *)0)
#endif

#define S8            signed char
#define S08            signed char
#define S16           signed short
#define S32           signed long
#define U8            unsigned char
#define U08            unsigned char
#define U16           unsigned short
#define U32           unsigned long
#define REG8          volatile unsigned char
#define REG16         volatile unsigned short
#define REG32         volatile unsigned long

#define UINT          unsigned int

#define std_Local       static
#define std_Void        void

#define STD_YES          1
#define STD_NO           0

#define STD_TRUE        STD_YES
#define STD_FALSE       STD_NO

#define STD_OK          STD_YES
#define STD_ERROR       STD_NO

#define STD_ON          STD_YES
#define STD_OFF         STD_NO

#define  BIT0        0x00000001
#define  BIT1        0x00000002
#define  BIT2        0x00000004
#define  BIT3        0x00000008
#define  BIT4        0x00000010
#define  BIT5        0x00000020
#define  BIT6        0x00000040
#define  BIT7        0x00000080
#define  BIT8        0x00000100
#define  BIT9        0x00000200
#define  BIT10       0x00000400
#define  BIT11       0x00000800
#define  BIT12       0x00001000
#define  BIT13       0x00002000
#define  BIT14       0x00004000
#define  BIT15       0x00008000
#define  BIT16       0x00010000
#define  BIT17       0x00020000
#define  BIT18       0x00040000
#define  BIT19       0x00080000
#define  BIT20       0x00100000
#define  BIT21       0x00200000
#define  BIT22       0x00400000
#define  BIT23       0x00800000
#define  BIT24       0x01000000
#define  BIT25       0x02000000
#define  BIT26       0x04000000
#define  BIT27       0x08000000
#define  BIT28       0x10000000
#define  BIT29       0x20000000
#define  BIT30       0x40000000
#define  BIT31       0x80000000

#endif
