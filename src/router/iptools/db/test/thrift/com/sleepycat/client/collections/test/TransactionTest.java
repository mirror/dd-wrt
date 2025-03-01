/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Iterator;
import java.util.List;
import java.util.SortedSet;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.sleepycat.client.collections.CurrentTransaction;
import com.sleepycat.client.collections.StoredCollections;
import com.sleepycat.client.collections.StoredContainer;
import com.sleepycat.client.collections.StoredIterator;
import com.sleepycat.client.collections.StoredList;
import com.sleepycat.client.collections.StoredSortedMap;
import com.sleepycat.client.collections.TransactionRunner;
import com.sleepycat.client.collections.TransactionWorker;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.SDeadlockException;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.STransactionConfig;
import com.sleepycat.client.util.RuntimeExceptionWrapper;
import com.sleepycat.client.util.test.SharedTestUtils;
import com.sleepycat.client.util.test.TestBase;
import com.sleepycat.client.util.test.TestEnv;

/**
 * @author Mark Hayes
 */
public class TransactionTest extends TestBase {

    private static final Long ONE = new Long(1);
    private static final Long TWO = new Long(2);
    private static final Long THREE = new Long(3);


    private SEnvironment env;
    private CurrentTransaction currentTxn;
    private SDatabase store;
    private StoredSortedMap map;
    private TestStore testStore = TestStore.BTREE_UNIQ;

    public TransactionTest() {

        customName = "TransactionTest";
    }

    @Before
    public void setUp()
        throws Exception {

        super.setUp();
        env = TestEnv.TXN.open(connection, "TransactionTests");
        currentTxn = CurrentTransaction.getInstance(env);
        store = testStore.open(env, dbName(0));
        map = new StoredSortedMap(store, testStore.getKeyBinding(),
                                  testStore.getValueBinding(), true);
    }

    @After
    public void tearDown() throws Exception {

        try {
            if (store != null) {
                store.close();
            }
            if (env != null) {
                env.close();
            }
        } catch (Exception e) {
            System.out.println("Ignored exception during tearDown: " + e);
        } finally {
            /* Ensure that GC can cleanup. */
            store = null;
            env = null;
            currentTxn = null;
            map = null;
            testStore = null;
            super.tearDown();
        }
    }

    private String dbName(int i) {

        return "txn-test-" + i;
    }

    @Test
    public void testGetters()
        throws Exception {

        assertNotNull(env);
        assertNotNull(currentTxn);
        assertNull(currentTxn.getTransaction());

        currentTxn.beginTransaction(null);
        assertNotNull(currentTxn.getTransaction());
        currentTxn.commitTransaction();
        assertNull(currentTxn.getTransaction());

        currentTxn.beginTransaction(null);
        assertNotNull(currentTxn.getTransaction());
        currentTxn.abortTransaction();
        assertNull(currentTxn.getTransaction());

        // read-uncommitted property should be inherited

        assertTrue(!isReadUncommitted(map));
        assertTrue(!isReadUncommitted(map.values()));
        assertTrue(!isReadUncommitted(map.keySet()));
        assertTrue(!isReadUncommitted(map.entrySet()));

        StoredSortedMap other = (StoredSortedMap)
            StoredCollections.configuredMap
                (map, SCursorConfig.READ_UNCOMMITTED);
        assertTrue(isReadUncommitted(other));
        assertTrue(isReadUncommitted(other.values()));
        assertTrue(isReadUncommitted(other.keySet()));
        assertTrue(isReadUncommitted(other.entrySet()));
        assertTrue(!isReadUncommitted(map));
        assertTrue(!isReadUncommitted(map.values()));
        assertTrue(!isReadUncommitted(map.keySet()));
        assertTrue(!isReadUncommitted(map.entrySet()));

        // read-committed property should be inherited

        assertTrue(!isReadCommitted(map));
        assertTrue(!isReadCommitted(map.values()));
        assertTrue(!isReadCommitted(map.keySet()));
        assertTrue(!isReadCommitted(map.entrySet()));

        other = (StoredSortedMap)
            StoredCollections.configuredMap
                (map, SCursorConfig.READ_COMMITTED);
        assertTrue(isReadCommitted(other));
        assertTrue(isReadCommitted(other.values()));
        assertTrue(isReadCommitted(other.keySet()));
        assertTrue(isReadCommitted(other.entrySet()));
        assertTrue(!isReadCommitted(map));
        assertTrue(!isReadCommitted(map.values()));
        assertTrue(!isReadCommitted(map.keySet()));
        assertTrue(!isReadCommitted(map.entrySet()));
    }

