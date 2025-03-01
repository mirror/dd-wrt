/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.client.util.RuntimeExceptionWrapper;
import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TCursor;
import com.sleepycat.thrift.TCursorGetConfig;
import com.sleepycat.thrift.TCursorGetMode;
import com.sleepycat.thrift.TCursorPutConfig;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TIsolationLevel;

import java.util.Collections;
import java.util.Set;

/**
 * A database cursor. Cursors are used for operating on collections of records,
 * for iterating over a database, and for saving handles to individual records,
 * so that they can be modified after they have been read.
 * <p>
 * If the cursor is to be used to perform operations on behalf of a
 * transaction, the cursor must be opened and closed within the context of that
 * single transaction.
 * <p>
 * If you do not close the cursor before closing the database handle or the
 * transaction handle that owns this cursor, then, closing a database handle or
 * a transaction handle closes these open cursors. Once the cursor close method
 * has been called, the handle may not be accessed again, regardless of the
 * close method's success or failure.
 * <p>
 * To obtain a cursor with default attributes:
 * <pre>
 *  SCursor cursor = myDatabase.openCursor(txn, null);
 * </pre>
 * To customize the attributes of a cursor, use a {@link SCursorConfig} object.
 * <pre>
 *  SCursorConfig config = new SCursorConfig();
 *  config.setDirtyRead(true);
 *  SCursor cursor = myDatabase.openCursor(txn, config);
 * </pre>
 * Modifications to the database during a sequential scan will be reflected in
 * the scan; that is, records inserted behind a cursor will not be returned
 * while records inserted in front of a cursor will be returned. In Queue and
 * Recno databases, missing entries (that is, entries that were never
 * explicitly created or that were created and then deleted) will be ignored
 * during a sequential scan.
 */
public class SCursor implements GetHelper, PutHelper, AutoCloseable {
    /** The remote cursor handle. */
    protected final TCursor tCursor;

    /** The remote service client. */
    protected final BdbService.Client client;

    /** The enclosing database. */
    private final SDatabase database;

    /** The enclosing transaction. */
    private final STransaction txn;

    /** The key to which this cursor currently refers. */
    private SDatabaseEntry currentKey;

    protected SCursor(TCursor tCursor, SDatabase database, STransaction txn,
            BdbService.Client client) {
        this.tCursor = tCursor;
        this.database = database;
        this.txn = txn;
        this.client = client;
        this.currentKey = null;
    }

