/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDatabaseType;

/**
 * Database types.
 */
public enum SDatabaseType {
    /** The database is a Btree. */
    BTREE,
    /** The database is a Hash. */
    HASH;
    /** The database is a Heap. Not supported in this release. */
    //HEAP,
    /** The database is a Queue. Not supported in this release. */
    //QUEUE,
    /** The database is a Recno. */
    //RECNO;

    static TDatabaseType toThrift(SDatabaseType type) {
        return TDatabaseType.valueOf(type.name());
    }

    static SDatabaseType toBdb(TDatabaseType type) {
        return SDatabaseType.valueOf(type.name());
    }
}
