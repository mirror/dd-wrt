/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.fail;

import java.util.Map;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.sleepycat.client.bind.EntityBinding;
import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.bind.serial.ClassCatalog;
import com.sleepycat.client.bind.serial.SerialBinding;
import com.sleepycat.client.bind.serial.StoredClassCatalog;
import com.sleepycat.client.bind.serial.TupleSerialBinding;
import com.sleepycat.client.bind.tuple.IntegerBinding;
import com.sleepycat.client.bind.tuple.TupleInput;
import com.sleepycat.client.bind.tuple.TupleOutput;
import com.sleepycat.client.collections.StoredMap;
import com.sleepycat.client.collections.TransactionRunner;
import com.sleepycat.client.collections.TransactionWorker;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestBase;
import com.sleepycat.client.util.test.TestEnv;

/**
 * Unit test for [#19085]. The collections API supports storing and retrieving
 * null values, as long as the value binding supports null values.
 */
public class NullValueTest extends TestBase
    implements TransactionWorker {
    
    private SEnvironment env;
    private ClassCatalog catalog;
    private SDatabase db;
    private TransactionRunner runner;
    
    public NullValueTest() {

        customName = "NullValueTest";
    }
    
    @Before
    public void setUp()
        throws Exception {

        SharedTestUtils.printTestName(customName);
        super.setUp();
        env = TestEnv.TXN.open(connection, customName);
        runner = new TransactionRunner(env);
        open();
    }
    
    @After
    public void tearDown() throws Exception {
        if (catalog != null) {
            try {
                catalog.close();
            } catch (SDatabaseException e) {
                System.out.println("During tearDown: " + e);
            }
        }
        if (db != null) {
            try {
                db.close();
            } catch (SDatabaseException e) {
                System.out.println("During tearDown: " + e);
            }
        }
        if (env != null) {
            try {
                env.close();
            } catch (SDatabaseException e) {
                System.out.println("During tearDown: " + e);
            }
        }
        catalog = null;
        db = null;
        env = null;
        super.tearDown();
    }
    
    private void open()
        throws Exception {
    
        SDatabaseConfig dbConfig = new SDatabaseConfig();
        dbConfig.setAllowCreate(true);
        DbCompat.setTypeBtree(dbConfig);

        SDatabase catalogDb = DbCompat.testOpenDatabase(env, null, "catalog",
                                                       null, dbConfig);
        catalog = new StoredClassCatalog(catalogDb);
    
        db = DbCompat.testOpenDatabase(env, null, "test", null, dbConfig);
    }
    
    @Test
    public void runTest()
        throws Exception {

        runner.run(this);
    }
    
    public void doWork() {
        expectSuccessWithBindingThatDoesSupportNull();
        expectExceptionWithBindingThatDoesNotSupportNull();
        expectExceptionWithWithEntityBinding();
    }
    
    private void expectSuccessWithBindingThatDoesSupportNull() {

        /* Pass null for baseClass to support null values. */
        final EntryBinding<String> dataBinding =
            new SerialBinding<String>(catalog, null /*baseClass*/);

        final StoredMap<Integer, String> map = new StoredMap<Integer, String>
            (db, new IntegerBinding(), dataBinding, true);
        
        /* Store a null value.*/
        map.put(1, null);

        /* Get the null value. */
        assertNull(map.get(1));

        for (String value : map.values()) {
            assertNull(value);
        }

        for (Map.Entry<Integer, String> entry : map.entrySet()) {
            assertEquals(Integer.valueOf(1), entry.getKey());
            assertNull(entry.getValue());
        }

        map.remove(1);
    }

    private void expectExceptionWithBindingThatDoesNotSupportNull() {

        /* Pass non-null for baseClass, null values will not be allowed. */
        final EntryBinding<String> dataBinding =
            new SerialBinding<String>(catalog, String.class /*baseClass*/);

        final StoredMap<Integer, String> map = new StoredMap<Integer, String>
            (db, new IntegerBinding(), dataBinding, true);

        try {
            map.put(1, null);
            fail();
        } catch (IllegalArgumentException expected) {
        }
    }

    public void expectExceptionWithWithEntityBinding() {

        final EntityBinding<MyEntity> entityBinding =
            new MyEntityBinding(catalog);

        final StoredMap<Integer, MyEntity> map =
            new StoredMap<Integer, MyEntity>
                (db, new IntegerBinding(), entityBinding, true);

        try {
            map.put(1, null);
            fail();
        } catch (IllegalArgumentException expected) {
        }
    }

    static class MyEntity {
        int key;
        String data;
    }

    static class MyEntityBinding extends TupleSerialBinding<String, MyEntity> {

        MyEntityBinding(ClassCatalog catalog) {
            super(catalog, String.class);
        }

        public MyEntity entryToObject(TupleInput keyInput, String data) {
            final MyEntity entity = new MyEntity();
            entity.key = keyInput.readInt();
            entity.data = data;
            return entity;
        }

        public void objectToKey(MyEntity entity, TupleOutput keyOutput) {
            keyOutput.writeInt(entity.key);
        }

        public String objectToData(MyEntity entity) {
            return entity.data;
        }
    }
}
