/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * A ReadWriteLockable is an object that contains a read-write lock.
 */
public abstract class ReadWriteLockable {
    /** The read-write lock. */
    private final ReentrantReadWriteLock rwLock = new ReentrantReadWriteLock();

    /* For test only. */
    ReentrantReadWriteLock getRwLock() {
        return this.rwLock;
    }

    /**
     * Acquire the read lock if possible.
     *
     * @return true if the read lock is acquired
     */
    protected final boolean tryReadLock() {
        return this.rwLock.readLock().tryLock();
    }

    /**
     * Release the read lock held by the current thread.
     */
    protected final void unlockRead() {
        this.rwLock.readLock().unlock();
    }

    /**
     * Acquire the write lock.
     */
    protected final void writeLock() {
        this.rwLock.writeLock().lock();
    }

    /**
     * Release the write lock held by the current thread.
     */
    protected final void unlockWrite() {
        this.rwLock.writeLock().unlock();
    }
}
