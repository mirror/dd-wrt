/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2005 Intel Corporation.  All rights reserved.
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
 * $Workfile: ILibParsers.h
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */

/*! \file ILibParsers.h 
	\brief MicroStack APIs for various functions and tasks
*/

#ifndef __ILibParsers__
#define __ILibParsers__

/*! \defgroup ILibParsers ILibParser Modules
	\{
*/

/*! \def MAX_HEADER_LENGTH
	Specifies the maximum allowed length for an HTTP Header
*/
#define MAX_HEADER_LENGTH 800

#ifdef MEMORY_CHECK
	#include <assert.h>
	#define MEMCHECK(x) x
#else
	#define MEMCHECK(x)
#endif

#ifdef _WIN32_WCE
	#define REQUIRES_MEMORY_ALIGNMENT
	#define errno 0
	#ifndef ERANGE
		#define ERANGE 1
	#endif
	#define time(x) GetTickCount()
#endif

#ifndef WIN32
	#define REQUIRES_MEMORY_ALIGNMENT
#endif

#if defined(WIN32) || defined (_WIN32_WCE)
	#ifndef MICROSTACK_NO_STDAFX
		#include "stdafx.h"
	#endif
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <netdb.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <sys/utsname.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <errno.h>
	#include <semaphore.h>
	#include <malloc.h>
	#include <fcntl.h>
	#include <signal.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>


#if defined(WIN32) || defined(_WIN32_WCE)
	#include <windows.h>
	#include <winioctl.h>
	#include <winbase.h>
#endif

#ifndef _WIN32_WCE
#include <time.h>
#include <sys/timeb.h>
#endif

#if defined(WIN32) || defined(_WIN32_WCE)
#ifndef FD_SETSIZE
	#define SEM_MAX_COUNT 64
#else
	#define SEM_MAX_COUNT FD_SETSIZE
#endif
	void ILibGetTimeOfDay(struct timeval *tp);
	#define sem_t HANDLE
	#define sem_init(x,pShared,InitValue) *x=CreateSemaphore(NULL,InitValue,SEM_MAX_COUNT,NULL)
	#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
	#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
	#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
	#define sem_post(x) ReleaseSemaphore(*x,1,NULL)

	#define strncasecmp(x,y,z) _strnicmp(x,y,z)
	#define strcasecmp(x,y) _stricmp(x,y)
	#define gettimeofday(tp,tzp) ILibGetTimeOfDay(tp)

	#if !defined(_WIN32_WCE)
		#define tzset() _tzset()
		#define daylight _daylight
		#define timezone _timezone
	#endif

	#ifndef stricmp
		#define stricmp(x,y) _stricmp(x,y)
	#endif

	#ifndef strnicmp
		#define strnicmp(x,y,z) _strnicmp(x,y,z)
	#endif

	#ifndef strcmpi
		#define strcmpi(x,y) _stricmp(x,y)
	#endif
#endif

/*! \def UPnPMIN(a,b)
	Returns the minimum of \a a and \a b.
*/
#define UPnPMIN(a,b) (((a)<(b))?(a):(b))
/*! \def ILibIsChainBeingDestroyed(Chain)
	Determines if the specified chain is in the process of being disposed.
*/
#define ILibIsChainBeingDestroyed(Chain) (*((int*)Chain))

typedef enum
{
	ILibServerScope_All=0,
	ILibServerScope_LocalLoopback=1,
	ILibServerScope_LocalSegment=2
}ILibServerScope;

#if defined(WIN32) || defined(_WIN32_WCE)
	typedef	void(*ILibChain_PreSelect)(void* object,void *readset, void *writeset, void *errorset, int* blocktime);
	typedef	void(*ILibChain_PostSelect)(void* object,int slct, void *readset, void *writeset, void *errorset);
