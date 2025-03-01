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
import static org.junit.Assert.*;

public class STransactionTest extends ClientTestBase {

    private SDatabase db;

    private STransaction txn;

    private SDatabaseEntry key;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        db = env.openDatabase(null, "db", "sub-db",
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE));
        txn = env.beginTransaction(null, null);
        key = entry("key");
        db.put(txn, key, entry("data"));
    }

    @Test
    public void testAbort() throws Exception {
        txn.abort();
        assertThat(db.exists(null, key), is(SOperationStatus.NOTFOUND));
    }

    @Test
    public void testCommit() throws Exception {
        txn.commit();
        assertThat(db.exists(null, key), is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testCommitNoSync() throws Exception {
        txn.commitNoSync();
        assertThat(db.exists(null, key), is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testCommitSync() throws Exception {
        txn.commitSync();
        assertThat(db.exists(null, key), is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testCommitWriteNoSync() throws Exception {
        txn.commitWriteNoSync();
        assertThat(db.exists(null, key), is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testGetPriority() throws Exception {
        assertThat(txn.getPriority(), is(100));
    }

    @Test
    public void testSetPriority() throws Exception {
        txn.setPriority(1000);
    }
}