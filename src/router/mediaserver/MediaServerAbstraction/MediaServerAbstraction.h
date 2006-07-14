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
 * $Workfile: MediaServerAbstraction.h
 *
 *
 *
 */

#ifndef MEDIASERVERABSTRACTION_H
#define MEDIASERVERABSTRACTION_H

/*! \file MediaServerAbstraction.h 
	\brief DLNA MediaServer API
*/

#include "MediaServer_MicroStack.h"
#include "DlnaHttpServer.h"
#include "CdsObject.h"

/*! \defgroup MediaServerAbstraction DLNA - Media Server Abstraction
	\brief This module allows management of a
	MediaServer's (e.g. DMS, M-DMS) state. Also works for DMS and M-DMS devices.
	
	\a MediaServerAbstraction is built on top of the generated DeviceBuilder API.
	This module essentially provides a level of abstraction from 
	the UPnP communications, such that upper application layers
	can work with structs when processing UPnP requests. This module 
	achieves this by implementing 4 roles.

 	The first role is to provide all of the dispatch points for the generated 
 	UPnP stack. Equivalently, callbacks for responding to a UPnP action/request come 
 	to all of methods defined in this module. These methods are identified by the
 	"MediaServer_ContentDirectory_" and "MediaServer_ConnectionManager_" prefixes. These methods
 	translate a UPnP request into arguments for various callbacks.
 
 	The second role is to provide callbacks to upper application layers. These
 	callbacks are identified by the \a MSA_Callback_ prefix. 
 
 	The third role for this module is to provide the methods that allow the
 	"logic behind a MediaServer" to respond to UPnP requests and update the
 	MediaServer's state as reported on the UPnP network. These "public" methods
 	are identified by the "MSA_" prefix.

	The fourth role for this module is to allow applications to set
	state parameters for the MediaServer. These "public" methods
 	are identified by the "MSA_" prefix.

	\note
 	- Each method call requires the object returned from \ref MSA_CreateMediaServer().
 
 	- All strings for the interface are assumed to be UTF8 compliant. This means
 	 that string arguments in callbacks will be sent in their UTF8-form
 	 and that all strings sent in response to a CP's action are properly
 	 encoded by the application layer in UTF8 form. 

	\warning Due to limitations in the underlying UPnP stack, 
	this module is designed to be a singleton. \ref MSA_CreateMediaServer() 
	should only be called once for an entire process.

	\warning 
 	<b>THREADING NOTE</b>
 	The \a MediaServerAbstraction is threadless. In practice, the 
	\a MediaServerAbstraction will execute callbacks on the
 	thread that executes the Microstack thread-chain. 
 	Upper layers that provide dispatch points for \a MediaServerAbstraction
	callbacks are expected
 	to return quickly, as not doing so will cause thread-starvation for UPnP 
 	communications. The dispatch points for \a MediaServerAbstraction
	callbacks should not perform
 	I/O operations, time-consuming computations, or employ locks that are not
 	sufficiently deterministic.

	\{
*/

/*!	\brief Object that represents the Media Server Abstraction Instance.
*/
struct MSA_Instance;

/*!	\brief Pointer to MSA_Instance object.
*/
typedef struct MSA_Instance* MSA;

struct MSA_Stats;
/*!	\brief Method signature used when the \ref MSA reports an update.

	\warning Applications should not reference the struct directly once
	execution returns to the \a MediaServerAbstraction.

	\param[in] msa_obj \ref MSA. Obtained from \ref MSA_CreateMediaServer().
	\param[in] stats New usage statistics.
 */
typedef void (*MSA_Callback_OnStatsChanged) (MSA msa_obj, struct MSA_Stats* stats);

/*!	\brief Structure for storing query parameters on Browse and Search requests.
*/
struct MSA_CdsQuery;
/*!	\brief Method signature for handling CDS Browse queries.

 	\note Applications should use the MSA_DeallocateCdsQuery()
 	to free resources held by browseArgs.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
	\param[in] cds_query The input arguments for a CDS query (\ref MSA_CdsQuery).
	\ref MSA_CdsQuery::IpAddrList is ordered such that the first
	IP address in the list is the one where the UPnP request
	was received. This list is its own local copy.
*/
typedef void (*MSA_Callback_OnBrowse) (MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query);

