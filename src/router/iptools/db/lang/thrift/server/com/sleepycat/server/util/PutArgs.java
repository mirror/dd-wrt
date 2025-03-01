/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.thrift.TDbPutConfig;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TKeyDataWithSecondaryKeys;

import java.util.List;
import java.util.stream.Collectors;

/**
 * PutArgs represents the arguments for a database or cursor put call.
 */
public class PutArgs extends InputArgs {
    /** The key argument. */
    public DatabaseEntry key;
    /** The data argument. */
    public DatabaseEntry data;

    /**
     * Construct the arguments for a Database put call.
     *
     * @param pairs the primary key/data pairs along with their secondary keys
     * @param isRecordKey if keys are record numbers
     * @param config the configuration
     */
    public PutArgs(List<TKeyDataWithSecondaryKeys> pairs, boolean isRecordKey,
            TDbPutConfig config) throws DatabaseException {
        if (pairs.size() > 1 || config == TDbPutConfig.OVERWRITE_DUP) {
            constructMultipleKey(pairs, isRecordKey);
        } else {
            constructSingle(pairs.get(0));
        }
    }

    /**
     * Construct the arguments for a Cursor put call.
     *
     * @param pair the primary key/data pair with its secondary keys
     */
    public PutArgs(TKeyDataWithSecondaryKeys pair) {
        constructSingle(pair);
    }

    private void constructSingle(TKeyDataWithSecondaryKeys pair) {
        if (pair.pdata.isSetPartial() && pair.pdata.partial) {
            throw new UnsupportedOperationException("Partial update is not " +
                    "supported for databases having secondary databases.");
        }
        this.key = Adapters.toBdbType(pair.pkey);
        this.data = Adapters.toBdbType(pair.pdata);
    }

    private void constructMultipleKey(List<TKeyDataWithSecondaryKeys> pairs,
            boolean isRecordKey) throws DatabaseException {
        this.key = convert(extract(pairs), isRecordKey);
        this.data = null;
    }

    private List<TKeyData> extract(List<TKeyDataWithSecondaryKeys> pairs) {
        return pairs.stream()
                .map(p -> new TKeyData().setKey(p.pkey).setData(p.pdata))
                .collect(Collectors.toList());
    }
}
