#ifndef _CYGENDIAN_H_
#define _CYGENDIAN_H_

#ifdef __CYGWIN__

#include <sys/param.h>

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#ifndef __BYTE_ORDER
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER	__LITTLE_ENDIAN
#endif

#endif /* __CYGWIN__ */

#endif /* _CYGENDIAN_H_ */
