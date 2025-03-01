/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.db;

/**
An interface specifying how Btree prefixes should be calculated.
*/
public interface BtreePrefixCalculator {
    /**
    The application-specific Btree prefix callback.
    <p>
    @param db
    The enclosing database handle.
    @param dbt1
    A database entry representing a database key.
    @param dbt2
    A database entry representing a database key.
    @return the number of bytes of the second key parameter that would be
    required by the Btree key comparison function to determine the second
    key parameter's ordering relationship with respect to the first key
    parameter
    */
    int prefix(Database db, DatabaseEntry dbt1, DatabaseEntry dbt2);
}
