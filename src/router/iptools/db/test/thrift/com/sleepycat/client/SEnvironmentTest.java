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
import static org.hamcrest.CoreMatchers.notNullValue;
import static org.hamcrest.CoreMatchers.nullValue;
import static org.hamcrest.MatcherAssert.assertThat;

public class SEnvironmentTest extends ClientTestBase {

    private SEnvironment env;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
    }

    @Test
    public void testClose() throws Exception {
        env.close();
        assertClosed(env);
    }

    @Test
    public void testGetConfig() throws Exception {
        SEnvironmentConfig config = env.getConfig();
        assertThat(config.getLockDetectMode(), is(SLockDetectMode.RANDOM));
    }

    @Test
    public void testSetConfig() throws Exception {
        SEnvironmentConfig config = env.getConfig();
        boolean oldFlag = config.getMultiversion();
        env.setConfig(new SEnvironmentConfig().setMultiversion(!oldFlag));
        assertThat(env.getConfig().getMultiversion(), is(!oldFlag));
    }

    @Test
    public void testGetHome() throws Exception {
        assertThat(env.getHome(), is("env"));
    }

    @Test
    public void testOpenDatabase() throws Exception {
        env.openDatabase(null, "db", null,
                new SDatabaseConfig().setAllowCreate(true).setType(
                        SDatabaseType.BTREE));
        Assert.assertThat(
                Files.isRegularFile(testRoot.resolve("env").resolve("db")),
                is(true));
    }

    @Test
    public void testOpenSecondaryDatabase() throws Exception {
        SDatabase primary = env.openDatabase(null, "primary", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE));

        SSecondaryConfig config = new SSecondaryConfig().setKeyCreator(
                (sdb, key, data, result) -> {
                    result.setData(data.getData());
                    return true;
                });
        config.setAllowCreate(true).setType(SDatabaseType.BTREE);

        env.openSecondaryDatabase(null, "secondary", null, primary, config);

        Assert.assertThat(
                Files.isRegularFile(
                        testRoot.resolve("env").resolve("secondary")),
                is(true));
    }

    @Test
    public void testRemoveDatabase() throws Exception {
        env.openDatabase(null, "db", null,
                new SDatabaseConfig().setAllowCreate(true).setType(
                        SDatabaseType.BTREE));
        env.removeDatabase(null, "db", null, true);

        Assert.assertThat(
                Files.exists(testRoot.resolve("env").resolve("db")),
                is(false));
    }

    @Test
    public void testRenameDatabase() throws Exception {
        env.openDatabase(null, "db", null,
                new SDatabaseConfig().setAllowCreate(true).setType(
                        SDatabaseType.BTREE));
        env.renameDatabase(null, "db", null, "new-db", true);

        Assert.assertThat(
                Files.exists(testRoot.resolve("env").resolve("db")),
                is(false));
        Assert.assertThat(
                Files.isRegularFile(testRoot.resolve("env").resolve("new-db")),
                is(true));
    }

    @Test
    public void testBeginTransaction() throws Exception {
        STransaction txn = env.beginTransaction(null, null);
        txn.abort();
    }

    @Test
    public void testCheckpoint() throws Exception {
        env.checkpoint(null);
    }

    @Test
    public void testGetCacheFileStats() throws Exception {
        env.getCacheFileStats(null);
    }

    @Test
    public void testGetCacheStats() throws Exception {
        assertThat(env.getCacheStats(null), notNullValue());
    }

    @Test
    public void testGetLockStats() throws Exception {
        assertThat(env.getLockStats(null), notNullValue());
    }

    @Test
    public void testGetLogStats() throws Exception {
        assertThat(env.getLogStats(null), notNullValue());
    }

    @Test
    public void testGetMutexStats() throws Exception {
        assertThat(env.getMutexStats(null), notNullValue());
    }

    @Test
    public void testGetTransactionStats() throws Exception {
        STransactionStats stats = env.getTransactionStats(null);
        assertThat(stats, notNullValue());
        assertThat(stats.getTxnarray().length, is(0));
    }

    @Test
    public void testGetMultipleStats() throws Exception {
        SMultiStats stats = env.getMultipleStats(
                new SMultiStatsConfig().setCacheConfig(true, null)
                        .setLogConfig(true, null));
        assertThat(stats.getCacheStats(), notNullValue());
        assertThat(stats.getLogStats(), notNullValue());
        assertThat(stats.getLockStats(), nullValue());
        assertThat(stats.getMutexStats(), nullValue());
    }
}