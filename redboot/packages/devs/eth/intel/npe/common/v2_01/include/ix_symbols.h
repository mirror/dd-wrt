/**
 * ============================================================================
 * = COPYRIGHT
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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
 * = PRODUCT
 *      Intel(r) IXP425 Software Release
 *
 * = FILENAME
 *      ix_symbols.h
 *
 * = DESCRIPTION
 *      This file declares all the global preprocessor symbols required by 
 *      the IXA SDK Framework API.
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = CHANGE HISTORY
 *      4/23/2002 10:41:13 AM - creation time 
 * ============================================================================
 */

#if !defined(__IX_SYMBOLS_H__)
#define __IX_SYMBOLS_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 * The IX_EXPORT_FUNCTION symbol will be used for compilation on different platforms.
 * We are planning to provide a simulation version of the library that should work
 * with the Transactor rather than the hardware. This implementation will be done on
 * WIN32 in the form of a DLL that will need to export functions and symbols.
 */
#if (_IX_OS_TYPE_ == _IX_OS_WIN32_)
#    if defined(_IX_LIB_INTERFACE_IMPLEMENTATION_)
#        define IX_EXPORT_FUNCTION __declspec( dllexport )
#    elif defined(_IX_LIB_INTERFACE_IMPORT_DLL_)
#        define IX_EXPORT_FUNCTION __declspec( dllimport )
#    else
#        define IX_EXPORT_FUNCTION extern 
#    endif
#elif (_IX_OS_TYPE_ == _IX_OS_WINCE_)
#    define IX_EXPORT_FUNCTION __declspec(dllexport)
#else
#    define IX_EXPORT_FUNCTION  extern
#endif


/**
 * This symbols should be defined when we want to build for a multithreaded environment
 */
#define _IX_MULTI_THREADED_     1
    

/**
 * This symbol should be defined in the case we to buils for a multithreaded environment
 * but we want that our modules to work as if they are used in a single threaded environment.
 */
/* #define _IX_RM_EXPLICIT_SINGLE_THREADED_    1  */

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_SYMBOLS_H__) */
