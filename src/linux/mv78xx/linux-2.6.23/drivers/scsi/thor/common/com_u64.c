#include "mv_include.h"

MV_U64 U64_ADD_U32(MV_U64 v64, MV_U32 v32)
{
#ifdef _64_BIT_COMPILER
	v64.value += v32;
#else
	v64.low += v32;
	v64.high = 0;	//TBD
#endif
	return v64;
}

MV_U64 U64_SUBTRACT_U32(MV_U64 v64, MV_U32 v32)
{
#ifdef _64_BIT_COMPILER
	v64.value -= v32;
#else
	v64.low -= v32;
	v64.high = 0;	//TBD
#endif
	return v64;
}

MV_U64 U64_MULTIPLY_U32(MV_U64 v64, MV_U32 v32)
{
#ifdef _64_BIT_COMPILER
	v64.value *= v32;
#else
	v64.low *= v32;
	v64.high = 0;	//TBD
#endif
	return v64;
}

MV_U32 U64_MOD_U32(MV_U64 v64, MV_U32 v32)
{
#ifdef _OS_LINUX
	return do_div(v64.value, v32);
#else
	return v64.value % v32;
#endif /* _OS_LINUX */
}

MV_U64 U64_DIVIDE_U32(MV_U64 v64, MV_U32 v32)
{
#ifdef _OS_LINUX
	do_div(v64.value, v32);
#else
#ifdef _64_BIT_COMPILER
	v64.value /= v32;
#else
	v64.high = 0;	//TBD
	v64.low /= v32;
#endif /* _64_BIT_COMPILER */

#endif /* _OS_LINUX */
	return v64;
}

MV_I32 U64_COMPARE_U32(MV_U64 v64, MV_U32 v32)
{
	if (v64.high > 0)
		return 1;
	if (v64.low > v32)
		return 1;
#ifdef _64_BIT_COMPILER
	else if (v64.value == v32)
#else
	else if (v64.low == v32)
#endif
		return 0;
	else
		return -1;
}

MV_U64 U64_ADD_U64(MV_U64 v1, MV_U64 v2)
{
#ifdef _64_BIT_COMPILER
	v1.value += v2.value;
#else
	v1.low += v2.low;
	v1.high = 0;	//TBD
	//v1.high += v2.high;
#endif
	return v1;
}

MV_U64 U64_SUBTRACT_U64(MV_U64 v1, MV_U64 v2)
{
#ifdef _64_BIT_COMPILER
	v1.value -= v2.value;
#else
	v1.low -= v2.low;
	v1.high = 0;	//TBD
	//v1.high -= v2.high;
#endif
	return v1;
}

MV_I32 U64_COMPARE_U64(MV_U64 v1, MV_U64 v2)
{
#ifdef _64_BIT_COMPILER
	if (v1.value > v2.value)
		return 1;
	else if (v1.value == v2.value)
		return 0;
	else
		return -1;
#else
#if 0
	if (v1.high > v2.high)
		return 1;
	else if((v1.low > v2.low) && (v1.high == v2.high))
		return 1;
		
	else if ((v1.low == v2.low) && (v1.high == v2.high))
		return 0;
	else
		return -1;
#endif
	//TBD
	if (v1.value > v2.value)
		return 1;
	else if (v1.value == v2.value)
		return 0;
	else
		return -1;

#endif

}

#ifdef _OS_BIOS
MV_U64 ZeroU64(MV_U64 v1)
{
	v1.low=0;v1.high=0;
	return	v1;
}
#endif

