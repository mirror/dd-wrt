/******************************************************************************
 * 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *             
 *****************************************************************************/

#if !defined(__IX_CC_ERROR_H__)
#define __IX_CC_ERROR_H__

#include <stddef.h>
#include "ix_types.h"
#include "ix_error.h"
#include "ix_symbols.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

IX_EXPORT_FUNCTION const char *ix_error_makestr(const char *fmt, ...);

/**
 * Core Component Generic Error Codes.
 *
 * This enumeration provides the common error codes.
 */
enum ix_cc_error_generic {
    IX_CC_ERROR_FIRST = IX_ERROR_MAKE_GROUP(CC),

    /**
     * Component not initialized.
     * This error code is returned when a function is called for 
     * service that is part of a component that has not been 
     * properly initialized.
     */
    IX_CC_ERROR_UNINIT,

    /**
     * Parameter not allowed to be NULL.
     * This error code is returned by functions that are sensitive to
     * a parameter being NULL.
     */
    IX_CC_ERROR_NULL,

    /**
     * Parameter outside allowable range.
     * This error code is returned by functions that are sensitive to
     * a parameter being outside a specific range.
     */
    IX_CC_ERROR_RANGE,

    /**
     * Parameter not appropriately aligned.
     * This error code is returned by functions that are sensitive to
     * a parameter being properly aligned.
     */
    IX_CC_ERROR_ALIGN,

    /**
     * Undefined exception code.
     * This value is used to indicate an undefined exception code.
     * This can be used in core component packet handler routines.
     */
    IX_CC_ERROR_UNDEFINED_EXCEP,

    /**
     * Undefined message type.
     * This value is used to indicate an undefined message type.
     * This can be used in core component message handler routines.
     */
    IX_CC_ERROR_UNDEFINED_MSG,

    /**
     * Out of memory.
     * This error code is returned by functions that try to allocate
     * memory, and fail. The type of memory is unspecified.
     */
    IX_CC_ERROR_OOM,

    /**
     * Out of SRAM memory.
     * This error code is returned by functions that try to allocate
     * SRAM memory, and fail.
     */
    IX_CC_ERROR_OOM_SRAM,

    /**
     * Out of DRAM memory.
     * This error code is returned by functions that try to allocate
     * DRAM memory, and fail.
     */
    IX_CC_ERROR_OOM_DRAM,
 
    /**
     * Out of system memory.
     * This error code is returned by functions that try to allocate
     * system memory, and fail.
     */
    IX_CC_ERROR_OOM_SYSTEM,
 
    /**
     * Out of 64-bit counter.
     * This error code is returned by functions that try to allocate
     * 64-bit counter from Resource Manager, and fail.
     */
    IX_CC_ERROR_OOM_64BIT_COUNTER,
 
    /**
     * Out of resources.
     * This error code is returned by functions that try to allocate
     * a non-memory resouce, and fail. 
     */
    IX_CC_ERROR_OOR,

    /**
     * Overflow
     * This value is used to indicate an overflow condition (as
     * defined by the function returning the error).
     */
    IX_CC_ERROR_OVERFLOW,

    /**
     * Underflow
     * This value is used to indicate an dnderflow condition (as
     * defined by the function returning the error).
     */
    IX_CC_ERROR_UNDERFLOW,

    /**
     * Entry not found.
     * This value is used to indicate the specified entry can not 
     * be found.  This can be used by funtions that do table 
     * look-up or search.  
     */
    IX_CC_ERROR_ENTRY_NOT_FOUND,

    /**
     * Duplicate entry.
     * This value may be used to indicate duplicated entry.
     * This may be used by funtions that add entries to a table.
     * The usage is defined by the function returning the error.
     */
    IX_CC_ERROR_DUPLICATE_ENTRY,

    /**
     * Conflicting entry.
     * This value may be used to indicate conflicting entry.
     * The usage is defined by the function returning the error.
     */
    IX_CC_ERROR_CONFLICTING_ENTRY,

