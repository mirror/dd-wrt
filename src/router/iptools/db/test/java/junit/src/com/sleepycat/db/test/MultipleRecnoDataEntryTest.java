
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
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import com.sleepycat.db.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import com.sleepycat.db.test.TestUtils;
public class MultipleRecnoDataEntryTest {
    public static final String MULTIPLERECNODATAENTRYTEST_DBNAME = "multiplerecnodataentrytest.db";
    @BeforeClass public static void ClassInit() {
        TestUtils.loadConfig(null);
        TestUtils.check_file_removed(TestUtils.getDBFileName(MULTIPLERECNODATAENTRYTEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(MULTIPLERECNODATAENTRYTEST_DBNAME));
    }

    @AfterClass public static void ClassShutdown() {
        TestUtils.check_file_removed(TestUtils.getDBFileName(MULTIPLERECNODATAENTRYTEST_DBNAME), true, true);
        TestUtils.removeall(true, true, TestUtils.BASETEST_DBDIR, TestUtils.getDBFileName(MULTIPLERECNODATAENTRYTEST_DBNAME));
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
        MultipleRecnoDataEntry write = new MultipleRecnoDataEntry(new byte[1024]);
        write.setUserBuffer(1024, true);
        for (int i = 1; i < 5; i++) {
            write.append(i, String.valueOf(i).getBytes());
        }

        MultipleRecnoDataEntry read = new MultipleRecnoDataEntry(write.getData());
        read.setUserBuffer(1024, true);
        DatabaseEntry recno = new DatabaseEntry();
        DatabaseEntry item = new DatabaseEntry();
        for (int i = 1; i < 5; i++) {
            assertTrue(read.next(recno, item));
            assertEquals(i, recno.getRecordNumber());
            assertEquals(String.valueOf(i), new String(item.getData(), item.getOffset(), item.getSize()));
        }
    }
}
