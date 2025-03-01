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

import java.nio.ByteOrder;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.notNullValue;
import static org.hamcrest.MatcherAssert.assertThat;

public class SSecondaryDatabaseTest extends ClientTestBase {

    private SDatabase primary;

    private SSecondaryDatabase secondary;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        primary = env.openDatabase(null, "primary", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE));

        SSecondaryConfig config = new SSecondaryConfig();
        config.setAllowCreate(true).setType(SDatabaseType.BTREE)
                .setBtreeRecordNumbers(true);
        config.setKeyCreator((sdb, key, data, result) -> {
            result.setData(new String(data.getData()).split(" ")[0].getBytes());
            return true;
        });
        secondary = env.openSecondaryDatabase(null, "secondary", null, primary,
                config);
        primary.put(null, entry("pKey"), entry("sKey data"));
    }

    @Test
    public void testClose() throws Exception {
        secondary.close();
        assertClosed(secondary);
    }

    @Test
    public void testGetPrimaryDatabase() throws Exception {
        assertThat(secondary.getPrimaryDatabase(), is(primary));
    }

    @Test
    public void testGet() throws Exception {
        assertDbPGet(secondary::get, entry("sKey"), new SDatabaseEntry(),
                new SDatabaseEntry(), "sKey", "pKey", "sKey data");
    }

    @Test
    public void testGetSearchBoth() throws Exception {
        assertDbPGet(secondary::getSearchBoth, entry("sKey"), entry("pKey"),
                new SDatabaseEntry(), "sKey", "pKey", "sKey data");
    }

    @Test
    public void testGetSearchRecordNumber() throws Exception {
        assertDbPGet(secondary::getSearchRecordNumber,
                new SDatabaseEntry()
                        .setRecordNumber(1, ByteOrder.nativeOrder()),
                new SDatabaseEntry(),
                new SDatabaseEntry(), "sKey", "pKey", "sKey data");
    }

    @Test
    public void testOpenCursor() throws Exception {
        assertThat(secondary.openCursor(null, null), notNullValue());
    }

    @Test
    public void testGet1() throws Exception {
        assertDbGet(secondary::get, entry("sKey"), new SDatabaseEntry(), "sKey",
                "sKey data");
    }

    @Test(expected = IllegalArgumentException.class)
    public void testGetSearchBoth1() throws Exception {
        SDatabaseEntry sKey = entry("sKey");
        SDatabaseEntry data = entry("sKey data");
        secondary.getSearchBoth(null, sKey, data, null);
    }

    @Test
    public void testGetSearchRecordNumber1() throws Exception {
        assertDbGet(secondary::getSearchRecordNumber,
                new SDatabaseEntry()
                        .setRecordNumber(1, connection.getServerByteOrder()),
                new SDatabaseEntry(),
                "sKey", "sKey data");
    }

    private void assertDbPGet(DbPGetFunc func,
            SDatabaseEntry sKey, SDatabaseEntry pKey, SDatabaseEntry pData,
            String expectedSKey, String expectedPKey, String expectedPData)
            throws Exception {
        assertThat(func.apply(null, sKey, pKey, pData, null),
                is(SOperationStatus.SUCCESS));
        assertThat(new String(sKey.getData()), is(expectedSKey));
        assertThat(new String(pKey.getData()), is(expectedPKey));
        assertThat(new String(pData.getData()), is(expectedPData));
    }

    @FunctionalInterface
    private interface DbPGetFunc {
        SOperationStatus apply(STransaction txn, SDatabaseEntry sKey,
                SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode mode);
    }
}