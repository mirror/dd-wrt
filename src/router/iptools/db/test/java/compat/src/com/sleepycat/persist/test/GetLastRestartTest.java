/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.persist.test;

import static com.sleepycat.persist.model.Relationship.MANY_TO_ONE;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.List;
import java.util.concurrent.TimeUnit;

import com.sleepycat.compat.DbCompat;
import org.junit.After;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DeadlockException;
import com.sleepycat.db.Transaction;
import com.sleepycat.persist.EntityCursor;
import com.sleepycat.persist.EntityIndex;
import com.sleepycat.persist.EntityStore;
import com.sleepycat.persist.PrimaryIndex;
import com.sleepycat.persist.SecondaryIndex;
import com.sleepycat.persist.StoreConfig;
import com.sleepycat.persist.model.Entity;
import com.sleepycat.persist.model.PrimaryKey;
import com.sleepycat.persist.model.SecondaryKey;
import com.sleepycat.util.test.TxnTestCase;

/**
 * Tests that getLast restarts work correctly. See RangeCursor.getLast.
 *
 * This tests getLast via the DPL API simply because it's convenient. It could
 * have been tested via the collections API, or directly using RangeCursor.
 *
 * @author Mark Hayes
 */
@RunWith(Parameterized.class)
public class GetLastRestartTest extends TxnTestCase {

    private static final int N_ITERS = 5000;

    @Entity
    static class MyEntity {

        @PrimaryKey
        private int priKey;

        @SecondaryKey(relate=MANY_TO_ONE)
        private Integer secKey;

        private MyEntity() {}

        MyEntity(final int priKey, final Integer secKey) {
            this.priKey = priKey;
            this.secKey = secKey;
        }
    }

    private EntityStore store;
    private PrimaryIndex<Integer, MyEntity> priIndex;
    private SecondaryIndex<Integer, Integer, MyEntity> secIndex;
    private volatile Thread insertThread;
    private volatile Exception insertException;

    @Parameters
    public static List<Object[]> genParams() {

        /* TXN_NULL in DB doesn't support multi-threading. */

        String[] txnTypes = new String[] {
            TxnTestCase.TXN_USER,
            TxnTestCase.TXN_AUTO,
            TxnTestCase.TXN_CDB };


        return getTxnParams(txnTypes, false);
    }

    public GetLastRestartTest(String type)
        throws DatabaseException {

        initEnvConfig();

        DbCompat.enableDeadlockDetection(envConfig, type.equals(TXN_CDB));

        /*
         * Use large lock timeout because getLast is retrying in a loop while
         * it holds a lock, since it doesn't release locks when it restarts the
         * operation. This is a disadvantage of implementing getLast on top of
         * the Cursor API, rather than in the cursor code. However, the looping
         * is more of a problem in this test than it should be in the real
         * life, because here we're tightly looping in another thread,
         * inserting/deleting a single record.
         */

        txnType = type;
        isTransactional = (txnType != TXN_NULL);
        customName = txnType;
    }

    private void open()
        throws DatabaseException {

        StoreConfig config = new StoreConfig();
        config.setAllowCreate(envConfig.getAllowCreate());
        config.setTransactional(envConfig.getTransactional());
        store = new EntityStore(env, "test", config);
        priIndex = store.getPrimaryIndex(Integer.class, MyEntity.class);
        secIndex = store.getSecondaryIndex(priIndex, Integer.class, "secKey");
    }

    private void close()
        throws DatabaseException {

        try {
            store.close();
        } finally {
            store = null;
            priIndex = null;
            secIndex = null;
        }
    }

    @Override
    @After
    public void tearDown()
        throws Exception {

        final boolean stoppedInserts = stopInserts();

        try {
            if (store != null) {
                close();
            }
        } catch (Throwable e) {
            System.out.println("During tearDown: " + e);
        }

        super.tearDown();

        if (!stoppedInserts) {
            fail("Could not kill insert thread");
        }

        if (insertException != null) {
            throw insertException;
        }
    }

    /**
     * Keys: 1, 2. Range: (-, 3) Expect: getLast == 2.
     *
     * RangeCursor.getLast calls getSearchKeyRange(3) which returns NOTFOUND.
     * It calls getLast, but insertThread has inserted key 3, so getLast lands
     * on key 3 which is outside the range. It must restart.
     */
    @Test
    public void testMainKeyRangeNoDups_GetLast()
        throws DatabaseException, InterruptedException {

        open();

        insert(1);
        insert(2);
        startInserts(3);
        checkRange(
            3 /*endKey*/, false /*endInclusive*/,
            2 /*expectLastKey*/);

        assertTrue(stopInserts());
        close();
    }

