/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.apache.thrift.TException;

/**
 * A functional interface which represents a remote BdbService invocation that
 * may throw all types of exceptions.
 */
@FunctionalInterface
interface RemoteServiceCallable<V> {
    V call() throws TException;
}
