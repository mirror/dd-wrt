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
import static org.hamcrest.MatcherAssert.assertThat;

public class SSecondaryCursorTest extends ClientTestBase {

    private SSecondaryCursor cursor;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        SDatabase primary = env.openDatabase(null, "primary", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE)
                        .setBtreeRecordNumbers(true));

        SSecondaryConfig config = new SSecondaryConfig();
        config.setAllowCreate(true).setType(SDatabaseType.BTREE)
                .setBtreeRecordNumbers(true);
        config.setKeyCreator((sdb, key, data, result) -> {
            result.setData(new String(data.getData()).split(" ")[0].getBytes());
            return true;
        });
        SSecondaryDatabase secondary =
                env.openSecondaryDatabase(null, "secondary", null, primary,
                        config);
        primary.put(null, entry("pKey"), entry("sKey data"));
        cursor = secondary.openCursor(env.beginTransaction(null, null), null);
    }

    @Test
    public void testGetCurrent() throws Exception {
        cursor.getFirst(null, null, null, null);
        assertCursorPGet(cursor::getCurrent, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetFirst() throws Exception {
        assertCursorPGet(cursor::getFirst, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetLast() throws Exception {
        assertCursorPGet(cursor::getLast, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetNext() throws Exception {
        assertCursorPGet(cursor::getNext, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetNextDup() throws Exception {
        cursor.getFirst(null, null, null, null);

        assertThat(cursor.getNextDup(null, null, null, null),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetNextNoDup() throws Exception {
        assertCursorPGet(cursor::getNextNoDup, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetPrev() throws Exception {
        assertCursorPGet(cursor::getPrev, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetPrevDup() throws Exception {
        cursor.getFirst(null, null, null, null);

        assertThat(cursor.getPrevDup(null, null, null, null),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetPrevNoDup() throws Exception {
        assertCursorPGet(cursor::getPrevNoDup, new SDatabaseEntry(),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetRecordNumber() throws Exception {
        SDatabaseEntry sKey = new SDatabaseEntry();
        SDatabaseEntry pKey = new SDatabaseEntry();
        cursor.getFirst(null, null, null, null);
        assertThat(cursor.getRecordNumber(sKey, pKey, null),
                is(SOperationStatus.SUCCESS));
        assertThat(sKey.getRecordNumber(connection.getServerByteOrder()),
                is(1));
        assertThat(pKey.getRecordNumber(connection.getServerByteOrder()),
                is(1));
    }

    @Test
    public void testGetSearchBoth() throws Exception {
        assertCursorPGet(cursor::getSearchBoth, entry("sKey"), entry("pKey"),
                new SDatabaseEntry(), "sKey", "pKey", "sKey data");
    }

    @Test
    public void testGetSearchBothRange() throws Exception {
        SDatabaseEntry sKey = entry("sKey");
        SDatabaseEntry pKey = entry("p");
        SDatabaseEntry data = new SDatabaseEntry();
        assertThat(cursor.getSearchBothRange(sKey, pKey, data, null),
                is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testGetSearchKey() throws Exception {
        assertCursorPGet(cursor::getSearchKey, entry("sKey"), entry("pKey"),
                new SDatabaseEntry(), "sKey", "pKey", "sKey data");
    }

    @Test
    public void testGetSearchKeyRange() throws Exception {
        assertCursorPGet(cursor::getSearchKeyRange, entry("s"),
                new SDatabaseEntry(), new SDatabaseEntry(), "sKey", "pKey",
                "sKey data");
    }

    @Test
    public void testGetSearchRecordNumber() throws Exception {
        assertCursorPGet(cursor::getSearchRecordNumber,
                new SDatabaseEntry()
                        .setRecordNumber(1, connection.getServerByteOrder()),
                new SDatabaseEntry(),
                new SDatabaseEntry(), "sKey", "pKey", "sKey data");
    }

    private void assertCursorPGet(CursorPGetFunc func,
            SDatabaseEntry sKey, SDatabaseEntry pKey, SDatabaseEntry pData,
            String expectedSKey, String expectedPKey, String expectedPData)
            throws Exception {
        assertThat(func.apply(sKey, pKey, pData, null),
                is(SOperationStatus.SUCCESS));
        assertThat(new String(sKey.getData()), is(expectedSKey));
        assertThat(new String(pKey.getData()), is(expectedPKey));
        assertThat(new String(pData.getData()), is(expectedPData));
    }

    private interface CursorPGetFunc {
        SOperationStatus apply(SDatabaseEntry sKey, SDatabaseEntry pKey,
                SDatabaseEntry pData, SLockMode mode);
    }
}