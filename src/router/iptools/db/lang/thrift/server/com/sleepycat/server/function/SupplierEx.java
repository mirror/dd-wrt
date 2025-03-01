/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.function;

/**
 * A supplier interface that allows checked exception to be thrown.
 */
@FunctionalInterface
public interface SupplierEx<T> {
    T getWithException() throws Exception;
}
