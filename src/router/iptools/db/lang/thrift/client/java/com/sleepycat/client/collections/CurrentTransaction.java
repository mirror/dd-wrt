/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.WeakHashMap;

import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.STransactionConfig;
import com.sleepycat.client.util.RuntimeExceptionWrapper;

/**
 * Provides access to the current transaction for the current thread within the
 * context of a Berkeley DB environment.  This class provides explicit
 * transaction control beyond that provided by the {@link TransactionRunner}
 * class.  However, both methods of transaction control manage per-thread
 * transactions.
 *
 * @author Mark Hayes
 */
public class CurrentTransaction {

    /* For internal use, this class doubles as an SEnvironment wrapper. */

    private static WeakHashMap<SEnvironment, CurrentTransaction> envMap =
        new WeakHashMap<SEnvironment, CurrentTransaction>();

    private SLockMode writeLockMode;
    private boolean cdbMode;
    private boolean txnMode;
    private boolean lockingMode;
    private ThreadLocal localTrans = new ThreadLocal();
    private ThreadLocal localCdbCursors;

    /*
     * Use a WeakReference to the SEnvironment to avoid pinning the environment
     * in the envMap.  The WeakHashMap envMap uses the SEnvironment as a weak
     * key, but this won't prevent GC of the SEnvironment if the map's value has
     * a hard reference to the SEnvironment.  [#15444]
     */
    private WeakReference<SEnvironment> envRef;

    /**
     * Gets the CurrentTransaction accessor for a specified Berkeley DB
     * environment.  This method always returns the same reference when called
     * more than once with the same environment parameter.
     *
     * @param env is an open Berkeley DB environment.
     *
     * @return the CurrentTransaction accessor for the given environment, or
     * null if the environment is not transactional.
     */
    public static CurrentTransaction getInstance(SEnvironment env) {

        CurrentTransaction currentTxn = getInstanceInternal(env);
        return currentTxn.isTxnMode() ? currentTxn : null;
    }

    /**
     * Gets the CurrentTransaction accessor for a specified Berkeley DB
     * environment.  Unlike getInstance(), this method never returns null.
     *
     * @param env is an open Berkeley DB environment.
     */
    static CurrentTransaction getInstanceInternal(SEnvironment env) {
        synchronized (envMap) {
            CurrentTransaction ct = envMap.get(env);
            if (ct == null) {
                ct = new CurrentTransaction(env);
                envMap.put(env, ct);
            }
            return ct;
        }
    }

    private CurrentTransaction(SEnvironment env) {
        envRef = new WeakReference<SEnvironment>(env);
        try {
            SEnvironmentConfig config = env.getConfig();
            txnMode = config.getTransactional();
            lockingMode = DbCompat.getInitializeLocking(config);
            if (txnMode || lockingMode) {
                writeLockMode = SLockMode.RMW;
            } else {
                writeLockMode = SLockMode.DEFAULT;
            }
            cdbMode = DbCompat.getInitializeCDB(config);
            if (cdbMode) {
                localCdbCursors = new ThreadLocal();
            }
        } catch (SDatabaseException e) {
            throw RuntimeExceptionWrapper.wrapIfNeeded(e);
        }
    }

    /**
     * Returns whether environment is configured for locking.
     */
    final boolean isLockingMode() {

        return lockingMode;
    }

    /**
     * Returns whether this is a transactional environment.
     */
    final boolean isTxnMode() {

        return txnMode;
    }

    /**
     * Returns whether this is a Concurrent Data Store environment.
     */
    final boolean isCdbMode() {

        return cdbMode;
    }

    /**
     * Return the SLockMode.RMW or null, depending on whether locking is
     * enabled.  SLockMode.RMW will cause an error if passed when locking
     * is not enabled.  Locking is enabled if locking or transactions were
     * specified for this environment.
     */
    final SLockMode getWriteLockMode() {

        return writeLockMode;
    }

    /**
     * Returns the underlying Berkeley DB environment.
     *
     * @return the SEnvironment.
     */
    public final SEnvironment getEnvironment() {

        return envRef.get();
    }

    /**
     * Returns the transaction associated with the current thread for this
     * environment, or null if no transaction is active.
     *
     * @return the STransaction.
     */
    public final STransaction getTransaction() {

        Trans trans = (Trans) localTrans.get();
        return (trans != null) ? trans.txn : null;
    }