    /**
     * Keys: 1, 2, 4. Range: (-, 3) Expect: getLast == 2.
     *
     * RangeCursor.getLast calls getSearchKeyRange(3) which lands on key 4. It
     * calls getPrev, but insertThread has inserted key 3, so getPrev lands on
     * key 3 which is outside the range. It must restart.
     */
    @Test
    public void testMainKeyRangeNoDups_GetPrev()
        throws DatabaseException, InterruptedException {

        open();

        insert(1);
        insert(2);
        insert(4);
        startInserts(3);
        checkRange(
            3 /*endKey*/, false /*endInclusive*/,
            2 /*expectLastKey*/);

        assertTrue(stopInserts());
        close();
    }

    /**
     * Records: 1/1, 2/2, 3/2. SecRange: [2, 2] Expect: getLast == 3/2.
     *
     * RangeCursor.getLast calls getSearchKeyRange(2) which returns 2/2. It
     * calls getNextNoDup which returns NOTFOUND. It calls getLast, but
     * insertThread has inserted key 4/3, so getLast lands on key 4/3 which is
     * outside the range. It must restart.
     */
    @Test
    public void testMainKeyRangeWithDups_GetLast()
        throws DatabaseException, InterruptedException {

        open();

        insert(1, 1);
        insert(2, 2);
        insert(3, 2);
        startInserts(4, 3);
        checkSecRange(
            2 /*secKey*/,
            3 /*expectLastPKey*/, 2 /*expectLastSecKey*/);

        assertTrue(stopInserts());
        close();
    }

    /**
     * Records: 1/1, 2/2, 3/2, 4/4. SecRange: [2, 2] Expect: getLast == 3/2.
     *
     * RangeCursor.getLast calls getSearchKeyRange(2) which returns 2/2. It
     * calls getNextNoDup which returns 4/4. It calls getPrev, but insertThread
     * has inserted key 5/3, so getPrev lands on key 5/3 which is outside the
     * range. It must restart.
     */
    @Test
    public void testMainKeyRangeWithDups_GetPrev()
        throws DatabaseException, InterruptedException {

        open();

        insert(1, 1);
        insert(2, 2);
        insert(3, 2);
        insert(4, 4);
        startInserts(5, 3);
        checkSecRange(
            2 /*secKey*/,
            3 /*expectLastPKey*/, 2 /*expectLastSecKey*/);

        assertTrue(stopInserts());
        close();
    }

    /**
     * Records: 1/1, 2/2, 3/2. SecRange: [2, 2] PKeyRange: [2, -)
     * Expect: getLast == 3/2.
     *
     * RangeCursor.getLast calls getSearchKey(2) which returns 2/2. It calls
     * getNextNoDup which returns NOTFOUND. It calls getLast, but insertThread
     * has inserted key 4/3, so getLast lands on key 4/3 which is outside the
     * range. It must restart.
     */
    @Test
    public void testDupRangeNoEndKey_GetLast()
        throws DatabaseException, InterruptedException {

        open();

        insert(1, 1);
        insert(2, 2);
        insert(3, 2);
        startInserts(4, 3);
        checkPKeyRange(
            2 /*secKey*/,
            2 /*beginPKey*/, true /*beginInclusive*/,
            null /*endPKey*/, false /*endInclusive*/,
            3 /*expectLastPKey*/, 2 /*expectLastSecKey*/);

        assertTrue(stopInserts());
        close();
    }

    /**
     * Records: 1/1, 2/2, 3/2, 4/4. SecRange: [2, 2] PKeyRange: [2, -)
     * Expect: getLast == 3/2.
     *
     * RangeCursor.getLast calls getSearchKey(2) which returns 2/2. It calls
     * getNextNoDup which returns 4/4. It calls getPrev, but insertThread has
     * inserted key 5/3, so getPrev lands on key 5/3 which is outside the
     * range. It must restart.
     */
    @Test
    public void testDupRangeNoEndKey_GetPrev()
        throws DatabaseException, InterruptedException {

        open();

        insert(1, 1);
        insert(2, 2);
        insert(3, 2);
        insert(4, 4);
        startInserts(5, 3);
        checkPKeyRange(
            2 /*secKey*/,
            2 /*beginPKey*/, true /*beginInclusive*/,
            null /*endPKey*/, false /*endInclusive*/,
            3 /*expectLastPKey*/, 2 /*expectLastSecKey*/);

        assertTrue(stopInserts());
        close();
    }

