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