    /**
     * Returns whether auto-commit may be performed by the collections API.
     * True is returned if no collections API transaction is currently active,
     * and no XA transaction is currently active.
     */
    boolean isAutoCommitAllowed()
        throws SDatabaseException {

        return getTransaction() == null &&
               DbCompat.getThreadTransaction(getEnvironment()) == null;
    }

    /**
     * Begins a new transaction for this environment and associates it with
     * the current thread.  If a transaction is already active for this
     * environment and thread, a nested transaction will be created.
     *
     * @param config the transaction configuration used for calling
     * {@link SEnvironment#beginTransaction}, or null to use the default
     * configuration.
     *
     * @return the new transaction.
     *
     *
     * @throws SDatabaseException if the transaction cannot be started, in which
     * case any existing transaction is not affected.
     *
     * @throws IllegalStateException if a transaction is already active and
     * nested transactions are not supported by the environment.
     */
    public final STransaction beginTransaction(STransactionConfig config)
        throws SDatabaseException {

        SEnvironment env = getEnvironment();
        Trans trans = (Trans) localTrans.get();
        if (trans != null) {
            if (trans.txn != null) {
                if (!DbCompat.NESTED_TRANSACTIONS) {
                    throw new IllegalStateException
                        ("Nested transactions are not supported");
                }
                STransaction parentTxn = trans.txn;
                trans = new Trans(trans, config);
                trans.txn = env.beginTransaction(parentTxn, config);
                localTrans.set(trans);
            } else {
                trans.txn = env.beginTransaction(null, config);
                trans.config = config;
            }
        } else {
            trans = new Trans(null, config);
            trans.txn = env.beginTransaction(null, config);
            localTrans.set(trans);
        }
        return trans.txn;
    }

    /**
     * Commits the transaction that is active for the current thread for this
     * environment and makes the parent transaction (if any) the current
     * transaction.
     *
     * @return the parent transaction or null if the committed transaction was
     * not nested.
     *
     *
     * @throws SDatabaseException if an error occurs committing the transaction.
     * The transaction will still be closed and the parent transaction will
     * become the current transaction.
     *
     * @throws IllegalStateException if no transaction is active for the
     * current thread for this environment.
     */
    public final STransaction commitTransaction()
        throws SDatabaseException, IllegalStateException {

        Trans trans = (Trans) localTrans.get();
        if (trans != null && trans.txn != null) {
            STransaction parent = closeTxn(trans);
            trans.txn.commit();
            return parent;
        } else {
            throw new IllegalStateException("No transaction is active");
        }
    }

    /**
     * Aborts the transaction that is active for the current thread for this
     * environment and makes the parent transaction (if any) the current
     * transaction.
     *
     * @return the parent transaction or null if the aborted transaction was
     * not nested.
     *
     *
     * @throws SDatabaseException if an error occurs aborting the transaction.
     * The transaction will still be closed and the parent transaction will
     * become the current transaction.
     *
     * @throws IllegalStateException if no transaction is active for the
     * current thread for this environment.
     */
    public final STransaction abortTransaction()
        throws SDatabaseException, IllegalStateException {

        Trans trans = (Trans) localTrans.get();
        if (trans != null && trans.txn != null) {
            STransaction parent = closeTxn(trans);
            trans.txn.abort();
            return parent;
        } else {
            throw new IllegalStateException("No transaction is active");
        }
    }

    /**
     * Returns whether the current transaction is a readUncommitted
     * transaction.
     */
    final boolean isReadUncommitted() {

        Trans trans = (Trans) localTrans.get();
        if (trans != null && trans.config != null) {
            return trans.config.getReadUncommitted();
        } else {
            return false;
        }
    }

    private STransaction closeTxn(Trans trans) {

        localTrans.set(trans.parent);
        return (trans.parent != null) ? trans.parent.txn : null;
    }

    private static class Trans {

        private Trans parent;
        private STransaction txn;
        private STransactionConfig config;

        private Trans(Trans parent, STransactionConfig config) {

            this.parent = parent;
            this.config = config;
        }
    }

