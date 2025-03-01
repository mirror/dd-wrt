/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.server.callbacks.ServerKeyCreator;
import com.sleepycat.server.config.BdbServiceConfig;
import com.sleepycat.server.util.FileUtils;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TDatabaseType;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TFileNotFoundException;
import com.sleepycat.thrift.TResourceInUseException;
import com.sleepycat.thrift.TSecondaryDatabaseConfig;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Properties;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.junit.Assert.fail;

public class DeleteResourceTest extends BdbServiceHandlerTestBase {

    private static final String ENV_HOME = "test_remove";

    private static final String DB_FILE = "test.db";

    private Path dataRoot;

    private TEnvironment env;

    private TDatabase db;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        dataRoot = Files.createTempDirectory("DataRoot");

        Properties properties = new Properties();
        properties.setProperty(BdbServiceConfig.ROOT_HOME,
                testRoot.toAbsolutePath().toString());
        properties.setProperty(BdbServiceConfig.ROOT_DATA,
                dataRoot.toAbsolutePath().toString());

        BdbServiceConfig config = new BdbServiceConfig(properties);
        config.initRootDirs();

        handler.setServerAndConfig(new MockServer(), config);

        env = handler.openEnvironment(ENV_HOME,
                new TEnvironmentConfig().setAllowCreate(true));

        db = handler.openDatabase(env, null, DB_FILE, null,
                new TDatabaseConfig().setAllowCreate(true)
                        .setType(TDatabaseType.BTREE));
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        FileUtils.deleteFileTree(dataRoot.toFile());
    }

    @Test
    public void testDeleteEnvironmentAndDatabases() throws Exception {
        Path envHome = testRoot.resolve(ENV_HOME);
        Path dataHome = dataRoot.resolve(ENV_HOME);
        Path dbFile = dataHome.resolve(DB_FILE);

        assertThat(Files.exists(envHome), is(true));
        assertThat(Files.exists(dbFile), is(true));

        handler.deleteEnvironmentAndDatabases(ENV_HOME, true);

        assertThat(Files.notExists(envHome), is(true));
        assertThat(Files.notExists(dataHome), is(true));
    }

    @Test(expected = TResourceInUseException.class)
    public void testDeleteEnvironmentAndDatabasesInUse() throws Exception {
        handler.deleteEnvironmentAndDatabases(ENV_HOME, false);
    }

    @Test
    public void testRemoveDatabase() throws Exception {
        Path dbFile = dataRoot.resolve(ENV_HOME).resolve(DB_FILE);

        assertThat(Files.exists(dbFile), is(true));

        handler.removeDatabase(env, null, DB_FILE, null, true);

        assertThat(Files.notExists(dbFile), is(true));
    }

    @Test(expected = TResourceInUseException.class)
    public void testRemoveDatabaseInUse() throws Exception {
        handler.removeDatabase(env, null, DB_FILE, null, false);
    }

    @Test(expected = TResourceInUseException.class)
    public void testRemoveDatabaseFileInUse() throws Exception {
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        TDatabase sub1 =
                handler.openDatabase(env, null, "multi.db", "sub1", config);
        handler.openDatabase(env, null, "multi.db", "sub2", config);

        handler.closeDatabaseHandle(sub1);
        handler.removeDatabase(env, null, "multi.db", null, false);
    }

    @Test
    public void testRemoveDatabaseSub() throws Exception {
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        TDatabase sub1 =
                handler.openDatabase(env, null, "multi.db", "sub1", config);
        handler.openDatabase(env, null, "multi.db", "sub2", config);

        handler.closeDatabaseHandle(sub1);
        handler.removeDatabase(env, null, "multi.db", "sub1", false);
    }

    @Test(expected = TResourceInUseException.class)
    public void testRemoveDatabaseSubInUse() throws Exception {
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        handler.openDatabase(env, null, "multi.db", "sub1", config);
        handler.openDatabase(env, null, "multi.db", "sub2", config);

        handler.removeDatabase(env, null, "multi.db", "sub1", false);
    }

    @Test
    public void testRemoveSecondaryDb() throws Exception {
        String sdbFileName = "second";
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        handler.openSecondaryDatabase(env, null, sdbFileName, null, db,
                new TSecondaryDatabaseConfig().setDbConfig(config));

        Path envDataHome = dataRoot.resolve(ENV_HOME);
        Path auxFile = envDataHome.resolve(
                ServerKeyCreator.getAuxiliaryName(sdbFileName));

        assertThat(Files.exists(auxFile), is(true));

        handler.removeDatabase(env, null, sdbFileName, null, true);

        assertThat(Files.exists(auxFile), is(false));
    }

    @Test
    public void testRemoveInMemoryDb() throws Exception {
        TDatabase inMemory = handler.openDatabase(env, null, null, "in_mem",
                new TDatabaseConfig().setAllowCreate(true)
                        .setType(TDatabaseType.BTREE));
        handler.closeDatabaseHandle(inMemory);

        handler.removeDatabase(env, null, null, "in_mem", true);

        try {
            handler.openDatabase(env, null, null, "in_mem",
                    new TDatabaseConfig());
            fail();
        } catch (TFileNotFoundException e) {
            // expected
        }
    }

    @Test
    public void testRemoveInMemorySecondaryDb() throws Exception {
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        TDatabase inMemSec =
                handler.openSecondaryDatabase(env, null, null, "in_mem", db,
                        new TSecondaryDatabaseConfig().setDbConfig(config));
        handler.closeDatabaseHandle(inMemSec);

        handler.removeDatabase(env, null, null, "in_mem", true);

        try {
            handler.openDatabase(env, null, null,
                    ServerKeyCreator.getAuxiliaryName("in_mem"),
                    new TDatabaseConfig());
            fail();
        } catch (TFileNotFoundException e) {
            // expected
        }
    }
}