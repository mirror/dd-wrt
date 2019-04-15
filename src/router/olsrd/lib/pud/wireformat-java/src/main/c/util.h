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

#ifndef _Included_org_olsr_plugin_pud_UplinkMessage
#define _Included_org_olsr_plugin_pud_UplinkMessage
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <olsr_types.h>
#include <jni.h>
#include <OlsrdPudWireFormat/wireFormat.h>

/*
 * Helpers
 */

/* get the uplink message (a byte array) from the java object */
static inline UplinkMessage * getUplinkMessage(JNIEnv * env, jobject this,
		jobject * dataObject, jboolean * isCopy) {
	UplinkMessage * uplinkMessage;

	jclass clazz = (*env)->GetObjectClass(env, this);
	jfieldID fid = (*env)->GetFieldID(env, clazz, "data", "[B");
	*dataObject = (*env)->GetObjectField(env, this, fid);
	uplinkMessage = (UplinkMessage *) ((*env)->GetByteArrayElements(env,
			*dataObject, isCopy));

	return uplinkMessage;
}

/* release (when it is a copy) the uplink message (a byte array) that was
 * retrieved from the java object
 *
 * mode = 0 (copy back + free)
 *        JNI_COMMIT (copy back + do not free)
 *        JNI_ABORT (do not copy back + free) */
static inline void releaseUplinkMessage(JNIEnv * env,
		UplinkMessage * uplinkMessage, jobject dataObject, jboolean isCopy,
		jint mode) {
	if (isCopy == JNI_TRUE) {
		(*env)->ReleaseByteArrayElements(env, dataObject,
				(jbyte *) uplinkMessage, mode);
	}
}

static inline PudOlsrPositionUpdate * getPositionUpdateMessage(
		UplinkMessage * uplinkMessage) {
	return getOlsrMessagePayload(
			!getUplinkMessageIPv6(&uplinkMessage->header) ? AF_INET : AF_INET6,
			&uplinkMessage->msg.olsrMessage);
}

static inline UplinkClusterLeader * getClusterLeaderMessage(
		UplinkMessage * uplinkMessage) {
	return &uplinkMessage->msg.clusterLeader;
}

static inline void constructIPAddress(JNIEnv * env, bool ipv4,
		union olsr_ip_addr * addr, jobject * object) {
	if (ipv4) {
		/* object = new Inet4Address(NULL, originator); */
		jclass clazz = (*env)->FindClass(env, "java/net/Inet4Address");
		jmethodID mid = (*env)->GetMethodID(env, clazz, "<init>",
				"(Ljava/lang/String;I)V");
		*object = (*env)->NewObject(env, clazz, mid, NULL, ntohl(addr->v4.s_addr));
	} else {
		jboolean isCopy;
		jfieldID fid;
		jobject ipaddressObject;
		jbyte * ipaddressByteArray;

		/* object = new Inet6Address(); */
		jclass clazz = (*env)->FindClass(env, "java/net/Inet6Address");
		jmethodID mid = (*env)->GetMethodID(env, clazz, "<init>", "()V");
		*object = (*env)->NewObject(env, clazz, mid);

		/* ipaddressByteArray = &object->ipaddress[0] (might be a copy) */
		fid = (*env)->GetFieldID(env, clazz, "ipaddress", "[B");
		ipaddressObject = (*env)->GetObjectField(env, *object, fid);
		ipaddressByteArray = ((*env)->GetByteArrayElements(env, ipaddressObject,
				&isCopy));

		/* copy the IP address into the byte array */
		memcpy(ipaddressByteArray, &addr->v6, sizeof(struct in6_addr));

		/* commit when needed */
		if (isCopy == JNI_TRUE) {
			(*env)->ReleaseByteArrayElements(env, ipaddressObject,
					ipaddressByteArray, 0);
		}
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _Included_org_olsr_plugin_pud_UplinkMessage */