#else
	typedef	void(*ILibChain_PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	typedef	void(*ILibChain_PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
#endif
typedef	void(*ILibChain_Destroy)(void* object);

int ILibGetCurrentTimezoneOffset_Minutes();
int ILibIsDaylightSavingsTime();
time_t ILibTime_Parse(char *timeString);
char* ILibTime_Serialize(time_t timeVal);


typedef void* ILibReaderWriterLock;
ILibReaderWriterLock ILibReaderWriterLock_Create();
ILibReaderWriterLock ILibReaderWriterLock_CreateEx(void *chain);
int ILibReaderWriterLock_ReadLock(ILibReaderWriterLock rwLock);
int ILibReaderWriterLock_ReadUnLock(ILibReaderWriterLock rwLock);
int ILibReaderWriterLock_WriteLock(ILibReaderWriterLock rwLock);
int ILibReaderWriterLock_WriteUnLock(ILibReaderWriterLock rwLock);
void ILibReaderWriterLock_Destroy(ILibReaderWriterLock rwLock);

/*! \defgroup DataStructures Data Structures
	\ingroup ILibParsers
	\{
	\}
*/


/*! \struct parser_result_field ILibParsers.h
	\brief Data Elements of \a parser_result
	\par
	This structure represents individual tokens, resulting from a call to
	\a ILibParseString and \a ILibParseStringAdv
*/
struct parser_result_field
{
	/*! \var data
		\brief Pointer to string
	*/
	char* data;

	/*! \var datalength
		\brief Length of \a data
	*/
	int datalength;

	/*! \var NextResult
		\brief Pointer to next token
	*/
	struct parser_result_field *NextResult;
};

/*! \struct parser_result ILibParsers.h
	\brief String Parsing Result Index
	\par
	This is returned from a successfull call to either \a ILibParseString or
	\a ILibParseStringAdv.
*/
struct parser_result
{
	/*! \var FirstResult
		\brief Pointer to the first token
	*/
	struct parser_result_field *FirstResult;
	/*! \var LastResult
		\brief Pointer to the last token
	*/
	struct parser_result_field *LastResult;
	/*! \var NumResults
		\brief The total number of tokens
	*/
	int NumResults;
};

/*! \struct packetheader_field_node ILibParsers.h
	\brief HTTP Headers
	\par
	This structure represents an individual header element. A list of these
	is referenced from a \a packetheader_field_node.
*/
struct packetheader_field_node
{
	/*! \var Field
		\brief Header Name
	*/
	char* Field;
	/*! \var FieldLength
		\brief Length of \a Field
	*/
	int FieldLength;
	/*! \var FieldData
		\brief Header Value
	*/
	char* FieldData;
	/*! \var FieldDataLength
		\brief Length of \a FieldData
	*/
	int FieldDataLength;
	/*! \var UserAllocStrings
		\brief Boolean indicating if the above strings are non-static memory
	*/
	int UserAllocStrings;
	/*! \var NextField
		\brief Pointer to the Next Header entry. NULL if this is the last one
	*/
	struct packetheader_field_node* NextField;
};

/*! \struct packetheader ILibParsers.h
	\brief Structure representing a packet formatted according to HTTP encoding rules
	\par
	This can be created manually by calling \a ILibCreateEmptyPacket(), or automatically via a call to \a ILibParsePacketHeader(...)
*/
struct packetheader
{
	/*! \var Directive
		\brief HTTP Method
		\par
		eg: \b GET /index.html HTTP/1.1
	*/
	char* Directive;
	/*! \var DirectiveLength
		\brief Length of \a Directive
	*/
	int DirectiveLength;
	/*! \var DirectiveObj
		\brief HTTP Method Path
		\par
		eg: GET \b /index.html HTTP/1.1
	*/
	char* DirectiveObj;
	/*! \var DirectiveObjLength
		\brief Length of \a DirectiveObj
	*/

	void *Reserved;

	int DirectiveObjLength;
	/*! \var StatusCode
		\brief HTTP Response Code
		\par
		eg: HTTP/1.1 \b 200 OK
	*/
	int StatusCode;
	/* \var StatusData
		\brief Status Meta Data
		\par
		eg: HTTP/1.1 200 \b OK
	*/
	char* StatusData;
	/*! \var StatusDataLength
		\brief Length of \a StatusData
	*/
	int StatusDataLength;
	/*! \var Version
		\brief HTTP Version
		\par
		eg: 1.1
	*/
	char* Version;
	/*! \var VersionLength
		\brief Length of \a Version
	*/
	int VersionLength;
	/*! \var Body
		\brief Pointer to HTTP Body
	*/
	char* Body;
	/*! \var BodyLength
		\brief Length of \a Body
	*/
	int BodyLength;
	/*! \var UserAllocStrings
		\brief Boolean indicating if Directive/Obj are non-static
		\par
		This only needs to be set, if you manually populate \a Directive and \a DirectiveObj.<br>
		It is \b recommended that you use \a ILibSetDirective
	*/
	int UserAllocStrings;	// Set flag if you allocate memory pointed to by Directive/Obj
	/*! \var UserAllocVersion
		\brief Boolean indicating if Version string is non-static
		\par
		This only needs to be set, if you manually populate \a Version.<br>
		It is \b recommended that you use \a ILibSetVersion
	*/	
	int UserAllocVersion;	// Set flag if you allocate memory pointed to by Version
	int ClonedPacket;
	
	/*! \var FirstField
		\brief Pointer to the first Header field
	*/
	struct packetheader_field_node* FirstField;
	/*! \var LastField
		\brief Pointer to the last Header field
	*/
	struct packetheader_field_node* LastField;
	
	/*! \var Source
		\brief The origin of this packet
		\par
		This is only populated if you obtain this structure from either \a ILibWebServer or
		\a ILibWebClient.
	*/
	struct sockaddr_in *Source;
	/*! \var ReceivingAddress
		\brief IP address that this packet was received on
		\par
		This is only populated if you obtain this structure from either \a ILibWebServer or
		\a ILibWebClient.
	*/
	int ReceivingAddress;
	void *HeaderTable;
};

/*! \struct ILibXMLNode
	\brief An XML Tree
	\par
	This is obtained via a call to \a ILibParseXML. It is \b highly \b recommended
	that you call \a ILibProcessXMLNodeList, so that the node pointers at the end of this
	structure will be populated. That will greatly simplify node traversal.<br><br>
	In order for namespaces to be resolved, you must call \a ILibXML_BuildNamespaceLookupTable(...)
	with root-most node that you would like to resolve namespaces for. It is recommended that you always use
	the root node, obtained from the initial call to \a ILibParseXML.<br><br>
	For most intents and purposes, you only need to work with the "StartElements"
*/
struct ILibXMLNode
{
	/*! \var Name
		\brief Local Name of the current element
	*/
	char* Name;			// Element Name
	/*! \var NameLength
		\brief Length of \a Name
	*/
	int NameLength;
	
	/*! \var NSTag
		\brief Namespace Prefix of the current element
		\par
		This can be resolved using a call to \a ILibXML_LookupNamespace(...)
	*/
	char* NSTag;		// Element Prefix
	/*! \var NSLength
		\brief Length of \a NSTag
	*/
	int NSLength;

	/*! \var StartTag
		\brief boolean indicating if the current element is a start element
	*/
	int StartTag;		// Non zero if this is a StartElement
	/*! \var EmptyTag
		\brief boolean indicating if this element is an empty element
	*/
	int EmptyTag;		// Non zero if this is an EmptyElement
	
	void *Reserved;		// DO NOT TOUCH
	void *Reserved2;	// DO NOT TOUCH
	
	/*! \var Next
		\brief Pointer to the child of the current node
	*/
	struct ILibXMLNode *Next;			// Next Node
	/*! \var Parent
		\brief Pointer to the Parent of the current node
	*/
	struct ILibXMLNode *Parent;			// Parent Node
	/*! \var Peer
		\brief Pointer to the sibling of the current node
	*/
	struct ILibXMLNode *Peer;			// Sibling Node
	struct ILibXMLNode *ClosingTag;		// Pointer to closing node of this element
	struct ILibXMLNode *StartingTag;	// Pointer to start node of this element
};

/*! \struct ILibXMLAttribute
	\brief A list of XML Attributes for a specified XML node
	\par
	This can be obtained via a call to \a ILibGetXMLAttributes(...)
*/
struct ILibXMLAttribute
{
	/*! \var Name
		\brief Local name of Attribute
	*/
	char* Name;						// Attribute Name
	/*! \var NameLength
		\brief Length of \a Name
	*/
	int NameLength;
	
	/*! \var Prefix
		\brief Namespace Prefix of this attribute
		\par
		This can be resolved by calling \a ILibXML_LookupNamespace(...) and passing in \a Parent as the current node
	*/
	char* Prefix;					// Attribute Namespace Prefix
	/*! \var PrefixLength
		\brief Lenth of \a Prefix
	*/
	int PrefixLength;
	
	/*! \var Parent
		\brief Pointer to the XML Node that contains this attribute
	*/
	struct ILibXMLNode *Parent;		// The XML Node this attribute belongs to

	/*! \var Value
		\brief Attribute Value
	*/
	char* Value;					// Attribute Value	
	/*! \var ValueLength
		\brief Length of \a Value
	*/
	int ValueLength;
	/*! \var Next
		\brief Pointer to the next attribute
	*/
	struct ILibXMLAttribute *Next;	// Next Attribute
};

char *ILibReadFileFromDisk(char *FileName);
int ILibReadFileFromDiskEx(char **Target, char *FileName);
void ILibWriteStringToDisk(char *FileName, char *data);
void ILibWriteStringToDiskEx(char *FileName, char *data, int dataLen);
void ILibDeleteFileFromDisk(char *FileName);


/*! \defgroup StackGroup Stack
	\ingroup DataStructures
	Stack Methods
	\{
*/
void ILibCreateStack(void **TheStack);
void ILibPushStack(void **TheStack, void *data);
void *ILibPopStack(void **TheStack);
void *ILibPeekStack(void **TheStack);
void ILibClearStack(void **TheStack);
/*! \} */

/*! \defgroup QueueGroup Queue
	\ingroup DataStructures
	Queue Methods
	\{
*/
void *ILibQueue_Create();
void ILibQueue_Destroy(void *q);
int ILibQueue_IsEmpty(void *q);
void ILibQueue_EnQueue(void *q, void *data);
void *ILibQueue_DeQueue(void *q);
void *ILibQueue_PeekQueue(void *q);
void ILibQueue_Lock(void *q);
void ILibQueue_UnLock(void *q);
long ILibQueue_GetCount(void *q);
/* \} */


/*! \defgroup XML XML Parsing Methods
	\ingroup ILibParsers
MicroStack supplied XML Parsing Methods
	\par
\b Note: None of the XML Parsing Methods copy or allocate memory
The lifetime of any/all strings is bound to the underlying string that was
parsed using ILibParseXML. If you wish to keep any of these strings for longer
then the lifetime of the underlying string, you must copy the string.
	\{
*/


//
// Parses an XML string. Returns a tree of ILibXMLNode elements.
//
struct ILibXMLNode *ILibParseXML(char *buffer, int offset, int length);

//
// Preprocesses the tree of ILibXMLNode elements returned by ILibParseXML.
// This method populates all the node pointers in each node for easy traversal.
// In addition, this method will also determine if the given XML document was well formed.
// Returns 0 if processing succeeded. Specific Error Codes are returned otherwise.
//
int ILibProcessXMLNodeList(struct ILibXMLNode *nodeList);

//
// Initalizes a namespace lookup table for a given parent node. 
// If you want to resolve namespaces, you must call this method exactly once
//
void ILibXML_BuildNamespaceLookupTable(struct ILibXMLNode *node);

//
// Resolves a namespace prefix.
//
char* ILibXML_LookupNamespace(struct ILibXMLNode *currentLocation, char *prefix, int prefixLength);

//
// Fetches all the data for an element. Returns the length of the populated data
//
int ILibReadInnerXML(struct ILibXMLNode *node, char **RetVal);

//
// Returns the attributes of an XML element. Returned as a linked list of ILibXMLAttribute.
//
struct ILibXMLAttribute *ILibGetXMLAttributes(struct ILibXMLNode *node);

void ILibDestructXMLNodeList(struct ILibXMLNode *node);
void ILibDestructXMLAttributeList(struct ILibXMLAttribute *attribute);

//
// Escapes an XML string.
// indata must be pre-allocated. 
//
int ILibXmlEscape(char* outdata, const char* indata);

//
// Returns the required size string necessary to escape this XML string
//
int ILibXmlEscapeLength(const char* data);

//
// Unescapes an XML string.
// Since Unescaped strings are always shorter than escaped strings,
// the resultant string will overwrite the supplied string, to save memory
//
int ILibInPlaceXmlUnEscape(char* data);

/*! \} */

/*! \defgroup ChainGroup Chain Methods
	\ingroup ILibParsers
	\brief Chaining Methods
	\{
*/
void *ILibCreateChain();
void ILibAddToChain(void *chain, void *object);
void ILibChain_SafeAdd(void *chain, void *object);
void ILibChain_SafeRemove(void *chain, void *object);
void ILibChain_SafeAdd_SubChain(void *chain, void *subChain);
void ILibChain_SafeRemove_SubChain(void *chain, void *subChain);
void ILibChain_DestroyEx(void *chain);
void ILibStartChain(void *chain);
void ILibStopChain(void *chain);
void ILibForceUnBlockChain(void *Chain);
/* \} */



/*! \defgroup LinkedListGroup Linked List
	\ingroup DataStructures
	\{
*/

//
// Initializes a new Linked List data structre
//
void* ILibLinkedList_Create();

//
// Returns the Head node of a linked list data structure
//
void* ILibLinkedList_GetNode_Head(void *LinkedList);		// Returns Node

//
// Returns the Tail node of a linked list data structure
//
void* ILibLinkedList_GetNode_Tail(void *LinkedList);		// Returns Node

//
// Returns the Next node of a linked list data structure
//
void* ILibLinkedList_GetNextNode(void *LinkedList_Node);	// Returns Node

//
// Returns the Previous node of a linked list data structure
//
void* ILibLinkedList_GetPreviousNode(void *LinkedList_Node);// Returns Node

//
// Returns the number of nodes contained in a linked list data structure
//
long ILibLinkedList_GetCount(void *LinkedList);

//
// Returns a shallow copy of a linked list data structure. That is, the structure
// is copied, but none of the data contents are copied. The pointer values are just copied.
//
void* ILibLinkedList_ShallowCopy(void *LinkedList);

//
// Returns the data pointer of a linked list element
//
void *ILibLinkedList_GetDataFromNode(void *LinkedList_Node);

//
// Creates a new element, and inserts it before the given node
//
void ILibLinkedList_InsertBefore(void *LinkedList_Node, void *data);

//
// Creates a new element, and inserts it after the given node
//
void ILibLinkedList_InsertAfter(void *LinkedList_Node, void *data);

//
// Removes the given node from a linked list data structure
//
void* ILibLinkedList_Remove(void *LinkedList_Node);

//
// Given a data pointer, will traverse the linked list data structure, deleting
// elements that point to this data pointer.
//
int ILibLinkedList_Remove_ByData(void *LinkedList, void *data);

//
// Creates a new element, and inserts it at the top of the linked list.
//
void ILibLinkedList_AddHead(void *LinkedList, void *data);

//
// Creates a new element, and appends it to the end of the linked list
//
void ILibLinkedList_AddTail(void *LinkedList, void *data);

void ILibLinkedList_Lock(void *LinkedList);
void ILibLinkedList_UnLock(void *LinkedList);
void ILibLinkedList_Destroy(void *LinkedList);
/*! \} */



/*! \defgroup HashTreeGroup Hash Table
	\ingroup DataStructures
	\b Note: Duplicate key entries will be overwritten.
	\{
*/

//
// Initialises a new Hash Table (tree) data structure
//
void* ILibInitHashTree();
void* ILibInitHashTree_CaseInSensitive();
void ILibDestroyHashTree(void *tree);

//
// Returns non-zero if the key entry is found in the table
//
int ILibHasEntry(void *hashtree, char* key, int keylength);

//
// Add a new entry into the hashtable. If the key is already used, it will be overwriten.
//
void ILibAddEntry(void* hashtree, char* key, int keylength, void *value);
void ILibAddEntryEx(void* hashtree, char* key, int keylength, void *value, int valueEx);
void* ILibGetEntry(void *hashtree, char* key, int keylength);
void ILibGetEntryEx(void *hashtree, char* key, int keylength, void **value, int *valueEx);
void ILibDeleteEntry(void *hashtree, char* key, int keylength);

//
// Returns an Enumerator to browse all the entries of the Hashtable
//
void *ILibHashTree_GetEnumerator(void *tree);
void ILibHashTree_DestroyEnumerator(void *tree_enumerator);

//
// Advance the Enumerator to the next element.
// Returns non-zero if there are no more entries to enumerate
//
int ILibHashTree_MoveNext(void *tree_enumerator);

//
// Obtains the value of a Hashtable Entry.
//
void ILibHashTree_GetValue(void *tree_enumerator, char **key, int *keyLength, void **data);
void ILibHashTree_GetValueEx(void *tree_enumerator, char **key, int *keyLength, void **data, int *dataEx);
void ILibHashTree_Lock(void *hashtree);
void ILibHashTree_UnLock(void *hashtree);

/*! \} */

/*! \defgroup LifeTimeMonitor LifeTimeMonitor
	\ingroup ILibParsers
	\brief Timed Callback Service
	\par
	These callbacks will always be triggered on the thread that calls ILibStartChain().
	\{
*/

//
// Adds an event trigger to be called after the specified time elapses, with the
// specified data object
//
#define ILibLifeTime_Add(LifetimeMonitorObject, data, seconds, Callback, Destroy) ILibLifeTime_AddEx(LifetimeMonitorObject, data, seconds*1000, Callback, Destroy)
void ILibLifeTime_AddEx(void *LifetimeMonitorObject,void *data, int milliseconds, void* Callback, void* Destroy);

//
// Removes all event triggers that contain the specified data object.
//
void ILibLifeTime_Remove(void *LifeTimeToken, void *data);

//
// Removes all events triggers
//
void ILibLifeTime_Flush(void *LifeTimeToken);
void *ILibCreateLifeTime(void *Chain);

/* \} */


/*! \defgroup StringParsing String Parsing
	\ingroup ILibParsers
	\{
*/

int ILibFindEntryInTable(char *Entry, char **Table);

//
// Trims preceding and proceding whitespaces from a string
//
int ILibTrimString(char **theString, int length);

char* ILibString_Cat(const char *inString1, int inString1Len, const char *inString2, int inString2Len);
char *ILibString_Copy(const char *inString, int length);
int ILibString_EndsWith(const char *inString, int inStringLength, const char *endWithString, int endWithStringLength);
int ILibString_EndsWithEx(const char *inString, int inStringLength, const char *endWithString, int endWithStringLength, int caseSensitive);
int ILibString_StartsWith(const char *inString, int inStringLength, const char *startsWithString, int startsWithStringLength);
int ILibString_StartsWithEx(const char *inString, int inStringLength, const char *startsWithString, int startsWithStringLength, int caseSensitive);
int ILibString_IndexOfEx(const char *inString, int inStringLength, const char *indexOf, int indexOfLength, int caseSensitive);
int ILibString_IndexOf(const char *inString, int inStringLength, const char *indexOf, int indexOfLength);
int ILibString_LastIndexOf(const char *inString, int inStringLength, const char *lastIndexOf, int lastIndexOfLength);
int ILibString_LastIndexOfEx(const char *inString, int inStringLength, const char *lastIndexOf, int lastIndexOfLength, int caseSensitive);
char *ILibString_Replace(const char *inString, int inStringLength, const char *replaceThis, int replaceThisLength, const char *replaceWithThis, int replaceWithThisLength);
char *ILibString_ToUpper(const char *inString, int length);
char *ILibString_ToLower(const char *inString, int length);
void ILibToUpper(const char *in, int inLength, char *out);
void ILibToLower(const char *in, int inLength, char *out);
//
// Parses the given string using the specified multichar delimiter.
// Returns a parser_result object, which points to a linked list
// of parser_result_field objects.
//
struct parser_result* ILibParseString(char* buffer, int offset, int length, char* Delimiter, int DelimiterLength);

//
// Same as ILibParseString, except this method ignore all delimiters that are contains within
// quotation marks
//
struct parser_result* ILibParseStringAdv(char* buffer, int offset, int length, char* Delimiter, int DelimiterLength);

//
// Releases resources used by string parser
//
void ILibDestructParserResults(struct parser_result *result);

//
// Parses a URI into IP Address, Port Number, and Path components
// Note: IP and Path must be freed.
//
void ILibParseUri(char* URI, char** IP, unsigned short* Port, char** Path);

//
// Parses a string into a Long or unsigned Long. 
// Returns non-zero on error condition
//
int ILibGetLong(char *TestValue, int TestValueLength, long* NumericValue);
int ILibGetULong(const char *TestValue, const int TestValueLength, unsigned long* NumericValue);
int ILibFragmentText(char *text, int textLength, char *delimiter, int delimiterLength, int tokenLength, char **RetVal);
int ILibFragmentTextLength(char *text, int textLength, char *delimiter, int delimiterLength, int tokenLength);


/* Base64 handling methods */
int ILibBase64Encode(unsigned char* input, const int inputlen, unsigned char** output);
int ILibBase64Decode(unsigned char* input, const int inputlen, unsigned char** output);

/* Compression Handling Methods */
char* ILibDecompressString(unsigned char* CurrentCompressed, const int bufferLength, const int DecompressedLength);

/* \} */


/*! \defgroup PacketParsing Packet Parsing
	\ingroup ILibParsers
	\{
*/

/* Packet Methods */

//
// Allocates an empty HTTP Packet
//
struct packetheader *ILibCreateEmptyPacket();

//
// Add a header into the packet. (String is copied)
//
void ILibAddHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength, char* FieldData, int FieldDataLength);
void ILibDeleteHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength);

