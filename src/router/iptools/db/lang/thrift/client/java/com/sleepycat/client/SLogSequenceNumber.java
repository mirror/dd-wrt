/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * The SLogSequenceNumber object is a log sequence number which specifies a
 * unique location in a log file. A SLogSequenceNumber consists of two unsigned
 * 32-bit integers -- one specifies the log file number, and the other
 * specifies the offset in the log file.
 */
public class SLogSequenceNumber {
    /** The log file number. */
    private final int file;

    /** The log file offset. */
    private final int offset;

    /**
     * Construct a SLogSequenceNumber with the specified file and offset.
     *
     * @param file the log file number.
     * @param offset the log file offset.
     */
    public SLogSequenceNumber(final int file, final int offset) {
        this.file = file;
        this.offset = offset;
    }

    /**
     * Return the file number component.
     *
     * @return the file number component
     */
    public int getFile() {
        return file;
    }

    /**
     * Return the file offset component.
     *
     * @return the file offset component
     */
    public int getOffset() {
        return offset;
    }

    @Override
    public String toString() {
        return "SLogSequenceNumber{" +
                "file=" + file +
                ", offset=" + offset +
                '}';
    }
}
