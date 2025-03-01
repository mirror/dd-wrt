/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections;

import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.compat.DbCompat.OpReadOptions;
import com.sleepycat.client.compat.DbCompat.OpResult;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SJoinConfig;
import com.sleepycat.client.SJoinCursor;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.util.keyrange.KeyRange;
import com.sleepycat.client.util.keyrange.RangeCursor;

import java.nio.ByteOrder;

/**
 * Represents a Berkeley DB cursor and adds support for indices, bindings and
 * key ranges.
 *
 * <p>This class operates on a view and takes care of reading and updating
 * indices, calling bindings, constraining access to a key range, etc.</p>
 *
 * @author Mark Hayes
 */
final class DataCursor implements Cloneable {

    /** Repositioned exactly to the key/data pair given. */
    static final int REPOS_EXACT = 0;
    /** Repositioned on a record following the key/data pair given. */
    static final int REPOS_NEXT = 1;
    /** Repositioned failed, no records on or after the key/data pair given. */
    static final int REPOS_EOF = 2;

    private RangeCursor cursor;
    private SJoinCursor joinCursor;
    private DataView view;
    private KeyRange range;
    private boolean writeAllowed;
    private boolean readUncommitted;
    private SDatabaseEntry keyThang;
    private SDatabaseEntry valueThang;
    private SDatabaseEntry primaryKeyThang;
    private SDatabaseEntry otherThang;
    private DataCursor[] indexCursorsToClose;

    /**
     * Creates a cursor for a given view.
     */
    DataCursor(DataView view, boolean writeAllowed)
        throws SDatabaseException {

        init(view, writeAllowed, null, null);
    }

    /**
     * Creates a cursor for a given view.
     */
    DataCursor(DataView view, boolean writeAllowed, SCursorConfig config)
        throws SDatabaseException {

        init(view, writeAllowed, config, null);
    }

    /**
     * Creates a cursor for a given view and single key range.
     * Used by unit tests.
     */
    DataCursor(DataView view, boolean writeAllowed, Object singleKey)
        throws SDatabaseException {

        init(view, writeAllowed, null, view.subRange(view.range, singleKey));
    }

    /**
     * Creates a cursor for a given view and key range.
     * Used by unit tests.
     */
    DataCursor(DataView view, boolean writeAllowed,
               Object beginKey, boolean beginInclusive,
               Object endKey, boolean endInclusive)
        throws SDatabaseException {

        init(view, writeAllowed, null,
             view.subRange
                (view.range, beginKey, beginInclusive, endKey, endInclusive));
    }

    /**
     * Creates a join cursor.
     */
    DataCursor(DataView view, DataCursor[] indexCursors,
               SJoinConfig joinConfig, boolean closeIndexCursors)
        throws SDatabaseException {

        if (view.isSecondary()) {
            throw new IllegalArgumentException(
                "The primary collection in a join must not be a secondary " +
                "database");
        }
        SCursor[] cursors = new SCursor[indexCursors.length];
        for (int i = 0; i < cursors.length; i += 1) {
            cursors[i] = indexCursors[i].cursor.getCursor();
        }
        joinCursor = view.db.join(cursors, joinConfig);
        init(view, false, null, null);
        if (closeIndexCursors) {
            indexCursorsToClose = indexCursors;
        }
    }

    /**
     * Clones a cursor preserving the current position.
     */
    DataCursor cloneCursor()
        throws SDatabaseException {

        checkNoJoinCursor();

        DataCursor o;
        try {
            o = (DataCursor) super.clone();
        } catch (CloneNotSupportedException neverHappens) {
            return null;
        }

        o.initThangs();
        KeyRange.copy(keyThang, o.keyThang);
        KeyRange.copy(valueThang, o.valueThang);
        if (primaryKeyThang != keyThang) {
            KeyRange.copy(primaryKeyThang, o.primaryKeyThang);
        }

        o.cursor = cursor.dup(true);
        return o;
    }

    /**
     * Returns the internal range cursor.
     */
    RangeCursor getCursor() {
        return cursor;
    }

