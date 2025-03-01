/*-
 * See the file LICENSE for redistribution information.
 * 
 * Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
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
public class DatabaseConfigTest {
    public static final String DATABASECONFIGTEST_DBNAME = "databaseconfigtest.db";
    @BeforeClass public static void ClassInit() {
        TestUtils.loadConfig(null);
        TestUtils.check_file_removed(TestUtils.getDBFileName(DATABASECONFIGTEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(DATABASECONFIGTEST_DBNAME));
    }

    @AfterClass public static void ClassShutdown() {
        TestUtils.check_file_removed(TestUtils.getDBFileName(DATABASECONFIGTEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(DATABASECONFIGTEST_DBNAME));
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
        DatabaseConfig config = new DatabaseConfig();
        config.setAllowCreate(true);
        config.setType(DatabaseType.BTREE);
        config.setBtreeRecordNumbers(true);
        Database db = new Database(DATABASECONFIGTEST_DBNAME, null, config);

        DatabaseConfig oldConfig = db.getConfig();
        oldConfig.setPriority(CacheFilePriority.HIGH);
        db.setConfig(oldConfig);

        db.close();
    }
}