    /**
     * Discard the cursor.
     * <p>
     * After the close method has been called, you cannot use the cursor handle
     * again.
     * <p>
     * It is not required to close the cursor explicitly before closing the
     * database handle or the transaction handle that owns this cursor because
     * closing a database handle or transaction handle closes those open
     * cursor.
     * <p>
     * However, it is recommended that you always close all cursor handles
     * immediately after their use to promote concurrency and to release
     * resources such as page locks.
     *
     * @throws SDatabaseException if any error occurs
     */
    @Override
    public void close() throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeCursorHandle(this.tCursor);
            return null;
        });
    }

    /**
     * Return a comparison of the two cursors. Two cursors are equal if and
     * only if they are positioned on the same item in the same database.
     *
     * @param otherCursor the other cursor to be compared
     * @return an integer representing the result of the comparison between this
     * cursor and {@code otherCursor} (another cursor handle used as the
     * comparator). 0 indicates that this cursor and {@code otherCursor} are
     * positioned on the same item, 1 indicates this cursor is greater than
     * {@code otherCursor}, -1 indicates that {@code otherCursor} is greater
     * than this cursor.
     * @throws SDatabaseException if any error occurs
     */
    public int compare(SCursor otherCursor) throws SDatabaseException {
        return remoteCall(() -> this.client
                .cursorCompare(this.tCursor, otherCursor.tCursor));
    }

    /**
     * Return a count of the number of data items for the key to which the
     * cursor refers.
     *
     * @return a count of the number of data items for the key to which the
     * cursor refers
     * @throws SDatabaseException if any error occurs
     */
    public int count() throws SDatabaseException {
        return remoteCall(() -> this.client.cursorCount(this.tCursor));
    }

    /**
     * Creates a new cursor that uses the same transaction and locker ID as the
     * original cursor.
     * <p>
     * This is useful when an application is using locking and requires two or
     * more cursors in the same thread of control.
     *
     * @param samePosition if true, the newly created cursor is initialized to
     * refer to the same position in the database as the original cursor (if
     * any) and hold the same locks (if any). If false, or the original cursor
     * does not hold a database position and locks, the returned cursor is
     * uninitialized and will behave like a newly created cursor.
     * @return a new cursor with the same transaction and locker ID as the
     * original cursor.
     * @throws SDatabaseException if any error occurs
     */
    public SCursor dup(boolean samePosition) throws SDatabaseException {
        return remoteCall(() -> {
            TCursor cursor = this.client.cursorDup(this.tCursor, samePosition);
            SCursor dup =
                    new SCursor(cursor, this.database, this.txn, this.client);
            if (samePosition) {
                dup.currentKey = this.currentKey;
            }
            return dup;
        });
    }

    /**
     * Return this cursor's configuration.
     * <p>
     * This may differ from the configuration used to open this object.
     *
     * @return this cursor's configuration
     * @throws SDatabaseException if any error occurs
     */
    public SCursorConfig getConfig() throws SDatabaseException {
        return remoteCall(() -> new SCursorConfig(
                this.client.getCursorConfig(this.tCursor)));
    }

    /**
     * Get the cache priority for pages referenced by the cursor.
     *
     * @return the cache priority for pages referenced by the cursor
     * @throws SDatabaseException if any error occurs
     */
    public SCacheFilePriority getPriority() throws SDatabaseException {
        return remoteCall(() -> SCacheFilePriority.toBdb(
                this.client.getCursorCachePriority(this.tCursor)));
    }

    /**
     * Set the cache priority for pages referenced by this cursor handle.
     * <p>
     * The priority of a page biases the replacement algorithm to be more or
     * less likely to discard a page when space is needed in the buffer pool.
     * The bias is temporary, and pages will eventually be discarded if they
     * are not referenced again. The setPriority method is only advisory, and
     * does not guarantee pages will be treated in a specific way.
     *
     * @param priority the cache priority
     * @throws SDatabaseException if any error occurs
     */
    public void setPriority(SCacheFilePriority priority)
            throws SDatabaseException {
        remoteCall(() -> {
            this.client.setCursorCachePriority(this.tCursor,
                    SCacheFilePriority.toThrift(priority));
            return null;
        });
    }

    /**
     * Return the {@link SDatabase} handle associated with this cursor.
     *
     * @return the {@link SDatabase} handle associated with this cursor
     */
    public SDatabase getDatabase() {
        return this.database;
    }

    STransaction getTransaction() {
        return this.txn;
    }

    @Override
    public Set<SSecondaryDatabase> getSecondaryDatabases() {
        return this.database.getSecondaryDatabases();
    }

    /**
     * Returns the key/data pair to which the cursor refers.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#KEYEMPTY} if the key/pair at the cursor
     * position has been deleted; otherwise, {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getCurrent(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.CURRENT);
    }

    /**
     * Move the cursor to the first key/data pair of the database, and return
     * that pair. If the first key has duplicate values, the first data item in
     * the set of duplicates is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getFirst(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.FIRST);
    }

    /**
     * Move the cursor to the last key/data pair of the database, and return
     * that pair. If the last key has duplicate values, the last data item in
     * the set of duplicates is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getLast(SDatabaseEntry key,
            SDatabaseEntry data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.LAST);
    }

    /**
     * Move the cursor to the next key/data pair and return that pair. If the
     * matching key has duplicate values, the first data item in the set of
     * duplicates is returned.
     * <p>
     * If the cursor is not yet initialized, move the cursor to the first
     * key/data pair of the database, and return that pair. Otherwise, the
     * cursor is moved to the next key/data pair of the database, and that pair
     * is returned. In the presence of duplicate key values, the value of the
     * key may not change.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNext(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.NEXT);
    }

    /**
     * If the next key/data pair of the database is a duplicate data record for
     * the current key/data pair, move the cursor to the next key/data pair of
     * the database and return that pair.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNextDup(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.NEXT_DUP);
    }

    /**
     * Move the cursor to the next non-duplicate key/data pair and return that
     * pair. If the matching key has duplicate values, the first data item in
     * the set of duplicates is returned.
     * <p>
     * If the cursor is not yet initialized, move the cursor to the first
     * key/data pair of the database, and return that pair. Otherwise, the
     * cursor is moved to the next non-duplicate key of the database, and that
     * key/data pair is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNextNoDup(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.NEXT_NO_DUP);
    }

    /**
     * Move the cursor to the previous key/data pair and return that pair. If
     * the matching key has duplicate values, the last data item in the set of
     * duplicates is returned.
     * <p>
     * If the cursor is not yet initialized, move the cursor to the last
     * key/data pair of the database, and return that pair. Otherwise, the
     * cursor is moved to the previous key/data pair of the database, and that
     * pair is returned. In the presence of duplicate key values, the value of
     * the key may not change.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getPrev(SDatabaseEntry key,
            SDatabaseEntry data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.PREV);
    }

    /**
     * If the previous key/data pair of the database is a duplicate data record
     * for the current key/data pair, move the cursor to the previous key/data
     * pair of the database and return that pair.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getPrevDup(SDatabaseEntry key,
            SDatabaseEntry data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.PREV_DUP);
    }

    /**
     * Move the cursor to the previous non-duplicate key/data pair and return
     * that pair. If the matching key has duplicate values, the last data item
     * in the set of duplicates is returned.
     * <p>
     * If the cursor is not yet initialized, move the cursor to the last
     * key/data pair of the database, and return that pair. Otherwise, the
     * cursor is moved to the previous non-duplicate key of the database, and
     * that key/data pair is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getPrevNoDup(SDatabaseEntry key,
            SDatabaseEntry data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.PREV_NO_DUP);
    }

    /**
     * Return the record number associated with the cursor. The record number
     * will be returned in the data parameter.
     * <p>
     * For this method to be called, the underlying database must be of type
     * Btree, and it must have been configured to support record numbers.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param data the data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getRecordNumber(SDatabaseEntry data,
            SLockMode lockMode) throws SDatabaseException {
        return cursorGet(null, data, lockMode, TCursorGetMode.GET_RECNO);
    }

    /**
     * Move the cursor to the specified key/data pair, where both the key and
     * data items must match.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key used as input
     * @param data the data used as input; only {@link SDatabaseEntry} is
     * supported
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchBoth(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        if (key.getPartial()) {
            throw new IllegalArgumentException("Partial key is not supported.");
        }
        if (data != null && !(data instanceof SDatabaseEntry)) {
            throw new IllegalArgumentException(
                    "data must be a SDatabaseEntry.");
        }
        return cursorGet(key, data, lockMode, TCursorGetMode.GET_BOTH);
    }

    /**
     * Move the cursor to the specified key and matching data item of the
     * database.
     * <p>
     * In the case of any database supporting sorted duplicate sets, the
     * returned key/data pair is for the smallest data item greater than or
     * equal to the specified data item (as determined by the duplicate
     * comparison function), permitting partial matches and range searches in
     * duplicate data sets.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key used as input
     * @param data the data used as input and returned as output; only {@link
     * SDatabaseEntry} is supported
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchBothRange(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        if (key.getPartial()) {
            throw new IllegalArgumentException("Partial key is not supported.");
        }
        if (data != null && !(data instanceof SDatabaseEntry)) {
            throw new IllegalArgumentException(
                    "data must be a SDatabaseEntry.");
        }
        return cursorGet(key, data, lockMode, TCursorGetMode.GET_BOTH_RANGE);
    }

    /**
     * Move the cursor to the given key of the database, and return the datum
     * associated with the given key. If the matching key has duplicate values,
     * the first data item in the set of duplicates is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key used as input
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchKey(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        if (key.getPartial()) {
            throw new IllegalArgumentException("Partial key is not supported.");
        }
        return cursorGet(key, data, lockMode, TCursorGetMode.SET);
    }

    /**
     * Move the cursor to the closest matching key of the database, and return
     * the data item associated with the matching key. If the matching key has
     * duplicate values, the first data item in the set of duplicates is
     * returned.
     * <p>
     * The returned key/data pair is for the smallest key greater than or equal
     * to the specified key (as determined by the key comparison function),
     * permitting partial key matches and range searches.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key used as input and returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchKeyRange(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        return cursorGet(key, data, lockMode, TCursorGetMode.SET_RANGE);
    }

    /**
     * Move the cursor to the specific numbered record of the database, and
     * return the associated key/data pair.
     * <p>
     * The specified key must be a record number as described in
     * {@link SDatabaseEntry}. This determines the record to be retrieved.
     * <p>
     * For this method to be called, the underlying database must be of type
     * Btree, and it must have been configured to support record numbers.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param key the key returned as output
     * @param data the data returned as output. Use {@link SMultipleDataEntry}
     * to return multiple duplicates of the key. Use {@link SMultiplePairs} to
     * return multiple key/data pairs.
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchRecordNumber(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode)
            throws SDatabaseException {
        if (key.getPartial()) {
            throw new IllegalArgumentException("Partial key is not supported.");
        }
        return cursorGet(key, data, lockMode, TCursorGetMode.SET_RECNO);
    }

    private SOperationStatus cursorGet(SDatabaseEntry key,
            SDatabaseEntryBase data, SLockMode lockMode, TCursorGetMode mode)
            throws SDatabaseException {
        SDatabaseEntry nonNullKey = key == null ? new SDatabaseEntry() : key;
        SOperationStatus status = remoteGet(nonNullKey, data, searchTerm -> {
            TCursorGetConfig config = createConfig(data, lockMode);
            config.setMode(mode);
            return this.client.cursorGet(this.tCursor, searchTerm, config);
        });

        if (data instanceof SMultiplePairs) {
            this.currentKey = null;
        } else if (mode != TCursorGetMode.GET_RECNO) {
            this.currentKey =
                    new SDatabaseEntry(new TDbt(nonNullKey.getThriftObj()));
        }

        return status;
    }

    protected TCursorGetConfig createConfig(SDatabaseEntryBase data,
            SLockMode lockMode) {
        TCursorGetConfig config = new TCursorGetConfig();
        if (lockMode != null) {
            switch (lockMode) {
                case READ_COMMITTED:
                    config.setIsoLevel(TIsolationLevel.READ_COMMITTED);
                    break;
                case READ_UNCOMMITTED:
                    config.setIsoLevel(TIsolationLevel.READ_UNCOMMITTED);
                    break;
                case RMW:
                    config.setRmw(true);
                    break;
            }
        }
        if (data instanceof SMultipleDataEntry) {
            config.setMultiple(true);
            config.setBatchSize(((SMultipleDataEntry) data).getBatchSize());
        } else if (data instanceof SMultiplePairs) {
            config.setMultiKey(true);
            config.setBatchSize(((SMultiplePairs) data).getBatchSize());
        }
        return config;
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the put method succeeds, the cursor is always positioned to refer to
     * the newly inserted item. If the put method fails for any reason, the
     * state of the cursor will be unchanged.
     * <p>
     * If the key already appears in the database and duplicates are supported,
     * the new data value is inserted at the correct sorted location. If the
     * key already appears in the database and duplicates are not supported,
     * the existing key/data pair will be replaced.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus put(SDatabaseEntry key, SDatabaseEntry data)
            throws SDatabaseException {
        return cursorPut(key, data, TCursorPutConfig.DEFAULT);
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the putKeyFirst method succeeds, the cursor is always positioned to
     * refer to the newly inserted item. If the putKeyFirst method fails for
     * any reason, the state of the cursor will be unchanged.
     * <p>
     * In the case of the Btree and Hash access methods, insert the specified
     * key/data pair into the database.
     * <p>
     * If the underlying database supports duplicate data items, and if the key
     * already exists in the database and a duplicate sort function has been
     * specified, the inserted data item is added in its sorted location. If
     * the key already exists in the database and no duplicate sort function
     * has been specified, the inserted data item is added as the first of the
     * data items for that key.
     * <p>
     * The putKeyFirst method may not be called for the Queue or Recno access
     * methods.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putKeyFirst(SDatabaseEntry key, SDatabaseEntry data)
            throws SDatabaseException {
        return cursorPut(key, data, TCursorPutConfig.KEY_FIRST);
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the putKeyLast method succeeds, the cursor is always positioned to
     * refer to the newly inserted item. If the putKeyLast method fails for any
     * reason, the state of the cursor will be unchanged.
     * <p>
     * In the case of the Btree and Hash access methods, insert the specified
     * key/data pair into the database.
     * <p>
     * If the underlying database supports duplicate data items, and if the key
     * already exists in the database and a duplicate sort function has been
     * specified, the inserted data item is added in its sorted location. If
     * the key already exists in the database and no duplicate sort function
     * has been specified, the inserted data item is added as the last of the
     * data items for that key.
     * <p>
     * The putKeyLast method may not be called for the Queue or Recno access
     * methods.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putKeyLast(SDatabaseEntry key, SDatabaseEntry data)
            throws SDatabaseException {
        return cursorPut(key, data, TCursorPutConfig.KEY_LAST);
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the putNoDupData method succeeds, the cursor is always positioned to
     * refer to the newly inserted item. If the putNoDupData method fails for
     * any reason, the state of the cursor will be unchanged.
     * <p>
     * In the case of the Btree and Hash access methods, insert the specified
     * key/data pair into the database, unless a key/data pair comparing
     * equally to it already exists in the database. If a matching key/data
     * pair already exists in the database, {@link SOperationStatus#KEYEXIST}
     * is returned.
     * <p>
     * This method may only be called if the underlying database has been
     * configured to support sorted duplicate data items.
     * <p>
     * This method may not be called for the Queue or Recno access methods.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#KEYEXIST} if a matching key/data pair
     * already exists in the database; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putNoDupData(SDatabaseEntry key,
            SDatabaseEntry data)
            throws SDatabaseException {
        return cursorPut(key, data, TCursorPutConfig.NO_DUP_DATA);
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the putNoOverwrite method succeeds, the cursor is always positioned
     * to refer to the newly inserted item. If the putNoOverwrite method fails
     * for any reason, the state of the cursor will be unchanged.
     * <p>
     * If the key already appears in the database, putNoOverwrite will return
     * {@link SOperationStatus#KEYEXIST}.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#KEYEXIST} if a matching key/data pair
     * already exists in the database; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putNoOverwrite(SDatabaseEntry key,
            SDatabaseEntry data)
            throws SDatabaseException {
        return cursorPut(key, data, TCursorPutConfig.NO_OVERWRITE);
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the putAfter method succeeds, the cursor is always positioned to
     * refer to the newly inserted item. If the putAfter method fails for any
     * reason, the state of the cursor will be unchanged.
     * <p>
     * In the case of the Btree and Hash access methods, insert the data
     * element as a duplicate element of the key to which the cursor refers.
     * The new element appears immediately after the current cursor position.
     * It is an error to call this method if the underlying Btree or Hash
     * database does not support duplicate data items. The key parameter is
     * ignored.
     * <p>
     * In the case of the Hash access method, the putAfter method will fail and
     * throw an exception if the current cursor record has already been
     * deleted.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#NOTFOUND} if the current cursor has
     * already been deleted and the underlying access method is Hash;
     * otherwise,
     * {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putAfter(SDatabaseEntry key, SDatabaseEntry data)
            throws SDatabaseException {
        if (this.currentKey == null) {
            throw new IllegalStateException("putAfter cannot be called when" +
                    " the cursor's position is not set or right after" +
                    " a MULTIPLE_KEY get operation or a delete operation.");
        }
        key.setDataFromTDbt(this.currentKey.getThriftObj());
        return cursorPut(key, data, TCursorPutConfig.AFTER);
    }

    /**
     * Store a key/data pair into the database.
     * <p>
     * If the putBefore method succeeds, the cursor is always positioned to
     * refer to the newly inserted item. If the putBefore method fails for any
     * reason, the state of the cursor will be unchanged.
     * <p>
     * In the case of the Btree and Hash access methods, insert the data
     * element as a duplicate element of the key to which the cursor refers.
     * The new element appears immediately before the current cursor position.
     * It is an error to call this method if the underlying Btree or Hash
     * database does not support duplicate data items. The key parameter is
     * ignored.
     * <p>
     * In the case of the Hash access method, the putBefore method will fail
     * and throw an exception if the current cursor record has already been
     * deleted.
     *
     * @param key the key entry operated on
     * @param data the data entry stored
     * @return {@link SOperationStatus#NOTFOUND} if the current cursor has
     * already been deleted and the underlying access method is Hash;
     * otherwise,
     * {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putBefore(SDatabaseEntry key, SDatabaseEntry data)
            throws SDatabaseException {
        if (this.currentKey == null) {
            throw new IllegalStateException("putBefore cannot be called when" +
                    " the cursor's position is not set or right after" +
                    " a MULTIPLE_KEY get operation or a delete operation.");
        }
        key.setDataFromTDbt(this.currentKey.getThriftObj());
        return cursorPut(key, data, TCursorPutConfig.BEFORE);
    }

    /**
     * Replaces the data in the key/data pair at the current cursor position.
     * <p>
     * Whether the putCurrent method succeeds or fails for any reason, the
     * state of the cursor will be unchanged.
     * <p>
     * Overwrite the data of the key/data pair to which the cursor refers with
     * the specified data item. This method will return {@link
     * SOperationStatus#NOTFOUND} if the cursor currently refers to an
     * already-deleted key/data pair.
     * <p>
     * For a database that does not support duplicates, the data may be changed
     * by this method.
     * <p>
     * If the old and new data are unequal, a {@link SDatabaseException} is
     * thrown. Changing the data in this case would change the sort order of
     * the record, which would change the cursor position, and this is not
     * allowed. To change the sort order of a record, delete it and then
     * re-insert it.
     *
     * @param data the data entry stored
     * @return {@link SOperationStatus#NOTFOUND} if the current cursor has
     * already been deleted; otherwise, {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus putCurrent(SDatabaseEntry data)
            throws SDatabaseException {
        if (this.currentKey == null) {
            throw new IllegalStateException("putCurrent cannot be called when" +
                    " the cursor's position is not set or right after" +
                    " a MULTIPLE_KEY get operation or a delete operation.");
        }
        return cursorPut(this.currentKey, data, TCursorPutConfig.CURRENT);
    }

    private SOperationStatus cursorPut(SDatabaseEntry key, SDatabaseEntry data,
            TCursorPutConfig config) throws SDatabaseException {
        SOperationStatus result =
                remotePut(Collections.singletonList(calculateSKey(key, data)),
                        key, pairs -> this.client
                                .cursorPut(this.tCursor, pairs.get(0), config));
        this.currentKey = new SDatabaseEntry(new TDbt(key.getThriftObj()));
        return result;
    }

    /**
     * Delete the key/data pair to which the cursor refers.
     * <p>
     * When called on a cursor opened on a database that has been made into a
     * secondary index, this method deletes the key/data pair from the primary
     * database and all secondary indices.
     * <p>
     * The cursor position is unchanged after a delete, and subsequent calls to
     * cursor functions expecting the cursor to refer to an existing key will
     * fail.
     *
     * @return {@link SOperationStatus#KEYEMPTY} if the key/pair at the cursor
     * position has been deleted; otherwise, {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus delete() throws SDatabaseException {
        getDatabase().updatePrimaryData(this.txn,
                Collections.singletonList(this.currentKey));

        return remoteCall(() -> SOperationStatus
                .toBdb(this.client.cursorDelete(this.tCursor)));
    }
}
