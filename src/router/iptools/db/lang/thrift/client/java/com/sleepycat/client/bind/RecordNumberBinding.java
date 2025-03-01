/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.bind;

import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabaseEntry;

import java.nio.ByteOrder;
import java.util.Objects;

/**
 * An <code>EntryBinding</code> that treats a record number key entry as a
 * <code>Long</code> key object.
 *
 * <p>Record numbers are returned as <code>Long</code> objects, although on
 * input any <code>Number</code> object may be used.</p>
 *
 * @author Mark Hayes
 */
public class RecordNumberBinding implements EntryBinding {

    private final ByteOrder byteOrder;

    /**
     * Creates a byte array binding.
     *
     * @param byteOrder the server's native byte order
     */
    public RecordNumberBinding(ByteOrder byteOrder) {
        this.byteOrder = Objects.requireNonNull(byteOrder);
    }

    // javadoc is inherited
    public Long entryToObject(SDatabaseEntry entry) {

        return Long.valueOf(entryToRecordNumber(entry, byteOrder));
    }

    // javadoc is inherited
    public void objectToEntry(Object object, SDatabaseEntry entry) {

        recordNumberToEntry(((Number) object).longValue(), entry, byteOrder);
    }

    /**
     * Utility method for use by bindings to translate a entry buffer to an
     * record number integer.
     *
     * @param entry the entry buffer.
     *
     * @param byteOrder the server's native byte order
     *
     * @return the record number.
     */
    public static long entryToRecordNumber(SDatabaseEntry entry,
                                           ByteOrder byteOrder) {

        return DbCompat.getRecordNumber(entry, byteOrder) & 0xFFFFFFFFL;
    }

    /**
     * Utility method for use by bindings to translate a record number integer
     * to a entry buffer.
     *
     * @param recordNumber the record number.
     *
     * @param entry the entry buffer to hold the record number.
     *
     * @param byteOrder the server's native byte order
     */
    public static void recordNumberToEntry(long recordNumber,
                                           SDatabaseEntry entry,
                                           ByteOrder byteOrder) {
        entry.setData(new byte[4], 0, 4);
        DbCompat.setRecordNumber(entry, (int) recordNumber, byteOrder);
    }
}
