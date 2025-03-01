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

public class SJoinCursorTest extends ClientTestBase {

    private SDatabase primary;

    private SJoinCursor joinCursor;

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
                .setSortedDuplicates(true);

        config.setKeyCreator(new KeyCreator(0));
        SSecondaryDatabase secondary1 = env.openSecondaryDatabase(null,
                "secondary1", null, primary, config);

        config.setKeyCreator(new KeyCreator(1));
        SSecondaryDatabase secondary2 = env.openSecondaryDatabase(null,
                "secondary2", null, primary, config);

        primary.put(null, entry("pKey1"), entry("key1_1 key2_1"));
        primary.put(null, entry("pKey2"), entry("key1_1 key2_2"));
        primary.put(null, entry("pKey3"), entry("key1_2 key2_1"));

        SSecondaryCursor cursor1 = secondary1.openCursor(null, null);
        cursor1.getSearchKey(entry("key1_1"), null, null);

        SSecondaryCursor cursor2 = secondary2.openCursor(null, null);
        cursor2.getSearchKey(entry("key2_2"), null, null);

        joinCursor = primary.join(new SCursor[]{cursor1, cursor2},
                new SJoinConfig());
    }

    @Test
    public void testClose() throws Exception {
        joinCursor.close();
        assertClosed(joinCursor);
    }

    @Test
    public void testGetConfig() throws Exception {
        assertThat(joinCursor.getConfig().getNoSort(), is(false));
    }

    @Test
    public void testGetDatabase() throws Exception {
        assertThat(joinCursor.getDatabase(), is(primary));
    }

    @Test
    public void testGetNext() throws Exception {
        SDatabaseEntry key = new SDatabaseEntry();
        assertThat(joinCursor.getNext(key, null), is(SOperationStatus.SUCCESS));
        assertThat(new String(key.getData()), is("pKey2"));
    }

    @Test
    public void testGetNext1() throws Exception {
        assertCursorGet(joinCursor::getNext, new SDatabaseEntry(),
                new SDatabaseEntry(), "pKey2", "key1_1 key2_2");
    }

    private static class KeyCreator implements SSecondaryKeyCreator {
        private int index;

        private KeyCreator(int index) {
            this.index = index;
        }

        @Override
        public boolean createSecondaryKey(SSecondaryDatabase secondary,
                SDatabaseEntry key, SDatabaseEntry data, SDatabaseEntry result)
                throws SDatabaseException {
            result.setData(
                    new String(data.getData()).split(" ")[index].getBytes());
            return true;
        }
    }
}