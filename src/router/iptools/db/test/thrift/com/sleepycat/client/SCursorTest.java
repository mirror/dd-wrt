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

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.notNullValue;
import static org.junit.Assert.assertThat;

public class SCursorTest extends ClientTestBase {

    private SEnvironment env;

    private SDatabase db;

    private SCursor cursor;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        db = env.openDatabase(null, "db", "sub-db",
                new SDatabaseConfig().setAllowCreate(true)
                        .setSortedDuplicates(true)
                        .setType(SDatabaseType.BTREE)
                        .setPriority(SCacheFilePriority.DEFAULT));
        cursor = initDb(db);
    }

    private SCursor initDb(SDatabase database) throws Exception {
        database.put(null, entry("key1"), entry("data1"));
        database.put(null, entry("key2"), entry("data2"));
        return database.openCursor(env.beginTransaction(null, null), null);
    }

    @Test
    public void testClose() throws Exception {
        cursor.close();
        assertClosed(cursor);
    }

    @Test
    public void testCompare() throws Exception {
        SCursor cursor2 = db.openCursor(null, null);
        cursor.getFirst(null, null, null);
        cursor2.getLast(null, null, null);
        assertThat(cursor.compare(cursor2), is(1));
    }

    @Test
    public void testCount() throws Exception {
        cursor.getFirst(null, null, null);
        assertThat(cursor.count(), is(1));
    }

    @Test
    public void testDup() throws Exception {
        assertThat(cursor.dup(false), notNullValue());
    }

    @Test
    public void testGetConfig() throws Exception {
        assertThat(cursor.getConfig(), notNullValue());
    }

    @Test
    public void testGetPriority() throws Exception {
        assertThat(cursor.getPriority(), is(SCacheFilePriority.DEFAULT));
    }

    @Test
    public void testSetPriority() throws Exception {
        cursor.setPriority(SCacheFilePriority.HIGH);
        assertThat(cursor.getPriority(), is(SCacheFilePriority.HIGH));
    }

    @Test
    public void testGetDatabase() throws Exception {
        assertThat(cursor.getDatabase(), is(db));
    }

    @Test
    public void testGetCurrent() throws Exception {
        cursor.getFirst(null, null, null);

        assertCursorGet(cursor::getCurrent,
                new SDatabaseEntry(), new SDatabaseEntry(), "key1", "data1");
    }

    @Test
    public void testGetFirst() throws Exception {
        assertCursorGet(cursor::getFirst,
                new SDatabaseEntry(), new SDatabaseEntry(), "key1", "data1");
    }

    @Test
    public void testGetLast() throws Exception {
        assertCursorGet(cursor::getLast,
                new SDatabaseEntry(), new SDatabaseEntry(), "key2", "data2");
    }

    @Test
    public void testGetNext() throws Exception {
        assertCursorGet(cursor::getNext,
                new SDatabaseEntry(), new SDatabaseEntry(), "key1", "data1");
    }

    @Test
    public void testGetNextDup() throws Exception {
        cursor.getFirst(null, null, null);

        assertThat(cursor.getNextDup(null, null, null),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetNextNoDup() throws Exception {
        assertCursorGet(cursor::getNextNoDup,
                new SDatabaseEntry(), new SDatabaseEntry(), "key1", "data1");
    }

    @Test
    public void testGetPrev() throws Exception {
        assertCursorGet(cursor::getPrev,
                new SDatabaseEntry(), new SDatabaseEntry(), "key2", "data2");
    }

    @Test
    public void testGetPrevDup() throws Exception {
        cursor.getLast(null, null, null);

        assertThat(cursor.getPrevDup(null, null, null),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetPrevNoDup() throws Exception {
        assertCursorGet(cursor::getPrevNoDup,
                new SDatabaseEntry(), new SDatabaseEntry(), "key2", "data2");
    }

    @Test
    public void testGetRecordNumber() throws Exception {
        SDatabase recNum = env.openDatabase(null, "recNum", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setBtreeRecordNumbers(true));
        SCursor c = initDb(recNum);

        SDatabaseEntry data = new SDatabaseEntry();
        c.getFirst(new SDatabaseEntry(), new SDatabaseEntry(), null);

        assertThat(c.getRecordNumber(data, null), is(SOperationStatus.SUCCESS));
        assertThat(data.getRecordNumber(connection.getServerByteOrder()),
                is(1));
    }

    @Test
    public void testGetSearchBoth() throws Exception {
        assertCursorGet(cursor::getSearchBoth,
                entry("key1"), entry("data1"), "key1", "data1");
    }

    @Test
    public void testGetSearchBothRange() throws Exception {
        assertCursorGet(cursor::getSearchBothRange,
                entry("key1"), entry("da"), "key1", "data1");
    }

    @Test
    public void testGetSearchKey() throws Exception {
        assertCursorGet(cursor::getSearchKey,
                entry("key1"), new SDatabaseEntry(), "key1", "data1");
    }

    @Test
    public void testGetSearchKeyRange() throws Exception {
        assertCursorGet(cursor::getSearchKeyRange,
                entry("key1"), new SDatabaseEntry(), "key1", "data1");
    }

    @Test
    public void testGetSearchRecordNumber() throws Exception {
        SDatabase recNum = env.openDatabase(null, "recNum", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setBtreeRecordNumbers(true));
        SCursor c = initDb(recNum);

        SDatabaseEntry key = new SDatabaseEntry();
        key.setRecordNumber(1, connection.getServerByteOrder());
        SDatabaseEntry data = new SDatabaseEntry();

        assertThat(c.getSearchRecordNumber(key, data, null),
                is(SOperationStatus.SUCCESS));
        assertThat(new String(data.getData()), is("data1"));
    }

    @Test
    public void testPut() throws Exception {
        assertThat(cursor.put(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutKeyFirst() throws Exception {
        assertThat(cursor.putKeyFirst(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutKeyLast() throws Exception {
        assertThat(cursor.putKeyLast(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutNoDupData() throws Exception {
        assertThat(cursor.putNoDupData(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutNoOverwrite() throws Exception {
        assertThat(cursor.putNoOverwrite(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutAfter() throws Exception {
        SDatabase dup = env.openDatabase(null, "dup", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setUnsortedDuplicates(true));
        SCursor c = initDb(dup);

        c.getFirst(null, null, null);
        assertThat(c.putAfter(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutBefore() throws Exception {
        SDatabase dup = env.openDatabase(null, "dup", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setUnsortedDuplicates(true));
        SCursor c = initDb(dup);

        c.getFirst(null, null, null);
        assertThat(c.putBefore(entry("key"), entry("data")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutCurrent() throws Exception {
        SDatabase dup = env.openDatabase(null, "dup", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setUnsortedDuplicates(true));
        SCursor c = initDb(dup);

        c.getFirst(null, null, null);
        assertThat(c.putCurrent(entry("data")), is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testDelete() throws Exception {
        cursor.getFirst(null, null, null);
        assertThat(cursor.delete(), is(SOperationStatus.SUCCESS));
    }
}