    @Test
    public void testTransactional()
        throws Exception {

        // is transactional because DB_AUTO_COMMIT was passed to
        // SDatabase.open()
        //
        assertTrue(map.isTransactional());
        store.close();
        store = null;

        // is not transactional
        //
        SDatabaseConfig dbConfig = new SDatabaseConfig();
        DbCompat.setTypeBtree(dbConfig);
        dbConfig.setAllowCreate(true);
        SDatabase db = DbCompat.testOpenDatabase
            (env, null, dbName(1), null, dbConfig);
        map = new StoredSortedMap(db, testStore.getKeyBinding(),
                                      testStore.getValueBinding(), true);
        map.put(ONE, ONE);
        readCheck(map, ONE, ONE);
        db.close();

        // is transactional
        //
        currentTxn.beginTransaction(null);
        db = DbCompat.testOpenDatabase
            (env, currentTxn.getTransaction(), dbName(2), null, dbConfig);
        currentTxn.commitTransaction();
        map = new StoredSortedMap(db, testStore.getKeyBinding(),
                                      testStore.getValueBinding(), true);
        assertTrue(map.isTransactional());
        currentTxn.beginTransaction(null);
        map.put(ONE, ONE);
        readCheck(map, ONE, ONE);
        currentTxn.commitTransaction();
        db.close();
    }

    @Test
    public void testExceptions()
        throws Exception {

        try {
            currentTxn.commitTransaction();
            fail();
        } catch (IllegalStateException expected) {}

        try {
            currentTxn.abortTransaction();
            fail();
        } catch (IllegalStateException expected) {}
    }

    @Test
    public void testNested()
        throws Exception {

        if (!DbCompat.NESTED_TRANSACTIONS) {
            return;
        }
        assertNull(currentTxn.getTransaction());

        STransaction txn1 = currentTxn.beginTransaction(null);
        assertNotNull(txn1);
        assertTrue(txn1 == currentTxn.getTransaction());

        assertNull(map.get(ONE));
        assertNull(map.put(ONE, ONE));
        assertEquals(ONE, map.get(ONE));

        STransaction txn2 = currentTxn.beginTransaction(null);
        assertNotNull(txn2);
        assertTrue(txn2 == currentTxn.getTransaction());
        assertTrue(txn1 != txn2);

        assertNull(map.put(TWO, TWO));
        assertEquals(TWO, map.get(TWO));

        STransaction txn3 = currentTxn.beginTransaction(null);
        assertNotNull(txn3);
        assertTrue(txn3 == currentTxn.getTransaction());
        assertTrue(txn1 != txn2);
        assertTrue(txn1 != txn3);
        assertTrue(txn2 != txn3);

        assertNull(map.put(THREE, THREE));
        assertEquals(THREE, map.get(THREE));

        STransaction txn = currentTxn.abortTransaction();
        assertTrue(txn == txn2);
        assertTrue(txn == currentTxn.getTransaction());
        assertNull(map.get(THREE));
        assertEquals(TWO, map.get(TWO));

        txn3 = currentTxn.beginTransaction(null);
        assertNotNull(txn3);
        assertTrue(txn3 == currentTxn.getTransaction());
        assertTrue(txn1 != txn2);
        assertTrue(txn1 != txn3);
        assertTrue(txn2 != txn3);

        assertNull(map.put(THREE, THREE));
        assertEquals(THREE, map.get(THREE));

        txn = currentTxn.commitTransaction();
        assertTrue(txn == txn2);
        assertTrue(txn == currentTxn.getTransaction());
        assertEquals(THREE, map.get(THREE));
        assertEquals(TWO, map.get(TWO));

        txn = currentTxn.commitTransaction();
        assertTrue(txn == txn1);
        assertTrue(txn == currentTxn.getTransaction());
        assertEquals(THREE, map.get(THREE));
        assertEquals(TWO, map.get(TWO));
        assertEquals(ONE, map.get(ONE));

        txn = currentTxn.commitTransaction();
        assertNull(txn);
        assertNull(currentTxn.getTransaction());
        assertEquals(THREE, map.get(THREE));
        assertEquals(TWO, map.get(TWO));
        assertEquals(ONE, map.get(ONE));
    }