/*!	\brief Method signature for handling CDS Search queries.

 	\note Applications should use the MSA_DeallocateCdsQuery()
 	to free resources held by searchArgs.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
	\param[in] cds_query The input arguments for a CDS query (\ref MSA_CdsQuery).
	\ref MSA_CdsQuery::IpAddrList is ordered such that the first
	IP address in the list is the one where the UPnP request
	was received. This list is its own local copy.
*/
typedef void (*MSA_Callback_OnSearch) (MSA msa_obj, struct MSA_CdsQuery* cds_query, void* upnp_session);

/*!	\brief Method signature for handling CDS GetSystemUpdateId action.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
*/
typedef void (*MSA_Callback_OnGetSystemUpdateID) (MSA msa_obj, void* upnp_session);

/*!	\brief Method signature for handling CDS GetSearchCapabilities action.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
*/
typedef void (*MSA_Callback_OnGetSearchCapabilities) (MSA msa_obj, void* upnp_session);

/*!	\brief Method signature for handling CDS GetSortCapabilities action.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
*/
typedef void (*MSA_Callback_OnGetSortCapabilities) (MSA msa_obj, void* upnp_session);

/*!	\brief Method signature for handling CMS GetProtocolInfo action.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
*/
typedef void (*MSA_Callback_OnGetProtocolInfo) (MSA msa_obj, void* upnp_session);

/*!	\brief Method signature for handling CMS GetCurrentConnectionIDs action.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
*/
typedef void (*MSA_Callback_OnGetCurrentConnectionIDs) (MSA msa_obj, void* upnp_session);

/*!	\brief Method signature for handling CMS GetCurrentConnectionInfo action.
 
	\param[in] msa_obj \ref MSA object. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
*/
typedef void (*MSA_Callback_OnGetCurrentConnectionInfo) (MSA msa_obj, void* upnp_session,  int connection_id);

#if defined(INCLUDE_FEATURE_UPLOAD)

/*!	\brief Method signature for handling DestroyObject requests.
	\param[in] msa_obj \ref MSA. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
	\param[in] create_obj_arg The struct contains the Object Id of the CDS Object specified by the Control Point.
*/
typedef void (*MSA_Callback_OnDestroyObject) (MSA msa_obj,  void* upnp_session, void* destroy_obj_arg);

/*!	\brief Method signature for handling upload requests.
	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
 	\param[in] upnp_session The UPnP Session Token for responding the request.
	\param[in] create_obj_arg The struct contains the neccessary parameters along with the CDS Object created by the Control Point.
	\param[in,out] cds_object The CDS object (and associated &lt;res&gt; elements)
	that was specified in the CDS:CreateObject request.
	Applications are expected to finalize the metadata on the CDS object. This
	includes the following tasks.
	- Setting the final metadata values for all informative properties (title, creator, etc.)
	- Setting the parentID (when the request indicates a parent container of <i>DLNA.ORG_AnyContainer</i>.
*/
typedef void (*MSA_Callback_OnCreateObject) (MSA msa_obj, void* upnp_session, void* create_obj_arg, struct CdsObject* cds_object);
#endif

/*!	\brief The \a MediaServerAbstraction instance.
*/

typedef void* MSA_State;

/*!	\brief These are internal error codes that can be used in the DMS.
*/
typedef enum
{
    /** \brief There is no error. */
	MSA_ERROR_NONE = 0,
    /** \brief Internal error, the internal state of the MSA object is not valid. */
	MSA_ERROR_INTERNALERROR = -1
} MSA_Error;

/** \brief Bit masks for determining if the action handler should be executed
    on a seperate thread.
*/
typedef enum 
{
    MSA_CDS_BROWSE						= 0x0001,
    MSA_CDS_SEARCH						= 0x0002,
    MSA_CDS_GETSYSTEMUPDATEID			= 0x0004,
    MSA_CDS_GETSEARCHCAPABILITIES		= 0x0008,
    MSA_CDS_GETSORTCAPABILITIES			= 0x0010,
    MSA_CMS_GETPROTOCOLINFO				= 0x0020,
    MSA_CMS_GETCURRENTCONNECTIONIDS		= 0x0040,
    MSA_CMS_GETCURRENTCONNECTIONINFO	= 0x0080,
#if defined(INCLUDE_FEATURE_UPLOAD)
    MSA_CDS_CREATEOBJECT				= 0x0100,
    MSA_CDS_DESTROYOBJECT				= 0x0200,
#endif
    MSA_DEFAULT_ALL_ACTION_HANDLER		= 0xffff /* default: ALL events are context switched! */
} MSA_ActionHandlerContextSwitchId;

struct MSA_Instance
{

