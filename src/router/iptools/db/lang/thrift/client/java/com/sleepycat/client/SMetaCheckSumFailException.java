/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * A MetaCheckSumFailException is thrown when a checksum mismatch is detected on
 * a database metadata page. Either the database is corrupted or the file is not
 * a Berkeley DB database file.
 */
public class SMetaCheckSumFailException extends SDatabaseException {

    public SMetaCheckSumFailException(String message, int errno) {
        super(message, errno);
    }

}