//
// Fetches the header value from the packet. (String is NOT copied)
// Returns NULL if the header field does not exist
//
char* ILibGetHeaderLine(struct packetheader *packet, char* FieldName, int FieldNameLength);

//
// Sets the HTTP version: 1.0, 1.1, etc. (string is copied)
//
void ILibSetVersion(struct packetheader *packet, char* Version, int VersionLength);

//
// Set the status code and data line. The status data is copied.
//
void ILibSetStatusCode(struct packetheader *packet, int StatusCode, char* StatusData, int StatusDataLength);

//
// Sets the method and path. (strings are copied)
//
void ILibSetDirective(struct packetheader *packet, char* Directive, int DirectiveLength, char* DirectiveObj, int DirectiveObjLength);

//
// Releases all resources consumed by this packet structure
//
void ILibDestructPacket(struct packetheader *packet);

//
// Parses a string into an packet structure.
// None of the strings are copied, so the lifetime of all the values are bound
// to the lifetime of the underlying string that is parsed.
//
struct packetheader* ILibParsePacketHeader(char* buffer, int offset, int length);

//
// Returns the packetized string and it's length. (must be freed)
//
int ILibGetRawPacket(struct packetheader *packet,char **buffer);

//
// Performs a deep copy of a packet structure
//
struct packetheader* ILibClonePacket(struct packetheader *packet);

