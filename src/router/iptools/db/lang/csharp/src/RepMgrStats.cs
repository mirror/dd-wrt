/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */
using System;
using System.Collections.Generic;
using System.Text;

namespace BerkeleyDB {
    /// <summary>
    /// Statistical information about the Replication Manager
    /// </summary>
    public class RepMgrStats {
        private Internal.RepMgrStatStruct st;
        internal RepMgrStats(Internal.RepMgrStatStruct stats) {
            st = stats;
        }
        /// <summary>
        /// Number of automatic replication process takeovers.
        /// </summary>
        public ulong AutoTakeovers { get { return st.st_takeovers; } }
        /// <summary>
        /// Existing connections dropped. 
        /// </summary>
        public ulong DroppedConnections { get { return st.st_connection_drop; } }
        /// <summary>
        /// Number of messages discarded due to excessive queue length.
        /// </summary>
        public ulong DroppedMessages { get { return st.st_msgs_dropped; } }
        /// <summary>
        /// Failed new connection attempts. 
        /// </summary>
        public ulong FailedConnections { get { return st.st_connect_fail; } }
        /// <summary>
        /// Number of insufficiently acknowledged messages. 
        /// </summary>
        public ulong FailedMessages { get { return st.st_perm_failed; } }
        /// <summary>
        /// Number of messages queued for network delay. 
        /// </summary>
        public ulong QueuedMessages { get { return st.st_msgs_queued; } }
        /// <summary>
        /// Incoming queue size: Gigabytes.
        /// </summary>
        public ulong IncomingQueueGBytes { get { return st.st_incoming_queue_gbytes; } }
        /// <summary>
        /// Incoming queue size: Gytes.
        /// </summary>
        public ulong IncomingQueueBytes { get { return st.st_incoming_queue_bytes; } }
        /// <summary>
        /// Number of msgs discarded due to incoming queue full.
        /// </summary>
        public ulong IncomingDroppedMessages { get { return st.st_incoming_msgs_dropped; } }
        /// <summary>
        /// Number of currently active election threads
        /// </summary>
        public uint ElectionThreads { get { return st.st_elect_threads; } }
        /// <summary>
        /// Election threads for which space is reserved
        /// </summary>
        public uint MaxElectionThreads { get { return st.st_max_elect_threads; } }
        /// <summary>
        /// Number of replication group participant sites.
        /// </summary>
        public uint ParticipantSites { get { return st.st_site_participants; } }
        /// <summary>
        /// Total number of replication group sites.
        /// </summary>
        public uint TotalSites { get { return st.st_site_total; } }
        /// <summary>
        /// Number of replication group view sites.
        /// </summary>
        public uint ViewSites { get { return st.st_site_views; } }
        /// <summary>
        /// Total number of outgoing write operations forwarded by this client.
        /// </summary>
        public ulong WriteOpsForwarded { get { return st.st_write_ops_forwarded; } }
        /// <summary>
        /// Total number of incoming forwarded write operations received by this master.
        /// </summary>
        public ulong WriteOpsReceived { get { return st.st_write_ops_received; } }
    }
}
