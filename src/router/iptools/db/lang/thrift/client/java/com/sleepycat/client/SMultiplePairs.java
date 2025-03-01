/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TKeyData;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.function.BiFunction;
import java.util.stream.Collectors;

/**
 * An abstract class representing a container that holds multiple key/data
 * {@link SDatabaseEntry} pairs. Use one of the concrete subclasses depending
 * on whether you need:
 * <ul>
 * <li>multiple key / data item pairs: {@link SMultipleKeyDataEntry}</li>
 * <li>multiple record number / data item pairs: {@link
 * SMultipleRecnoDataEntry}</li>
 * </ul>
 */
public abstract class SMultiplePairs implements SDatabaseEntryBase {
    /** The batch size in bytes. */
    private int batchSize;

    /** The list of data entries. */
    private final List<Pair> entries;

    /** The iterator indicating the next returned item. */
    private Iterator<Pair> iterator;

    protected SMultiplePairs() {
        this.entries = new LinkedList<>();
    }

    void setEntriesFromKeyDatas(List<TKeyData> pairs) {
        this.entries.clear();
        pairs.forEach(p -> append(new SDatabaseEntry(p.key),
                new SDatabaseEntry(p.data)));
        this.iterator = this.entries.iterator();
    }

    <T> List<T> map(BiFunction<SDatabaseEntry, SDatabaseEntry, T> function) {
        return this.entries.stream().map(p -> function.apply(p.key, p.data))
                .collect(Collectors.toList());
    }

    protected void append(SDatabaseEntry key, SDatabaseEntry data) {
        this.entries.add(new Pair(key, data));
    }

    /**
     * Return the amount of data to be returned in a single get() call.
     *
     * @return the amount of data to be returned in a single get() call.
     */
    public int getBatchSize() {
        return batchSize;
    }

    /**
     * When used as an output argument to a database or cursor get() call, set
     * the amount of data (in total bytes) to be returned.
     * <p>
     * This method is only advisory. The exact amount of data returned may be
     * more or less than the amount specified.
     *
     * @param batchSize the amount of data to be returned in a single get()
     * call
     */
    public void setBatchSize(int batchSize) {
        this.batchSize = batchSize;
    }

    /**
     * Get the next key/data pair in the returned set. This method may only be
     * called after a successful call to a {@link SDatabase} or {@link SCursor}
     * get method with this object as the data parameter.
     * <p>
     * When used with the Queue and Recno access methods, data.getData() will
     * return null for deleted records
     *
     * @param key an entry that is set to refer to the next key element in the
     * returned set
     * @param data an entry that is set to refer to the next data element in
     * the
     * returned set
     * @return indicates whether a value was found. A return of false indicates
     * that the end of the set was reached.
     */
    public boolean next(SDatabaseEntry key, SDatabaseEntry data) {
        if (this.iterator.hasNext()) {
            Pair pair = this.iterator.next();
            key.setDataFromTDbt(pair.key.getThriftObj());
            data.setDataFromTDbt(pair.data.getThriftObj());
            return true;
        } else {
            return false;
        }
    }

    private static class Pair {
        final SDatabaseEntry key;
        final SDatabaseEntry data;

        Pair(SDatabaseEntry key, SDatabaseEntry data) {
            this.key = key;
            this.data = data;
        }
    }
}
