/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.db;

/**
An interface specifying in which slice database records are
read or written based on the given key. If this interface is
not used, then the entire contents of the key are used to 
determine which slice contains a given database record. Use
this interface only if a portion of the key should be used to
determine slice placement.
<p>
You configure the database with this handler using the
{@link DatabaseConfig#setSliceCallback} method.
*/
public interface Slice {
    /**
    The application-specific database slice callback.
    <p>
    @param db
    The enclosing database handle.
    @param key
    A database entry representing the database key that is being put into the database, or
    used to retrieve an existing record in the database.
    @param sliceKey
    A database entry containing data used to determine to what slice the key is applied.
    @return
    True on success, false if there is an error.
    */
    boolean slice(final Database db, final DatabaseEntry key, DatabaseEntry sliceKey);
}
