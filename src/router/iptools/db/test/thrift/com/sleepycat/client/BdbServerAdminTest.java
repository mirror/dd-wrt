/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.nio.file.Files;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class BdbServerAdminTest extends ClientTestBase {

    private BdbServerAdmin admin;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        admin = connection.adminService();
    }

    @Test
    public void testPing() throws Exception {
        admin.ping();
    }

    @Test
    public void testGetServerBdbVersion() throws Exception {
        assertThat(admin.getServerBdbVersion().isEmpty(), is(false));
    }

    @Test
    public void testShutdownServer() throws Exception {
        admin.shutdownServer();
    }

    @Test
    public void testCloseEnvironmentHandles() throws Exception {
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        admin.closeEnvironmentHandles("env", 0);

        assertClosed(env);
    }

    @Test
    public void testDeleteEnvironmentAndDatabases() throws Exception {
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        env.openDatabase(null, "db", null,
                new SDatabaseConfig().setAllowCreate(true).setType(
                        SDatabaseType.BTREE));

        admin.deleteEnvironmentAndDatabases("env", true);

        Assert.assertThat(Files.isDirectory(testRoot.resolve("env")),
                is(false));
    }

    @Test
    public void testDeleteEnvironmentAndDatabasesBusy() throws Exception {
        try (SEnvironment ignored = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true))) {

            thrown.expect(SResourceInUseException.class);
            admin.deleteEnvironmentAndDatabases("env", false);
        }
    }

    @Test
    public void testCloseDatabaseHandles() throws Exception {
        try (SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true))) {
            SDatabase db = env.openDatabase(null, "db", null,
                    new SDatabaseConfig().setAllowCreate(true)
                            .setType(SDatabaseType.BTREE));

            admin.closeDatabaseHandles("env", "db", null, 0);

            assertClosed(db);
        }
    }
}