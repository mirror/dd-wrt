/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.sleepycat.client.bind.serial.StoredClassCatalog;
import com.sleepycat.client.bind.serial.TupleSerialMarshalledKeyCreator;
import com.sleepycat.client.bind.serial.test.MarshalledObject;
import com.sleepycat.client.collections.CurrentTransaction;
import com.sleepycat.client.collections.TupleSerialFactory;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SForeignKeyDeleteAction;
import com.sleepycat.client.SSecondaryConfig;
import com.sleepycat.client.SSecondaryDatabase;
import com.sleepycat.client.util.ExceptionUnwrapper;
import com.sleepycat.client.util.RuntimeExceptionWrapper;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestBase;
import com.sleepycat.client.util.test.TestEnv;

/**
 * @author Mark Hayes
 */
@RunWith(Parameterized.class)
public class ForeignKeyTest extends TestBase {

    private static final SForeignKeyDeleteAction[] ACTIONS = {
        SForeignKeyDeleteAction.ABORT,
        SForeignKeyDeleteAction.NULLIFY,
        SForeignKeyDeleteAction.CASCADE,
    };
    private static final String[] ACTION_LABELS = {
        "ABORT",
        "NULLIFY",
        "CASCADE",
    };

    @Parameters
    public static List<Object[]> genParams() {
        List<Object[]> params = new ArrayList<Object[]>();
        for (TestEnv testEnv : TestEnv.ALL) {
            int i = 0;
            for (SForeignKeyDeleteAction action : ACTIONS) {
                params.add(new Object[]{testEnv, action, ACTION_LABELS[i]});
                i ++;
            }
        }
        
        return params;
    }
    
    private TestEnv testEnv;
    private SEnvironment env;
    private StoredClassCatalog catalog;
    private TupleSerialFactory factory;
    private SDatabase store1;
    private SDatabase store2;
    private SSecondaryDatabase index1;
    private SSecondaryDatabase index2;
    private Map storeMap1;
    private Map storeMap2;
    private Map indexMap1;
    private Map indexMap2;
    private final SForeignKeyDeleteAction onDelete;

    public ForeignKeyTest(TestEnv testEnv, SForeignKeyDeleteAction onDelete,
                          String onDeleteLabel) {

        customName = 
            "ForeignKeyTest-" + testEnv.getName() + '-' + onDeleteLabel;

        this.testEnv = testEnv;
        this.onDelete = onDelete;
    }

    @Before
    public void setUp()
        throws Exception {

        super.setUp();
        SharedTestUtils.printTestName(customName);
        env = testEnv.open(connection, customName);
        createDatabase();
    }

    @After
    public void tearDown() throws Exception {

        try {
            if (index1 != null) {
                index1.close();
            }
            if (index2 != null) {
                index2.close();
            }
            if (store1 != null) {
                store1.close();
            }
            if (store2 != null) {
                store2.close();
            }
            if (catalog != null) {
                catalog.close();
            }
            if (env != null) {
                env.close();
            }
        } catch (Exception e) {
            System.out.println("Ignored exception during tearDown: " + e);
        } finally {
            /* Ensure that GC can cleanup. */
            env = null;
            testEnv = null;
            catalog = null;
            store1 = null;
            store2 = null;
            index1 = null;
            index2 = null;
            factory = null;
            storeMap1 = null;
            storeMap2 = null;
            indexMap1 = null;
            indexMap2 = null;
            super.tearDown();
        }
    }

    @Test
    public void runTest()
        throws Exception {

        try {
            createViews();
            writeAndRead();
        } catch (Exception e) {
            throw ExceptionUnwrapper.unwrap(e);
        }
    }

    private void createDatabase()
        throws Exception {

        catalog = new StoredClassCatalog(openDb("catalog.db"));
        factory = new TupleSerialFactory(catalog);
        assertSame(catalog, factory.getCatalog());

        store1 = openDb("store1.db");
        store2 = openDb("store2.db");
        index1 = openSecondaryDb(factory, "1", store1, "index1.db", null);
        index2 = openSecondaryDb(factory, "2", store2, "index2.db", store1);
    }

    private SDatabase openDb(String file)
        throws Exception {

        SDatabaseConfig config = new SDatabaseConfig();
        DbCompat.setTypeBtree(config);
        config.setAllowCreate(true);

        return DbCompat.testOpenDatabase(env, null, file, null, config);
    }

