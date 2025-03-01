/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TOperationStatus;

/**
 * Status values from database operations.
 */
public enum SOperationStatus {
    /**
     * The cursor operation was unsuccessful because the current record was
     * deleted.
     */
    KEYEMPTY,
    /**
     * The operation to insert data was configured to not allow overwrite and
     * the key already exists in the database.
     */
    KEYEXIST,
    /** The requested key/data pair was not found. */
    NOTFOUND,
    /** The operation was successful. */
    SUCCESS;

    static TOperationStatus toThrift(SOperationStatus status) {
        switch (status) {
            case KEYEMPTY:
                return TOperationStatus.KEY_EMPTY;
            case KEYEXIST:
                return TOperationStatus.KEY_EXIST;
            case NOTFOUND:
                return TOperationStatus.NOT_FOUND;
            case SUCCESS:
                return TOperationStatus.SUCCESS;
            default:
                throw new IllegalArgumentException("Unknown status");
        }
    }

    static SOperationStatus toBdb(TOperationStatus status) {
        switch (status) {
            case KEY_EMPTY:
                return KEYEMPTY;
            case KEY_EXIST:
                return KEYEXIST;
            case NOT_FOUND:
                return NOTFOUND;
            case SUCCESS:
                return SUCCESS;
            default:
                throw new IllegalArgumentException("Unknown status");
        }
    }
}
