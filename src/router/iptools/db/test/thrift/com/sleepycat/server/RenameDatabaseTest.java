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
import com.sleepycat.thrift.TSecondaryDatabaseConfig;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Properties;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class RenameDatabaseTest extends BdbServiceHandlerTestBase {
    private static final String ENV_HOME = "test_rename";

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
    public void testRenameSecondaryDb() throws Exception {
        String sdbFileName = "second";
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        handler.openSecondaryDatabase(env, null, sdbFileName, null, db,
                new TSecondaryDatabaseConfig().setDbConfig(config));

        Path envDataHome = dataRoot.resolve(ENV_HOME);


        Path auxFile = envDataHome.resolve(
                ServerKeyCreator.getAuxiliaryName(sdbFileName));

        assertThat(Files.exists(auxFile), is(true));

        handler.renameDatabase(env, null, sdbFileName, null, "newName", true);

        assertThat(Files.exists(auxFile), is(false));

        auxFile = envDataHome
                .resolve(ServerKeyCreator.getAuxiliaryName("newName"));
        assertThat(Files.exists(auxFile), is(true));
    }

    @Test
    public void testRenameInMemoryDatabase() throws Exception {
        TDatabase inMemory = handler.openDatabase(env, null, null, "in_mem",
                new TDatabaseConfig().setAllowCreate(true)
                        .setType(TDatabaseType.BTREE));
        handler.closeDatabaseHandle(inMemory);

        handler.renameDatabase(env, null, null, "in_mem", "new_in_mem", true);

        handler.openDatabase(env, null, null, "new_in_mem",
                new TDatabaseConfig());
    }

    @Test
    public void testRenameSecondaryInMemoryDatabase() throws Exception {
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        TDatabase inMemSec =
                handler.openSecondaryDatabase(env, null, null, "in_mem", db,
                        new TSecondaryDatabaseConfig().setDbConfig(config));
        handler.closeDatabaseHandle(inMemSec);

        TDatabase inMemAux = handler.openDatabase(env, null, null,
                ServerKeyCreator.getAuxiliaryName("in_mem"),
                new TDatabaseConfig());
        handler.closeDatabaseHandle(inMemAux);

        handler.renameDatabase(env, null, null, "in_mem", "new_in_mem", true);

        handler.openDatabase(env, null, null,
                ServerKeyCreator.getAuxiliaryName("new_in_mem"),
                new TDatabaseConfig());
    }
}
