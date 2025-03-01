/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * The base exception for all client driver exceptions.
 */
public class SDatabaseException extends RuntimeException {

    private final int errno;

    public SDatabaseException(String message) {
        this(message, 0);
    }

    public SDatabaseException(Exception cause) {
        this(cause == null ? null : cause.toString(), cause, 0);
    }

    public SDatabaseException(String message, int errno) {
        this(message, null, errno);
    }

    public SDatabaseException(String message, Throwable cause) {
        this(message, cause, 0);
    }

    public SDatabaseException(String message, Throwable cause, int errno) {
        super(message, cause);
        this.errno = errno;
    }

    public int getErrno() {
        return this.errno;
    }
}
