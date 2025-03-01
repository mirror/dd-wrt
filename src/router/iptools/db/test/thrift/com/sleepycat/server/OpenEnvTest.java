/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.db.Environment;
import com.sleepycat.thrift.TDatabaseException;
import com.sleepycat.thrift.TDatabaseExceptionType;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TIOException;
import org.junit.Assert;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class OpenEnvTest extends BdbServiceHandlerTestBase {

    @Test
    public void testOpenEnvironment() throws Exception {
        createEnvironment("test_home").close();
        handler.openEnvironment("test_home", new TEnvironmentConfig());
    }

    @Test(expected = TIOException.class)
    public void testOpenEnvironmentNotExist() throws Exception {
        handler.openEnvironment("notExist", new TEnvironmentConfig());
    }

    @Test
    public void testOpenEnvironmentCreate() throws Exception {
        TEnvironment env = handler.openEnvironment("notExist",
                new TEnvironmentConfig().setAllowCreate(true));
        assertEnvironmentExist("notExist");
        handler.getEnvironmentConfig(env);
    }

    @Test
    public void testOpenEnvironmentCreateExist() throws Exception {
        createEnvironment("test_home").close();
        handler.openEnvironment("test_home",
                new TEnvironmentConfig().setAllowCreate(true));
    }

    @Test
    public void testOpenEnvironmentPanic() throws Exception {
        suppressPanicMessage(() -> {
            Environment env = createEnvironment("test_recovery");
            env.panic(true);

            try {
                handler.openEnvironment("test_recovery", null);
                Assert.fail();
            } catch (TDatabaseException e) {
                assertThat(e.type, is(TDatabaseExceptionType.RUN_RECOVERY));
            }
        });
    }

    @Test
    public void testOpenEnvironmentRecovery() throws Exception {
        suppressPanicMessage(() -> {
            Environment env = createEnvironment("test_recovery");
            env.panic(true);

            handler.openEnvironment("test_recovery",
                    new TEnvironmentConfig().setAllowCreate(true)
                            .setRunRecovery(true));
        });
    }

    @Test
    public void testOpenEnvironmentRecoveryCloseHandles() throws Exception {
        createEnvironment("test_recovery").close();
        TEnvironment env = handler.openEnvironment("test_recovery", null);

        handler.openEnvironment("test_recovery",
                new TEnvironmentConfig().setAllowCreate(true)
                        .setRunRecovery(true));

        assertEnvironmentClosed(env);
    }

    @SuppressWarnings("SameParameterValue")
    private void assertEnvironmentExist(String homeDir) throws Exception {
        Path envHome = testRoot.resolve(homeDir);
        assertThat(Files.isDirectory(envHome), is(true));
        assertThat(Files.isRegularFile(envHome.resolve("__db.001")), is(true));
    }

    private void suppressPanicMessage(RunnableEx block) throws Exception {
        PrintStream stdErr = System.err;

        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        System.setErr(new PrintStream(buffer, true));

        block.run();

        System.setErr(stdErr);
        for (String line : buffer.toString().split("\n")) {
            if (!line.startsWith("BDB0060") && !line.startsWith("BDB1558") && !line.trim().isEmpty()) {
                System.err.println(line);
            }
        }
    }

    private interface RunnableEx {
        void run() throws Exception;
    }

}