	/** \brief	The PreSelect, PostSelect, and Destroy fields must
		remain here as the object needs to be compatible with the
		thread-chaining framework.
	 */
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	/*
	 *	Pointer to generated upnp microstack object.
	 */
	void*	DeviceMicroStack;

    /** \brief This is the internal state of the DMS instance.

        The contents of the variable are hidden from the vendor on purpose.
    */
    MSA_State InternalState;

    /** \brief This is used by both the vendor application and the MSA microstack.

        This value should never be NULL but can be a thread pool of 0 threads.
    */
    ILibThreadPool ThreadPool;

    /** \brief 	This is the lifetime monitor object used in moderation
    */

	void* LifeTimeMonitor;

	/*
	 *	This bit mask indicates whether the UPnP action handler will be called using context switch.
	 */
    MSA_ActionHandlerContextSwitchId		ContextSwitchBitMask;

    /** \brief 	Function pointer to update the statistics on all UPnP action invocation.
    */
	MSA_Callback_OnStatsChanged				OnStats;

    /** \brief 	Function pointer to the handler to handle a CDS Browse request.
    */
	MSA_Callback_OnBrowse					OnBrowse;

    /** \brief 	Function pointer to the handler to handle a CDS Search request.
    */
	MSA_Callback_OnSearch					OnSearch;

    /** \brief 	Function pointer to the handler to handle a CDS GetSystemUpdateId request.
    */
	MSA_Callback_OnGetSystemUpdateID		OnGetSystemUpdateID;

    /** \brief 	Function pointer to the handler to handle a CDS GetSearchCapabilities request.
    */
	MSA_Callback_OnGetSearchCapabilities	OnGetSearchCapabilities;

    /** \brief 	Function pointer to the handler to handle a CDS GetSortCapabilities request.
    */
	MSA_Callback_OnGetSortCapabilities		OnGetSortCapabilities;

    /** \brief 	Function pointer to the handler to handle a CMS GetProtocolInfo request.
    */
	MSA_Callback_OnGetProtocolInfo			OnGetProtocolInfo;

    /** \brief 	Function pointer to the handler to handle a CMS GetCurrentConnectionIDs request.
    */
	MSA_Callback_OnGetCurrentConnectionIDs	OnGetCurrentConnectionIDs;

    /** \brief 	Function pointer to the handler to handle a CMS GetCurrentConnectionInfo request.
    */
	MSA_Callback_OnGetCurrentConnectionInfo OnGetCurrentConnectionInfo;

#if defined(INCLUDE_FEATURE_UPLOAD)
    /** \brief 	Function pointer to the handler to handle a CDS CreateObject request.
    */
	MSA_Callback_OnCreateObject				OnCreateObject;

    /** \brief 	Function pointer to the handler to handle a CDS DestroyObject request.
    */
	MSA_Callback_OnDestroyObject			OnDestroyObject;
#endif
};

/*!	\brief Provides statistical info on the calls made on the media server.
 */
struct MSA_Stats
{
	unsigned int		Count_Browse;
	unsigned int		Count_Search;
	unsigned int		Count_GetSystemUpdateID;
	unsigned int		Count_GetSearchCapabilities;
	unsigned int		Count_GetSortCapabilities;
	unsigned int		Count_GetCurrentConnectionIDs;
	unsigned int		Count_GetCurrentConnectionInfo;
	unsigned int		Count_GetProtocolInfo;
};

/*!	\brief Specifies the type of CDS query.
 */
enum MSA_Enum_QueryTypes
{
	/*!	\brief CDS:Browse request with the <i>BrowseMetadata</i> option.
	*/
	MSA_Query_BrowseMetadata,

	/*!	\brief CDS:Browse request with the <i>BrowseDirectChildren</i> option.
	*/
	MSA_Query_BrowseDirectChildren,

	/*!	\brief CDS:Search request.
	*/
	MSA_Query_Search
};

/*!	\brief Encapsulates input arguments for a CDS:Browse or CDS:Search request.
 */
struct MSA_CdsQuery
{
	/*!	\brief Specifies what type of query.
	 */
	enum MSA_Enum_QueryTypes QueryType;

	/*!	\brief ObjectID specified by the control point.
	 */
	char* ObjectID;

	/*!	\brief Metadata filter settings. 
		Comma-delimited list of [namespace_prefix]:[tag_name] strings, 
		that describe what CDS metadata to return in the response. 
		In this framework, the application layer can enforcing 
		metadata filtering or the \ref MSA
		can handle it.
	 */
	char* Filter;

