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
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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