    /**
     * Internal error.
     * This value is used to indicate an internal error 
     * condition such as invalid fingerprint information in a
     * data structure (as defined by the function returning the 
     * error).
     */
    IX_CC_ERROR_INTERNAL,

    /**
     * This value is used in a call to a component's set_property
     * messaging API to indicate the component is a slave of 
     * the specified dynamic proprerty.
     */
    IX_CC_ERROR_PROPERTY_CLIENT,

    /**
     * Feature Not Implemented.
     * This error code is returned when a function is called for
     * service that is not yet implemented.
     */
    IX_CC_ERROR_UNIMPL,

    /**
     * Assertion Error.
     * This code should be used for generic ASSERTION type failures.
     */
    IX_CC_ERROR_ASSERTION,

    /**
     * This value is used to indicate that an operation failed because
     * the module ran out of internal resources.  For example, an array
     * cannot contain more entries, etc.
     */
    IX_CC_ERROR_FULL,
    /**
     * Send packet or message failure - indicates that an operation failed due to the 
     * error conditions in underlying infrastructure.  
     * This error code is returned from packet or message handlers of component 
     * based on the error from packet_send framework function.  
     * This error may indicate full condition in the communication rings.
     */
    IX_CC_ERROR_SEND_FAIL,

    IX_CC_ERROR_LAST
};

typedef struct ix_s_error_hunk ix_error_hunk;

struct ix_s_error_hunk {
    ix_error                ref;
    ix_error                self;
    ix_error_level          level;
    ix_error_number         code;
    char                    buf[1];
};

/** Some functions have printf-like variable argument lists.
    Using this tag in the prototype of a function that parses
    a string just like "printf" will help GCC provide warnings
    about mismatches between printf format parameters and the
    parameter types provided.
    See the GCC documentation for details.
*/
#if !defined(__GNUC__) || defined(TESTRT)
#define PRINTFLIKE(f,v)
#else /*__GNUC__*/
#define PRINTFLIKE(f,v) __attribute((format(__printf__,(f),(v))))
#endif/*__GNUC__*/

#define IX_ERROR_CHAIN(__err, __code, __level, __s)                     \
   ix_error_new(__err, __code, __level,                                 \
           IX_ERROR_LEAD_FMT("Error detected")                          \
           "\t%s", IX_ERROR_LEAD_ARGS, ix_error_makestr __s)

#define IX_ERROR_LOCAL(__code, __s) \
   IX_ERROR_CHAIN(0, __code, IX_ERROR_LEVEL_LOCAL, __s)

#define IX_ERROR_WARNING(__code, __s) \
   IX_ERROR_CHAIN(0, __code, IX_ERROR_LEVEL_WARNING, __s)

#define IX_ERROR_REMOTE(__code, __s) \
   IX_ERROR_CHAIN(0, __code, IX_ERROR_LEVEL_REMOTE, __s)

#define IX_ERROR_GLOBAL(__code, __s) \
   IX_ERROR_CHAIN(0, __code, IX_ERROR_LEVEL_GLOBAL, __s)

#define IX_ERROR_PANIC(__code, __s) \
   IX_ERROR_CHAIN(0, __code, IX_ERROR_LEVEL_PANIC, __s)

/*
 * CRT: Call and Return with a Translation
 * Execute the statement and if there was an error, translate the error
 * code and level to the values provided.  Return the error.
 */
#define IX_ERROR_CRT(__expr, __code, __level) _IX_ERROR_WRAP(           \
   {                                                                    \
       ix_error __err = __expr;                                         \
       if (__err != IX_SUCCESS) {                                       \
           return ix_error_new(__err, __code, __level,                  \
	     IX_ERROR_LEAD_FMT("Operation failed")			\
             "\t%s", IX_ERROR_LEAD_ARGS, #__expr);                      \
       }                                                                \
   })