	/*!	\brief The index of the first media object to return. Zero-based value.
	 
		\note Only applicable when \ref MSA_CdsQuery::QueryType 
		is not \ref MSA_Query_BrowseMetadata.
	 */
	unsigned int StartingIndex;
	
	/*!	\brief The maximum number of media objects to return. 
		Zero means return all media objects.

		\note Only applicable when \ref MSA_CdsQuery::QueryType 
		is not \ref MSA_Query_BrowseMetadata.
	*/
	unsigned int RequestedCount;

	/*!	\brief SortCriteria strings have the following form:
		[+/-][namespace prefix]:[tag name],[+/-][namespace prefix]:[tag name],...	 

		The application layer is responsible for providing results
		according to the correct sorted order. The application layer
		can configure the MediaServer's reported set of sortable
		fields by using \ref MSA_SetSortableProperties().
	 */
	char* SortCriteria;

	/*!	\brief If \ref MSA_CdsQuery::QueryType == \ref MSA_Query_Search, 
		then this field specifies the search query, as specified in the format
		of the CDS specification.

		The application layer is responsible for providing results
		according for specified query. The application layer
		can configure the MediaServer's reported set of sortable
		fields by using \ref MSA_SetSearchableProperties().
	*/
	char* SearchCriteria;

	/*!	\brief Miscellaneous field for application-level logic.
	
		\note Application must free this field if it allocates to it.
	 */
	void* UserObject;

	/*!	\brief Provides IP address of the network interface that received
		this CDS query.
	*/
	int RequestedOnPort;

	/*!	\brief Provides IP port number of the network interface that received
		this CDS query.
	*/
	int RequestedOnAddress;

	/*!	\brief Provides the list of IP addresses that 
		are active on this machine when the request
		was received.
	
		The application is responsible for checking to see if the
		\ref MSA_CdsQuery::Filter includes the
		<b>ALLIP</b> string. 
		- If <b>ALLIP</b> is included
		the application is supposed to create a <i>CdsResource</i> 
		for each available IP address that the content can
		be requested on. 
		- If <b>ALLIP</b> is not included, then 
		the application is supposed to create a <i>CdsResource</i> 
		for only the first IP address.
	*/
	int* IpAddrList;

	/*	\brief The number of IP addresses in \ref MSA_CdsQuery::IpAddrList.
	*/
	int IpAddrListLen;
};

#if defined(INCLUDE_FEATURE_UPLOAD)

/*!	\brief Provides connection info on the upload requests made on the media server.
 */
struct MSA_CdsCreateObj
{
	/*
	 *	IN: Miscellaneous field for application-level logic.
	 *	    Application must free this field if it allocates to it.

	 */
	void *UserObject;

	/*
	 *	Provides IP address and port information 
	 *	regarding the interface that received the request.
	 */
	int RequestedOnPort;
	int RequestedOnAddress;

	/*
	 *	Provides the list of IP addresses that 
	 *	are active on this machine when the request
	 *	was received.
	 */
	int *IpAddrList;
	int IpAddrListLen;
};

struct MSA_CdsDestroyObj
{
	/*!	\brief ObjectID specified by the control point.
	 */
	char* ObjectID;
	/*
	 *	IN: Miscellaneous field for application-level logic.
	 *	    Application must free this field if it allocates to it.
	 */
	void *UserObject;
};

/*!	\brief Application call this method to accept a CreateChildContainer request on \ref MSA_Callback_OnCreateObject callback.
	\warning You should only call this method on CreateChildContainer requests.  You should use \ref MSA_ForCreateObjectResponse_AcceptUpload
	for upload requests on CDS items.

	\param[in] msa_obj \ref MSA. Obtained from \ref MSA_CreateMediaServer().
	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnCreateObject callback.
	\param[in] create_obj_arg \ref MSA_CdsCreateObj argument.  Obtained from the \ref MSA_Callback_OnCreateObject callback
	\param[in] cds_obj The CDS object that was specified in the CDS:CreateObject request.

	\returns Non-zero value if accepting request completes sucessfully. 
 */
int MSA_ForCreateObjectResponse_Accept(MSA msa_obj, void* upnp_session, struct MSA_CdsCreateObj* create_obj_arg, struct CdsObject* cds_obj);