    @Test
    public void testRunnerCommit()
        throws Exception {

        commitTest(false);
    }

    @Test
    public void testExplicitCommit()
        throws Exception {

        commitTest(true);
    }

    private void commitTest(final boolean explicit)
        throws Exception {

        final TransactionRunner runner = new TransactionRunner(env);
        runner.setAllowNestedTransactions(DbCompat.NESTED_TRANSACTIONS);

        assertNull(currentTxn.getTransaction());

        runner.run(new TransactionWorker() {
            public void doWork() throws Exception {
                final STransaction txn1 = currentTxn.getTransaction();
                assertNotNull(txn1);
                assertNull(map.put(ONE, ONE));
                assertEquals(ONE, map.get(ONE));

                runner.run(new TransactionWorker() {
                    public void doWork() throws Exception {
                        final STransaction txn2 = currentTxn.getTransaction();
                        assertNotNull(txn2);
                        if (DbCompat.NESTED_TRANSACTIONS) {
                            assertTrue(txn1 != txn2);
                        } else {
                            assertTrue(txn1 == txn2);
                        }
                        assertNull(map.put(TWO, TWO));
                        assertEquals(TWO, map.get(TWO));
                        assertEquals(ONE, map.get(ONE));
                        if (DbCompat.NESTED_TRANSACTIONS && explicit) {
                            currentTxn.commitTransaction();
                        }
                    }
                });

                STransaction txn3 = currentTxn.getTransaction();
                assertSame(txn1, txn3);

                assertEquals(TWO, map.get(TWO));
                assertEquals(ONE, map.get(ONE));
            }
        });

        assertNull(currentTxn.getTransaction());
    }

    @Test
    public void testRunnerAbort()
        throws Exception {

        abortTest(false);
    }

    @Test
    public void testExplicitAbort()
        throws Exception {

        abortTest(true);
    }

    private void abortTest(final boolean explicit)
        throws Exception {

        final TransactionRunner runner = new TransactionRunner(env);
        runner.setAllowNestedTransactions(DbCompat.NESTED_TRANSACTIONS);

        assertNull(currentTxn.getTransaction());

        runner.run(new TransactionWorker() {
            public void doWork() throws Exception {
                final STransaction txn1 = currentTxn.getTransaction();
                assertNotNull(txn1);
                assertNull(map.put(ONE, ONE));
                assertEquals(ONE, map.get(ONE));

                if (DbCompat.NESTED_TRANSACTIONS) {
                    try {
                        runner.run(new TransactionWorker() {
                            public void doWork() throws Exception {
                                final STransaction txn2 =
                                        currentTxn.getTransaction();
                                assertNotNull(txn2);
                                assertTrue(txn1 != txn2);
                                assertNull(map.put(TWO, TWO));
                                assertEquals(TWO, map.get(TWO));
                                if (explicit) {
                                    currentTxn.abortTransaction();
                                } else {
                                    throw new IllegalArgumentException(
                                                                "test-abort");
                                }
                            }
                        });
                        assertTrue(explicit);
                    } catch (IllegalArgumentException e) {
                        assertTrue(!explicit);
                        assertEquals("test-abort", e.getMessage());
                    }
                }

                STransaction txn3 = currentTxn.getTransaction();
                assertSame(txn1, txn3);

                assertEquals(ONE, map.get(ONE));
                assertNull(map.get(TWO));
            }
        });

        assertNull(currentTxn.getTransaction());
    }

