/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.junit.Before;
import org.junit.Test;

import java.util.Arrays;

import static com.sleepycat.client.SDatabaseType.BTREE;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class PopulateSecondaryTest extends ClientTestBase {

    private SEnvironment env;

    private SDatabase primary;

    @Before
    public void setUp() throws Exception {
        super.setUp();
        env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        primary = env.openDatabase(null, "primary", null,
                new SDatabaseConfig().setAllowCreate(true).setType(BTREE));

        populateData();
    }

    @Test
    public void testCreateEmpty() throws Exception {
        SSecondaryDatabase sdb = openSecondaryDatabase(new SSecondaryConfig());
        assertThat(sdb.isEmpty(null), is(true));
    }

    @Test
    public void testPopulate() throws Exception {
        SSecondaryDatabase sdb = openSecondaryDatabase(
                new SSecondaryConfig().setAllowPopulate(true));
        sdb.close();
        try (SDatabase db = env.openDatabase(null, "secondary", null, null)) {
            assertDbData(db, new String[][]{
                    {"fKey_1", "pKey_1"},
                    {"fKey_1", "pKey_2"},
                    {"fKey_2", "pKey_1"},
                    {"fKey_2", "pKey_3"},
                    {"fKey_3", "pKey_2"},
                    {"fKey_3", "pKey_3"},
                    {"fKey_3", "pKey_4"}
            });
        }
    }

    @Test
    public void testOpenPopulate() throws Exception {
        openSecondaryDatabase(new SSecondaryConfig()).close();
        testPopulate();
    }

    @Test
    public void testPopulateNonEmpty() throws Exception {
        try (SSecondaryDatabase ignored = openSecondaryDatabase(
                new SSecondaryConfig())) {
            primary.put(null, entry("pKey_5"), entry("fKey_5"));
        }
        try (SDatabase db = env.openDatabase(null, "secondary", null, null)) {
            assertDbData(db, new String[][]{{"fKey_5", "pKey_5"}});
        }

        openSecondaryDatabase(new SSecondaryConfig().setAllowPopulate(true))
                .close();

        try (SDatabase db = env.openDatabase(null, "secondary", null, null)) {
            assertDbData(db, new String[][]{{"fKey_5", "pKey_5"}});
        }
    }

    private void populateData() throws Exception {
        primary.put(null, entry("pKey_1"), entry("fKey_1,fKey_2"));
        primary.put(null, entry("pKey_2"), entry("fKey_1,fKey_3"));
        primary.put(null, entry("pKey_3"), entry("fKey_2,fKey_3"));
        primary.put(null, entry("pKey_4"), entry("fKey_3"));
    }

    private SSecondaryDatabase openSecondaryDatabase(SSecondaryConfig config)
            throws Exception {
        config.setAllowCreate(true).setSortedDuplicates(true).setType(BTREE);
        config.setMultiKeyCreator((secondary, key, data, results) ->
                Arrays.stream(new String(data.getData()).split(","))
                        .filter(s -> !s.trim().isEmpty())
                        .map(this::entry)
                        .forEach(results::add));
        return env.openSecondaryDatabase(null, "secondary", null, primary,
                config);
    }
}
