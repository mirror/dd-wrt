/*====================================================================*
 *
 *   types.h - custom data type definitions and declarations;
 *
 *   this file is a subset of the original that includes only those
 *   definitions and declaration needed for toolkit programs;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *   Contributor(s):
 *
 *   Werner Henze <w.henze@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef TYPES_HEADER
#define TYPES_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#if defined (_WIN64)
	#define SIZE_T_SPEC "%I64d"
	#define OFF_T_SPEC "%ld"
#elif defined (WIN32)
	#define SIZE_T_SPEC "%d"
	#define OFF_T_SPEC "%ld"
#elif defined (__APPLE__)
	#define SIZE_T_SPEC "%zu"
	#define OFF_T_SPEC "%lld"
#elif defined (__OpenBSD__)
	#define SIZE_T_SPEC "%zu"
	#define OFF_T_SPEC "%lld"
#elif defined (__linux__)
	#define SIZE_T_SPEC "%zu"
	#define OFF_T_SPEC "%ld"
#else
error "Unknown environment."
#endif

/*====================================================================*
 *   macros;
 *--------------------------------------------------------------------*/

#define MONIKER(x) #x
#define LITERAL(x) MONIKER(x)
#define SIZEOF(array) (sizeof(array)/sizeof(array[0]))

/*====================================================================*
 *   new POSIX types;
 *--------------------------------------------------------------------*/

typedef signed errno_t;
typedef signed signo_t;
typedef unsigned char byte;

/*====================================================================*
 *   define C++ style true and false for use in standard C programs;
 *--------------------------------------------------------------------*/

#ifndef __cplusplus

typedef enum

{
	false,
	true
}

bool;

#endif

/*====================================================================*
 *   cope with structure packing vagaries;
 *--------------------------------------------------------------------*/

#ifndef __packed
#ifdef __GNUC__
#define __packed __attribute__ ((packed))
#else
#define __packed
#endif
#endif

/*====================================================================*
 *   define flagword (bitmap) variable type for clarity;
 *--------------------------------------------------------------------*/

typedef signed file_t;
typedef signed sock_t;
typedef signed code_t;
typedef unsigned char byte_t;
typedef unsigned type_t;
typedef unsigned flag_t;
typedef struct _file_

{
	file_t file;
	char const *name;
}

file;
typedef struct _term_

{
	char const * term;
	char const * text;
}

TERM;
typedef struct _type_

{
	type_t type;
	char const * name;
}

TYPE;
typedef struct _code_

{
	code_t code;
	char const * name;
}

CODE;

/*====================================================================*
 *   end definitions and declarations;
 *--------------------------------------------------------------------*/

#endif