    /**
     * Constructor helper.
     */
    private void init(DataView view,
                      boolean writeAllowed,
                      SCursorConfig config,
                      KeyRange range)
        throws SDatabaseException {

        if (config == null) {
            config = view.cursorConfig;
        }
        this.view = view;
        this.writeAllowed = writeAllowed && view.writeAllowed;
        this.range = (range != null) ? range : view.range;
        readUncommitted = config.getReadUncommitted() ||
                          view.currentTxn.isReadUncommitted();
        initThangs();

        if (joinCursor == null) {
            cursor = new MyRangeCursor
                (this.range, config, view, this.writeAllowed);
        }
    }

    /**
     * Constructor helper.
     */
    private void initThangs() {
        keyThang = new SDatabaseEntry();
        primaryKeyThang = view.isSecondary() ? (new SDatabaseEntry())
                                             : keyThang;
        valueThang = new SDatabaseEntry();
    }

    /**
     * Set entries from given byte arrays.
     */
    private void setThangs(byte[] keyBytes,
                           byte[] priKeyBytes,
                           byte[] valueBytes) {

        keyThang.setData(KeyRange.copyBytes(keyBytes));

        if (keyThang != primaryKeyThang) {
            primaryKeyThang.setData(KeyRange.copyBytes(priKeyBytes));
        }

        valueThang.setData(KeyRange.copyBytes(valueBytes));
    }

    /**
     * Closes the associated cursor.
     */
    void close()
        throws SDatabaseException {

        if (joinCursor != null) {
            SJoinCursor toClose = joinCursor;
            joinCursor = null;
            toClose.close();
        }
        if (cursor != null) {
            SCursor toClose = cursor.getCursor();
            cursor = null;
            view.currentTxn.closeCursor(toClose);
        }
        if (indexCursorsToClose != null) {
            DataCursor[] toClose = indexCursorsToClose;
            indexCursorsToClose = null;
            for (int i = 0; i < toClose.length; i += 1) {
                toClose[i].close();
            }
        }
    }

    /**
     * Repositions to a given raw key/data pair, or just past it if that record
     * has been deleted.
     *
     * @return REPOS_EXACT, REPOS_NEXT or REPOS_EOF.
     */
    int repositionRange(byte[] keyBytes,
                        byte[] priKeyBytes,
                        byte[] valueBytes,
                        boolean lockForWrite)
        throws SDatabaseException {

        OpReadOptions options = OpReadOptions.make(getLockMode(lockForWrite));
        OpResult result = null;

        /* Use the given key/data byte arrays. */
        setThangs(keyBytes, priKeyBytes, valueBytes);

        /* Position on or after the given key/data pair. */
        if (view.dupsAllowed) {
            result = cursor.getSearchBothRange(keyThang, primaryKeyThang,
                                               valueThang, options);
        }
        if (result == null || !result.isSuccess()) {
            result = cursor.getSearchKeyRange(keyThang, primaryKeyThang,
                                              valueThang, options);
        }

        /* Return the result of the operation. */
        if (result.isSuccess()) {
            if (!KeyRange.equalBytes(keyBytes, 0, keyBytes.length,
                                     keyThang.getData(),
                                     keyThang.getOffset(),
                                     keyThang.getSize())) {
                return REPOS_NEXT;
            }
            if (view.dupsAllowed) {
                SDatabaseEntry thang = view.isSecondary() ? primaryKeyThang
                                                         : valueThang;
                byte[] bytes = view.isSecondary() ? priKeyBytes
                                                  : valueBytes;
                if (!KeyRange.equalBytes(bytes, 0, bytes.length,
                                         thang.getData(),
                                         thang.getOffset(),
                                         thang.getSize())) {
                    return REPOS_NEXT;
                }
            }
            return REPOS_EXACT;
        } else {
            return REPOS_EOF;
        }
    }

    /**
     * Repositions to a given raw key/data pair.
     *
     * @throws IllegalStateException when the database has unordered keys or
     * unordered duplicates.
     *
     * @return whether the search succeeded.
     */
    boolean repositionExact(byte[] keyBytes,
                            byte[] priKeyBytes,
                            byte[] valueBytes,
                            boolean lockForWrite)
        throws SDatabaseException {

        OpReadOptions options = OpReadOptions.make(getLockMode(lockForWrite));
        OpResult result;

        /* Use the given key/data byte arrays. */
        setThangs(keyBytes, priKeyBytes, valueBytes);

        /* Position on the given key/data pair. */
        if (view.recNumRenumber) {
            /* getSearchBoth doesn't work with recno-renumber databases. */
            result = cursor.getSearchKey(keyThang, primaryKeyThang,
                                         valueThang, options);
        } else {
            result = cursor.getSearchBoth(keyThang, primaryKeyThang,
                                          valueThang, options);
        }

        return result.isSuccess();
    }

