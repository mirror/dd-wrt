/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import java.nio.ByteOrder;

/**
 * A container that holds multiple record number / data item pairs.
 */
public class SMultipleRecnoDataEntry extends SMultiplePairs {

    /**
     * Append an entry to the container.
     *
     * @param recno the record number of the record to be added
     * @param byteOrder the byte order used to decode the record number. If
     * this entry is retrieved from or will be sent to a server, this must be
     * the server's byte order. See
     * {@link BdbServerConnection#getServerByteOrder()}.
     * @param data an array containing the data to be added
     */
    public void append(int recno, ByteOrder byteOrder, byte[] data) {
        append(new SDatabaseEntry().setRecordNumber(recno, byteOrder),
                new SDatabaseEntry(data));
    }
}
