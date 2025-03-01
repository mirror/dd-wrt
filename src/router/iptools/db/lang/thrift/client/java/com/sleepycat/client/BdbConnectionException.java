/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * Signals that an error occurred while attempting to connect to a BdbServer.
 */
public class BdbConnectionException extends SDatabaseException {

    public BdbConnectionException(String message, Throwable cause) {
        super(message, cause);
    }

}