    /**
     * Returns the view for this cursor.
     */
    DataView getView() {

        return view;
    }

    /**
     * Returns the range for this cursor.
     */
    KeyRange getRange() {

        return range;
    }

    /**
     * Returns whether write is allowed for this cursor, as specified to the
     * constructor.
     */
    boolean isWriteAllowed() {

        return writeAllowed;
    }

    /**
     * Returns the key object for the last record read.
     */
    Object getCurrentKey() {
        return view.makeKey(keyThang, primaryKeyThang);
    }

    /**
     * Returns the value object for the last record read.
     */
    Object getCurrentValue() {
        return view.makeValue(primaryKeyThang, valueThang);
    }

    /**
     * Returns the internal key entry.
     */
    SDatabaseEntry getKeyThang() {
        return keyThang;
    }

    /**
     * Returns the internal primary key entry, which is the same object as the
     * key entry if the cursor is not for a secondary database.
     */
    SDatabaseEntry getPrimaryKeyThang() {
        return primaryKeyThang;
    }

    /**
     * Returns the internal value entry.
     */
    SDatabaseEntry getValueThang() {
        return valueThang;
    }

    /**
     * Returns whether record number access is allowed.
     */
    boolean hasRecNumAccess() {

        return view.recNumAccess;
    }

    /**
     * Returns the record number for the last record read.
     */
    int getCurrentRecordNumber()
        throws SDatabaseException {

        if (view.btreeRecNumDb) {
            /* BTREE-RECNO access. */
            if (otherThang == null) {
                otherThang = new SDatabaseEntry();
            }
            DbCompat.getCurrentRecordNumber(cursor.getCursor(), otherThang,
                                            getLockMode(false));
            return DbCompat.getRecordNumber(otherThang, getServerByteOrder());
        } else {
            /* QUEUE or RECNO database. */
            return DbCompat.getRecordNumber(keyThang, getServerByteOrder());
        }
    }

    private ByteOrder getServerByteOrder() {
        return view.getServerByteOrder();
    }

    /**
     * Binding version of SCursor.getCurrent(), no join cursor allowed.
     */
    SOperationStatus getCurrent(boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        return cursor.getCurrent(
            keyThang, primaryKeyThang, valueThang,
            OpReadOptions.make(getLockMode(lockForWrite))).status();
    }

    /**
     * Binding version of SCursor.getFirst(), join cursor is allowed.
     */
    SOperationStatus getFirst(boolean lockForWrite)
        throws SDatabaseException {

        SLockMode lockMode = getLockMode(lockForWrite);
        if (joinCursor != null) {
            return joinCursor.getNext(keyThang, valueThang, lockMode);
        } else {
            return cursor.getFirst(
                keyThang, primaryKeyThang, valueThang,
                OpReadOptions.make(lockMode)).status();
        }
    }

    /**
     * Binding version of SCursor.getNext(), join cursor is allowed.
     */
    SOperationStatus getNext(boolean lockForWrite)
        throws SDatabaseException {

        SLockMode lockMode = getLockMode(lockForWrite);
        if (joinCursor != null) {
            return joinCursor.getNext(keyThang, valueThang, lockMode);
        } else {
            return cursor.getNext(
                keyThang, primaryKeyThang, valueThang,
                OpReadOptions.make(lockMode)).status();
        }
    }

    /**
     * Binding version of SCursor.getNext(), join cursor is allowed.
     */
    SOperationStatus getNextNoDup(boolean lockForWrite)
        throws SDatabaseException {

        SLockMode lockMode = getLockMode(lockForWrite);
        OpReadOptions options = OpReadOptions.make(lockMode);
        if (joinCursor != null) {
            return joinCursor.getNext(keyThang, valueThang, lockMode);
        } else if (view.dupsView) {
            return cursor.getNext
                (keyThang, primaryKeyThang, valueThang, options).status();
        } else {
            return cursor.getNextNoDup
                (keyThang, primaryKeyThang, valueThang, options).status();
        }
    }

