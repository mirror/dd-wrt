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
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import com.sleepycat.db.*;
import java.io.*;
import java.util.Arrays;

import com.sleepycat.db.test.TestUtils;
import com.sleepycat.db.test.SliceCallback;
public class SliceTest {
    public static final String SLICETEST_DBNAME = "slicetesttest.db";
    public static final String DB_CONFIG_NAME = "DB_CONFIG";
    @BeforeClass public static void ClassInit() {
        TestUtils.loadConfig(null);
        TestUtils.check_file_removed(
            TestUtils.getDBFileName(SLICETEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR,
             TestUtils.getDBFileName(SLICETEST_DBNAME));
    }

    @AfterClass public static void ClassShutdown() {
        TestUtils.check_file_removed(
            TestUtils.getDBFileName(DB_CONFIG_NAME), true, true);
	TestUtils.check_file_removed(
	    TestUtils.getDBFileName(SLICETEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR,
	    TestUtils.getDBFileName(SLICETEST_DBNAME));
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
        throws DatabaseException, IOException
    {
	// Skip the test if slices are not enabled.
	if (!Environment.slices_enabled())
	    return;
	// Write the DB_CONFIG file
	FileOutputStream config_out = new FileOutputStream(
	    TestUtils.getDBFileName(DB_CONFIG_NAME));
	String config_line = "set_slice_count 2\n";
	config_out.write(config_line.getBytes());
	config_out.close();

	// Open the environment.
	EnvironmentConfig envConfig = new EnvironmentConfig();
	envConfig.setTransactional(true);
	envConfig.setAllowCreate(true);
	File envFile = new File(TestUtils.BASETEST_DBDIR);
	Environment dbenv = new Environment(envFile, envConfig);
	// Test that getSliceCount works.
	int slice_count = dbenv.getSliceCount();
	assertEquals(2, slice_count);

	Environment [] slice_envs = dbenv.getSlices();
	assertEquals(2, slice_envs.length);
	for (int i = 0; i < slice_count; i++) {
	    File home = slice_envs[i].getHome();
	    String expected = "__db.slice00" + i;
	    assertEquals("Check the home directory of the slices.",
	        expected, home.getName());
	}
	
	// Open a sliced database with a callback.
	SliceCallback sliceCallback = new SliceCallback();
        DatabaseConfig config = new DatabaseConfig();
        config.setAllowCreate(true);
        config.setType(DatabaseType.BTREE);
	config.setSliceCallback(sliceCallback);
	config.setSliced(true);
	assertTrue("Test setting slices in DatabaseConfig", config.getSliced());
	Transaction txn = dbenv.beginTransaction(
	    null, TransactionConfig.DEFAULT);
        Database db = dbenv.openDatabase(txn, SLICETEST_DBNAME, null, config);
	txn.commit();

	// Test getSlices
	Database [] slices = db.getSlices();
	assertEquals("Test database getSlices", slice_count, slices.length);

	// Insert a record into each slice
	String zero = "0";
	String one = "1";
	String two = "2";
	String three = "3";
	DatabaseEntry key0 = new DatabaseEntry(zero.getBytes());
        DatabaseEntry key1 = new DatabaseEntry(one.getBytes());
	DatabaseEntry data0 = new DatabaseEntry(two.getBytes());
	DatabaseEntry data1 = new DatabaseEntry(three.getBytes());
	txn = dbenv.beginTransaction(null, TransactionConfig.DEFAULT);
	if ((db.put(txn, key0, data0)) != OperationStatus.SUCCESS)
	    fail("Put key0 failed.");
	txn.commit();
	txn = dbenv.beginTransaction(null, TransactionConfig.DEFAULT);
	if ((db.put(txn, key1, data1)) != OperationStatus.SUCCESS)
	    fail("Put key1 failed.");
	TransactionStats txnStats = dbenv.getTransactionStats(null);
	TransactionStats.Active [] activeStats = txnStats.getTxnarray();
	for (int i = 0; i < activeStats.length; i++) {
	    int [] sliceStats = activeStats[i].getSliceTxns();
	    assertEquals("Test slice transaction stats",
	      Arrays.toString(sliceStats), "[0, -2147483645]");
	}
	txn.commit();
	
	// Test sliceLookup
	Database slice_db = db.sliceLookup(key0);
	DatabaseEntry data = new DatabaseEntry();
	if ((slices[0].get(
	    null, key0, data, LockMode.DEFAULT)) != OperationStatus.SUCCESS)
	    fail("Get key0 failed.");
	assertTrue("Check data0", 
	    Arrays.equals(data0.getData(), data.getData()));
	data = new DatabaseEntry();
	slice_db = db.sliceLookup(key1);
	if ((slices[1].get(
	    null, key1, data, LockMode.DEFAULT)) != OperationStatus.SUCCESS)
	    fail("Get key0 failed.");
	assertTrue("Check data1", 
	    Arrays.equals(data1.getData(), data.getData()));

        db.close();
	dbenv.close();
    }
}
