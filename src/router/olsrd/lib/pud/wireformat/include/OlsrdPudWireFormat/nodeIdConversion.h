#ifndef _PUD_NODEIDCONVERSION_H_
#define _PUD_NODEIDCONVERSION_H_

#include <OlsrdPudWireFormat/wireFormat.h>

void getNodeTypeStringFromOlsr(int ipVersion,
		PudOlsrPositionUpdate * olsrGpsMessage, char * nodeIdTypeBuffer,
		int nodeIdTypeBufferSize);

void getNodeIdStringFromOlsr(int ipVersion, union olsr_message *olsrMessage,
		const char **nodeIdStr, char *nodeIdStrBuffer,
		unsigned int nodeIdStrBufferSize);

bool setupNodeIdBinaryMAC(nodeIdBinaryType * nodeIdBinary, unsigned char * mac);

bool setupNodeIdBinaryLongLong(nodeIdBinaryType * nodeIdBinary,
		unsigned long long longValue, size_t bytes);

bool setupNodeIdBinaryDoubleLongLong(nodeIdBinaryType * nodeIdBinary,
    unsigned long long value1, unsigned char * dst1, size_t bytes1,
    unsigned long long value2, unsigned char * dst2, size_t bytes2);

bool setupNodeIdBinaryString(nodeIdBinaryType * nodeIdBinary, char * nodeId,
		size_t nodeIdLength);

bool setupNodeIdBinaryIp(nodeIdBinaryType * nodeIdBinary, void * ip,
		size_t ipLength);

#endif /* _PUD_NODEIDCONVERSION_H_ */