//
// Escapes a string according to HTTP escaping rules.
// indata must be pre-allocated
//
int ILibHTTPEscape(char* outdata, const char* indata);

//
// Returns the size of string required to escape this string,
// according to HTTP escaping rules
//
int ILibHTTPEscapeLength(const char* data);

//
// Unescapes the escaped string sequence
// Since escaped string sequences are always longer than unescaped
// string sequences, the resultant string is overwritten onto the supplied string
//
int ILibInPlaceHTTPUnEscape(char* data);
/* \} */

/*! \defgroup NetworkHelper Network Helper
	\ingroup ILibParsers
	\{
*/

char *ILibIPAddress_ToDottedQuad(int ip);

//
// Obtain an array of IP Addresses available on the local machine.
//
int ILibGetLocalIPAddressList(int** pp_int);
#if defined(WINSOCK2)
	int ILibGetLocalIPAddressNetMask(int address);
#endif
#if defined(WIN32) || defined(_WIN32_WCE)
	unsigned short ILibGetDGramSocket(int local, HANDLE *TheSocket);
	unsigned short ILibGetStreamSocket(int local, unsigned short PortNumber,HANDLE *TheSocket);
#else
	unsigned short ILibGetDGramSocket(int local, int *TheSocket);
	unsigned short ILibGetStreamSocket(int local, unsigned short PortNumber,int *TheSocket);
#endif

/* \} */

void* dbg_malloc(int sz);
void dbg_free(void* ptr);
int dbg_GetCount();

/* \} */   // End of ILibParser Group
#endif
