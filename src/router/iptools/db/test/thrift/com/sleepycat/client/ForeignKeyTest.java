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
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.util.Arrays;
import java.util.stream.Collectors;

import static com.sleepycat.client.SDatabaseType.BTREE;
import static org.hamcrest.CoreMatchers.containsString;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

@RunWith(Parameterized.class)
public class ForeignKeyTest extends ClientTestBase {

    @Parameterized.Parameters(name = "{0}")
    public static SForeignKeyDeleteAction[] actions() {
        return SForeignKeyDeleteAction.values();
    }

    private SForeignKeyDeleteAction action;

    private SDatabase primary;

    private SDatabase foreign;

    public ForeignKeyTest(SForeignKeyDeleteAction action) {
        this.action = action;
    }

    @Before
    public void setUp() throws Exception {
        super.setUp();
        SEnvironment env = connection.openEnvironment("env",
                new SEnvironmentConfig().setAllowCreate(true));
        primary = env.openDatabase(null, "primary", null,
                new SDatabaseConfig().setAllowCreate(true)
                        .setType(BTREE));
        foreign = env.openDatabase(null, "foreign", null,
                new SDatabaseConfig().setAllowCreate(true).setType(BTREE));

        SSecondaryConfig config = new SSecondaryConfig();
        config.setAllowCreate(true).setType(BTREE).setSortedDuplicates(true);
        config.setMultiKeyCreator((sdb, key, data, results) ->
                Arrays.stream(new String(data.getData()).split(","))
                        .filter(s -> !s.trim().isEmpty())
                        .map(this::entry)
                        .forEach(results::add));
        config.setForeignKeyDatabase(foreign);
        config.setForeignKeyDeleteAction(action);
        if (action == SForeignKeyDeleteAction.NULLIFY) {
            config.setForeignMultiKeyNullifier((secDb, key, data, secKey) -> {
                String fKey = new String(secKey.getData());
                Iterable<String> items =
                        Arrays.stream(new String(data.getData()).split(","))
                                .filter(s -> !s.equals(fKey))
                                .collect(Collectors.toList());
                data.setData(String.join(",", items).getBytes());
                return true;
            });
        }

        env.openSecondaryDatabase(null, "secondary", null, primary, config);
    }

    @Test
    public void testPut() throws Exception {
        foreign.put(null, entry("fKey_1"), entry("fData_1"));
        foreign.put(null, entry("fKey_2"), entry("fData_2"));
        assertThat(primary.put(null, entry("pKey"), entry("fKey_1,fKey_2")),
                is(SOperationStatus.SUCCESS));
    }

    @Test
    public void testPutWithoutFK() throws Exception {
        expectForeignConflict();
        primary.put(null, entry("pKey"), entry("fKey"));
    }

    @Test
    public void testDelete() throws Exception {
        populateData();

        if (action == SForeignKeyDeleteAction.ABORT) {
            expectForeignConflict();
        }
        assertThat(foreign.delete(null, entry("fKey_2")),
                is(SOperationStatus.SUCCESS));

        if (action == SForeignKeyDeleteAction.CASCADE) {
            assertDbData(primary, new String[][]{
                    {"pKey_2", "fKey_1,fKey_3"},
                    {"pKey_4", "fKey_3"}
            });
        } else if (action == SForeignKeyDeleteAction.NULLIFY) {
            assertDbData(primary, new String[][]{
                    {"pKey_1", "fKey_1"},
                    {"pKey_2", "fKey_1,fKey_3"},
                    {"pKey_3", "fKey_3"},
                    {"pKey_4", "fKey_3"}
            });
        }
    }

    @Test
    public void testDeleteMultiple() throws Exception {
        populateData();

        SMultipleDataEntry keys = new SMultipleDataEntry();
        keys.append("fKey_1".getBytes());
        keys.append("fKey_2".getBytes());

        if (action == SForeignKeyDeleteAction.ABORT) {
            expectForeignConflict();
        }
        assertThat(foreign.deleteMultiple(null, keys),
                is(SOperationStatus.SUCCESS));

        if (action == SForeignKeyDeleteAction.CASCADE) {
            assertDbData(primary, new String[][]{
                    {"pKey_4", "fKey_3"}
            });
        } else if (action == SForeignKeyDeleteAction.NULLIFY) {
            assertDbData(primary, new String[][]{
                    {"pKey_1", ""},
                    {"pKey_2", "fKey_3"},
                    {"pKey_3", "fKey_3"},
                    {"pKey_4", "fKey_3"}
            });
        }
    }

    @Test
    public void testDeleteMultipleKey() throws Exception {
        populateData();

        SMultipleKeyDataEntry pairs = new SMultipleKeyDataEntry();
        pairs.append("fKey_1".getBytes(), "fData_1".getBytes());
        pairs.append("fKey_3".getBytes(), "fData_3".getBytes());

        if (action == SForeignKeyDeleteAction.ABORT) {
            expectForeignConflict();
        }
        assertThat(foreign.deleteMultipleKey(null, pairs),
                is(SOperationStatus.SUCCESS));

        if (action == SForeignKeyDeleteAction.CASCADE) {
            assertThat(primary.isEmpty(null), is(true));
        } else if (action == SForeignKeyDeleteAction.NULLIFY) {
            assertDbData(primary, new String[][]{
                    {"pKey_1", "fKey_2"},
                    {"pKey_2", ""},
                    {"pKey_3", "fKey_2"},
                    {"pKey_4", ""}
            });
        }
    }

    @Test
    public void testCursorDelete() throws Exception {
        populateData();

        STransaction txn =
                foreign.getEnvironment().beginTransaction(null, null);

        try (SCursor cursor = foreign.openCursor(txn, null)) {
            cursor.getSearchKey(entry("fKey_1"), null, null);
            if (action == SForeignKeyDeleteAction.ABORT) {
                expectForeignConflict();
            }
            assertThat(cursor.delete(), is(SOperationStatus.SUCCESS));
        } catch (Exception e) {
            txn.abort();
            throw e;
        }
        txn.commit();

        if (action == SForeignKeyDeleteAction.CASCADE) {
            assertDbData(primary, new String[][]{
                    {"pKey_3", "fKey_2,fKey_3"},
                    {"pKey_4", "fKey_3"}
            });
        } else if (action == SForeignKeyDeleteAction.NULLIFY) {
            assertDbData(primary, new String[][]{
                    {"pKey_1", "fKey_2"},
                    {"pKey_2", "fKey_3"},
                    {"pKey_3", "fKey_2,fKey_3"},
                    {"pKey_4", "fKey_3"}
            });
        }
    }

    private void populateData() throws Exception {
        foreign.put(null, entry("fKey_1"), entry("fData_1"));
        foreign.put(null, entry("fKey_2"), entry("fData_2"));
        foreign.put(null, entry("fKey_3"), entry("fData_3"));
        primary.put(null, entry("pKey_1"), entry("fKey_1,fKey_2"));
        primary.put(null, entry("pKey_2"), entry("fKey_1,fKey_3"));
        primary.put(null, entry("pKey_3"), entry("fKey_2,fKey_3"));
        primary.put(null, entry("pKey_4"), entry("fKey_3"));
    }

    private void expectForeignConflict() {
        thrown.expect(SDatabaseException.class);
        thrown.expectMessage(containsString("BDB0065 DB_FOREIGN_CONFLICT"));
    }
}