    @Test
    public void testReadCommittedCollection()
        throws Exception {

        StoredSortedMap degree2Map = (StoredSortedMap)
            StoredCollections.configuredSortedMap
                (map, SCursorConfig.READ_COMMITTED);

        // original map is not read-committed
        assertTrue(!isReadCommitted(map));

        // all read-committed containers are read-uncommitted
        assertTrue(isReadCommitted(degree2Map));
        assertTrue(isReadCommitted
            (StoredCollections.configuredMap
                (map, SCursorConfig.READ_COMMITTED)));
        assertTrue(isReadCommitted
            (StoredCollections.configuredCollection
                (map.values(), SCursorConfig.READ_COMMITTED)));
        assertTrue(isReadCommitted
            (StoredCollections.configuredSet
                (map.keySet(), SCursorConfig.READ_COMMITTED)));
        assertTrue(isReadCommitted
            (StoredCollections.configuredSortedSet
                ((SortedSet) map.keySet(),
                 SCursorConfig.READ_COMMITTED)));

        if (DbCompat.RECNO_METHOD) {
            // create a list just so we can call configuredList()
            SDatabase listStore = TestStore.RECNO_RENUM.open(env, "foo");
            List list = new StoredList(listStore, TestStore.VALUE_BINDING,
                                       true);
            assertTrue(isReadCommitted
                (StoredCollections.configuredList
                    (list, SCursorConfig.READ_COMMITTED)));
            listStore.close();
        }

        map.put(ONE, ONE);
        doReadCommitted(degree2Map, null);
    }

    private static boolean isReadCommitted(Object container) {
        StoredContainer storedContainer = (StoredContainer) container;
        /* We can't use getReadCommitted until is is added to DB core. */
        return storedContainer.getCursorConfig() != null &&
               storedContainer.getCursorConfig().getReadCommitted();
    }

    @Test
    public void testReadCommittedTransaction()
        throws Exception {

        STransactionConfig config = new STransactionConfig();
        config.setReadCommitted(true);
        doReadCommitted(map, config);
    }

    private void doReadCommitted(final StoredSortedMap degree2Map,
                                 STransactionConfig txnConfig)
        throws Exception {

        map.put(ONE, ONE);
        TransactionRunner runner = new TransactionRunner(env);
        runner.setTransactionConfig(txnConfig);
        assertNull(currentTxn.getTransaction());
        runner.run(new TransactionWorker() {
            public void doWork() throws Exception {
                assertNotNull(currentTxn.getTransaction());

                /* Do a read-committed get(), the lock is not retained. */
                assertEquals(ONE, degree2Map.get(ONE));

                /*
                 * If we were not using read-committed, the following write of
                 * key ONE with an auto-commit transaction would self-deadlock
                 * since two transactions in the same thread would be
                 * attempting to lock the same key, one for write and one for
                 * read.  This test passes if we do not deadlock.
                 */
                SDatabaseEntry key = new SDatabaseEntry();
                SDatabaseEntry value = new SDatabaseEntry();
                testStore.getKeyBinding().objectToEntry(ONE, key);
                testStore.getValueBinding().objectToEntry(TWO, value);
                store.put(null, key, value);
            }
        });
        assertNull(currentTxn.getTransaction());
    }

    @Test
    public void testReadUncommittedCollection()
        throws Exception {

        StoredSortedMap dirtyMap = (StoredSortedMap)
            StoredCollections.configuredSortedMap
                (map, SCursorConfig.READ_UNCOMMITTED);

        // original map is not read-uncommitted
        assertTrue(!isReadUncommitted(map));

        // all read-uncommitted containers are read-uncommitted
        assertTrue(isReadUncommitted(dirtyMap));
        assertTrue(isReadUncommitted
            (StoredCollections.configuredMap
                (map, SCursorConfig.READ_UNCOMMITTED)));
        assertTrue(isReadUncommitted
            (StoredCollections.configuredCollection
                (map.values(), SCursorConfig.READ_UNCOMMITTED)));
        assertTrue(isReadUncommitted
            (StoredCollections.configuredSet
                (map.keySet(), SCursorConfig.READ_UNCOMMITTED)));
        assertTrue(isReadUncommitted
            (StoredCollections.configuredSortedSet
                ((SortedSet) map.keySet(), SCursorConfig.READ_UNCOMMITTED)));

        if (DbCompat.RECNO_METHOD) {
            // create a list just so we can call configuredList()
            SDatabase listStore = TestStore.RECNO_RENUM.open(env, "foo");
            List list = new StoredList(listStore, TestStore.VALUE_BINDING,
                                       true);
            assertTrue(isReadUncommitted
                (StoredCollections.configuredList
                    (list, SCursorConfig.READ_UNCOMMITTED)));
            listStore.close();
        }

        doReadUncommitted(dirtyMap);
    }

