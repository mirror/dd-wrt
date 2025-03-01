/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.db.Environment;
import com.sleepycat.db.EnvironmentConfig;
import com.sleepycat.server.config.BdbServiceConfig;
import com.sleepycat.server.util.FileUtils;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TRuntimeException;
import org.apache.thrift.TException;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Properties;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class BdbServiceHandlerTestBase {

    protected Path testRoot;

    protected BdbServiceHandler handler;

    @Before
    public void setUp() throws Exception {
        testRoot = Files.createTempDirectory("BdbServiceTest");
        handler = new BdbServiceHandler();

        Properties properties = new Properties();
        properties.setProperty(BdbServiceConfig.ROOT_HOME,
                testRoot.toAbsolutePath().toString());

        BdbServiceConfig config = new BdbServiceConfig(properties);
        config.initRootDirs();

        handler.setServerAndConfig(new MockServer(), config);
    }

    @After
    public void tearDown() throws Exception {
        handler.shutdown();
        FileUtils.deleteFileTree(testRoot.toFile());
    }

    protected Environment createEnvironment(String homeDir) throws Exception {
        File envHome = testRoot.resolve(homeDir).toFile();
        Files.createDirectories(envHome.toPath());
        EnvironmentConfig config = new EnvironmentConfig();
        config.setAllowCreate(true);
        config.setInitializeCache(true);
        config.setInitializeLocking(true);
        config.setInitializeLogging(true);
        config.setThreaded(true);
        config.setTransactional(true);
        return new Environment(envHome, config);
    }

    protected void assertEnvironmentClosed(TEnvironment env) throws Exception {
        try {
            handler.getEnvironmentConfig(env);
            Assert.fail();
        } catch (TRuntimeException e) {
            assertThat(e.fullClassName,
                    is(IllegalArgumentException.class.getName()));
        }
    }

    private boolean checkRuntimeException(TException e, Class<?> exClass) {
        return e instanceof TRuntimeException &&
                ((TRuntimeException) e).fullClassName.equals(exClass.getName());
    }

    protected boolean isIllegalArgumentException(TException e) {
        return checkRuntimeException(e, IllegalArgumentException.class);
    }

    protected boolean isUnsupportedOperationException(TException e) {
        return checkRuntimeException(e, UnsupportedOperationException.class);
    }

}