#define IX_ERROR_CGT(__expr, __err, __code, __level, __label)           \
   _IX_ERROR_WRAP(                                                      \
       __err = __expr;                                                  \
       if (__err != IX_SUCCESS)                                         \
       {                                                                \
           __err = ix_error_new(__err, __code, __level,                 \
                   IX_ERROR_LEAD_FMT("Operation failed")                \
                   "\t%s", IX_ERROR_LEAD_ARGS, #__expr);                \
           goto __label;                                                \
       })

#define IX_ERROR_CT(__expr, __err, __code, __level) _IX_ERROR_WRAP(     \
   __err = __expr;                                                      \
   if (__err != IX_SUCCESS)                                             \
       __err = ix_error_new(__err, __code, __level,                     \
               IX_ERROR_LEAD_FMT("Operation failed")                    \
               "\t%s", IX_ERROR_LEAD_ARGS, #__expr))

/** Initialize Error System.
    The ix_error codes are opaque integers, but "behind" them is a
    significant amount of storage for tracking specific error levels,
    codes, references to other errors, and textual information about
    the specific report. This function allows the caller to expand the
    number of error storage blocks and/or to expand the size of the
    text buffers in each block. Note that once a call has been made
    establishing a count and length, subsequent counts can increase
    the counts and lengths, but no call will decrease the values.

    There may be silent maximum values enforced by the error package.

    @param	count	desired minimum number of error log blocks
    @param	length	desired minimum length for error text storage
*/
IX_EXPORT_FUNCTION 
void                    ix_error_init(int count, int length);

/** Terminate Error Mechanism.

    This function can be called to clean up and free all dynamically
    allocated storage associated with the error mechanism. When this
    function is called, the error mechanism moves into the unitialized
    state; any subsequent calls to it (other than initialization) will
    have unspecified results.

    NEVER call this API from within a function that returns an ix_error.

    NEVER call this API from within the Linux Kernel, unless you are
    the actual IXASL driver and the whole Kernel IXASL package has
    alredy been shut down.

    In general, the only time an application should call this is
    immediately before terminating a thread that was NOT created using
    the approproate OSAL functions.

    Improper use of this function may result in process locks,
    processes core dumps, and system crashes.
*/
IX_EXPORT_FUNCTION 
void                    ix_error_fini(void);

/** Construct Error.

    This function logs the specified error information into an error
    buffer and returns a token that can be used to retrieve the
    information again later. The error string that is constructed is
    truncated to fit within the storage area, if necessary.

    This function always returns a nonzero error code that represents
    the error described in its parameters.

    @param	ref	subsidiary error
    @param	code	error number
    @param	level	error severity
    @param	fmt	message format string (like printf)
    @param	...	more parameters (like printf)
    @returns an error token.
*/
IX_EXPORT_FUNCTION 
ix_error
ix_error_new(ix_error ref, ix_error_number code, ix_error_level level,
	     const char *fmt,
	     /* more args handled like printf */
	     ...)
PRINTFLIKE(4, 5);

/** Recover error information.

    This function recovers the error details from an error token into
    the specified areas.

    The return location parameters may be NULL, indicating that the
    caller is not interested in that information.

    @param	err	error token to decode
    @param	stalep	where to store stale indication.
    @param	levelp	where to return error severity (or NULL)
    @param	codep	where to return error number (or NULL)
    @param	refp	where to return subsidiary error (or NULL)
    @param	buf	where to copy formatted error string (or NULL)
    @param	len	maximum size to put into buf
    @returns an error token, zero for success.
*/
IX_EXPORT_FUNCTION 
void
ix_error_scan(ix_error err,
	      int *stalep,
	      ix_error_number *codep,
	      ix_error_level *levelp,
	      ix_error *refp, char *buf, size_t len);

/** Compare error token against level and/or number

    This function scans the face of the provided error token, and
    returns true if it is valid (not stale) and matches the specified
    level and/or number.

    If the level number is given as "0" then no level comparison is
    done. If the error number is given as "0" then no error number
    comparison is done.

    @param	err	error token to decode
    @param	numb	error number to compare (or "0" for "any")
    @param	levl	error level to compare (or "0" for "any")
    @returns true if the error matches.
 */
IX_EXPORT_FUNCTION 
int
ix_error_is(ix_error err,
	    ix_error_number numb,
	    ix_error_level levl);

/** Check for STOP type error token.

    This token scans the face of the provided error token, and returns
    true if it is valid (not stale) and reflects a recognized simple
    STOP condition.

    @param	err	error token to decode
    @returns true if the error matches.
 */
IX_EXPORT_FUNCTION 
int
ix_error_is_stop(ix_error err);

/** locate error information.

    this function translates an error handle into a pointer to the
    appropriate matching error hunk. a null pointer is returned if
    the provided handle is zero, or if it refers to a stale hunk.

    @param	err	error token to decode
    @param	retp	where to return hunk pointer
*/
IX_EXPORT_FUNCTION 
void
ix_error_find(ix_error err,
	      ix_error_hunk **retp,
	      size_t *sizep);

/** set up new error hunk.

    this function locates the error hunk storage, and sets up
    its contents appropriately, up to but not including filling
    in the text string area, which it initializes to a zero
    length string. The length of the longest string that can be
    stored in the hunk's buffer (not including the zero at the
    end of the string) is returned through the size return value.

    this function always returns a nonzero error code that represents
    the error described in its parameters.

    @param	retp	where to return address of error hunk
    @param	sizep	where to return maximum string length
    @param	level	error severity
    @param	code	error number
    @param	ref	subsidiary error
    @returns an error token.
*/
IX_EXPORT_FUNCTION 
ix_error
ix_error_build(ix_error_hunk **retp,
	       size_t *sizep,
	       ix_error_number code,
	       ix_error_level level,
	       ix_error ref);

/** In user mode Dump error information to stdio FILE stream.
    But in case of kernel mode, ignore the first parameter.
    The error information will be dumped to OS system log.
 */
#if (defined (__linux__) && defined (__KERNEL__))

#include <linux/fs.h>

IX_EXPORT_FUNCTION
void
ix_error_dump(const void *fp, ix_error err);

#else

#include <stdio.h>

IX_EXPORT_FUNCTION
void
ix_error_dump(FILE * fp, ix_error err);

#endif /* end (defined (__linux__) && defined (__KERNEL__)) */



/* ========================================================================
 *		ERROR ASSISTANCE MACROS
 *
 *	It appears that a small number of nearly identical chunks of
 *	code are appearing in a large number of places to provide
 *	error checks. These macros may be useful to reduce the visible
 *	size of the error checks.
 */

#if defined(__FUNC__)
#define	IX_ERROR_FUNC_FMT	" in %s()"
#define	IX_ERROR_FUNC_ARG	, __FUNC__
#elif defined(__GNUC__)
#define	IX_ERROR_FUNC_FMT	" in %s()"
#define	IX_ERROR_FUNC_ARG	, __FUNCTION__
#else
#define	IX_ERROR_FUNC_ARG
#define	IX_ERROR_FUNC_FMT
#endif

#if defined(IX_ERROR_FILE_IDENT)
static const char * const ix_error_file_ident = IX_ERROR_FILE_IDENT;
#define	IX_ERROR_IDENT_FMT	"\n\t%s"
#define	IX_ERROR_IDENT_ARG	, ix_error_file_ident
#else
#define	IX_ERROR_IDENT_ARG
#define	IX_ERROR_IDENT_FMT
#endif

/* Common Error Leadin
 *	Macros provided for leading part of format string
 *	and a coordinated set of parameters, so we can change
 *	what all our messages look like at once (for instance,
 *	adding file ident strings or function name strings
 *	to all messages, as I just did).
 */
#define	IX_ERROR_LEAD_FMT(m)	"%s:%d: " m IX_ERROR_FUNC_FMT IX_ERROR_IDENT_FMT "\n"
#define	IX_ERROR_LEAD_ARGS	__FILE__, __LINE__ IX_ERROR_FUNC_ARG IX_ERROR_IDENT_ARG

/** Generic Error Forwarding Macro

    The first parameter is statement #1.
    The second parameter is error variable #2.
    The third parameter is text #3
    The fourth parameter is statement #4.

    Statement #1 is executed; it is expected to leave an error
    token in variable #2. The value of variable #2 is tested, and if
    it is nonzero, the error is decorated with available information
    including source line text #3 and statement #4 is exectued.
 */
#define IX_ERROR_MAD(__s, __e, __t, __x) _IX_ERROR_WRAP(                \
    {                                                                   \
        __s;                                                            \
        if (__e)                                                        \
        {                                                               \
            __e = ix_error_new(__e, 0, 0,                               \
                    IX_ERROR_LEAD_FMT("Operation failed") "\t%s",       \
                    IX_ERROR_LEAD_ARGS, __t);                           \
            __x;                                                        \
        }                                                               \
    })

/** Error Forwarding: Call 
    Evaluate expression #1. If the returned error token is nonzero,
    decorate it and return the new error token.
 */
#define	IX_ERROR_C(__x, __e)		IX_ERROR_MAD(__e = __x, __e, #__x, ;)

/** Error Forwarding: Call with Return
    Evaluate expression #1. If the returned error token is nonzero,
    decorate it and return the new error token.
 */
#define	IX_ERROR_CR(__x) IX_ERROR_MAD(ix_error __e = __x, __e, #__x, return __e)

/** Error Forwarding: Call with Print
    Evaluate expression #1. If the returned error token is nonzero,
    decorate it and print the resulting error chain.
 */
#if !defined(TESTRT)
#if (defined (__linux__) && defined (__KERNEL__))

#define	IX_ERROR_CP(__x)		IX_ERROR_MAD(ix_error __e = __x, __e, #__x, ix_error_dump(0, __e))

#else

#define	IX_ERROR_CP(__x)		IX_ERROR_MAD(ix_error __e = __x, __e, #__x, ix_error_dump(stderr, __e))

#endif /* end (defined (__linux__) && defined (__KERNEL__)) */

#else
#define	IX_ERROR_CP(__x)		IX_ERROR_MAD(ix_error __e = __x, __e, #__x, ;)
#endif

/** Error Forwarding: Call with Goto
    Evaluate expression #1 storing the returned error token in error
    variable #2. If the returned error token is nonzero,
    decorate it and goto label #3.
 */
#define	IX_ERROR_CG(__x, __e, __g)	IX_ERROR_MAD(__e = __x, __e, #__x, goto __g)

/** Dump secondary errors
    If variable #2 > 0, evaluate expression #1. If the returned error
    token is non-zero, decorate it and print the resulting error chain.
    If variable #2 is 0, assign error token returned by expression #1 to
    variable #2.
 */

#define IX_ERROR_EPS(__x, __e) \
    do {    \
        ix_error ___i = (__x);    \
        __e = (__e) ? (ix_error_dump(stderr, ___i), (__e)) : ___i;    \
    } while(0)

/** Return error if parameter is null.

    This macro tests the value of an expression; if the result is
    zero, then an ix_error is constructed indicating that a parameter
    to a function was specified as NULL.

    @param	__n	parameter number to include in error message
    @param     	__v	parameter to check and include in error message
*/
#define	IX_ERROR_CHECK_ARG_NULL(__n, __v) _IX_ERROR_WRAP(               \
    if (0 == (__v))                                                     \
		{return ix_error_new                                             \
            (0, IX_CC_ERROR_NULL, IX_ERROR_LEVEL_WARNING,               \
             IX_ERROR_LEAD_FMT("NULL check failed")                     \
             "\tparameter %d (%s)"                                      \
             " should never be zero."                                   \
             , IX_ERROR_LEAD_ARGS                                       \
             , __n, #__v                                                \
        );})