    /**
     * Records: 1/1, 2/2, 3/2, 5/2. SecRange: [2, 2] PKeyRange: (-, 4)
     * Expect: getLast == 3/2.
     *
     * RangeCursor.getLast calls getSearchBothRange(2, 4) which returns 5/2. It
     * calls getPrevDup, but insertThread has inserted key 4/2, so getPrevDup
     * lands on key 4/2 which is outside the range. It must restart.
     */
    @Test
    public void testDupRangeWithEndKey()
        throws DatabaseException, InterruptedException {

        open();

        insert(1, 1);
        insert(2, 2);
        insert(3, 2);
        insert(5, 2);
        startInserts(4, 2);
        checkPKeyRange(
            2 /*secKey*/,
            null /*beginPKey*/, false /*beginInclusive*/,
            4 /*endPKey*/, false /*endInclusive*/,
            3 /*expectLastPKey*/, 2 /*expectLastSecKey*/);

        assertTrue(stopInserts());
        close();
    }

    private void checkRange(int endKey,
                            boolean endInclusive,
                            int expectLastKey)
        throws DatabaseException {

        for (int i = 0; i < N_ITERS; i += 1) {
            final Transaction txn = txnBeginCursor();

            final EntityCursor<MyEntity> c = priIndex.entities(
                txn, null, false, endKey, endInclusive, null);

            try {
                final MyEntity e = c.last();
                assertNotNull(e);
                assertEquals(expectLastKey, e.priKey);
            } finally {
                c.close();
                txnCommit(txn);
            }
        }
    }

    private void checkSecRange(int secKey,
                               int expectLastPKey,
                               Integer expectLastSecKey)
        throws DatabaseException {

        final EntityIndex<Integer, MyEntity> subIndex =
            secIndex.subIndex(secKey);

        for (int i = 0; i < N_ITERS; i += 1) {
            final Transaction txn = txnBeginCursor();

            final EntityCursor<MyEntity> c = subIndex.entities(txn, null);

            try {
                final MyEntity e = c.last();
                assertNotNull(e);
                assertEquals(expectLastPKey, e.priKey);
                assertEquals(expectLastSecKey, e.secKey);
            } finally {
                c.close();
                txnCommit(txn);
            }
        }
    }

    private void checkPKeyRange(int secKey,
                                Integer beginPKey,
                                boolean beginInclusive,
                                Integer endPKey,
                                boolean endInclusive,
                                int expectLastPKey,
                                Integer expectLastSecKey)
        throws DatabaseException {

        final EntityIndex<Integer, MyEntity> subIndex =
            secIndex.subIndex(secKey);

        for (int i = 0; i < N_ITERS; i += 1) {
            final Transaction txn = txnBeginCursor();

            final EntityCursor<MyEntity> c = subIndex.entities(
                txn, beginPKey, beginInclusive, endPKey, endInclusive, null);

            try {
                final MyEntity e = c.last();
                assertNotNull(e);
                assertEquals(expectLastPKey, e.priKey);
                assertEquals(expectLastSecKey, e.secKey);
            } finally {
                c.close();
                txnCommit(txn);
            }
        }
    }

    private void startInserts(int priKey)
        throws DatabaseException {

        startInserts(priKey, null);
    }

    private void startInserts(final int priKey, final Integer secKey)
        throws DatabaseException {

        final EntityIndex<Integer, MyEntity> subIndex =
            (secKey != null) ?
            secIndex.subIndex(secKey) :
            null;

        insertThread = new Thread() {
            @Override
            public void run() {
                try {
                    while (insertThread != null) {
                        try {
                            if (secKey != null) {
                                insert(priKey, secKey);
                                subIndex.delete(priKey);
                            } else {
                                insert(priKey);
                                priIndex.delete(priKey);
                            }
                        } catch (DeadlockException e) {
                            // ignore on DB, but not JE.
                        }
                    }
                } catch (Exception e) {
                    insertException = e;
                }
            }
        };

        insertThread.start();
    }

    private boolean stopInserts()
        throws InterruptedException {

        if (insertThread == null) {
            return true;
        }

        final Thread t = insertThread;
        insertThread = null;

        /*
         * First try stopping without interrupts, since they will invalidate
         * the Environment.
         */
        long start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 10 * 1000) {
            if (!t.isAlive()) {
                return true;
            }
            Thread.sleep(10);
        }

        /* Try thread interrupts as a last resort. */
        start = System.currentTimeMillis();
        while (System.currentTimeMillis() - start < 10 * 1000) {
            t.interrupt();
            Thread.sleep(10);
            if (!t.isAlive()) {
                return true;
            }
        }

        return false;
    }

    private void insert(int priKey)
        throws DatabaseException {

        insert(priKey, null);
    }

    private void insert(int priKey, Integer secKey)
        throws DatabaseException {

        priIndex.put(new MyEntity(priKey, secKey));
    }
}
