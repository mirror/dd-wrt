/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TDatabaseType;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TRuntimeException;
import org.apache.thrift.TException;
import org.junit.Assert;
import org.junit.Test;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class DeleteResourceRaceTest extends BdbServiceHandlerTestBase {

    private volatile Exception exception;

    private volatile TDatabase db;

    @Test
    public void testDeleteEnvironmentAndDatabases() throws Exception {
        TDatabaseConfig createConfig =
                new TDatabaseConfig().setAllowCreate(true)
                        .setType(TDatabaseType.BTREE);
        for (int i = 0; i < 100; i++) {
            TEnvironment env = handler.openEnvironment("test_race",
                    new TEnvironmentConfig().setAllowCreate(true));
            handler.openDatabase(env, null, "test.db", null, createConfig);
            handler.closeEnvironmentHandle(env);

            Thread t1 = new Thread(() -> {
                try {
                    TEnvironment e = handler.openEnvironment("test_race", null);
                    long start = System.currentTimeMillis();
                    while (System.currentTimeMillis() - start <= 60000) {
                        db = handler.openDatabase(e, null, "test.db", null,
                                createConfig);
                    }
                } catch (TException ex) {
                    if (!isIllegalArgumentException(ex)) {
                        exception = ex;
                    }
                }
            });
            t1.start();

            Thread t2 = new Thread(() -> {
                try {
                    Thread.sleep((long) ((Math.random() + 0.1) * 50));
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
                try {
                    handler.deleteEnvironmentAndDatabases("test_race", true);
                } catch (TException e) {
                    exception = e;
                }
            });
            t2.start();

            t2.join();
            t1.join();
            if (exception != null)
                throw exception;
            try {
                handler.getDatabaseConfig(db);
                Assert.fail();
            } catch (TRuntimeException e) {
                assertThat(e.fullClassName,
                        is(IllegalArgumentException.class.getName()));
            }
        }
    }
}