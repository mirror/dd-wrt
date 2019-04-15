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
	 * @return the validity time (in seconds) of the position update message
	 */
	public native long getPositionUpdateValidityTime();

	/**
	 * @return the presence field of the position update message
	 */
	public native long getPositionUpdatePresent();

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
	 * @return the speed (in kph) of the position update message
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
