/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * Locking modes for database operations. Locking modes are required parameters
 * for operations that retrieve data or modify the database.
 */
public enum SLockMode {
    /**
     * Acquire read locks for read operations and write locks for write
     * operations.
     */
    DEFAULT,
    /**
     * Read committed isolation provides for cursor stability but not
     * repeatable reads.
     */
    READ_COMMITTED,
    /** Read modified but not yet committed data. */
    READ_UNCOMMITTED,
    /** Acquire write locks instead of read locks when doing the retrieval. */
    RMW
}