    private static boolean isReadUncommitted(Object container) {
        StoredContainer storedContainer = (StoredContainer) container;
        return storedContainer.getCursorConfig() != null &&
               storedContainer.getCursorConfig().getReadUncommitted();
    }

    @Test
    public void testReadUncommittedTransaction()
        throws Exception {

        TransactionRunner runner = new TransactionRunner(env);
        STransactionConfig config = new STransactionConfig();
        config.setReadUncommitted(true);
        runner.setTransactionConfig(config);
        assertNull(currentTxn.getTransaction());
        runner.run(new TransactionWorker() {
            public void doWork() throws Exception {
                assertNotNull(currentTxn.getTransaction());
                doReadUncommitted(map);
            }
        });
        assertNull(currentTxn.getTransaction());
    }

    /**
     * Tests that the CurrentTransaction static WeakHashMap does indeed allow
     * GC to reclaim tine environment when it is closed.  At one point this was
     * not working because the value object in the map has a reference to the
     * environment.  This was fixed by wrapping the SEnvironment in a
     * WeakReference.  [#15444]
     *
     * This test only succeeds intermittently, probably due to its reliance
     * on the GC call.
     */
    @Test
    public void testCurrentTransactionGC()
        throws Exception {

        /*
         * This test can have indeterminate results because it depends on
         * a finalize count, so it's not part of the default run.
         */
        if (!SharedTestUtils.runLongTests()) {
            return;
        }

        final StringBuilder finalizedFlag = new StringBuilder();

        class MyEnv extends SEnvironment {

            /**
             * @throws FileNotFoundException from DB core.
             */
            MyEnv(File home, SEnvironmentConfig config)
                throws SDatabaseException, FileNotFoundException {

                super(create(home, config));
            }

            @Override
            protected void finalize() {
                finalizedFlag.append('.');
            }
        }

        MyEnv myEnv = new MyEnv(new File(env.getHome()), env.getConfig());
        CurrentTransaction myCurrTxn = CurrentTransaction.getInstance(myEnv);

        store.close();
        store = null;
        map = null;

        env.close();
        env = null;

        myEnv.close();
        myEnv = null;

        myCurrTxn = null;
        currentTxn = null;

        for (int i = 0; i < 10; i += 1) {
            byte[] x = null;
            try {
                 x = new byte[Integer.MAX_VALUE - 1];
            } catch (OutOfMemoryError expected) {
            }
            assertNull(x);
            System.gc();
        }

        for (int i = 0; i < 10; i += 1) {
            System.gc();
        }

        assertTrue(finalizedFlag.length() > 0);
    }

    private synchronized void doReadUncommitted(StoredSortedMap dirtyMap)
        throws Exception {

        // start thread one
        ReadUncommittedThreadOne t1 = new ReadUncommittedThreadOne(env, this);
        t1.start();
        wait();

        // put ONE
        synchronized (t1) { t1.notify(); }
        wait();
        readCheck(dirtyMap, ONE, ONE);
        assertTrue(!dirtyMap.isEmpty());

        // abort ONE
        synchronized (t1) { t1.notify(); }
        t1.join();
        readCheck(dirtyMap, ONE, null);
        assertTrue(dirtyMap.isEmpty());

        // start thread two
        ReadUncommittedThreadTwo t2 = new ReadUncommittedThreadTwo(env, this);
        t2.start();
        wait();

        // put TWO
        synchronized (t2) { t2.notify(); }
        wait();
        readCheck(dirtyMap, TWO, TWO);
        assertTrue(!dirtyMap.isEmpty());

        // commit TWO
        synchronized (t2) { t2.notify(); }
        t2.join();
        readCheck(dirtyMap, TWO, TWO);
        assertTrue(!dirtyMap.isEmpty());
    }