    /**
     * Opens a cursor for a given database, dup'ing an existing CDB cursor if
     * one is open for the current thread.
     */
    SCursor openCursor(SDatabase db,
                      SCursorConfig cursorConfig,
                      boolean writeCursor,
                      STransaction txn)
        throws SDatabaseException {

        if (cdbMode) {
            CdbCursors cdbCursors = null;
            WeakHashMap cdbCursorsMap = (WeakHashMap) localCdbCursors.get();
            if (cdbCursorsMap == null) {
                cdbCursorsMap = new WeakHashMap();
                localCdbCursors.set(cdbCursorsMap);
            } else {
                cdbCursors = (CdbCursors) cdbCursorsMap.get(db);
            }
            if (cdbCursors == null) {
                cdbCursors = new CdbCursors();
                cdbCursorsMap.put(db, cdbCursors);
            }

            /*
             * In CDB mode the cursorConfig specified by the user is ignored
             * and only the writeCursor parameter is honored.  This is the only
             * meaningful cursor attribute for CDB, and here we count on
             * writeCursor flag being set correctly by the caller.
             */
            List cursors;
            SCursorConfig cdbConfig;
            if (writeCursor) {
                if (cdbCursors.readCursors.size() > 0) {

                    /*
                     * Although CDB allows opening a write cursor when a read
                     * cursor is open, a self-deadlock will occur if a write is
                     * attempted for a record that is read-locked; we should
                     * avoid self-deadlocks at all costs
                     */
                    throw new IllegalStateException
                        ("Cannot open CDB write cursor when read cursor " +
                         "is open");
                }
                cursors = cdbCursors.writeCursors;
                cdbConfig = new SCursorConfig();
                DbCompat.setWriteCursor(cdbConfig, true);
            } else {
                cursors = cdbCursors.readCursors;
                cdbConfig = null;
            }
            SCursor cursor;
            if (cursors.size() > 0) {
                SCursor other = ((SCursor) cursors.get(0));
                cursor = other.dup(false);
            } else {
                cursor = db.openCursor(null, cdbConfig);
            }
            cursors.add(cursor);
            return cursor;
        } else {
            return db.openCursor(txn, cursorConfig);
        }
    }

    /**
     * Duplicates a cursor for a given database.
     *
     * @param writeCursor true to open a write cursor in a CDB environment, and
     * ignored for other environments.
     *
     * @param samePosition is passed through to SCursor.dup().
     *
     * @return the open cursor.
     *
     * @throws SDatabaseException if a database problem occurs.
     */
    SCursor dupCursor(SCursor cursor, boolean writeCursor, boolean samePosition)
        throws SDatabaseException {

        if (cdbMode) {
            WeakHashMap cdbCursorsMap = (WeakHashMap) localCdbCursors.get();
            if (cdbCursorsMap != null) {
                SDatabase db = cursor.getDatabase();
                CdbCursors cdbCursors = (CdbCursors) cdbCursorsMap.get(db);
                if (cdbCursors != null) {
                    List cursors = writeCursor ? cdbCursors.writeCursors
                                               : cdbCursors.readCursors;
                    if (cursors.contains(cursor)) {
                        SCursor newCursor = cursor.dup(samePosition);
                        cursors.add(newCursor);
                        return newCursor;
                    }
                }
            }
            throw new IllegalStateException("SCursor to dup not tracked");
        } else {
            return cursor.dup(samePosition);
        }
    }

    /**
     * Closes a cursor.
     *
     * @param cursor the cursor to close.
     *
     * @throws SDatabaseException if a database problem occurs.
     */
    void closeCursor(SCursor cursor)
        throws SDatabaseException {

        if (cursor == null) {
            return;
        }
        if (cdbMode) {
            WeakHashMap cdbCursorsMap = (WeakHashMap) localCdbCursors.get();
            if (cdbCursorsMap != null) {
                SDatabase db = cursor.getDatabase();
                CdbCursors cdbCursors = (CdbCursors) cdbCursorsMap.get(db);
                if (cdbCursors != null) {
                    if (cdbCursors.readCursors.remove(cursor) ||
                        cdbCursors.writeCursors.remove(cursor)) {
                        cursor.close();
                        return;
                    }
                }
            }
            throw new IllegalStateException
                ("Closing CDB cursor that was not known to be open");
        } else {
            cursor.close();
        }
    }

    /**
     * Returns true if a CDB cursor is open and therefore a SDatabase write
     * operation should not be attempted since a self-deadlock may result.
     */
    boolean isCDBCursorOpen(SDatabase db) {
        if (cdbMode) {
            WeakHashMap cdbCursorsMap = (WeakHashMap) localCdbCursors.get();
            if (cdbCursorsMap != null) {
                CdbCursors cdbCursors = (CdbCursors) cdbCursorsMap.get(db);

                if (cdbCursors != null &&
                    (cdbCursors.readCursors.size() > 0 ||
                     cdbCursors.writeCursors.size() > 0)) {
                    return true;
                }
            }
        }
        return false;
    }

    static final class CdbCursors {

        List writeCursors = new ArrayList();
        List readCursors = new ArrayList();
    }
}
