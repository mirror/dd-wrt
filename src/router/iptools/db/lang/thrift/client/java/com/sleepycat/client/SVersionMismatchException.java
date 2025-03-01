/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * Thrown if the version of the Berkeley DB library used by the server doesn't
 * match the version that created the database environment. In this case, the
 * server must be shut down and the correct library version must be installed
 * on the server.
 */
public class SVersionMismatchException extends SDatabaseException {

    public SVersionMismatchException(String message, int errno) {
        super(message, errno);
    }

}
