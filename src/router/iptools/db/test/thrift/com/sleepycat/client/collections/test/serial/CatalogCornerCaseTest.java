/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */
package com.sleepycat.client.collections.test.serial;

import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.sleepycat.client.bind.serial.StoredClassCatalog;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestBase;
import com.sleepycat.client.util.test.TestEnv;

/**
 * @author Mark Hayes
 */
public class CatalogCornerCaseTest extends TestBase {

    private SEnvironment env;

    public CatalogCornerCaseTest() {

        customName = "CatalogCornerCaseTest";
    }

    @Before
    public void setUp()
        throws Exception {

        super.setUp();
        SharedTestUtils.printTestName(customName);
        env = TestEnv.TXN.open(connection, customName);
    }

    @After
    public void tearDown() throws Exception {

        try {
            if (env != null) {
                env.close();
            }
        } catch (Exception e) {
            System.out.println("Ignored exception during tearDown: " + e);
        } finally {
            /* Ensure that GC can cleanup. */
            env = null;
            super.tearDown();
        }
    }

    @Test
    public void testReadOnlyEmptyCatalog()
        throws Exception {

        String file = "catalog.db";

        /* Create an empty database. */
        SDatabaseConfig config = new SDatabaseConfig();
        config.setAllowCreate(true);
        DbCompat.setTypeBtree(config);
        SDatabase db =
            DbCompat.testOpenDatabase(env, null, file, null, config);
        db.close();

        /* Open the empty database read-only. */
        config.setAllowCreate(false);
        config.setReadOnly(true);
        db = DbCompat.testOpenDatabase(env, null, file, null, config);

        /* Expect exception when creating the catalog. */
        try {
            new StoredClassCatalog(db);
            fail();
        } catch (RuntimeException e) { }
        db.close();
    }
}