    /**
     * Binding version of SCursor.getNextDup(), no join cursor allowed.
     */
    SOperationStatus getNextDup(boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        if (view.dupsView) {
            return null;
        } else {
            return cursor.getNextDup(
                keyThang, primaryKeyThang, valueThang,
                OpReadOptions.make(getLockMode(lockForWrite))).status();
        }
    }

    /**
     * Binding version of SCursor.getLast(), no join cursor allowed.
     */
    SOperationStatus getLast(boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        return cursor.getLast(
            keyThang, primaryKeyThang, valueThang,
            OpReadOptions.make(getLockMode(lockForWrite))).status();
    }

    /**
     * Binding version of SCursor.getPrev(), no join cursor allowed.
     */
    SOperationStatus getPrev(boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        return cursor.getPrev(
            keyThang, primaryKeyThang, valueThang,
            OpReadOptions.make(getLockMode(lockForWrite))).status();
    }

    /**
     * Binding version of SCursor.getPrevNoDup(), no join cursor allowed.
     */
    SOperationStatus getPrevNoDup(boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        SLockMode lockMode = getLockMode(lockForWrite);
        OpReadOptions options = OpReadOptions.make(lockMode);
        if (view.dupsView) {
            return null;
        } else {
            return cursor.getPrevNoDup(
                keyThang, primaryKeyThang, valueThang, options).status();
        }
    }

    /**
     * Binding version of SCursor.getPrevDup(), no join cursor allowed.
     */
    SOperationStatus getPrevDup(boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        if (view.dupsView) {
            return null;
        } else {
            return cursor.getPrevDup(
                keyThang, primaryKeyThang, valueThang,
                OpReadOptions.make(getLockMode(lockForWrite))).status();
        }
    }

    /**
     * Binding version of SCursor.getSearchKey(), no join cursor allowed.
     * Searches by record number in a BTREE-RECNO db with RECNO access.
     */
    SOperationStatus getSearchKey(Object key, Object value,
                                 boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        if (view.dupsView) {
            if (view.useKey(key, value, primaryKeyThang, view.dupsRange)) {
                KeyRange.copy(view.dupsKey, keyThang);
                return cursor.getSearchBoth(
                    keyThang, primaryKeyThang, valueThang,
                    OpReadOptions.make(getLockMode(lockForWrite))).status();
            }
        } else {
            if (view.useKey(key, value, keyThang, range)) {
                return doGetSearchKey(lockForWrite);
            }
        }
        return SOperationStatus.NOTFOUND;
    }

    /**
     * Pass-thru version of SCursor.getSearchKey().
     * Searches by record number in a BTREE-RECNO db with RECNO access.
     */
    private SOperationStatus doGetSearchKey(boolean lockForWrite)
        throws SDatabaseException {

        OpReadOptions options = OpReadOptions.make(getLockMode(lockForWrite));
        if (view.btreeRecNumAccess) {
            return cursor.getSearchRecordNumber(
                keyThang, primaryKeyThang, valueThang, options).status();
        } else {
            return cursor.getSearchKey(
                keyThang, primaryKeyThang, valueThang, options).status();
        }
    }

    /**
     * Binding version of SCursor.getSearchKeyRange(), no join cursor allowed.
     */
    SOperationStatus getSearchKeyRange(Object key, Object value,
                                      boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        OpReadOptions options = OpReadOptions.make(getLockMode(lockForWrite));
        if (view.dupsView) {
            if (view.useKey(key, value, primaryKeyThang, view.dupsRange)) {
                KeyRange.copy(view.dupsKey, keyThang);
                return cursor.getSearchBothRange(
                    keyThang, primaryKeyThang, valueThang, options).status();
            }
        } else {
            if (view.useKey(key, value, keyThang, range)) {
                return cursor.getSearchKeyRange(
                    keyThang, primaryKeyThang, valueThang, options).status();
            }
        }
        return SOperationStatus.NOTFOUND;
    }

