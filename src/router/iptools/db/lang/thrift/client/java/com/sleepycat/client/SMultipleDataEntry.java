/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDbt;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * A container that holds multiple data {@link SDatabaseEntry}s returned by a
 * single {@link SDatabase} or {@link SCursor} get call.
 */
public class SMultipleDataEntry implements SDatabaseEntryBase {
    /** The batch size in bytes. */
    private int batchSize;

    /** The list of data entries. */
    private final List<SDatabaseEntry> entries;

    /** The iterator indicating the next returned item. */
    private Iterator<SDatabaseEntry> iterator;

    /**
     * Construct a SMultipleDataEntry with no data entries.
     */
    public SMultipleDataEntry() {
        this.entries = new LinkedList<>();
    }

    void setEntriesFromDbts(List<TDbt> dbts) {
        this.entries.clear();
        dbts.forEach(t -> this.entries.add(new SDatabaseEntry(t)));
        this.iterator = this.entries.iterator();
    }

    <T> List<T> map(Function<SDatabaseEntry, T> function) {
        return this.entries.stream().map(function)
                .collect(Collectors.toList());
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
     * Append an entry to the bulk buffer.
     *
     * @param data the record to be appended
     */
    public void append(byte[] data) {
        entries.add(new SDatabaseEntry(data));
    }

    /**
     * Get the next data element in the returned set. This method may only be
     * called after a successful call to a {@link SDatabase} or {@link SCursor}
     * get method with this object as the data parameter.
     * <p>
     * When used with the Queue and Recno access methods, data.getData() will
     * return null for deleted records
     *
     * @param data an entry that is set to refer to the next data element in
     * the returned set
     * @return indicates whether a value was found. A return of false indicates
     * that the end of the set was reached.
     */
    public boolean next(SDatabaseEntry data) {
        if (this.iterator.hasNext()) {
            data.setDataFromTDbt(this.iterator.next().getThriftObj());
            return true;
        } else {
            return false;
        }
    }
}