/*!	\brief Application call this method to accept a upload request associated with a CDS item on \ref MSA_Callback_OnCreateObject callback.
	\warning You should not call this method on CreateChildContainer requests.

	\param[in] msa_obj \ref MSA. Obtained from \ref MSA_CreateMediaServer().
	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnCreateObject callback.
	\param[in] create_obj_arg \ref MSA_CdsCreateObj argument.  Obtained from the \ref MSA_Callback_OnCreateObject callback
	\param[in] cds_obj The CDS object that was specified in the CDS:CreateObject request.
	\param[in] upload_file The local file path specified to save the uploaded file.
	\param[in] callback_response \ref DHS_OnResponseDone callback when transfer completes.

	\returns Non-zero value if accepting request completes sucessfully. 
 */
int MSA_ForCreateObjectResponse_AcceptUpload(MSA msa_obj, void* upnp_session, struct MSA_CdsCreateObj* create_obj_arg, struct CdsObject* cds_obj);

/*!	\brief Application call this method to reject a CreateChildContainer or upload request on \ref MSA_Callback_OnCreateObject callback.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnCreateObject callback.
	\param[in] create_obj_arg \ref MSA_CdsCreateObj argument.  Obtained from the \ref MSA_Callback_OnCreateObject callback
	\param[in] cds_obj The CDS object that was specified in the CDS:CreateObject request.
	\param[in] error_code UPnP error code to send back to the control point.
	\param[in] error_msg UPnP error message associated with the error code to send back to the control point.

	\returns Non-zero value if rejecting request completes sucessfully. 
*/
int MSA_ForCreateObjectResponse_Reject(MSA msa_obj, void* upnp_session, struct MSA_CdsCreateObj* create_obj_arg, struct CdsObject* cds_obj, int error_code, char* error_msg);

#endif

/*!	\brief This method creates the MediaServer object that abstracts state information
	for a MediaServer. The returned object will be added to the specified chain.
	Remember to call \ref MSA_UpdateIpInfo() after this method.

	\note If the application's metadata back-end has an asynchronous API,
	then use of the threadpool (\a use_thread_pool) is usually the more efficient choice. Specifically,
	the application's handlers for callbacks will issue the request 
	on the back-end metadata system as and return execution to the UPnP module
	quickly. When the back-end metadata system completes the request,
	the application should then call the various \a MSA_ForQueryResponse_xxx
	methods to actually transmit the response to the UPnP control point
	on a separate thread.

	\param[in] chain Microstack thread-chain object. see \a ILibCreateChain().
	\param[in] upnp_stack The state object for the UPnP stack, obtained from
	\a MediaServer_CreateMicroStack().
	\param[in] lifetime_monitor A lifetime monitor, for timing purposes
	obtained from \a ILibCreateLifeTime().
	\param[in] system_update_id
	The initial system update id value.
	\param[in] sink_protocol_info
	The comma-separated-value list of protocolInfo values, 
	supported by this MediaServer as a sink. Generally an empty string ("").
	\param[in] source_protocol_info
	The comma-separated-value list of protocolInfo values, 
	supported by this MediaServer as a source. 
	Generally the union set of protocolInfo values that
	a control point will find if the entire CDS is enumerated.
	\param[in] sortable_fields
	The comma-separated-value list of metadata properties
	that this MediaServer can use to sort results.
	\param[in] searchable_fields
	The comma-separated-value list of metadata properties
	that this MediaServer can use for search-based queries.
	\param[in] thread_pool
	- If this value is not NULL, then all callbacks will be executed using
	the generated UPnP stack's threadpool library. This effectively causes
	the callback to execute on a context switch, allowing the application
	layer to respond to the query in a more synchronous manner.
	- If this value is NULL, then callbacks will execute on the thread
	dedicated to the UPnP stack. This model of executing callbacks is
	recommended only when the application layer will perform its own
	context-switch and respond to the CDS request on a different thread.
	\param[in] msa_user_obj A miscellaneous object that is sent when 
	callbacks are executed.
 */
MSA MSA_CreateMediaServer(void* chain, MediaServer_MicroStackToken upnp_stack, void* lifetime_monitor, unsigned int system_update_id, const char* sink_protocol_info, const char* source_protocol_info, const char* sortable_fields, const char* searchable_fields, ILibThreadPool thread_pool, void* msa_user_obj);

/*!	\brief Allows applications to deallocate the \ref MSA_CdsQuery object
	that was executed by an \ref MSA_Callback_OnQuery.
	
	\param[in] cds_query Address of the \ref  MSA_CdsQuery specified in the callback.
 */
void MSA_DeallocateCdsQuery(struct MSA_CdsQuery* cds_query);

