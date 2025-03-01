/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TCursor;
import com.sleepycat.thrift.TCursorGetConfig;
import com.sleepycat.thrift.TCursorGetMode;

/**
 * A database cursor for a secondary database.
 * <p>
 * Secondary cursors are returned by {@link SSecondaryDatabase#openCursor}. The
 * distinguishing characteristics of a secondary cursor are:
 * <ul>
 * <li>Direct calls to {@code put()} methods on a secondary cursor are
 * prohibited.</li>
 * <li>The {@link SCursor#delete} method of a secondary cursor will delete the
 * primary record and as well as all its associated secondary records.</li>
 * <li>Calls to all get methods will return the data from the associated
 * primary database.</li>
 * <li>Additional get method signatures are provided to return the primary key
 * in an additional {@code pKey} parameter.</li>
 * <li>Calls to {@link #dup} will return a {@link SSecondaryCursor}.</li>
 * </ul>
 * To obtain a secondary cursor with default attributes:
 * <pre>
 *  SSecondaryCursor cursor = myDb.openSecondaryCursor(txn, null);
 * </pre>
 * To customize the attributes of a cursor, use a {@link SCursorConfig} object.
 * <pre>
 *  SCursorConfig config = new SCursorConfig();
 *  config.setDirtyRead(true);
 *  SSecondaryCursor cursor = myDb.openSecondaryCursor(txn, config);
 * </pre>
 */
public class SSecondaryCursor extends SCursor {

    SSecondaryCursor(TCursor tCursor, SSecondaryDatabase database,
            STransaction txn, BdbService.Client client) {
        super(tCursor, database, txn, client);
    }

    /**
     * Return the {@link SSecondaryDatabase} handle associated with this
     * cursor.
     *
     * @return the {@link SSecondaryDatabase} handle associated with this cursor
     */
    @Override
    public SSecondaryDatabase getDatabase() {
        return (SSecondaryDatabase) super.getDatabase();
    }

    /**
     * Returns a new {@link SSecondaryCursor} for the same transaction as the
     * original cursor.
     *
     * @param samePosition if true, the newly created cursor is initialized to
     * refer to the same position in the database as the original cursor (if
     * any) and hold the same locks (if any). If false, or the original cursor
     * does not hold a database position and locks, the returned cursor is
     * uninitialized and will behave like a newly created cursor.
     * @return a new cursor with the same transaction and locker ID as the
     * original cursor
     * @throws SDatabaseException if any error occurs
     */
    @Override
    public SSecondaryCursor dup(boolean samePosition)
            throws SDatabaseException {
        return remoteCall(() -> {
            TCursor cursor = this.client.cursorDup(this.tCursor, samePosition);
            return new SSecondaryCursor(cursor, getDatabase(), getTransaction(),
                    this.client);
        });
    }

    /**
     * Returns the key/data pair to which the cursor refers.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#KEYEMPTY} if the key/pair at the cursor
     * position has been deleted; otherwise, {@link SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getCurrent(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.CURRENT);
    }

    /**
     * Move the cursor to the first key/data pair of the database, and return
     * that pair. If the first key has duplicate values, the first data item in
     * the set of duplicates is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getFirst(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.FIRST);
    }

    /**
     * Move the cursor to the last key/data pair of the database, and return
     * that pair. If the last key has duplicate values, the last data item in
     * the set of duplicates is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getLast(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.LAST);
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
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNext(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.NEXT);
    }

    /**
     * If the next key/data pair of the database is a duplicate data record for
     * the current key/data pair, move the cursor to the next key/data pair of
     * the database and return that pair.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNextDup(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.NEXT_DUP);
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
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNextNoDup(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode,
                TCursorGetMode.NEXT_NO_DUP);
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
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getPrev(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.PREV);
    }

    /**
     * If the previous key/data pair of the database is a duplicate data record
     * for the current key/data pair, move the cursor to the previous key/data
     * pair of the database and return that pair.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getPrevDup(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.PREV_DUP);
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
     * @param sKey the secondary key returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getPrevNoDup(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode,
                TCursorGetMode.PREV_NO_DUP);
    }

    /**
     * Return the record number associated with the cursor. The record number
     * will be returned in the data parameter.
     * <p>
     * For this method to be called, the underlying database must be of type
     * Btree, and it must have been configured to support record numbers.
     * <p>
     * When called on a cursor opened on a database that has been made into a
     * secondary index, the method returns the record numbers of both the
     * secondary and primary databases. If either underlying database is not of
     * type Btree or is not configured with record numbers, the out-of-band
     * record number of 0 is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param secondaryRecno the secondary record number returned as output
     * @param primaryRecno the primary record number returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getRecordNumber(SDatabaseEntry secondaryRecno,
            SDatabaseEntry primaryRecno, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(null, secondaryRecno, primaryRecno, lockMode,
                TCursorGetMode.GET_RECNO);
    }

    /**
     * Move the cursor to the specified secondary and primary key, where both
     * the primary and secondary key items must match.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key used as input
     * @param pKey the primary key used as input
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchBoth(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.GET_BOTH);
    }

    /**
     * Move the cursor to the specified secondary key and closest matching
     * primary key of the database.
     * <p>
     * In the case of any database supporting sorted duplicate sets, the
     * returned key/data pair is for the smallest primary key greater than or
     * equal to the specified primary key (as determined by the key comparison
     * function), permitting partial matches and range searches in duplicate
     * data sets.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key used as input
     * @param pKey the primary key used as input and returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchBothRange(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode,
                TCursorGetMode.GET_BOTH_RANGE);
    }

    /**
     * Move the cursor to the given key of the database, and return the datum
     * associated with the given key. If the matching key has duplicate values,
     * the first data item in the set of duplicates is returned.
     * <p>
     * If this method fails for any reason, the position of the cursor will be
     * unchanged.
     *
     * @param sKey the secondary key used as input
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchKey(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode, TCursorGetMode.SET);
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
     * @param sKey the secondary key used as input and returned as output
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchKeyRange(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(sKey, pKey, pData, lockMode,
                TCursorGetMode.SET_RANGE);
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
     * @param secondaryRecno the secondary record number used as input
     * @param pKey the primary key returned as output
     * @param pData the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getSearchRecordNumber(SDatabaseEntry secondaryRecno,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode)
            throws SDatabaseException {
        return cursorPGet(secondaryRecno, pKey, pData, lockMode,
                TCursorGetMode.SET_RECNO);
    }

    private SOperationStatus cursorPGet(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData, SLockMode lockMode,
            TCursorGetMode mode) throws SDatabaseException {
        return remotePGet(sKey, pKey, pData, searchTerm -> {
            TCursorGetConfig config = createConfig(pData, lockMode);
            config.setMode(mode);
            return this.client.cursorGetWithPKey(this.tCursor, searchTerm,
                    config);
        });
    }
}