#if defined(IX_DEBUG)
#define	IX_ERROR_DCHECK_ARG_NULL(__n, __v)				\
	IX_ERROR_CHECK_ARG_NULL(__n, __v)
#else /*!IX_DEBUG*/
#define	IX_ERROR_DCHECK_ARG_NULL(__n, __v)				\
	_IX_ERROR_WRAP(;)
#endif


/** Maximum alignment mask for alignment check.
    This value is processor dependent, but the
    correct value is "3" for both Pentium and StrongARM.
*/
#define	IX_ERROR_CHECK_ARG_ALIGN_MASK	3

/** Return error if parameter is not aligned.

    This macro verifies that the specified pointer is appropriately
    aligned to be able to directly address objects of the specified
    type.

    NOTE: this uses typical RISC rules for alignment checking, so it
    may declare an error when some CISC processors can still do the
    access with a small penalty (and when some RISC operating systems
    can patch up the alignment fault and pretend things worked.)

    @param	__n	parameter number to include in error message
    @param     	__p	parameter to check and include in error message
    @param	__t	sizeof operand for alignment modulus
*/
#define	IX_ERROR_CHECK_ARG_ALIGN(__n, __p, __t)	_IX_ERROR_WRAP(         \
    if (0 != ((unsigned)(__p)                                           \
          & IX_ERROR_CHECK_ARG_ALIGN_MASK                               \
          & (sizeof (__t) - 1)                                          \
          & ~sizeof (__t)))                                             \
        return ix_error_new                                             \
            (0, IX_CC_ERROR_ALIGN, IX_ERROR_LEVEL_WARNING,              \
             IX_ERROR_LEAD_FMT("ALIGN check failed")                    \
             "\tparameter %d (%s)"                                      \
             " needs to be %d-byte (%s) aligned,\n"                     \
             "\tbut has the value %p"                                   \
             , IX_ERROR_LEAD_ARGS                                       \
             , __n, #__p                                                \
             , sizeof (__t), #__t                                       \
             , (volatile const void *)(__p)                             \
            );)

