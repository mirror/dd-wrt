/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
package com.sleepycat.db;

import com.sleepycat.db.internal.DbEnv;

/**
Thrown when a slice environment or database is missing or corrupt.
 *
This is a fatal error.  When a fatal error occurs, this
exception will be thrown, and all subsequent calls will also fail.
When this occurs, shut down the application and run recovery.
*/
public class SliceCorruptException extends DatabaseException {
    /* package */ SliceCorruptException(final String s,
                                   final int errno,
                                   final DbEnv dbenv) {
        super(s, errno, dbenv);
    }
}