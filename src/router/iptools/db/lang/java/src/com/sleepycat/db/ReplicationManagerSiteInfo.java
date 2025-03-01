/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.db;

import com.sleepycat.db.internal.DbConstants;

/** 
A simple wrapper class to hold information needed to define a replication site.  
<p>
ReplicationManagerSiteInfo objects are returned by
{@link com.sleepycat.db.Environment#getReplicationManagerSiteList
Environment.getReplicationManagerSiteList}
*/
public class ReplicationManagerSiteInfo
{
    /** The replication site's address */
    public ReplicationHostAddress addr;
    /** The replication site's identifier */
    public int eid;
    private int flags;
    private int status;
            
    /** 
    Create a ReplicationManagerSiteInfo with the given information, isConnected defaults to false.
    @param hostAddr the host address
    @param eid the environment ID
    */
    public ReplicationManagerSiteInfo(ReplicationHostAddress hostAddr, int eid)
    {
	this(hostAddr, eid, false, false, false);
    }
	
    /** 
    Create a ReplicationManagerSiteInfo with the given information.
    @param hostAddr the host address
    @param eid the environment ID
    @param isConnected is connected
    */
    public ReplicationManagerSiteInfo(ReplicationHostAddress hostAddr, int eid, boolean isConnected)
    {
	this(hostAddr, eid, isConnected, false, false);
    }

    /** 
    Create a ReplicationManagerSiteInfo with the given information.
    @param hostAddr the host address
    @param eid the environment ID
    @param isConnected is connected
    @param isPeer is peer
    @param isView is view
    */
    public ReplicationManagerSiteInfo(ReplicationHostAddress hostAddr, int eid, boolean isConnected, boolean isPeer, boolean isView)
    {
	this.addr = hostAddr;
	this.eid = eid;
	this.status = isConnected ? DbConstants.DB_REPMGR_CONNECTED : 0;
	this.flags = isPeer ? DbConstants.DB_REPMGR_ISPEER : 0;
	this.flags |= isView ? DbConstants.DB_REPMGR_ISVIEW : 0;
    }

    /**
     * The status of the replication site's connection.
     * @return the status of the replication site's connection
     */
    public ReplicationManagerConnectionStatus getConnectionStatus() {
        return ReplicationManagerConnectionStatus.fromInt(this.status);
    }

    /**
     * The replication site is connected.
     *
     * @deprecated  Replaced by {@link #getConnectionStatus()}
     * @return if the replication site is connected
     */
    @Deprecated
    public boolean isConnected() {
        return (this.status == DbConstants.DB_REPMGR_CONNECTED);
    }

    /**
    The replication site is a peer.
    @return if the replication site is a peer
    */ 
    public boolean isPeer() {
	return ((this.flags & DbConstants.DB_REPMGR_ISPEER) != 0);
    }

    /**
    The replication site is a view.
    @return if the replication site is a view
    */
    public boolean isView() {
        return ((this.flags & DbConstants.DB_REPMGR_ISVIEW) != 0);
    }
    
}
