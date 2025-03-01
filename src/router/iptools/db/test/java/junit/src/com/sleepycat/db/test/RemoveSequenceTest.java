/*-
 * See the file LICENSE for redistribution information.
 * 
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */


package com.sleepycat.db.test;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import com.sleepycat.db.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import com.sleepycat.db.test.TestUtils;
public class RemoveSequenceTest {
    public static final String REMOVESEQUENCETEST_DBNAME = "removesequencetest.db";
    @BeforeClass public static void ClassInit() {
        TestUtils.loadConfig(null);
        TestUtils.check_file_removed(TestUtils.getDBFileName(REMOVESEQUENCETEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(REMOVESEQUENCETEST_DBNAME));
    }

    @AfterClass public static void ClassShutdown() {
        TestUtils.check_file_removed(TestUtils.getDBFileName(REMOVESEQUENCETEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(REMOVESEQUENCETEST_DBNAME));
    }

    @Before public void PerTestInit()
        throws Exception {
    }

    @After public void PerTestShutdown()
        throws Exception {
    }
    /*
     * Test case implementations.
     * To disable a test mark it with @Ignore
     * To set a timeout(ms) notate like: @Test(timeout=1000)
     * To indicate an expected exception notate like: (expected=Exception)
     */

    @Test public void test1()
        throws DatabaseException, FileNotFoundException
    {
        EnvironmentConfig envConfig = new EnvironmentConfig();
        envConfig.setAllowCreate(true);
        envConfig.setInitializeCache(true);
        envConfig.setInitializeLocking(true);
        envConfig.setInitializeLogging(true);
        envConfig.setTransactional(true);

        Environment env = new Environment(TestUtils.BASETEST_DBFILE, envConfig);

        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setAllowCreate(true);
        dbConfig.setTransactional(true);
        dbConfig.setType(DatabaseType.BTREE);

        Database db = env.openDatabase(null, REMOVESEQUENCETEST_DBNAME, null, dbConfig);

        SequenceConfig seqConfig = new SequenceConfig();
        seqConfig.setAllowCreate(true);
        DatabaseEntry key = new DatabaseEntry("seq".getBytes());

        Sequence seq = db.openSequence(null, key, seqConfig);
        seq.close();

        db.removeSequence(null, key, seqConfig);

        db.close();
        env.close();
    }
}
