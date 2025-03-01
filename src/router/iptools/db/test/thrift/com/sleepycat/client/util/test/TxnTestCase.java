/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.util.test;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.After;
import org.junit.Before;

import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.STransactionConfig;
import com.sleepycat.client.util.DualTestCase;

/**
 * Permutes test cases over three transaction types: null (non-transactional),
 * auto-commit, and user (explicit).
 *
 * <p>Overrides runTest, setUp and tearDown to open/close the environment and
 * to set up protected members for use by test cases.</p>
 *
 * <p>If a subclass needs to override setUp or tearDown, the overridden method
 * should call super.setUp or super.tearDown.</p>
 *
 * <p>When writing a test case based on this class, write it as if a user txn
 * were always used: call txnBegin, txnCommit and txnAbort for all write
 * operations.  Use the isTransactional protected field for setup of a database
 * config.</p>
 */
public abstract class TxnTestCase extends DualTestCase {

    public static final String TXN_AUTO = "txn-auto";
    public static final String TXN_USER = "txn-user";

    protected File envHome;
    protected SEnvironment env;
    protected SEnvironmentConfig envConfig;
    protected String txnType;
    protected boolean isTransactional;

    public static List<Object[]> getTxnParams(String[] txnTypes, boolean rep) {
        final List<Object[]> list = new ArrayList<>();
        for (final String type : getTxnTypes(txnTypes, rep)) {
            list.add(new Object[] {type});
        }
        return list;
    }

    public static String[] getTxnTypes(String[] txnTypes, boolean rep) {
        if (txnTypes == null) {
            txnTypes = new String[] { TxnTestCase.TXN_USER,
                                      TxnTestCase.TXN_AUTO };
        }
        return txnTypes;
    }

    @Before
    public void setUp()
        throws Exception {

        super.setUp();
        envHome = SharedTestUtils.getNewDir();
        openEnv();
    }

    @After
    public void tearDown()
        throws Exception {

        closeEnv();
        env = null;
        super.tearDown();
    }
    
    protected void initEnvConfig() {
        if (envConfig == null) {
            envConfig = new SEnvironmentConfig();
            envConfig.setAllowCreate(true);
            
            /* Always use write-no-sync (by default) to speed up tests. */
            if (!envConfig.getTxnNoSync() && !envConfig.getTxnWriteNoSync()) {
                envConfig.setTxnWriteNoSync(true);
            }
        }
    }

    /**
     * Closes the environment and sets the env field to null.
     * Used for closing and reopening the environment.
     */
    public void closeEnv()
        throws SDatabaseException {

        if (env != null) {
            close(env);
            env = null;
        }
    }

    /**
     * Opens the environment based on the txnType for this test case.
     * Used for closing and reopening the environment.
     */
    public void openEnv()
        throws SDatabaseException {

        if (txnType == TXN_AUTO) {
            TestEnv.TXN.copyConfig(envConfig);
            env = create(envHome, envConfig);
        } else if (txnType == TXN_USER) {
            TestEnv.TXN.copyConfig(envConfig);
            env = create(envHome, envConfig);
        } else {
            assert false;
        }
    }

    /**
     * Begin a txn if in TXN_USER mode; otherwise return null;
     */
    protected STransaction txnBegin()
        throws SDatabaseException {

        return txnBegin(null, null);
    }

    /**
     * Begin a txn if in TXN_USER mode; otherwise return null;
     */
    protected STransaction txnBegin(STransaction parentTxn,
                                   STransactionConfig config)
        throws SDatabaseException {

        if (txnType == TXN_USER) {
            return env.beginTransaction(parentTxn, config);
        }
        return null;
    }

    /**
     * Begin a txn if in TXN_USER or TXN_AUTO mode; otherwise return null;
     */
    protected STransaction txnBeginCursor()
        throws SDatabaseException {

        return txnBeginCursor(null, null);
    }

    /**
     * Begin a txn if in TXN_USER or TXN_AUTO mode; otherwise return null;
     */
    protected STransaction txnBeginCursor(STransaction parentTxn,
                                         STransactionConfig config)
        throws SDatabaseException {

        if (txnType == TXN_USER || txnType == TXN_AUTO) {
            return env.beginTransaction(parentTxn, config);
        } else {
            return null;
        }
    }
    
    /**
     * Create a write cursor config;
     */
    public SCursorConfig getWriteCursorConfig() {
        return null;
    } 

    /**
     * Commit a txn if non-null.
     */
    protected void txnCommit(STransaction txn)
        throws SDatabaseException {

        if (txn != null) {
            txn.commit();
        }
    }

    /**
     * Commit a txn if non-null.
     */
    protected void txnAbort(STransaction txn)
        throws SDatabaseException {

        if (txn != null) {
            txn.abort();
        }
    }
}
