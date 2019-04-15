/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "org_olsr_plugin_pud_PositionUpdate.h"

#include "util.h"
#include <OlsrdPudWireFormat/nodeIdConversion.h>

/*
 * OLSR header
 */

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getOlsrMessageOriginator
 * Signature: ()Ljava/net/InetAddress;
 */
JNIEXPORT jobject JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getOlsrMessageOriginator
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	jobject object;
	bool ipv4 = !getUplinkMessageIPv6(&uplinkMessage->header);

	union olsr_ip_addr * addr;
	if (ipv4) {
		addr = (union olsr_ip_addr *) &uplinkMessage->msg.olsrMessage.v4.originator;
	} else {
		addr = (union olsr_ip_addr *) &uplinkMessage->msg.olsrMessage.v6.originator;
	}
	constructIPAddress(env, ipv4, addr, &object);

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return object;
}

/*
 * PudOlsrPositionUpdate
 */

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateVersion
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateVersion
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	uint8_t version = getPositionUpdateVersion(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jint) version;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateValidityTime
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateValidityTime
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	unsigned long validityTime = getValidityTime(
			&getPositionUpdateMessage(uplinkMessage)->validityTime);

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jlong) validityTime;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdatePresent
 * Signature: ()L
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdatePresent
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	uint32_t present = getPositionUpdatePresent(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jlong) present;
}

/*
 * GpsInfo
 */

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateTime
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateTime
  (JNIEnv * env, jobject this, jlong baseDate, jlong timezoneOffset) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	jlong baseDateSeconds = baseDate / 1000LL;
	jlong baseDateMilliSeconds = baseDate % 1000LL;

	struct tm timeStruct;
	time_t updateTimeSeconds;

	getPositionUpdateTime(getPositionUpdateMessage(uplinkMessage),
			baseDateSeconds, &timeStruct);

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	updateTimeSeconds = mktime(&timeStruct);
	return (updateTimeSeconds * 1000LL) + baseDateMilliSeconds - timezoneOffset;
}


/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateLatitude
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateLatitude
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	double lat = getPositionUpdateLatitude(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jdouble) lat;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateLongitude
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateLongitude
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	double lon = getPositionUpdateLongitude(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jdouble) lon;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateAltitude
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateAltitude
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	long alt = getPositionUpdateAltitude(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jlong) alt;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateSpeed
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateSpeed
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	unsigned long speed = getPositionUpdateSpeed(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jlong) speed;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateTrack
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateTrack
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	unsigned long track = getPositionUpdateTrack(
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jlong) track;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateHdop
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateHdop
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	double hdop = getPositionUpdateHdop(getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jdouble) hdop;
}

/*
 * NodeInfo
 */

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateNodeIdType
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateNodeIdType
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	NodeIdType nodeIdType = getPositionUpdateNodeIdType(
			!getUplinkMessageIPv6(&uplinkMessage->header) ? AF_INET : AF_INET6,
			getPositionUpdateMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jint) nodeIdType;
}

/*
 * Class:     org_olsr_plugin_pud_PositionUpdate
 * Method:    getPositionUpdateNodeId
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_olsr_plugin_pud_PositionUpdate_getPositionUpdateNodeId
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

    const char * nodeIdStr;
    char nodeIdStrBuffer[PUD_TX_NODEID_BUFFERSIZE];

	getNodeIdStringFromOlsr	(
			!getUplinkMessageIPv6(&uplinkMessage->header) ? AF_INET : AF_INET6,
			&uplinkMessage->msg.olsrMessage, &nodeIdStr,
			&nodeIdStrBuffer[0], sizeof(nodeIdStrBuffer));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (*env)->NewStringUTF(env, nodeIdStr);
}
