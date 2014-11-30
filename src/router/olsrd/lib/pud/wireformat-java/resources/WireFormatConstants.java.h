#include "library.h"
#include "wireFormat.h"
#ifndef GIT_SHA
  #define GIT_SHA ""
#endif /* GIT_SHA */

package org.olsr.plugin.pud;

public class WireFormatConstants {
	public static final String LibraryName = PUD_WIRE_FORMAT_LIBRARYNAME;

	public static final String PLUGINVERSION = PLUGIN_VER;
	public static final String GITSHA = GIT_SHA;

	public static final int VERSION = PUD_WIRE_FORMAT_VERSION;

	public static final int PRESENT_ID = PUD_PRESENT_ID;
	public static final int PRESENT_GATEWAY = PUD_PRESENT_GATEWAY;

	public static final int TIME_BITS = PUD_TIME_BITS;

	public static final int LATITUDE_BITS = PUD_LATITUDE_BITS;
	public static final int TX_LATITUDE_DIGITS = PUD_TX_LATITUDE_DIGITS;
	public static final String TX_LATITUDE_DECIMALS = PUD_TX_LATITUDE_DECIMALS;

	public static final int LONGITUDE_BITS = PUD_LONGITUDE_BITS;
	public static final int TX_LONGITUDE_DIGITS = PUD_TX_LONGITUDE_DIGITS;
	public static final String TX_LONGITUDE_DECIMALS = PUD_TX_LONGITUDE_DECIMALS;
	public static final int ALTITUDE_BITS = PUD_ALTITUDE_BITS;

	public static final long ALTITUDE_MIN = PUD_ALTITUDE_MIN;
	public static final long ALTITUDE_MAX = PUD_ALTITUDE_MAX;
	public static final int TX_ALTITUDE_DIGITS = PUD_TX_ALTITUDE_DIGITS;

	public static final int SPEED_BITS = PUD_SPEED_BITS;
	public static final long SPEED_MAX = PUD_SPEED_MAX;
	public static final int TX_SPEED_DIGITS = PUD_TX_SPEED_DIGITS;

	public static final int TRACK_BITS = PUD_TRACK_BITS;
	public static final int TX_TRACK_DIGITS = PUD_TX_TRACK_DIGITS;

	public static final int HDOP_BITS = PUD_HDOP_BITS;
	public static final double HDOP_RESOLUTION = PUD_HDOP_RESOLUTION;
	public static final double HDOP_MAX = PUD_HDOP_MAX;
	public static final int TX_HDOP_DIGITS = PUD_TX_HDOP_DIGITS;
	public static final String TX_HDOP_DECIMALS = PUD_TX_HDOP_DECIMALS;

	public static final int NODEIDTYPE_MAC_BYTES = PUD_NODEIDTYPE_MAC_BYTES;
	public static final int NODEIDTYPE_MSISDN_BYTES = PUD_NODEIDTYPE_MSISDN_BYTES;
	public static final int NODEIDTYPE_TETRA_BYTES = PUD_NODEIDTYPE_TETRA_BYTES;
	public static final int NODEIDTYPE_UUID_BYTES  = PUD_NODEIDTYPE_UUID_BYTES;
	public static final int NODEIDTYPE_UUID_BYTES1 = PUD_NODEIDTYPE_UUID_BYTES1;
	public static final int NODEIDTYPE_UUID_BYTES2 = PUD_NODEIDTYPE_UUID_BYTES2;
	public static final int NODEIDTYPE_UUID_CHARS  = PUD_NODEIDTYPE_UUID_CHARS;
	public static final int NODEIDTYPE_UUID_CHARS1 = PUD_NODEIDTYPE_UUID_CHARS1;
	public static final int NODEIDTYPE_UUID_CHARS2 = PUD_NODEIDTYPE_UUID_CHARS2;
	public static final int NODEIDTYPE_MMSI_BYTES = PUD_NODEIDTYPE_MMSI_BYTES;
	public static final int NODEIDTYPE_URN_BYTES = PUD_NODEIDTYPE_URN_BYTES;
	public static final int NODEIDTYPE_MIP_BYTES  = PUD_NODEIDTYPE_MIP_BYTES;
	public static final int NODEIDTYPE_MIP_BYTES1 = PUD_NODEIDTYPE_MIP_BYTES1;
	public static final int NODEIDTYPE_MIP_BYTES2 = PUD_NODEIDTYPE_MIP_BYTES2;
	public static final int NODEIDTYPE_MIP_CHARS  = PUD_NODEIDTYPE_MIP_CHARS;
	public static final int NODEIDTYPE_MIP_CHARS1 = PUD_NODEIDTYPE_MIP_CHARS1;
	public static final int NODEIDTYPE_MIP_CHARS2 = PUD_NODEIDTYPE_MIP_CHARS2;
	public static final int NODEIDTYPE_192_BYTES = PUD_NODEIDTYPE_192_BYTES;
	public static final int NODEIDTYPE_193_BYTES = PUD_NODEIDTYPE_193_BYTES;
	public static final int NODEIDTYPE_194_BYTES = PUD_NODEIDTYPE_194_BYTES;
	public static final int NODEIDTYPE_IPV4_BYTES = PUD_NODEIDTYPE_IPV4_BYTES;
	public static final int NODEIDTYPE_IPV6_BYTES = PUD_NODEIDTYPE_IPV6_BYTES;
	public static final int TX_NODEIDTYPE_DIGITS = PUD_TX_NODEIDTYPE_DIGITS;
}
