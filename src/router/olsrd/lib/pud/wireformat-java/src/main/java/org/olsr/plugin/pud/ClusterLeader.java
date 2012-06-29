package org.olsr.plugin.pud;

import java.net.InetAddress;

/**
 * This class represents an uplink message, type cluster leader.
 */
public class ClusterLeader extends UplinkMessage {
	private static final long serialVersionUID = -1425806435563378359L;

	/**
	 * Default constructor
	 */
	public ClusterLeader() {
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
	public ClusterLeader(byte[] data, int dataSize) {
		super(data, dataSize);
	}

	/*
	 * UplinkClusterLeader
	 */

	/**
	 * @return the version of the cluster leader message
	 */
	public native int getClusterLeaderVersion();

	/**
	 * @return the validity time (in seconds) of the cluster leader message
	 */
	public native long getClusterLeaderValidityTime();

	/**
	 * @return the (OLSR main) IP address of the OLSR node that sent the cluster
	 *         leader message
	 */
	public native InetAddress getClusterLeaderOriginator();

	/**
	 * @return the (OLSR main) IP address of the OLSR node that is the cluster
	 *         leader for the OLSR node that sent the cluster leader message
	 */
	public native InetAddress getClusterLeaderClusterLeader();
}