#if defined(IX_DEBUG)
#define	IX_ERROR_DCHECK_ARG_ALIGN(__n, __p, __t)			\
	IX_ERROR_CHECK_ARG_ALIGN(__n, __p, __t)
#else /*!IX_DEBUG*/
#define	IX_ERROR_DCHECK_ARG_ALIGN(__n, __p, __t)			\
	_IX_ERROR_WRAP(;)
#endif

/** Return error if parameter is out of range.

    This macro tests the value of an expression; if the result is
    outside a specified range, then an ix_error is constructed
    indicating that a parameter to a function was out of range.

    @param __n	parameter number to include in error message
    @param __v	parameter to check and include in error message
    @param __l	lowest acceptable value for parameter
    @param __h	highest acceptable value for parameter
    @param __o	printf format string for parameter values
*/
#define	IX_ERROR_CHECK_ARG_RANGE(__n,__v,__l,__h,__o)                   \
do{                                                                     \
    if ((__v < __l) || (__v > __h)){				        \
        return ix_error_new					        \
            (0, IX_CC_ERROR_RANGE, IX_ERROR_LEVEL_WARNING,	        \
             IX_ERROR_LEAD_FMT("RANGE check failed")		        \
             "\tparameter %d (%s)"				        \
             " value " __o					        \
             " was not in range [" __o ".." __o "]"		        \
             , IX_ERROR_LEAD_ARGS				        \
             , __n, #__v					        \
             , __v, __l, __h					        \
            );}                                                         \
}while(0)