    /**
     * Find the given key and value using getSearchBoth if possible or a
     * sequential scan otherwise, no join cursor allowed.
     */
    SOperationStatus findBoth(Object key, Object value, boolean lockForWrite)
        throws SDatabaseException {

        checkNoJoinCursor();
        OpReadOptions options = OpReadOptions.make(getLockMode(lockForWrite));
        view.useValue(value, valueThang, null);
        if (view.dupsView) {
            if (view.useKey(key, value, primaryKeyThang, view.dupsRange)) {
                KeyRange.copy(view.dupsKey, keyThang);
                if (otherThang == null) {
                    otherThang = new SDatabaseEntry();
                }
                SOperationStatus status = cursor.getSearchBoth(
                    keyThang, primaryKeyThang, otherThang, options).status();
                if (status == SOperationStatus.SUCCESS &&
                    KeyRange.equalBytes(otherThang, valueThang)) {
                    return status;
                }
            }
        } else if (view.useKey(key, value, keyThang, range)) {
            if (view.isSecondary()) {
                if (otherThang == null) {
                    otherThang = new SDatabaseEntry();
                }
                SOperationStatus status = cursor.getSearchKey(
                    keyThang, primaryKeyThang, otherThang, options).status();
                while (status == SOperationStatus.SUCCESS) {
                    if (KeyRange.equalBytes(otherThang, valueThang)) {
                        return status;
                    }
                    status = cursor.getNextDup(
                        keyThang, primaryKeyThang, otherThang,
                        options).status();
                }
                /* if status != SUCCESS set range cursor to invalid? */
            } else {
                return cursor.getSearchBoth(
                    keyThang, null, valueThang, options).status();
            }
        }
        return SOperationStatus.NOTFOUND;
    }

    /**
     * Find the given value using getSearchBoth if possible or a sequential
     * scan otherwise, no join cursor allowed.
     */
    SOperationStatus findValue(Object value, boolean findFirst)
        throws SDatabaseException {

        checkNoJoinCursor();

        if (view.entityBinding != null && !view.isSecondary() &&
            (findFirst || !view.dupsAllowed)) {
            return findBoth(null, value, false);
        } else {
            if (otherThang == null) {
                otherThang = new SDatabaseEntry();
            }
            view.useValue(value, otherThang, null);
            SOperationStatus status = findFirst ? getFirst(false)
                                               : getLast(false);
            while (status == SOperationStatus.SUCCESS) {
                if (KeyRange.equalBytes(valueThang, otherThang)) {
                    break;
                }
                status = findFirst ? getNext(false) : getPrev(false);
            }
            return status;
        }
    }

    /**
     * Calls SCursor.count(), no join cursor allowed.
     */
    int count()
        throws SDatabaseException {

        checkNoJoinCursor();
        if (view.dupsView) {
            return 1;
        } else {
            return cursor.count();
        }
    }

    /**
     * Binding version of SCursor.putCurrent().
     */
    SOperationStatus putCurrent(Object value)
        throws SDatabaseException {

        checkWriteAllowed(false);
        view.useValue(value, valueThang, keyThang);

        /*
         * Workaround for a DB core problem: With HASH type a put() with
         * different data is allowed.
         */
        boolean hashWorkaround = (view.dupsOrdered && !view.ordered);
        if (hashWorkaround) {
            if (otherThang == null) {
                otherThang = new SDatabaseEntry();
            }
            cursor.getCurrent(
                keyThang, primaryKeyThang, otherThang, OpReadOptions.EMPTY);
            if (KeyRange.equalBytes(valueThang, otherThang)) {
                return SOperationStatus.SUCCESS;
            } else {
                throw new IllegalArgumentException(
                  "Current data differs from put data with sorted duplicates");
            }
        }

        return cursor.putCurrent(valueThang);
    }

    /**
     * Binding version of SCursor.putAfter().
     */
    SOperationStatus putAfter(Object value)
        throws SDatabaseException {

        checkWriteAllowed(false);
        view.useValue(value, valueThang, null); /* why no key check? */
        return cursor.putAfter(keyThang, valueThang);
    }

