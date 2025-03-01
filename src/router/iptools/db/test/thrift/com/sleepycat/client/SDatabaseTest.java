/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.junit.Before;
import org.junit.Test;

import static org.hamcrest.CoreMatchers.hasItem;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.notNullValue;
import static org.hamcrest.MatcherAssert.assertThat;

public class SDatabaseTest extends ClientTestBase {

    private SEnvironment env;

    private SDatabase db;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        db = env.openDatabase(null, "db", "sub-db",
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setPriority(SCacheFilePriority.DEFAULT)
                        .setBtreeRecordNumbers(true));
        db.put(null, entry("key"), entry("data"));
    }

    @Test
    public void testClose() throws Exception {
        db.close();
        assertClosed(db);
    }

    @Test
    public void testGetConfig() throws Exception {
        SDatabaseConfig config = db.getConfig();
        assertThat(config.getSortedDuplicates(), is(false));
    }

    @Test
    public void testSetConfig() throws Exception {
        db.setConfig(new SDatabaseConfig().setPriority(SCacheFilePriority.LOW));
        assertThat(db.getConfig().getPriority(), is(SCacheFilePriority.LOW));
    }

    @Test
    public void testGetDatabaseFile() throws Exception {
        assertThat(db.getDatabaseFile(), is("db"));
    }

    @Test
    public void testGetDatabaseName() throws Exception {
        assertThat(db.getDatabaseName(), is("sub-db"));
    }

    @Test
    public void testGetEnvironment() throws Exception {
        assertThat(db.getEnvironment(), is(env));
    }

    @Test
    public void testGetSecondaryDatabases() throws Exception {
        assertThat(db.getSecondaryDatabases().size(), is(0));
    }

    @Test
    public void testGetSecondaryDatabasesWithSecDb() throws Exception {
        SSecondaryConfig config = new SSecondaryConfig().setKeyCreator(
                (sdb, key, data, result) -> {
                    result.setData(data.getData());
                    return true;
                });
        config.setAllowCreate(true).setType(SDatabaseType.BTREE);

        SSecondaryDatabase sdb =
                env.openSecondaryDatabase(null, "secondary", null, db, config);

        assertThat(db.getSecondaryDatabases().size(), is(1));
        assertThat(db.getSecondaryDatabases(), hasItem(sdb));

        sdb.close();

        assertThat(db.getSecondaryDatabases().size(), is(0));
    }

    @Test
    public void testGet() throws Exception {
        assertDbGet(db::get, entry("key"), new SDatabaseEntry(), "key", "data");

        assertThat(db.get(null, entry("bad"), null, null),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetSearchBoth() throws Exception {
        assertDbGet(db::getSearchBoth, entry("key"), entry("data"),
                "key", "data");
    }

    @Test
    public void testGetSearchRecordNumber() throws Exception {
        assertDbGet(db::getSearchRecordNumber,
                new SDatabaseEntry()
                        .setRecordNumber(1, connection.getServerByteOrder()),
                new SDatabaseEntry(),
                "key", "data");
    }

    @Test
    public void testExists() throws Exception {
        assertThat(db.exists(null, entry("bad")),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetKeyRange() throws Exception {
        assertThat(db.getKeyRange(null, entry("key")), notNullValue());
    }

    @Test
    public void testPut() throws Exception {
        assertThat(db.put(null, entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutNoDupData() throws Exception {
        SDatabase sorted = env.openDatabase(null, "sorted", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setSortedDuplicates(true));

        assertThat(sorted.putNoDupData(null, entry("key1"), entry("data1")),
                is(SOperationStatus.SUCCESS));

        sorted.close();
    }

    @Test
    public void testPutNoOverwrite() throws Exception {
        assertThat(db.putNoOverwrite(null, entry("key1"), entry("data1")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutMultipleKey() throws Exception {
        SMultipleKeyDataEntry pairs = new SMultipleKeyDataEntry();
        pairs.append("key1".getBytes(), "data1".getBytes());
        pairs.append("key2".getBytes(), "data2".getBytes());

        assertThat(db.putMultipleKey(null, pairs, true),
                is(SOperationStatus.SUCCESS));

        assertThat(db.exists(null, entry("key1")),
                is(SOperationStatus.SUCCESS));
        assertThat(db.exists(null, entry("key2")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testDelete() throws Exception {
        assertThat(db.exists(null, entry("key")), is(SOperationStatus.SUCCESS));

        db.delete(null, entry("key"));

        assertThat(db.exists(null, entry("key")),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testDeleteMultiple() throws Exception {
        SMultipleKeyDataEntry pairs = new SMultipleKeyDataEntry();
        pairs.append("key1".getBytes(), "data1".getBytes());
        pairs.append("key2".getBytes(), "data2".getBytes());

        db.putMultipleKey(null, pairs, true);

        SMultipleDataEntry keys = new SMultipleDataEntry();
        keys.append("key1".getBytes());
        keys.append("key2".getBytes());

        db.deleteMultiple(null, keys);

        assertThat(db.exists(null, entry("key1")),
                is(SOperationStatus.NOTFOUND));
        assertThat(db.exists(null, entry("key2")),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testDeleteMultipleKey() throws Exception {
        SMultipleKeyDataEntry pairs = new SMultipleKeyDataEntry();
        pairs.append("key1".getBytes(), "data1".getBytes());
        pairs.append("key2".getBytes(), "data2".getBytes());

        db.putMultipleKey(null, pairs, true);

        db.deleteMultipleKey(null, pairs);

        assertThat(db.exists(null, entry("key1")),
                is(SOperationStatus.NOTFOUND));
        assertThat(db.exists(null, entry("key2")),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testOpenCursor() throws Exception {
        SCursor cursor = db.openCursor(null, null);
        cursor.close();
    }

    @Test
    public void testOpenSequence() throws Exception {
        SSequence seq =
                db.openSequence(null, entry("seq"),
                        new SSequenceConfig().setAllowCreate(true));
        seq.close();
    }

    @Test
    public void testRemoveSequence() throws Exception {
        SSequence seq =
                db.openSequence(null, entry("seq"),
                        new SSequenceConfig().setAllowCreate(true));
        seq.close();
        db.removeSequence(null, entry("seq"), false, true);
    }

    @Test
    public void testCompact() throws Exception {
        assertThat(db.compact(null, entry("start"), entry("stop"),
                new SDatabaseEntry(), null), notNullValue());
    }

    @Test
    public void testTruncate() throws Exception {
        assertThat(db.truncate(null, false), is(-1));
    }

    @Test
    public void testGetStats() throws Exception {
        assertThat(db.getStats(null, null), notNullValue());
    }
}