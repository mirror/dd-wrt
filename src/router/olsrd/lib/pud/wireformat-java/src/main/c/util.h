#ifndef _Included_org_olsr_plugin_pud_UplinkMessage
#define _Included_org_olsr_plugin_pud_UplinkMessage
#ifdef __cplusplus
extern "C" {
#endif

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
#endif
#endif