    /**
     * Binding version of SCursor.putBefore().
     */
    SOperationStatus putBefore(Object value)
        throws SDatabaseException {

        checkWriteAllowed(false);
        view.useValue(value, valueThang, keyThang);
        return cursor.putBefore(keyThang, valueThang);
    }

    /**
     * Binding version of SCursor.put(), optionally returning the old value and
     * optionally using the current key instead of the key parameter.
     */
    SOperationStatus put(Object key, Object value, Object[] oldValue,
                        boolean useCurrentKey)
        throws SDatabaseException {

        initForPut(key, value, oldValue, useCurrentKey);
        return cursor.put(keyThang, valueThang);
    }

    /**
     * Binding version of SCursor.putNoOverwrite(), optionally using the current
     * key instead of the key parameter.
     */
    SOperationStatus putNoOverwrite(Object key, Object value,
                                   boolean useCurrentKey)
        throws SDatabaseException {

        initForPut(key, value, null, useCurrentKey);
        return cursor.putNoOverwrite(keyThang, valueThang);
    }

    /**
     * Binding version of SCursor.putNoDupData(), optionally returning the old
     * value and optionally using the current key instead of the key parameter.
     */
    SOperationStatus putNoDupData(Object key, Object value, Object[] oldValue,
                                 boolean useCurrentKey)
        throws SDatabaseException {

        initForPut(key, value, oldValue, useCurrentKey);
        if (view.dupsOrdered) {
            return cursor.putNoDupData(keyThang, valueThang);
        } else {
            if (view.dupsAllowed) {
                /* Unordered duplicates. */
                SOperationStatus status = cursor.getSearchBoth(
                    keyThang, primaryKeyThang, valueThang,
                    OpReadOptions.make(getLockMode(false))).status();
                if (status == SOperationStatus.SUCCESS) {
                    return SOperationStatus.KEYEXIST;
                } else {
                    return cursor.put(keyThang, valueThang);
                }
            } else {
                /* No duplicates. */
                return cursor.putNoOverwrite(keyThang, valueThang);
            }
        }
    }

    /**
     * Do setup for a put() operation.
     */
    private void initForPut(Object key, Object value, Object[] oldValue,
                            boolean useCurrentKey)
        throws SDatabaseException {

        checkWriteAllowed(false);
        if (!useCurrentKey && !view.useKey(key, value, keyThang, range)) {
            throw new IllegalArgumentException("key out of range");
        }
        if (oldValue != null) {
            oldValue[0] = null;
            if (!view.dupsAllowed) {
                SOperationStatus status = doGetSearchKey(true);
                if (status == SOperationStatus.SUCCESS) {
                    oldValue[0] = getCurrentValue();
                }
            }
        }
        view.useValue(value, valueThang, keyThang);
    }

    /**
     * Sets the key entry to the begin key of a single key range, so the next
     * time a putXxx() method is called that key will be used.
     */
    void useRangeKey() {
        if (!range.isSingleKey()) {
            throw DbCompat.unexpectedState();
        }
        KeyRange.copy(range.getSingleKey(), keyThang);
    }

    /**
     * Perform an arbitrary database 'delete' operation.
     */
    SOperationStatus delete()
        throws SDatabaseException {

        checkWriteAllowed(true);
        return cursor.delete();
    }

    /**
     * Returns the lock mode to use for a getXxx() operation.
     */
    SLockMode getLockMode(boolean lockForWrite) {

        /* Read-uncommmitted takes precedence over write-locking. */

        if (readUncommitted) {
            return SLockMode.READ_UNCOMMITTED;
        } else if (lockForWrite) {
            return view.currentTxn.getWriteLockMode();
        } else {
            return SLockMode.DEFAULT;
        }
    }

    /**
     * Throws an exception if a join cursor is in use.
     */
    private void checkNoJoinCursor() {

        if (joinCursor != null) {
            throw new UnsupportedOperationException
                ("Not allowed with a join cursor");
        }
    }

    /**
     * Throws an exception if write is not allowed or if a join cursor is in
     * use.
     */
    private void checkWriteAllowed(boolean allowSecondary) {

        checkNoJoinCursor();

        if (!writeAllowed || (!allowSecondary && view.isSecondary())) {
            throw new UnsupportedOperationException
                ("Writing is not allowed");
        }
    }
}
