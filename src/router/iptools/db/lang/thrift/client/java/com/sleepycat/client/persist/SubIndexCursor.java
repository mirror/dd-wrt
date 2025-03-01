/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.util.keyrange.RangeCursor;

/**
 * The cursor for a SubIndex treats Dup and NoDup operations specially because
 * the SubIndex never has duplicates -- the keys are primary keys.  So a
 * next/prevDup operation always returns null, and a next/prevNoDup operation
 * actually does next/prev.
 *
 * @author Mark Hayes
 */
class SubIndexCursor<V> extends BasicCursor<V> {

    SubIndexCursor(RangeCursor cursor, ValueAdapter<V> adapter) {
        super(cursor, adapter, false/*updateAllowed*/);
    }

    @Override
    public EntityCursor<V> dup()
        throws SDatabaseException {

        return new SubIndexCursor<V>(cursor.dup(true), adapter);
    }

    @Override
    public V nextDup(SLockMode lockMode) {
        checkInitialized();
        return null;
    }

    @Override
    public V nextNoDup(SLockMode lockMode)
        throws SDatabaseException {

        return next(lockMode);
    }

    @Override
    public V prevDup(SLockMode lockMode) {
        checkInitialized();
        return null;
    }

    @Override
    public V prevNoDup(SLockMode lockMode)
        throws SDatabaseException {

        return prev(lockMode);
    }

}
