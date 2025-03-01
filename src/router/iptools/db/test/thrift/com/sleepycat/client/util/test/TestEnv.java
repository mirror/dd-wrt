/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.util.test;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import com.sleepycat.client.BdbServerConnection;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;

/**
 * @author Mark Hayes
 */
public class TestEnv {

    public static final TestEnv TXN;
    static {
        SEnvironmentConfig config;

        config = newEnvConfig();
        TXN = new TestEnv("txn", config);
    }

    private static SEnvironmentConfig newEnvConfig() {

        SEnvironmentConfig config = new SEnvironmentConfig();
        config.setTxnNoSync(Boolean.getBoolean(SharedTestUtils.NO_SYNC));
        return config;
    }

    public static final TestEnv[] ALL;
    static {
        ALL = new TestEnv[] { TXN };
    }

    private final String name;
    private final SEnvironmentConfig config;

    protected TestEnv(String name, SEnvironmentConfig config) {

        this.name = name;
        this.config = config;
    }

    public String getName() {

        return name;
    }

    public SEnvironmentConfig getConfig() {
        return config;
    }

    void copyConfig(SEnvironmentConfig copyToConfig) {
    }

    public boolean isTxnMode() {

        return config.getTransactional();
    }

    public boolean isCdbMode() {

        return DbCompat.getInitializeCDB(config);
    }

    public SEnvironment open(BdbServerConnection conn, String testName)
        throws IOException, SDatabaseException {

        return open(conn, testName, true);
    }

    public SEnvironment open(BdbServerConnection conn,
        String testName, boolean create)
        throws IOException, SDatabaseException {

        config.setAllowCreate(create);
        /* OLDEST deadlock detection on DB matches the use of timeouts on JE.*/
        DbCompat.setLockDetectModeOldest(config);
        File dir = getDirectory(testName, create);
        return conn.openEnvironment(dir.getPath(), config);
    }

    public File getDirectory(String testName) {
        return getDirectory(testName, true);
    }

    public File getDirectory(String testName, boolean create) {
        if (create) {
            return SharedTestUtils.getNewDir(testName);
        } else {
            return SharedTestUtils.getExistingDir(testName);
        }
    }
}