#if defined(IX_DEBUG)
#define	IX_ERROR_DCHECK_ARG_RANGE(__n,__v,__l,__h,__o)	                \
	IX_ERROR_CHECK_ARG_RANGE(__n,__v,__l,__h,__o)
#else /*!IX_DEBUG*/
#define	IX_ERROR_DCHECK_ARG_RANGE(__n,__v,__l,__h,__o)	                \
	_IX_ERROR_WRAP(;)
#endif

/** Return error if parameter is less than minimum.

    This macro tests the value of an expression; if the result is
    less than a minimum, then an ix_error is constructed
    indicating that a parameter to a function was out of range.

    @param __n	parameter number to include in error message
    @param __v	parameter to check and include in error message
    @param __l	lowest acceptable value for parameter
    @param __o	printf format string for parameter values
*/
#define IX_ERROR_CHECK_ARG_MIN(__n,__v,__l,__o)                         \
do {                                                                    \
    if (__v < __l){						        \
        return ix_error_new					        \
            (0, IX_CC_ERROR_RANGE, IX_ERROR_LEVEL_WARNING, 	        \
             IX_ERROR_LEAD_FMT("MIN check failed")		        \
             "\tparameter %d (%s)"				        \
             " value " __o					        \
             " was less than minimum [" __o "]"			        \
             , IX_ERROR_LEAD_ARGS				        \
             , __n, #__v					        \
             , __v, __l						        \
            );}                                                         \
} while(0)

