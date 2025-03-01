/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import java.util.Iterator;

import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.compat.DbCompat.OpReadOptions;
import com.sleepycat.client.compat.DbCompat.OpResult;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.util.keyrange.RangeCursor;

/**
 * Implements EntityCursor and uses a ValueAdapter so that it can enumerate
 * either keys or entities.
 *
 * @author Mark Hayes
 */
class BasicCursor<V> implements EntityCursor<V> {

    RangeCursor cursor;
    ValueAdapter<V> adapter;
    boolean updateAllowed;
    SDatabaseEntry key;
    SDatabaseEntry pkey;
    SDatabaseEntry data;

    BasicCursor(RangeCursor cursor,
                ValueAdapter<V> adapter,
                boolean updateAllowed) {
        this.cursor = cursor;
        this.adapter = adapter;
        this.updateAllowed = updateAllowed;
        key = adapter.initKey();
        pkey = adapter.initPKey();
        data = adapter.initData();
    }

    public V first()
        throws SDatabaseException {

        return first(null);
    }

    public V first(SLockMode lockMode)
        throws SDatabaseException {

        return returnValue(
            cursor.getFirst(key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V last()
        throws SDatabaseException {

        return last(null);
    }

    public V last(SLockMode lockMode)
        throws SDatabaseException {

        return returnValue(
            cursor.getLast(key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V next()
        throws SDatabaseException {

        return next(null);
    }

    public V next(SLockMode lockMode)
        throws SDatabaseException {

        return returnValue(
            cursor.getNext(key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V nextDup()
        throws SDatabaseException {

        return nextDup(null);
    }

    public V nextDup(SLockMode lockMode)
        throws SDatabaseException {

        checkInitialized();
        return returnValue(
            cursor.getNextDup(key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V nextNoDup()
        throws SDatabaseException {

        return nextNoDup(null);
    }

    public V nextNoDup(SLockMode lockMode)
        throws SDatabaseException {

        return returnValue(
            cursor.getNextNoDup(
                key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V prev()
        throws SDatabaseException {

        return prev(null);
    }

    public V prev(SLockMode lockMode)
        throws SDatabaseException {

        return returnValue(
            cursor.getPrev(key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V prevDup()
        throws SDatabaseException {

        return prevDup(null);
    }

    public V prevDup(SLockMode lockMode)
        throws SDatabaseException {

        checkInitialized();
        return returnValue(
            cursor.getPrevDup(key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V prevNoDup()
        throws SDatabaseException {

        return prevNoDup(null);
    }

    public V prevNoDup(SLockMode lockMode)
        throws SDatabaseException {

        return returnValue(
            cursor.getPrevNoDup(
                key, pkey, data, OpReadOptions.make(lockMode)));
    }

    public V current()
        throws SDatabaseException {

        return current(null);
    }

    public V current(SLockMode lockMode)
        throws SDatabaseException {

        checkInitialized();
        return returnValue(
            cursor.getCurrent(key, pkey, data, OpReadOptions.make(lockMode)));
    }


    public int count()
        throws SDatabaseException {

        checkInitialized();
        return cursor.count();
    }



    public Iterator<V> iterator() {
        return iterator(null);
    }

    public Iterator<V> iterator(SLockMode lockMode) {
        return new BasicIterator(this, lockMode);
    }

    public boolean update(V entity)
        throws SDatabaseException {


        if (!updateAllowed) {
            throw new UnsupportedOperationException(
                "Update not allowed on a secondary index");
        }
        checkInitialized();
        adapter.valueToData(entity, data);

        return cursor.getCursor().putCurrent(data) == SOperationStatus.SUCCESS;
    }


    public boolean delete()
        throws SDatabaseException {


        checkInitialized();
        return cursor.getCursor().delete() == SOperationStatus.SUCCESS;
    }


    public EntityCursor<V> dup()
        throws SDatabaseException {

        return new BasicCursor<V>(cursor.dup(true), adapter, updateAllowed);
    }

    public void close()
        throws SDatabaseException {

        cursor.close();
    }



    void checkInitialized()
        throws IllegalStateException {

        if (!cursor.isInitialized()) {
            throw new IllegalStateException
                ("Cursor is not initialized at a valid position");
        }
    }

    V returnValue(OpResult opResult) {
        V value;
        if (opResult.isSuccess()) {
            value = adapter.entryToValue(key, pkey, data);
        } else {
            value = null;
        }
        /* Clear entries to save memory. */
        adapter.clearEntries(key, pkey, data);
        return value;
    }

}