/*!	\brief Application-level code can call this method within their
	handler of an \ref MSA_Callback_OnQuery callback
	when they need to respond to the control point with an error.

	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnXXX callback.
	\param[in] error_code This value can be one of the values in \ref MSA_ErrorCodes
	or it can be be a vendor-defined value between the range of [800,899] inclusive.
	\param[in] error_msg This is the custom-message to include with the error code.
	The string must be encoded with UTF8 compliance.
 */
void MSA_ForResponse_RespondError(MSA msa_obj, void* upnp_session, int error_code, const char* error_msg);

/*!	\brief Application-level code can call this method within their
	implementation of an \ref MSA_Callback_OnQuery handler when they need to 
	begin the response to the control point. 
	This method should be called before calling 
	\ref MSA_ForQueryResponse_ResultArgumentRaw(), 
	\ref MSA_ForQueryResponse_ResultArgumentStruct, or
	\ref MSA_ForQueryResponse_FinishResponse.
 
	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnBrowse callback.
	\param[in] cds_query CDS query specified in the \ref MSA_Callback_OnQuery callback.
	\param[in] send_didl_header_flag 
	- If nonzero, the DIDL-Lite header is sent. 
	- If nonzero, the application is responsible for sending that header
	through \ref MSA_ForQueryResponse_ResultArgumentRaw().
	\returns 
	- A negative if an error occured during transmission.
	In case of a negative return value, a performance recommendation is to 
	call \ref MSA_ForQueryResponse_Cancelled() to acknowledge the failure and 
	allow the system to clean up.
	- A zero value indicates all data was sent to the receiver.
	- A positive value indicates that the data has been sent to the IP stack, but that the receiver has not acknowledged it.
 */
int MSA_ForQueryResponse_Start(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, int send_didl_header_flag);

/*!	\brief Application-level code can call this method within their
	implementation of \ref MSA_Callback_OnQuery when they need to respond to the
	control point with data in the <i>Result</i> argument. This method must
	be called after \ref MSA_ForQueryResponse_Start() but
	before \ref MSA_ForQueryResponse_FinishResponse().

	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnBrowse callback.
	\param[in] cds_query CDS query specified in the \ref MSA_Callback_OnQuery callback.
	\param[in] xml_escaped_utf8_didl
	DIDL-Lite response data, where the data is properly XML-escaped and also in UTF8 encoding.
	The upper application layer can call this an arbitrary number of times, so the
	upper application layer can decide between making fewer calls with larger
	DIDL-Lite data blocks or more calls with smaller DIDL-Lite data blocks.
	\param[in] didl_size
	The number of bytes in xmlEscapedUtf8Didl, not including a trailing null-terminator.
	\returns
	- A negative if an error occured during transmission.
	In case of a negative return value, a performance recommendation is to 
	call \ref MSA_ForQueryResponse_Cancelled() to acknowledge the failure and 
	allow the system to clean up.
	- A zero value indicates all data was sent to the receiver.
	- A positive value indicates that the data has been sent to the IP stack, but that the receiver has not acknowledged it.
 */
int MSA_ForQueryResponse_ResultArgumentRaw(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, const char* xml_escaped_utf8_didl, int didl_size);

/*!	\brief Application-level code can call this method within their
	implementation of \ref MSA_Callback_OnQuery when they need to respond to the
	control point with data in the <i>Result</i> argument. This method must
	be called after \ref MSA_ForQueryResponse_Start() but
	before \ref MSA_ForQueryResponse_FinishResponse().

	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnBrowse callback.
	\param[in] cds_query CDS query specified in the \ref MSA_Callback_OnQuery callback.
	\param[in] cds_object a <i>CdsObject</i> such that all of the
	associated string values are properly encoded in UTF-8.
	\returns
	- A negative if an error occured during transmission.
	In case of a negative return value, a performance recommendation is to 
	call \ref MSA_ForQueryResponse_Cancelled() to acknowledge the failure and 
	allow the system to clean up.
	- A zero value indicates all data was sent to the receiver.
	- A positive value indicates that the data has been sent to the IP stack, but that the receiver has not acknowledged it.
 */
int MSA_ForQueryResponse_ResultArgumentStruct(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, struct CdsObject* cds_object);

