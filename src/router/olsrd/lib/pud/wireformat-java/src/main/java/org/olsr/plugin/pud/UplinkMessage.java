package org.olsr.plugin.pud;

import java.io.Serializable;

/**
 * This class represents an uplink message.
 * 
 * It stores the received uplink message in a byte array and provides access
 * methods for all uplink header fields.
 */
public abstract class UplinkMessage implements Serializable {
	private static final long serialVersionUID = -1514460974030674657L;

	static {
		LibraryLoader.load();
	}

	/*
	 * Uplink Message Store
	 */

	/** the uplink message */
	protected byte[] data = null;

	/** the number of bytes in the received message */
	protected int dataSize = 0;

	/**
	 * Default constructor
	 */
	public UplinkMessage() {
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
	public UplinkMessage(byte[] data, int dataSize) {
		super();
		this.data = data;
		this.dataSize = dataSize;
	}

	/*
	 * Getters
	 */

	/**
	 * @return the received message
	 */
	public final byte[] getData() {
		return data;
	}

	/**
	 * @return the number of bytes in the received message
	 */
	public final int getDataSize() {
		return dataSize;
	}

	/*
	 * Uplink Message Types
	 */

	/**
	 * @return the uplink message type for a position update
	 */
	public static native int getUplinkMessageTypePosition();

	/**
	 * @return the uplink message type for a cluster leader message
	 */
	public static native int getUplinkMessageTypeClusterLeader();

	/*
	 * UplinkHeader
	 */

	/**
	 * @return the length of the uplink message header
	 */
	public static native int getUplinkMessageHeaderLength();

	/**
	 * @param data
	 *            the uplink message
	 * @param offset
	 *            the offset in the data where the uplink message begins
	 *
	 * @return the uplink message type
	 */
	public static int getUplinkMessageType(byte[] data, int offset) {
		return data[offset];
	}

	/**
	 * @param data
	 *            the uplink message
	 * @param offset
	 *            the offset in the data where the uplink message begins
	 *
	 * @return the number of bytes in the payload of the uplink message
	 */
	public static int getUplinkMessageLength(byte[] data, int offset) {
		return ((data[offset + 1] << 8) | data[offset + 2]);
	}

	/**
	 * @param data
	 *            the uplink message
	 * @param offset
	 *            the offset in the data where the uplink message begins
	 *
	 * @return true when the contained message is an IPv6 message, false
	 *         otherwise (IPv4)
	 */
	public static boolean isUplinkMessageIPv6(byte[] data, int offset) {
		return ((data[offset + 3] & 0x01) != 0);
	}

	/**
	 * @return the uplink message type
	 */
	public int getUplinkMessageType() {
		return UplinkMessage.getUplinkMessageType(data, 0);
	}

	/**
	 * @return the number of bytes in the payload of the uplink message
	 */
	public int getUplinkMessageLength() {
		return UplinkMessage.getUplinkMessageLength(data, 0);
	}

	/**
	 * @return true when the contained message is an IPv6 message, false
	 *         otherwise (IPv4)
	 */
	public boolean isUplinkMessageIPv6() {
		return UplinkMessage.isUplinkMessageIPv6(data, 0);
	}
}
