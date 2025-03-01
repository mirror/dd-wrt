/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
package com.sleepycat.db;

import com.sleepycat.db.internal.DbConstants;
import com.sleepycat.db.internal.DbEnv;


/**
A MetaCheckSumFailException is thrown when a checksum mismatch is detected
on a database metadata page.  Either the database is corrupted or the file
is not a Berkeley DB database file. 
*/
public class MetaCheckSumFailException extends DatabaseException {
    private String message;

    /* package */ MetaCheckSumFailException(final String message,
  				final int errno, final DbEnv dbenv) {
        super(message, errno, dbenv);
	this.message = message;
    }

   /** {@inheritDoc} */
    public String toString() {
        return message;
    }

}
