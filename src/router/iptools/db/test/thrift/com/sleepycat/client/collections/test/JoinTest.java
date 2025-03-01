/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import java.util.Map;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.sleepycat.client.bind.serial.StoredClassCatalog;
import com.sleepycat.client.bind.serial.test.MarshalledObject;
import com.sleepycat.client.collections.StoredCollection;
import com.sleepycat.client.collections.StoredContainer;
import com.sleepycat.client.collections.StoredIterator;
import com.sleepycat.client.collections.StoredMap;
import com.sleepycat.client.collections.TransactionRunner;
import com.sleepycat.client.collections.TransactionWorker;
import com.sleepycat.client.collections.TupleSerialFactory;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SSecondaryConfig;
import com.sleepycat.client.SSecondaryDatabase;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestBase;
import com.sleepycat.client.util.test.TestEnv;

/**
 * @author Mark Hayes
 */
public class JoinTest extends TestBase
    implements TransactionWorker {

    private static final String MATCH_DATA = "d4"; // matches both keys = "yes"
    private static final String MATCH_KEY  = "k4"; // matches both keys = "yes"
    private static final String[] VALUES = {"yes", "yes"};

    private SEnvironment env;
    private TransactionRunner runner;
    private StoredClassCatalog catalog;
    private TupleSerialFactory factory;
    private SDatabase store;
    private SSecondaryDatabase index1;
    private SSecondaryDatabase index2;
    private StoredMap storeMap;
    private StoredMap indexMap1;
    private StoredMap indexMap2;

    public JoinTest() {
        customName = "JoinTest";
    }

    @Before
    public void setUp()
        throws Exception {

        super.setUp();
        SharedTestUtils.printTestName(customName);
        env = TestEnv.TXN.open(connection, customName);
        runner = new TransactionRunner(env);
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
            if (store != null) {
                store.close();
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
            index1 = null;
            index2 = null;
            store = null;
            catalog = null;
            env = null;
            runner = null;
            factory = null;
            storeMap = null;
            indexMap1 = null;
            indexMap2 = null;
            super.tearDown();
        }
    }

    @Test
    public void runTest()
        throws Exception {

        runner.run(this);
    }

    public void doWork() {
        createViews();
        writeAndRead();
    }

    private void createDatabase()
        throws Exception {

        catalog = new StoredClassCatalog(openDb("catalog.db"));
        factory = new TupleSerialFactory(catalog);
        assertSame(catalog, factory.getCatalog());

        store = openDb("store.db");
        index1 = openSecondaryDb(store, "index1.db", "1");
        index2 = openSecondaryDb(store, "index2.db", "2");
    }

    private SDatabase openDb(String file)
        throws Exception {

        SDatabaseConfig config = new SDatabaseConfig();
        DbCompat.setTypeBtree(config);
        config.setAllowCreate(true);

        return DbCompat.testOpenDatabase(env, null, file, null, config);
    }

    private SSecondaryDatabase openSecondaryDb(SDatabase primary,
                                              String file,
                                              String keyName)
        throws Exception {

        SSecondaryConfig secConfig = new SSecondaryConfig();
        DbCompat.setTypeBtree(secConfig);
        secConfig.setAllowCreate(true);
        DbCompat.setSortedDuplicates(secConfig, true);
        secConfig.setKeyCreator(factory.getKeyCreator(MarshalledObject.class,
                                                      keyName));

        return DbCompat.testOpenSecondaryDatabase
            (env, null, file, null, primary, secConfig);
    }

    private void createViews() {
        storeMap = factory.newMap(store, String.class,
                                         MarshalledObject.class, true);
        indexMap1 = factory.newMap(index1, String.class,
                                           MarshalledObject.class, true);
        indexMap2 = factory.newMap(index2, String.class,
                                           MarshalledObject.class, true);
    }

    private void writeAndRead() {
        // write records: Data, PrimaryKey, IndexKey1, IndexKey2
        assertNull(storeMap.put(null,
            new MarshalledObject("d1", "k1", "no",  "yes")));
        assertNull(storeMap.put(null,
            new MarshalledObject("d2", "k2", "no",  "no")));
        assertNull(storeMap.put(null,
            new MarshalledObject("d3", "k3", "no",  "yes")));
        assertNull(storeMap.put(null,
            new MarshalledObject("d4", "k4", "yes", "yes")));
        assertNull(storeMap.put(null,
            new MarshalledObject("d5", "k5", "yes", "no")));

        Object o;
        Map.Entry e;

        // join values with index maps
        o = doJoin((StoredCollection) storeMap.values());
        assertEquals(MATCH_DATA, ((MarshalledObject) o).getData());

        // join keySet with index maps
        o = doJoin((StoredCollection) storeMap.keySet());
        assertEquals(MATCH_KEY, o);

        // join entrySet with index maps
        o = doJoin((StoredCollection) storeMap.entrySet());
        e = (Map.Entry) o;
        assertEquals(MATCH_KEY, e.getKey());
        assertEquals(MATCH_DATA, ((MarshalledObject) e.getValue()).getData());
    }

    private Object doJoin(StoredCollection coll) {

        StoredContainer[] indices = { indexMap1, indexMap2 };
        StoredIterator i = coll.join(indices, VALUES, null);
        try {
            assertTrue(i.hasNext());
            Object result = i.next();
            assertNotNull(result);
            assertFalse(i.hasNext());
            return result;
        } finally { i.close(); }
    }
}
