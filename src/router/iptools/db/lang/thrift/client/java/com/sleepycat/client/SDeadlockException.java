/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * DeadlockException is thrown to a thread of control when multiple threads
 * competing for a lock are deadlocked, when a lock request has timed out or
 * when a lock request would need to block and the transaction has been
 * configured to not wait for locks.
 */
public class SDeadlockException extends SDatabaseException {

    public SDeadlockException(String message, int errno) {
        super(message, errno);
    }

}