#if defined(IX_DEBUG)
#define	IX_ERROR_DCHECK_ARG_MIN(__n,__v,__l,__o)			\
	IX_ERROR_CHECK_ARG_MIN(__n,__v,__l,__o)
#else /*!IX_DEBUG*/
#define	IX_ERROR_DCHECK_ARG_MIN(__n,__v,__l,__o)			\
	_IX_ERROR_WRAP(;)
#endif

/** Return error if parameter is greater than maximum.

    This macro tests the value of an expression; if the result is
    greater than a maximum, then an ix_error is constructed
    indicating that a parameter to a function was out of range.

    @param __n	parameter number to include in error message
    @param __v	parameter to check and include in error message
    @param __h	highest acceptable value for parameter
    @param __o	printf format string for parameter values
*/
#define	IX_ERROR_CHECK_ARG_MAX(__n,__v,__h,__o)                         \
do{                                                                     \
    if (__v > __h){						        \
        return ix_error_new					        \
            (0, IX_CC_ERROR_RANGE, IX_ERROR_LEVEL_WARNING, 	        \
             IX_ERROR_LEAD_FMT("MAX check failed")		        \
             "\tparameter %d (%s)"				        \
             " value " __o					        \
             " was greater than maximum [" __o "]"		        \
             , IX_ERROR_LEAD_ARGS				        \
                , __n, #__v					        \
             , __v, __h						        \
            );}                                                         \
}while(0)

#if defined(IX_DEBUG)
#define	IX_ERROR_DCHECK_ARG_MAX(__n,__v,__h,__o)			\
	IX_ERROR_CHECK_ARG_MAX(__n,__v,__h,__o)
#else /*!IX_DEBUG*/
#define	IX_ERROR_DCHECK_ARG_MAX(__n,__v,__h,__o)			\
	_IX_ERROR_WRAP(;)
#endif

/*****************************************************************************
 * The rest of this file is for implementation purposes only.  The macros
 * and functions described below should not be used directly by the client
 * of the ix_error system.
 *****************************************************************************/

/*
 * In all cases but TestRT, wrap the statement in a do while zero.  This
 * ensures the macro is not "misused" -- i.e. is only used as a statement,
 * not as an expression.  Remove for TestRT because having the do while
 * affects the line coverage and code complexity scores.
 */
#if defined(IX_DEBUG)
#define _IX_ERROR_WRAP(__s) do { __s; } while (0)
#else
#define _IX_ERROR_WRAP(__s) __s
#endif

#if defined(IX_DEBUG)
#define IX_CONDITION_ASSERT(arg_Condition)\
 {\
  if (!(arg_Condition)) \
   {ixOsalStdLog("%s:%d Boundary Check failed\n",__FILE__,__LINE__); return IX_FAIL;} \
 }
#else
#define IX_CONDITION_ASSERT(arg_Condition)  
#endif

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_CC_ERROR_H__) */