    private SSecondaryDatabase openSecondaryDb(TupleSerialFactory factory,
                                              String keyName,
                                              SDatabase primary,
                                              String file,
                                              SDatabase foreignStore)
        throws Exception {

        TupleSerialMarshalledKeyCreator keyCreator =
                factory.getKeyCreator(MarshalledObject.class, keyName);

        SSecondaryConfig secConfig = new SSecondaryConfig();
        DbCompat.setTypeBtree(secConfig);
        secConfig.setAllowCreate(true);
        secConfig.setKeyCreator(keyCreator);
        if (foreignStore != null) {
            secConfig.setForeignKeyDatabase(foreignStore);
            secConfig.setForeignKeyDeleteAction(onDelete);
            if (onDelete == SForeignKeyDeleteAction.NULLIFY) {
                secConfig.setForeignKeyNullifier(keyCreator);
            }
        }

        return DbCompat.testOpenSecondaryDatabase
            (env, null, file, null, primary, secConfig);
    }

    private void createViews() {
        storeMap1 = factory.newMap(store1, String.class,
                                   MarshalledObject.class, true);
        storeMap2 = factory.newMap(store2, String.class,
                                   MarshalledObject.class, true);
        indexMap1 = factory.newMap(index1, String.class,
                                   MarshalledObject.class, true);
        indexMap2 = factory.newMap(index2, String.class,
                                   MarshalledObject.class, true);
    }

    private void writeAndRead()
        throws Exception {

        CurrentTransaction txn = CurrentTransaction.getInstance(env);
        if (txn != null) {
            txn.beginTransaction(null);
        }

        MarshalledObject o1 = new MarshalledObject("data1", "pk1", "ik1", "");
        assertNull(storeMap1.put(null, o1));

        assertEquals(o1, storeMap1.get("pk1"));
        assertEquals(o1, indexMap1.get("ik1"));

        MarshalledObject o2 = new MarshalledObject("data2", "pk2", "", "pk1");
        assertNull(storeMap2.put(null, o2));

        assertEquals(o2, storeMap2.get("pk2"));
        assertEquals(o2, indexMap2.get("pk1"));

        if (txn != null) {
            txn.commitTransaction();
            txn.beginTransaction(null);
        }

        /*
         * store1 contains o1 with primary key "pk1" and index key "ik1".
         *
         * store2 contains o2 with primary key "pk2" and foreign key "pk1",
         * which is the primary key of store1.
         */

        if (onDelete == SForeignKeyDeleteAction.ABORT) {

            /* Test that we abort trying to delete a referenced key. */

            try {
                storeMap1.remove("pk1");
                fail();
            } catch (RuntimeExceptionWrapper expected) {
                assertTrue(expected.getCause() instanceof SDatabaseException);
                assertTrue(!DbCompat.NEW_JE_EXCEPTIONS);
            }
            if (txn != null) {
                txn.abortTransaction();
                txn.beginTransaction(null);
            }

            /* Test that we can put a record into store2 with a null foreign
             * key value. */

            o2 = new MarshalledObject("data2", "pk2", "", "");
            assertNotNull(storeMap2.put(null, o2));
            assertEquals(o2, storeMap2.get("pk2"));

            /* The index2 record should have been deleted since the key was set
             * to null above. */

            assertNull(indexMap2.get("pk1"));

            /* Test that now we can delete the record in store1, since it is no
             * longer referenced. */

            assertNotNull(storeMap1.remove("pk1"));
            assertNull(storeMap1.get("pk1"));
            assertNull(indexMap1.get("ik1"));

        } else if (onDelete == SForeignKeyDeleteAction.NULLIFY) {

            /* Delete the referenced key. */

            assertNotNull(storeMap1.remove("pk1"));
            assertNull(storeMap1.get("pk1"));
            assertNull(indexMap1.get("ik1"));

            /* The store2 record should still exist, but should have an empty
             * secondary key since it was nullified. */

            o2 = (MarshalledObject) storeMap2.get("pk2");
            assertNotNull(o2);
            assertEquals("data2", o2.getData());
            assertEquals("pk2", o2.getPrimaryKey());
            assertEquals("", o2.getIndexKey1());
            assertEquals("", o2.getIndexKey2());

        } else if (onDelete == SForeignKeyDeleteAction.CASCADE) {

            /* Delete the referenced key. */

            assertNotNull(storeMap1.remove("pk1"));
            assertNull(storeMap1.get("pk1"));
            assertNull(indexMap1.get("ik1"));

            /* The store2 record should have deleted also. */

            assertNull(storeMap2.get("pk2"));
            assertNull(indexMap2.get("pk1"));

        } else {
            throw new IllegalStateException();
        }

        /*
         * Test that a foreign key value may not be used that is not present
         * in the foreign store. "pk2" is not in store1 in this case.
         */
        assertNull(storeMap1.get("pk2"));
        MarshalledObject o3 = new MarshalledObject("data3", "pk3", "", "pk2");
        try {
            storeMap2.put(null, o3);
            fail();
        } catch (RuntimeExceptionWrapper expected) {
            assertTrue(expected.getCause() instanceof SDatabaseException);
            assertTrue(!DbCompat.NEW_JE_EXCEPTIONS);
        }

        if (txn != null) {
            txn.abortTransaction();
        }
    }
}
