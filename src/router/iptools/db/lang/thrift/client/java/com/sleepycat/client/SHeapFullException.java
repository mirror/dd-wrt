/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * A HeapFullException is thrown when an attempt was made to add or update a
 * record in a Heap database. However, the size of the database was constrained
 * using DatabaseConfig.setHeapsize(), and that limit has been reached.
 */
public class SHeapFullException extends SDatabaseException {

    public SHeapFullException(String message, int errno) {
        super(message, errno);
    }

}
