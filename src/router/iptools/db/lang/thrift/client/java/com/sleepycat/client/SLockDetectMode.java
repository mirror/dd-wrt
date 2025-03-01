/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TLockDetectMode;

/**
 * Deadlock detection mode.
 */
@SuppressWarnings("unused")
public enum SLockDetectMode {
    /** Reject lock requests which have timed out. */
    EXPIRE,
    /** Reject the lock request for the locker ID with the most locks. */
    MAX_LOCKS,
    /** Reject the lock request for the locker ID with the most write locks. */
    MAX_WRITE,
    /** Reject the lock request for the locker ID with the fewest locks. */
    MIN_LOCKS,
    /** Reject the lock request for the locker ID with the fewest write locks. */
    MIN_WRITE,
    /** Reject the lock request for the locker ID with the oldest lock. */
    OLDEST,
    /** Reject the lock request for a random locker ID. */
    RANDOM,
    /** Reject the lock request for the locker ID with the youngest lock. */
    YOUNGEST;

    static TLockDetectMode toThrift(SLockDetectMode mode) {
        return TLockDetectMode.valueOf(mode.name());
    }

    static SLockDetectMode toBdb(TLockDetectMode mode) {
        return SLockDetectMode.valueOf(mode.name());
    }
}
