/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2006 Intel Corporation.  All rights reserved.
 * 
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors.  Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright and
 * trade secret laws and treaty provisions.  No part of the Material
 * may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without
 * Intel's prior express written permission.
 
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 * 
 * $Workfile: CdsErrors.h
 *
 *
 *
 */


#ifndef _CDS_ERRORS_H
#define _CDS_ERRORS_H

/*! \file CdsErrors.h 
	\brief Common CDS error codes.
*/

/*! \defgroup CdsErrors CDS Helper - Errors
	\brief This module provides information about CDS error codes and error strings.
	\{
*/

/*!	\brief UPnP/CDS/SOAP error code for a failed request.
*/
#define CDS_EC_ACTION_FAILED					501

/*!	\brief UPnP/CDS/SOAP error message for a failed request.
*/
#define CDS_EM_ACTION_FAILED					"Action failed. Internal error encountered."

/*!	\brief UPnP/CDS/SOAP error code when a query involves an object ID that does not exist.
*/
#define CDS_EC_OBJECT_ID_NO_EXIST				701

/*!	\brief UPnP/CDS/SOAP error message when a query involves an object ID that does not exist.
*/
#define CDS_EM_OBJECT_ID_NO_EXIST				"ObjectID does not exist."

/*!	\brief UPnP/CDS/SOAP error code when a query involves a CDS item when the request expects a CDS container.
*/
#define CDS_EC_NO_SUCH_CONTAINER				710

/*!	\brief UPnP/CDS/SOAP error message when a query involves a CDS item when the request expects a CDS container.
*/
#define CDS_EM_NO_SUCH_CONTAINER				"The specified ObjectID or ContainerID identifies an object that is not a container."

/*!	\brief These are UPnP layer error code, used in responding the UPnP actions.
 */
#define CDS_EC_INVALID_BROWSEFLAG				402

#define CDS_EM_INVALID_BROWSEFLAG				"Invalid value specified for BrowseFlag."

#define CDS_EC_INTERNAL_ERROR					500

#define	CDS_EM_INTERNAL_ERROR					"Unknown or internal error encountered."

#define CMS_EC_CONNECTION_DOES_NOT_EXIST		706

#define CMS_EM_CONNECTION_DOES_NOT_EXIST		"Connection does not exist."

/*!	\brief Enumeration of common CDS error codes.
 */
enum Enum_CdsErrors
{
	/*!	\brief No error encountered.
	*/
	CdsError_None = 0,
	
	/*!	\brief Use when the action failed for an unknown reason.
	*/
	CdsError_ActionFailed = CDS_EC_ACTION_FAILED,
	
	/*!	\brief Use when the CDS query specified an object ID that does not exist.
	*/
	CdsError_NoSuchObject = CDS_EC_OBJECT_ID_NO_EXIST,

	/*!	\brief Use when the CDS query required a CDS container but the specified object was a CDS item.
	*/
	CdsError_NoSuchContainer = CDS_EC_NO_SUCH_CONTAINER
};

/*!	\brief Static array of error message strings, such that the order corresponds
	to the order of error codes in \ref CDS_ErrorCodes.
*/
extern const char *CDS_ErrorStrings[];

/*!	\brief Static array of error message codes, such that the order corresponds
	to the order of error messages in \ref CDS_ErrorStrings.
*/
extern const int CDS_ErrorCodes[];

/*! \} */

#endif
