/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDatabaseException;
import com.sleepycat.thrift.TFileNotFoundException;
import com.sleepycat.thrift.TIOException;
import com.sleepycat.thrift.TResourceInUseException;
import com.sleepycat.thrift.TRuntimeException;
import org.apache.thrift.TException;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Constructor;

/**
 * RemoteCallHelper implements a few helper methods for making remote
 * BdbService invocations.
 */
interface RemoteCallHelper {

    default <V> V remoteCallWithIOException(RemoteServiceCallable<V> remote)
            throws IOException {
        try {
            return handleRuntimeExceptions(remote);
        } catch (TFileNotFoundException e) {
            throw new FileNotFoundException(e.message);
        } catch (TIOException e) {
            throw new IOException(e.message);
        } catch (TException e) {
            throw new RuntimeException(e);
        }
    }

    default <V> V remoteCall(RemoteServiceCallable<V> remote) {
        try {
            return handleRuntimeExceptions(remote);
        } catch (TException e) {
            throw new RuntimeException(e);
        }
    }

    default <V> V handleRuntimeExceptions(RemoteServiceCallable<V> remote)
            throws TException {
        try {
            return remote.call();
        } catch (TResourceInUseException e) {
            throw new SResourceInUseException(e.message);
        } catch (TDatabaseException e) {
            switch (e.type) {
                case DEADLOCK:
                    throw new SDeadlockException(e.message, e.errorNumber);
                case HEAP_FULL:
                    throw new SHeapFullException(e.message, e.errorNumber);
                case LOCK_NOT_GRANTED:
                    throw new SLockNotGrantedException(e.message,
                            e.errorNumber);
                case META_CHECKSUM_FAIL:
                    throw new SMetaCheckSumFailException(e.message,
                            e.errorNumber);
                case RUN_RECOVERY:
                    throw new SRunRecoveryException(e.message, e.errorNumber);
                case VERSION_MISMATCH:
                    throw new SVersionMismatchException(e.message,
                            e.errorNumber);
                default:
                    throw new SDatabaseException(e.message, e.errorNumber);
            }
        } catch (TRuntimeException e) {
            try {
                Class<?> exClass = Class.forName(e.getFullClassName());
                Constructor constructor = exClass.getConstructor(String.class);
                throw (RuntimeException) constructor.newInstance(e.message);
            } catch (RuntimeException e1) {
                throw e1;
            } catch (Exception e1) {
                throw new RuntimeException(e.message, e);
            }
        }
    }
}