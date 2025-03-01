/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * Thrown when the database environment needs to be recovered. Errors can occur
 * in where the only solution is to shut down the application and run recovery.
 * When a fatal error occurs, this exception will be thrown, and all subsequent
 * calls will also fail in the same way. When this occurs, recovery should be
 * performed.
 */
public class SRunRecoveryException extends SDatabaseException {

    public SRunRecoveryException(String message, int errno) {
        super(message, errno);
    }

}
