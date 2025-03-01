/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TRuntimeException;
import org.apache.thrift.TException;
import org.junit.Assert;
import org.junit.Test;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class CloseEnvTest extends BdbServiceHandlerTestBase {

    private volatile Exception exception = null;

    @Test
    public void testCloseEnvironment() throws Exception {
        TEnvironment env = handler.openEnvironment("test_close",
                new TEnvironmentConfig().setAllowCreate(true));
        handler.closeEnvironmentHandle(env);

        assertEnvironmentClosed(env);
    }

    @Test
    public void testCloseEnvironmentClosed() throws Exception {
        TEnvironment env = handler.openEnvironment("test_close",
                new TEnvironmentConfig().setAllowCreate(true));
        handler.closeEnvironmentHandle(env);

        try {
            handler.closeEnvironmentHandle(env);
            Assert.fail();
        } catch (TRuntimeException e) {
            assertThat(isIllegalArgumentException(e), is(true));
        }
    }

    @Test
    public void testCloseAllEnvironmentHandles() throws Exception {
        TEnvironmentConfig config =
                new TEnvironmentConfig().setAllowCreate(true);
        TEnvironment env1 = handler.openEnvironment("test_close", config);

        long t1 = System.currentTimeMillis();
        try {
            Thread.sleep(1000L);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        long t2 = System.currentTimeMillis();

        TEnvironment env2 = handler.openEnvironment("test_close", config);

        long t3 = System.currentTimeMillis();

        handler.closeEnvironmentHandles("test_close", (t3 - t1) / 2);

        long t4 = System.currentTimeMillis();

        /*
         * Because thread scheduling is undetermined, we cannot get the exact
         * time when the two handles are accessed or when closeAll() evaluates
         * "now". Therefore, to make the test result 100% stable, we have to
         * use time bounds to determine the states of the two handles.
         *
         * env1 was accessed <= t1; env2 was accessed between t2 and t3; "now"
         * was evaluated by closeAll() between t3 and t4.
         *
         * We have env1 <= t1 <= t2 <= env2 <= t3 <= now <= t4. The threshold
         * for closing handles is now - (t3 - t1) / 2, so
         * t1 <= threshold, and env1 must have been closed.
         */
        assertEnvironmentClosed(env1);

        // if threshold <= t4 - (t3 - t1) / 2 <= t2, env2 must be open
        if (t4 - (t3 - t1) / 2 <= t2) {
            handler.closeEnvironmentHandle(env2);
        }
    }

    @Test
    public void testCloseRaceWithAccess() throws Exception {
        for (int i = 0; i < 100; i++) {
            TEnvironment env = handler.openEnvironment("test_race",
                    new TEnvironmentConfig().setAllowCreate(true));

            Thread t1 = new Thread(() -> {
                try {
                    long start = System.currentTimeMillis();
                    while (System.currentTimeMillis() - start <= 60000) {
                        handler.getEnvironmentConfig(env);
                    }
                } catch (TException e) {
                    if (!isIllegalArgumentException(e)) {
                        exception = e;
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
                    handler.closeEnvironmentHandle(env);
                } catch (TException e) {
                    exception = e;
                }
            });
            t2.start();

            t2.join();
            t1.join();
            if (exception != null)
                throw exception;
        }
    }
}