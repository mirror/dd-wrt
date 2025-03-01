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

public class SSequenceTest extends ClientTestBase {

    private SDatabase db;

    private SSequence seq;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        db = env.openDatabase(null, "db", "sub-db",
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(SDatabaseType.BTREE));
        seq = db.openSequence(null, entry("seq"),
                new SSequenceConfig().setAllowCreate(true).setInitialValue(10)
                        .setRange(10, 100));
    }

    @Test
    public void testGetDatabase() throws Exception {
        assertThat(seq.getDatabase(), is(db));
    }

    @Test
    public void testGetKey() throws Exception {
        assertThat(new String(seq.getKey().getData()), is("seq"));
    }

    @Test
    public void testClose() throws Exception {
        seq.close();
        assertClosed(seq);
    }

    @Test
    public void testGet() throws Exception {
        assertThat(seq.get(null, 3), is(10L));
        assertThat(seq.get(null, 3), is(13L));
    }
}