    private static class ReadUncommittedThreadOne extends Thread {

        private final CurrentTransaction currentTxn;
        private final TransactionTest parent;
        private final StoredSortedMap map;

        private ReadUncommittedThreadOne(SEnvironment env,
                                         TransactionTest parent) {

            this.currentTxn = CurrentTransaction.getInstance(env);
            this.parent = parent;
            this.map = parent.map;
        }

        @Override
        public synchronized void run() {

            try {
                assertNull(currentTxn.getTransaction());
                assertNotNull(currentTxn.beginTransaction(null));
                assertNotNull(currentTxn.getTransaction());
                readCheck(map, ONE, null);
                synchronized (parent) { parent.notify(); }
                wait();

                // put ONE
                assertNull(map.put(ONE, ONE));
                readCheck(map, ONE, ONE);
                synchronized (parent) { parent.notify(); }
                wait();

                // abort ONE
                assertNull(currentTxn.abortTransaction());
                assertNull(currentTxn.getTransaction());
            } catch (Exception e) {
                throw new RuntimeExceptionWrapper(e);
            }
        }
    }

    private static class ReadUncommittedThreadTwo extends Thread {

        private final SEnvironment env;
        private final CurrentTransaction currentTxn;
        private final TransactionTest parent;
        private final StoredSortedMap map;

        private ReadUncommittedThreadTwo(SEnvironment env,
                                         TransactionTest parent) {

            this.env = env;
            this.currentTxn = CurrentTransaction.getInstance(env);
            this.parent = parent;
            this.map = parent.map;
        }

        @Override
        public synchronized void run() {

            try {
                final TransactionRunner runner = new TransactionRunner(env);
                final Object thread = this;
                assertNull(currentTxn.getTransaction());

                runner.run(new TransactionWorker() {
                    public void doWork() throws Exception {
                        assertNotNull(currentTxn.getTransaction());
                        readCheck(map, TWO, null);
                        synchronized (parent) { parent.notify(); }
                        thread.wait();

                        // put TWO
                        assertNull(map.put(TWO, TWO));
                        readCheck(map, TWO, TWO);
                        synchronized (parent) { parent.notify(); }
                        thread.wait();

                        // commit TWO
                    }
                });
                assertNull(currentTxn.getTransaction());
            } catch (Exception e) {
                throw new RuntimeExceptionWrapper(e);
            }
        }
    }

    private static void readCheck(StoredSortedMap checkMap, Object key,
                                  Object expect) {
        if (expect == null) {
            assertNull(checkMap.get(key));
            assertTrue(checkMap.tailMap(key).isEmpty());
            assertTrue(!checkMap.tailMap(key).containsKey(key));
            assertTrue(!checkMap.keySet().contains(key));
            assertTrue(checkMap.duplicates(key).isEmpty());
            Iterator i = checkMap.keySet().iterator();
            try {
                while (i.hasNext()) {
                    assertTrue(!key.equals(i.next()));
                }
            } finally { StoredIterator.close(i); }
        } else {
            assertEquals(expect, checkMap.get(key));
            assertEquals(expect, checkMap.tailMap(key).get(key));
            assertTrue(!checkMap.tailMap(key).isEmpty());
            assertTrue(checkMap.tailMap(key).containsKey(key));
            assertTrue(checkMap.keySet().contains(key));
            assertTrue(checkMap.values().contains(expect));
            assertTrue(!checkMap.duplicates(key).isEmpty());
            assertTrue(checkMap.duplicates(key).contains(expect));
            Iterator i = checkMap.keySet().iterator();
            try {
                boolean found = false;
                while (i.hasNext()) {
                    if (expect.equals(i.next())) {
                        found = true;
                    }
                }
                assertTrue(found);
            }
            finally { StoredIterator.close(i); }
        }
    }

    /**
     * Tests transaction retries performed by TransationRunner.
     *
     * This test is too sensitive to how lock conflict detection works on JE to
     * make it work properly on DB core.
     */

    /**
     * Tests transaction retries performed by TransationRunner.
     *
     * This test is too sensitive to how lock conflict detection works on JE to
     * make it work properly on DB core.
     */
}
