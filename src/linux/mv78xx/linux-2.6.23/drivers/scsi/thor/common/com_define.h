#ifndef COM_DEFINE_H
#define COM_DEFINE_H


/*
 *	This file defines Marvell OS independent primary data type for all OS.
 *
 *	We have macros to differentiate different CPU and OS.
 *
 *	CPU definitions:
 *	_CPU_X86_16B	
 *	Specify 16bit x86 platform, this is used for BIOS and DOS utility.
 *	_CPU_X86_32B
 *	Specify 32bit x86 platform, this is used for most OS drivers.
 *	_CPU_IA_64B
 *	Specify 64bit IA64 platform, this is used for IA64 OS drivers.
 *	_CPU_AMD_64B
 *	Specify 64bit AMD64 platform, this is used for AMD64 OS drivers.
 *
 *	OS definitions:
 *	_OS_WINDOWS
 *	_OS_LINUX
 *	_OS_FREEBSD
 *	_OS_BIOS
 */
 
#if !defined(IN)
	#define IN
#endif

#if !defined(OUT)
	#define OUT
#endif

#if defined(_OS_LINUX)
	#define BUFFER_PACKED		__attribute__((packed))
#elif defined(_OS_WINDOWS)
	#define BUFFER_PACKED
#elif defined(_OS_BIOS)
	#define BUFFER_PACKED
#endif

#define MV_BIT(x)			(1L << (x))

#if !defined(NULL)
#define NULL 0
#endif 

#define MV_TRUE							1
#define MV_FALSE						0

typedef unsigned char	MV_BOOLEAN, *MV_PBOOLEAN;
typedef unsigned char	MV_U8, *MV_PU8;
typedef signed char	MV_I8, *MV_PI8;

typedef unsigned short	MV_U16, *MV_PU16;
typedef signed short	MV_I16, *MV_PI16;

typedef void		MV_VOID, *MV_PVOID;

#ifdef _OS_BIOS
typedef MV_U8 GEN_FAR* MV_LPU8;
typedef MV_I8 GEN_FAR* MV_LPI8;
typedef MV_U16 GEN_FAR* MV_LPU16;
typedef MV_I16 GEN_FAR* MV_LPI16;

typedef MV_U32 GEN_FAR* MV_LPU32;
typedef MV_I32 GEN_FAR* MV_LPI32;
typedef void GEN_FAR* MV_LPVOID;
#else
typedef void		*MV_LPVOID;
#endif

/* Pre-define segment in C code*/
#if defined(_OS_BIOS)
#define BASEATTR __based(__segname("_CODE")) 
#define BASEATTRData __based(__segname("_CODE")) 
#else
#define BASEATTR 
#endif

/* For debug version only */
#ifdef DEBUG_BIOS
	#define MV_DUMP32(_x_) mvDebugDumpU32(_x_)
	#define MV_DUMP16(_x_)  mvDebugDumpU16(_x_)
	#define MV_DUMP8(_x_)  mvDebugDumpU8(_x_)
	#define MV_DUMPC32(_x_)  mvDebugDumpU32(_x_)
	#define MV_DUMPC16(_x_)  mvDebugDumpU16(_x_)
	#define MV_DUMPC8(_x_)  mvDebugDumpU8(_x_)
	#define MV_DUMPE32(_x_) //mvDebugDumpU32(_x_)
	#define MV_DUMPE16(_x_)  mvDebugDumpU16(_x_)
	#define MV_DUMPE8(_x_)  mvDebugDumpU8(_x_)
	#define MV_DUMPRUN(_x_)  mvDebugDumpU16(_x_)
	#define MV_HALTKEY		waitForKeystroke()
	#define MV_ENTERLINE	mvChangLine()
	//#define MV_DUMPRUN(_x_)  //mvDebugDumpRun(_x_)

#else
	#define MV_DUMPC32(_x_)
	#define MV_DUMPC16(_x_)
	#define MV_DUMPC8(_x_)	
	#define MV_DUMPE32(_x_) 
	#define MV_DUMPE16(_x_) 
	#define MV_DUMPE8(_x_) 
	#define MV_DUMP32(_x_) 
	#define MV_DUMP16(_x_)
	#define MV_DUMP8(_x_)
	#define MV_DUMPRUN(_x_) 
	#define MV_HALTKEY
	#define MV_ENTERLINE
#endif

#if defined(_OS_LINUX)
	/*#include <linux/types.h>*/
	/** unsigned/signed long is 64bit for AMD64, so use unsigned int instead */
typedef unsigned int MV_U32, *MV_PU32;
typedef   signed int MV_I32, *MV_PI32;
typedef unsigned long MV_ULONG, *MV_PULONG;
typedef   signed long MV_ILONG, *MV_PILONG;

#else
	/** unsigned/signed long is 32bit for x86, IA64 and AMD64 */
	typedef unsigned long MV_U32, *MV_PU32;
	typedef   signed long MV_I32, *MV_PI32;
#endif

#if defined(_OS_WINDOWS)

	typedef unsigned __int64 _MV_U64;
	typedef   signed __int64 _MV_I64;
#elif defined(_OS_LINUX)
	typedef unsigned long long _MV_U64;
	typedef   signed long long _MV_I64;
#elif defined(_OS_FREEBSD)

#else

#endif

#ifdef _OS_LINUX

	#ifdef _64_SYS_
		#define _SUPPORT_64_BIT
	#else
		#ifdef _SUPPORT_64_BIT
			#error Error 64_BIT CPU Macro
		#endif
	#endif

#elif defined(_OS_BIOS)
	#undef	_SUPPORT_64_BIT
#else
	#define _SUPPORT_64_BIT
#endif

/*
 * Primary Data Type
 */
#if defined(_OS_LINUX) || defined(_OS_WINDOWS)
	/* Windows and Linux compiler supports 64 bit data structure. */
	typedef union {
		struct {
			MV_U32 low;
			MV_U32 high;
		};
		_MV_U64 value;
	} MV_U64, *PMV_U64;
#else
	/* BIOS compiler doesn't support 64 bit data structure. */
	typedef union {
		struct {
			MV_U32 low;
			MV_U32 high;
		};

		struct {
			MV_U32 value;
			MV_U32 value1;
		};
	} _MV_U64,MV_U64, *MV_PU64, *PMV_U64;
#endif

/* PTR_INTEGER is necessary to convert between pointer and integer. */
#if defined(_SUPPORT_64_BIT)
	typedef _MV_U64 MV_PTR_INTEGER;
#else
	typedef MV_U32 MV_PTR_INTEGER;
#endif

/* LBA is the logical block access */
typedef MV_U64 MV_LBA;

#if defined(_CPU_16B)
	typedef MV_U32 MV_PHYSICAL_ADDR;
#else
	typedef MV_U64 MV_PHYSICAL_ADDR;
#endif

#endif /* COM_DEFINE_H */

