/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.integration;

import com.sleepycat.client.BdbServerConnection;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseType;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.SslConfig;

public class SslClient {
    public static void main(String[] args) throws Exception {
        BdbServerConnection conn =
                BdbServerConnection.connectSsl("localhost", 8080,
                        new SslConfig().setTrustStore(
                                "/Users/ynli/tmp/bdb_server/truststore.jks",
                                "sleepycat"), 10000);

        SEnvironment env = conn.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        SDatabase db = env.openDatabase(null, "db", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE));

        db.put(null, new SDatabaseEntry("key".getBytes()),
                new SDatabaseEntry("data".getBytes()));

        db.close();
        env.close();

        conn.adminService().shutdownServer();
        conn.close();
    }
}
