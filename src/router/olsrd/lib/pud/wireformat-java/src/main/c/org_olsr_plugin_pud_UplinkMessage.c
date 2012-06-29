#include "org_olsr_plugin_pud_UplinkMessage.h"

#include <OlsrdPudWireFormat/wireFormat.h>

/*
 * Uplink Message Types
 */

/*
 * Class:     org_olsr_plugin_pud_UplinkMessage
 * Method:    getUplinkMessageTypePosition
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_olsr_plugin_pud_UplinkMessage_getUplinkMessageTypePosition
  (JNIEnv * env __attribute__ ((unused)), jclass this __attribute__ ((unused))) {
	return (jint) POSITION;
}

/*
 * Class:     org_olsr_plugin_pud_UplinkMessage
 * Method:    getUplinkMessageTypeClusterLeader
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_olsr_plugin_pud_UplinkMessage_getUplinkMessageTypeClusterLeader
  (JNIEnv * env __attribute__ ((unused)), jclass this __attribute__ ((unused))) {
	return (jint) CLUSTERLEADER;
}

/*
 * Class:     org_olsr_plugin_pud_UplinkMessage
 * Method:    getUplinkMessageHeaderLength
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_olsr_plugin_pud_UplinkMessage_getUplinkMessageHeaderLength
  (JNIEnv * env __attribute__ ((unused)), jclass this __attribute__ ((unused))) {
	return (jint)sizeof(UplinkHeader);
}