/*!	\brief Application-level code can call this method within their
	implementation of \ref MSA_Callback_OnQuery when they need to finish the
	response to a control point.

	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnBrowse callback.
	\param[in] cds_query CDS query specified in the \ref MSA_Callback_OnQuery callback.
	\param[in] send_did_footer_flag
	- If nonzero, the DIDL-Lite footer is sent. 
	- If nonzero, the application will have sent the DIDL-Lite footer in a previous
	call to \ref MSA_ForQueryResponse_ResultArgumentRaw().
	\param[in] number_returned 
	The number of DIDL-Lite CDS objects returned in the response.
	\param[in] total_matches
	The total number of DIDL-Lite CDS objects that could be returned 
	if the request had specified a <i>RequestedCount</i> ==0.
	\param[in] update_id
	The updateID of the container or item.

	\returns 
	- A negative if an error occured during transmission.
	In case of a negative return value, a performance recommendation is to 
	call \ref MSA_ForQueryResponse_Cancelled() to acknowledge the failure and 
	allow the system to clean up.
	\warning In case a negative return value, DO NOT call \ref MSA_ForQueryResponse_Cancelled 
	because \a MediaServerAbstraction will automatically assume
	that you will not make additional calls to other
	\a MSA_ForQueryResponse_xxx methods for the given CDS request.
	(i.e. MSA does the necessary cleanup automatically.)
	- A zero value indicates all data was sent to the receiver.
	- A positive value indicates that the data has been sent to the IP stack, but that the receiver has not acknowledged it.
 */
int MSA_ForQueryResponse_FinishResponse(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, int send_did_footer_flag, unsigned int number_returned, unsigned int total_matches, unsigned int update_id);

/*!	\brief Call this method if \a MSA_ForQueryResponse_xxx returned nonzero value, or
	when \ref MSA_ForQueryResponse_FinishResponse() is not called.
	
	This method the tells generated UPnP source code that the app layer 
	has no intention to call additional MSA_ForQueryResponse_xxx methods.

	\param[in] upnp_session The UPnP session token specified in the \ref MSA_Callback_OnBrowse callback.
	\param[in] cds_query CDS query specified in the \ref MSA_Callback_OnQuery callback.
 */
void MSA_ForQueryResponse_Cancelled(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query);

/*!	\brief Applications call this method to increment the MediaServers's reported
	value for the CDS.SystemUpdateID state variable.
	
	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
 */
void MSA_IncrementSystemUpdateID(MSA msa_obj);

/*!	\brief Applications call this method to get the MediaServers's reported
	value for the CDS.SystemUpdateID state variable.
	
	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
 */
unsigned int MSA_GetSystemUpdateID(MSA msa_obj);

/*!	\brief Applications call this method to set the MediaServers's reported
	value for the CDS.SystemUpdateID state variable.
	
	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] system_update_id The system update id value.
 */
void MSA_SetSystemUpdateID(MSA msa_obj, unsigned int system_update_id);

/*!	\brief Applications call this method to instruct the MediaServer's reported
	UpdateID values for individual container objects. This method
	will automatically moderate the delivery of events for the
	CDS.ContainerUpdateID state variable.
	
	Although this method automatically moderates the delivery of the UPnP event,
	applications should avoid calling this method repetitively with the same
	containerID in a short period of time. The primary performance issue with this
	method is that it has to prevent duplicate containerID entries in the
	state variable.

	\note Unlike the CDS.SystemUpdateID, the \a MediaServerAbstraction object
	does not track container's UpdateID values over time.
 
	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] container_id The ID of the container that changed.
	\param[in] container_update_id The new updateID of the container that changed.
 */
void MSA_UpdateContainerID(MSA msa_obj, const char* container_id, unsigned int container_update_id);

/*!	\brief Applications call this method to update the list of IP addresses
	for the MediaServer. 
	This method needs to be called after \ref MSA_CreateMediaServer().

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] ip_addr_list Array of IP addresses that the MediaServer object
	should use when advertising itself over SSDP.
	\param[in] ip_addr_list_len Number of entries in \a ip_addr_list.
 */
void MSA_UpdateIpInfo(MSA msa_obj, int *ip_addr_list, int ip_addr_list_len);

/*!	\brief Applications should call this method before calling any
	\a MSA_ function that takes an \ref MSA to ensure
	thread-safe access to the data.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
*/
void MSA_LockState(MSA msa_obj);

/*!	\brief Applications should call this method after calling any
	\a MSA_ function that takes an \ref MSA to release
	the \ref MSA object.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
*/
void MSA_UnLockState(MSA msa_obj);

/*!	\brief Applications call this to retrieve the UserObject that was passed in when calling
	\ref MSA_CreateMediaServer.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
*/
void* MSA_GetUserObject(MSA msa_obj);

