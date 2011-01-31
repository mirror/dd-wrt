/*
 * asn_config.h - configures the ANSI/non ansi, defines 
 *                decoder alloc routines and buffer routines
 *
 * MS 91 
 * Copyright (C) 1992 Michael Sample and the University of British Columbia
 *
 * This library is free software; you can redistribute it and/or
 * modify it provided that this copyright/license information is retained
 * in original form.
 *
 * If you modify this file, you must clearly indicate your changes.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef _asn_config_h_
#define _asn_config_h_

#include <stdio.h>
#include <ctype.h> /*  for isprint() in asn_octs.c */
#include <setjmp.h> /* for jmp_buf type, setjmp and longjmp */
#include <string.h> /* memcpy, etc */
#include <stdlib.h> /* for free */

/* for pow() used in asn_real.c - must include to avoid casting err on pow */
#include <math.h>  

/*
 * define IEEE_REAL_FMT if your system/compiler uses the native ieee double
 * this should improve the performance of encoding reals.
 * If your system has the IEEE library routines (iszero, isinf etc)
 * then define IEEE_REAL_LIB.  If neither are defined then
 * frexp is used.  Performance is probaby best for IEEE_REAL_FMT.
 *
 *  #define IEEE_REAL_FMT
 *  #define IEEE_REAL_LIB
 */



/*
 * define __USE_ANSI_C__ if your compiler supports it
 */
#define __USE_ANSI_C__

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/*
 *  Inspired by gdb 4.0, for better or worse...
 *  (grabbed from Barry Brachman - MS)
 * 
 *  These macros munge C routine declarations such
 *  that they work for ANSI or non-ANSI C compilers
 */
#ifdef __USE_ANSI_C__

#define PROTO(X)			X
#define PARAMS(arglist, args)  		(args)
#define NOPARAMS()	        	(void)
#define _AND_				,
#define DOTS				, ...

#else

#define PROTO(X)			()
#define PARAMS(arglist, args)	 	arglist args;
#define NOPARAMS()	        	()
#define _AND_				;
#define DOTS
#define void                            char

#endif



/* used to test if optionals are present */
#define NOT_NULL(ptr) ((ptr) != NULL)


/*
 * Asn1Error(char* str) - configure error handler
 */
#define Asn1Error(str) fprintf(stderr,"%s", str);


/*
 * Asn1Warning(char* str) - configure warning mechanism
 * (currently never called)
 */
#define Asn1Warning(str) fprintf(stderr,"%s", str);


/*
 * configure memory scheme used by decoder to allocate memory
 * for the decoded value.
 * The Asn1Free will be called in the optionally generated
 * hierachical free routines.
 *
 * nibble_alloc allocs from a single buffer and EVERYTHING
 * is freed by a single fcn call. Individual elmts cannot be freed
 */
#include "nibble_alloc.h"

#define Asn1Alloc(size)  NibbleAlloc(size)
#define Asn1Free(ptr)  /* empty */
#define CheckAsn1Alloc(ptr, env)\
   if ((ptr) == NULL)\
      longjmp(env, -27);


#define ENV_TYPE jmp_buf
/*
 * configure buffer routines that the encoders (write)
 * and decoders(read) use.  This config technique kind
 * of bites but is allows efficient macro calls.  The
 * Generated code & lib routines  call/use the "Buf????"
 * version of the macro - you define their meaning here.
 */
#ifdef USE_EXP_BUF

#include "exp_buf.h"

#define BUF_TYPE ExpBuf**
#define BufGetByte(b)              ExpBufGetByte(b)
#define BufGetSeg(b, lenPtr)       ExpBufGetSeg(b, lenPtr)
#define BufCopy(dst, b, len)       ExpBufCopy( dst, b, len)
#define BufSkip(b, len)            ExpBufSkip(b, len)
#define BufPeekByte(b)             ExpBufPeekByte( b)
#define BufPutByteRvs(b, byte)     ExpBufPutByteRvs( b, byte)
#define BufPutSegRvs(b, data, len) ExpBufPutSegRvs( b, data, len)
#define BufReadError(b)            ExpBufReadError(b)
#define BufWriteError(b)           ExpBufWriteError(b)

#else /* SBUF or MIN_BUF */

#ifdef USE_MIN_BUF

#include "min_buf.h"

#define BUF_TYPE char**
#define BufGetByte(b)              MinBufGetByte(b)
#define BufGetSeg(b, lenPtr)       MinBufGetSeg(b, lenPtr)
#define BufCopy(dst, b, len)       MinBufCopy(dst, b, len)
#define BufSkip(b, len)            MinBufSkip(b, len)
#define BufPeekByte(b)             MinBufPeekByte(b)
#define BufPutByteRvs(b, byte)     MinBufPutByteRvs(b, byte)
#define BufPutSegRvs(b, data, len) MinBufPutSegRvs(b, data, len)
#define BufReadError(b)            MinBufReadError(b)
#define BufWriteError(b)           MinBufWriteError(b)

#else /* SBUF */

#include "sbuf.h"

#define BUF_TYPE SBuf*

#define BufGetByte(b)             SBufGetByte(b)
#define BufGetSeg(b, lenPtr)      SBufGetSeg(b, lenPtr)
#define BufCopy(dst, b, len)      SBufCopy(dst, b, len)
#define BufSkip(b, len)           SBufSkip(b, len)
#define BufPeekByte(b)            SBufPeekByte(b)
#define BufPutByteRvs(b, byte)    SBufPutByteRvs(b, byte)
#define BufPutSegRvs(b, data, len) SBufPutSegRvs(b, data, len)
#define BufReadError(b)           SBufReadError(b)
#define BufWriteError(b)          SBufWriteError(b)

#endif
#endif

#include "print.h"  /* for printing set up */

#endif /* conditional include */
