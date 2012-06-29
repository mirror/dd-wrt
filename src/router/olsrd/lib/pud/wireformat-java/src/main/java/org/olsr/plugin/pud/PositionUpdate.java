package org.olsr.plugin.pud;

import java.net.InetAddress;
import java.util.Date;

/**
 * This class represents an uplink message, type position update.
 */
public class PositionUpdate extends UplinkMessage {
	private static final long serialVersionUID = -7253852907534653248L;

	/**
	 * Default constructor
	 */
	public PositionUpdate() {
		super();
	}

	/**
	 * Constructor
	 * 
	 * @param data
	 *            the received message
	 * @param dataSize
	 *            the number of bytes in the received message
	 */
	public PositionUpdate(byte[] data, int dataSize) {
		super(data, dataSize);
	}

	/*
	 * OLSR header
	 */

	/**
	 * @return the (OLSR main) IP address of the OLSR node that sent the uplink
	 *         message
	 */
	public native InetAddress getOlsrMessageOriginator();

	/*
	 * PudOlsrPositionUpdate
	 */

	/**
	 * @return the version of the position update message
	 */
	public native int getPositionUpdateVersion();

	/**
	 * @return the validity time (in seconds) the position update message
	 */
	public native long getPositionUpdateValidityTime();

	/**
	 * @return the content mask the position update message
	 */
	public native int getPositionUpdateSMask();

	/**
	 * @return the content flags the position update message
	 */
	public native int getPositionUpdateFlags();

	/*
	 * GpsInfo
	 */

	/**
	 * @param baseDate
	 *            the base date relative to which the time of the position
	 *            update message must be determined (milliseconds since Epoch)
	 * @param timezoneOffset
	 *            the offset (in milliseconds) of the local timezone to UTC. A
	 *            positive value means 'west of UTC', a negative value means
	 *            'east of UTC'. For CET, this value is -3600000.
	 * @return the time of the position update message (milliseconds since
	 *         Epoch, UTC)
	 */
	public native long getPositionUpdateTime(long baseDate, long timezoneOffset);

	/**
	 * @return the latitude (in degrees) of the position update message
	 */
	public native double getPositionUpdateLatitude();

	/**
	 * @return the longitude (in degrees) of the position update message
	 */
	public native double getPositionUpdateLongitude();

	/**
	 * @return the altitude (in meters) of the position update message
	 */
	public native long getPositionUpdateAltitude();

	/**
	 * @return the speed (in kph) of the the position update message
	 */
	public native long getPositionUpdateSpeed();

	/**
	 * @return the track angle (in degrees) of the position update message
	 */
	public native long getPositionUpdateTrack();

	/**
	 * @return the HDOP of the position update message
	 */
	public native double getPositionUpdateHdop();

	/*
	 * NodeInfo
	 */

	/**
	 * @return the nodeIdType of the position update message
	 */
	public native int getPositionUpdateNodeIdType();

	/**
	 * @return the nodeId of the position update message
	 */
	public native String getPositionUpdateNodeId();
}