/*!	\brief Applications call this to set the protocolInfo
	values that describe what the MediaServer is able to serve .
	(i.e. <i>CDS.SourceProtocolInfo</i> state variable)

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] source_protocol_info A comma-separated list of protocolInfo values.
*/
void MSA_SetSourceProtocolInfo(MSA msa_obj, const char* source_protocol_info);

/*!	\brief Applications call this to retrieve the MediaServer's currently
	advertised list of <i>SourceProtocolInfo</i> values.
	values that describe what the MediaServer is able to serve.

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after the returned value is no longer 
	being accessed by the application layer.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
*/
const char* MSA_GetSourceProtocolInfo(MSA msa_obj);

/*!	\brief Applications call this to set the protocolInfo
	values that describe what the MediaServer is able to serve .
	(i.e. <i>CDS.SinkProtocolInfo</i> state variable)

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] sink_protocol_info A comma-separated list of protocolInfo values.
*/
void MSA_SetSinkProtocolInfo(MSA msa_obj, const char* sink_protocol_info);

/*!	\brief Applications call this to retrieve the MediaServer's currently
	advertised list of <i>SinkProtocolInfo</i> values.
	values that describe what the MediaServer is able to serve.

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after the returned value is no longer 
	being accessed by the application layer.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
*/
const char* MSA_GetSinkProtocolInfo(MSA msa_obj);

/*!	\brief Applications call this to set the MediaServer's currently
	advertised list of sortable metadata properties for use in
	the <i>SortCriteria</i> argument of CDS:Browse and CDS:Search requests.

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] properties Comma separated list of properties that can be used for the
	<i>SortCriteria</i> argument of a CDS:Browse or CDS:Search request.

*/
void MSA_SetSortableProperties(MSA msa_obj, const char* properties);

/*!	\brief Applications call this to get the MediaServer's currently
	advertised list of sortable metadata properties.

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after the returned value is no longer 
	being accessed by the application layer.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\returns A null-terminated string that is a comma-separated list of sortable metadata properties.
*/
const char* MSA_GetSortableProperties(MSA msa_obj);

/*!	\brief Applications call this to set the MediaServer's currently
	advertised list of searchable metadata properties for use in
	the <i>SearchCriteria</i> argument of CDS:Search requests.

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after returning from the method call.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\param[in] properties Comma separated list of properties that can be used for the
	<i>SearchCriteria</i> argument of a CDS:Search request.
*/
void MSA_SetSearchableProperties(MSA msa_obj, const char* properties);

/*!	\brief Applications call this to get the MediaServer's currently
	advertised list of searchable metadata properties.

	\note Applications should call \ref MSA_LockState() before calling this method
	and \ref MSA_UnLockState() after the returned value is no longer 
	being accessed by the application layer.

	\param[in] msa_obj \ref MSA instance. Obtained from \ref MSA_CreateMediaServer().
	\returns A null-terminated string that is a comma-separated list of searchable metadata properties.
*/
const char* MSA_GetSearchableProperties(MSA msa_obj);

/** Generic respond methods for all supported UPnP action **/

void MSA_RespondBrowse(MSA msa_obj, void* upnp_session, const char* result, const unsigned int number_returned, const unsigned int total_matches, const unsigned int update_id);

void MSA_RespondSearch(MSA msa_obj, void* upnp_session, const char* result, const unsigned int number_returned, const unsigned int total_matches, const unsigned int update_id);

void MSA_RespondGetSystemUpdateID(MSA msa_obj, void* upnp_session, const unsigned int id);

void MSA_RespondGetSearchCapabilities(MSA msa_obj, void* upnp_session, const char* search_caps);

void MSA_RespondGetSortCapabilities(MSA msa_obj, void* upnp_session, const char* sort_caps);

void MSA_RespondGetProtocolInfo(MSA msa_obj, void* upnp_session, const char* source, const char* sink);

void MSA_RespondGetCurrentConnectionIDs(MSA msa_obj, void* upnp_session, const char* connection_ids);

void MSA_RespondGetCurrentConnectionInfo(MSA msa_obj, void* upnp_session, const int rcs_ic, const int av_transport_id, const char* protocol_info, const char* peer_connection_manager, const int peer_connection_id, const char* direction, const char* status);

#if defined(INCLUDE_FEATURE_UPLOAD)
void MSA_RespondCreateObject(MSA msa_obj, void* upnp_session, const char* object_id, const char* result);

void MSA_RespondDestroyObject(MSA msa_obj, void* upnp_session);
#endif
/*! \} */

#endif

