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
