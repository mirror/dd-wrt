/**
 * @file IxAtmdAssert_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Assert prototypes
 *
 * This header defines the assert features in IxAtmdAcc
 *
 * @note - @a IX_ATMDACC_ENSURE can be disabled in production code :
 *         it is used to detect abnormal situations during integration
 *         steps.
 *
 * @note - @a IX_ATMDACC_ABORT  should redirect to the system-wide
 *        assert mechanism. It is used when no recovery is possible.
 *        (e.g. : a severe error in an interrupt)
 *
 * @note - @a NDEBUG, when not defined, redirect these macros to a
 *        function which display an error message, the stop the
 *        program execution.
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IXATMDASSERT_P_H
#define IXATMDASSERT_P_H

#include "IxOsal.h"

#ifndef NDEBUG

/**
* @brief Customized version of an assert which display a contextual message,
*
* @oaram condition (in) condition to test
* @param fname (in) file name
* @param line (in) line number
* @param conditionString (in) condition to display
* @param infoString (in) text to display
*
* @return none
*/
void ixAtmdAccAssert (BOOL condition,
                       char *fname,
                       unsigned int line,
                       char *conditionString,
                       char *infoString);

/**
* @def IX_ATMDACC_ENSURE
* @brief test a condition and display the comment if the condition is false
*/
#define IX_ATMDACC_ENSURE(c,s) ixAtmdAccAssert((c), __FILE__, __LINE__, #c, s)

/**
* @def IX_ATMDACC_ABORT
* @brief test a condition and display the comment if the condition is false
*/
#define IX_ATMDACC_ABORT(c,s)  ixAtmdAccAssert((c), __FILE__, __LINE__, #c, s)

#else

/**
* @def IX_ATMDACC_ENSURE
* @brief tests removed from the production code
*/
#define IX_ATMDACC_ENSURE(c,s) /* nothing to do */

/**
* @def IX_ATMDACC_ABORT
* @brief cannot recover from a severe error
*/
#define IX_ATMDACC_ABORT(c,s)  IX_OSAL_ASSERT(c)

#endif  /*#ifndef NDEBUG */

#endif /* IXATMDASSERT_P_H */


