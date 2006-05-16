## --------------------------------------------------------- ##
## Check for a sane <limits.h> header.                       ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_CHECK_HEADER_SANE_LIMITS_H,
[AC_CACHE_CHECK([whether limits.h is sane],
mfx_cv_header_sane_limits_h,
[AC_TRY_COMPILE([#include <limits.h>
#define MFX_0xffff          0xffff
#define MFX_0xffffffffL     4294967295ul
#if !defined(CHAR_BIT) || (CHAR_BIT != 8)
#  include "error CHAR_BIT"
#endif
#if !defined(UCHAR_MAX)
#  include "error UCHAR_MAX"
#endif
#if !defined(USHRT_MAX)
#  include "error USHRT_MAX"
#endif
#if !defined(UINT_MAX)
#  include "error UINT_MAX"
#endif
#if !defined(ULONG_MAX)
#  include "error ULONG_MAX"
#endif
#if !defined(SHRT_MAX)
#  include "error SHRT_MAX"
#endif
#if !defined(INT_MAX)
#  include "error INT_MAX"
#endif
#if !defined(LONG_MAX)
#  include "error LONG_MAX"
#endif
#if (UCHAR_MAX < 1)
#  include "error UCHAR_MAX"
#endif
#if (USHRT_MAX < 1)
#  include "error USHRT_MAX"
#endif
#if (UINT_MAX < 1)
#  include "error UINT_MAX"
#endif
#if (ULONG_MAX < 1)
#  include "error ULONG_MAX"
#endif
#if (UCHAR_MAX < 0xff)
#  include "error UCHAR_MAX"
#endif
#if (USHRT_MAX < MFX_0xffff)
#  include "error USHRT_MAX"
#endif
#if (UINT_MAX < MFX_0xffff)
#  include "error UINT_MAX"
#endif
#if (ULONG_MAX < MFX_0xffffffffL)
#  include "error ULONG_MAX"
#endif
#if (USHRT_MAX > UINT_MAX)
#  include "error USHRT_MAX vs UINT_MAX"
#endif
#if (UINT_MAX > ULONG_MAX)
#  include "error UINT_MAX vs ULONG_MAX"
#endif
],[
#if (USHRT_MAX == MFX_0xffff)
{ typedef char a_short2[1 - 2 * !(sizeof(short) == 2)]; }
#elif (USHRT_MAX >= MFX_0xffff)
{ typedef char a_short2[1 - 2 * !(sizeof(short) > 2)]; }
#endif
#if (UINT_MAX == MFX_0xffff)
{ typedef char a_int2[1 - 2 * !(sizeof(int) == 2)]; }
#elif (UINT_MAX >= MFX_0xffff)
{ typedef char a_int2[1 - 2 * !(sizeof(int) > 2)]; }
#endif
#if (ULONG_MAX == MFX_0xffff)
{ typedef char a_long2[1 - 2 * !(sizeof(long) == 2)]; }
#elif (ULONG_MAX >= MFX_0xffff)
{ typedef char a_long2[1 - 2 * !(sizeof(long) > 2)]; }
#endif
#if (USHRT_MAX == MFX_0xffffffffL)
{ typedef char a_short4[1 - 2 * !(sizeof(short) == 4)]; }
#elif (USHRT_MAX >= MFX_0xffffffffL)
{ typedef char a_short4[1 - 2 * !(sizeof(short) > 4)]; }
#endif
#if (UINT_MAX == MFX_0xffffffffL)
{ typedef char a_int4[1 - 2 * !(sizeof(int) == 4)]; }
#elif (UINT_MAX >= MFX_0xffffffffL)
{ typedef char a_int4[1 - 2 * !(sizeof(int) > 4)]; }
#endif
#if (ULONG_MAX == MFX_0xffffffffL)
{ typedef char a_long4[1 - 2 * !(sizeof(long) == 4)]; }
#elif (ULONG_MAX >= MFX_0xffffffffL)
{ typedef char a_long4[1 - 2 * !(sizeof(long) > 4)]; }
#endif
],
mfx_cv_header_sane_limits_h=yes,
mfx_cv_header_sane_limits_h=no)])
])
