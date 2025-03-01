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
import com.sleepycat.db.MultipleDataEntry;
import com.sleepycat.db.MultipleEntry;
import com.sleepycat.db.MultipleKeyDataEntry;
import com.sleepycat.db.MultipleRecnoDataEntry;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TKeyData;

import java.util.List;

/**
 * InputArgs is a base class that provides helper methods for converting
 * Thrift-typed input arguments to Bdb-typed input arguments.
 */
abstract class InputArgs {

    /**
     * Converting a list of Thrift key/data pairs to a Bdb MultipleEntry.
     *
     * @param pairs a list of pairs
     * @param isRecordKey if keys are record numbers
     * @return a MultipleEntry
     * @throws DatabaseException if any error occurs
     */
    protected MultipleEntry convert(List<TKeyData> pairs, boolean isRecordKey)
            throws DatabaseException {
        if (pairs.isEmpty()) {
            throw new IllegalArgumentException("Pairs cannot be empty.");
        }

        TKeyData first = pairs.get(0);
        if (isRecordKey) {
            return toRecnoData(pairs);
        } else if (!first.isSetData()) {
            return toData(pairs);
        } else {
            return toKeyData(pairs);
        }
    }

    /**
     * Return the buffer size required to encode the specified key/data pair.
     *
     * @param pair the key/data pair
     * @return the required buffer size
     */
    private int getKeyDataSize(TKeyData pair) {
        if (!pair.isSetKey()) {
            throw new IllegalArgumentException("Key must be set.");
        }
        int size = getTDbtSize(pair.key);
        if (pair.isSetData()) {
            size += getTDbtSize(pair.data);
        }
        return size;
    }

    private int getTDbtSize(TDbt dbt) {
        int size = dbt.isSetData() ? dbt.getData().length : 0;
        return size + Integer.BYTES * 2;
    }

    private MultipleEntry toRecnoData(List<TKeyData> pairs) throws
            DatabaseException {
        MultipleRecnoDataEntry arg =
                initEntry(pairs, new MultipleRecnoDataEntry());
        for (TKeyData pair : pairs) {
            arg.append(new DatabaseEntry(pair.key.getData()).getRecordNumber(),
                    pair.isSetData() ? pair.data.getData() : new byte[0]);
        }
        return arg;
    }

    private MultipleEntry toData(List<TKeyData> pairs)
            throws DatabaseException {
        MultipleDataEntry arg = initEntry(pairs, new MultipleDataEntry());
        for (TKeyData pair : pairs) {
            arg.append(pair.key.getData());
        }
        return arg;
    }

    private MultipleEntry toKeyData(List<TKeyData> pairs)
            throws DatabaseException {
        MultipleKeyDataEntry arg = initEntry(pairs, new MultipleKeyDataEntry());
        for (TKeyData pair : pairs) {
            arg.append(pair.key.getData(), pair.data.getData());
        }
        return arg;
    }

    private <T extends MultipleEntry> T initEntry(List<TKeyData> pairs,
            T entry) {
        int bufSize = pairs.stream().mapToInt(this::getKeyDataSize).sum() +
                Integer.BYTES;
        entry.setData(new byte[bufSize]);
        entry.setUserBuffer(bufSize, true);
        return entry;
    }
}
