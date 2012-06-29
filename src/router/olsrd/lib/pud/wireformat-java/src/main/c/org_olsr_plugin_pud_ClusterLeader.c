#include "org_olsr_plugin_pud_ClusterLeader.h"

#include "util.h"

/*
 * Class:     org_olsr_plugin_pud_ClusterLeader
 * Method:    getClusterLeaderVersion
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_olsr_plugin_pud_ClusterLeader_getClusterLeaderVersion
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	uint8_t version = getClusterLeaderVersion(
			getClusterLeaderMessage(uplinkMessage));

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jint) version;
}

/*
 * Class:     org_olsr_plugin_pud_ClusterLeader
 * Method:    getClusterLeaderValidityTime
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_olsr_plugin_pud_ClusterLeader_getClusterLeaderValidityTime
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	unsigned long validityTime = getValidityTime(
			&getClusterLeaderMessage(uplinkMessage)->validityTime);

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return (jlong) validityTime;
}

/*
 * Class:     org_olsr_plugin_pud_ClusterLeader
 * Method:    getClusterLeaderOriginator
 * Signature: ()Ljava/net/InetAddress;
 */
JNIEXPORT jobject JNICALL Java_org_olsr_plugin_pud_ClusterLeader_getClusterLeaderOriginator
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	jobject object;
	bool ipv4 = !getUplinkMessageIPv6(&uplinkMessage->header);

	union olsr_ip_addr * addr;
	if (ipv4) {
		addr = (union olsr_ip_addr *) &uplinkMessage->msg.clusterLeader.leader.v4.originator;
	} else {
		addr = (union olsr_ip_addr *) &uplinkMessage->msg.clusterLeader.leader.v6.originator;
	}
	constructIPAddress(env, ipv4, addr, &object);

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return object;
}

/*
 * Class:     org_olsr_plugin_pud_ClusterLeader
 * Method:    getClusterLeaderClusterLeader
 * Signature: ()Ljava/net/InetAddress;
 */
JNIEXPORT jobject JNICALL Java_org_olsr_plugin_pud_ClusterLeader_getClusterLeaderClusterLeader
  (JNIEnv * env, jobject this) {
	jobject dataObject;
	jboolean isCopy;
	UplinkMessage * uplinkMessage = getUplinkMessage(env, this, &dataObject,
			&isCopy);

	jobject object;
	bool ipv4 = !getUplinkMessageIPv6(&uplinkMessage->header);

	union olsr_ip_addr * addr;
	if (ipv4) {
		addr = (union olsr_ip_addr *) &uplinkMessage->msg.clusterLeader.leader.v4.clusterLeader;
	} else {
		addr = (union olsr_ip_addr *) &uplinkMessage->msg.clusterLeader.leader.v6.clusterLeader;
	}
	constructIPAddress(env, ipv4, addr, &object);

	releaseUplinkMessage(env, uplinkMessage, dataObject, isCopy, JNI_ABORT);

	return object;
}
