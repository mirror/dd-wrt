/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TQueueStat;

/**
 * The SQueueStats object is used to return Queue database statistics.
 */
public class SQueueStats implements SDatabaseStats {
    /** The Thrift object. */
    private final TQueueStat stat;

    SQueueStats(TQueueStat stat) {
        this.stat = stat;
    }

    /**
     * The next available record number.
     *
     * @return the next available record number
     */
    public int getCurRecno() {
        return this.stat.curRecno;
    }

    /**
     * The underlying database extent size, in pages.
     *
     * @return the underlying database extent size, in pages
     */
    public int getExtentSize() {
        return this.stat.extentSize;
    }

    /**
     * The first undeleted record in the database.
     *
     * @return the first undeleted record in the database
     */
    public int getFirstRecno() {
        return this.stat.firstRecno;
    }

    /**
     * The magic number that identifies the file as a Queue file.
     *
     * @return the magic number that identifies the file as a Queue file
     */
    public int getMagic() {
        return this.stat.magic;
    }

    /**
     * The number of records in the database.
     * <p>
     * If the {@link SDatabase#getStats} call was configured by the {@link
     * SStatsConfig#setFast} method, the count will be the last saved value
     * unless it has never been calculated, in which case it will be 0.
     *
     * @return the number of records in the database
     */
    public int getNumData() {
        return this.stat.numData;
    }

    /**
     * The number of records in the database.
     * <p>
     * If the {@link SDatabase#getStats} call was configured by the {@link
     * SStatsConfig#setFast} method, the count will be the last saved value
     * unless it has never been calculated, in which case it will be 0.
     *
     * @return the number of records in the database
     */
    public int getNumKeys() {
        return this.stat.numKeys;
    }

    /**
     * The number of pages in the database.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of pages in the database
     */
    public int getPages() {
        return this.stat.pages;
    }

    /**
     * The number of bytes free in database pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free in database pages
     */
    public int getPagesFree() {
        return this.stat.pagesFree;
    }

    /**
     * The underlying database page size, in bytes.
     *
     * @return the underlying database page size, in bytes
     */
    public int getPageSize() {
        return this.stat.pageSize;
    }

    /**
     * The length of the records.
     *
     * @return the length of the records
     */
    public int getReLen() {
        return this.stat.reLen;
    }

    /**
     * The padding byte value for the records.
     *
     * @return the padding byte value for the records
     */
    public int getRePad() {
        return this.stat.rePad;
    }

    /**
     * The version of the Queue database.
     *
     * @return the version of the Queue database
     */
    public int getVersion() {
        return this.stat.version;
    }

    /**
     * For convenience, the SQueueStats class has a toString method
     * that lists all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "QueueStats:"
                + "\n  magic=" + getMagic()
                + "\n  version=" + getVersion()
                + "\n  nkeys=" + getNumKeys()
                + "\n  ndata=" + getNumData()
                + "\n  pagesize=" + getPageSize()
                + "\n  extentsize=" + getExtentSize()
                + "\n  pages=" + getPages()
                + "\n  re_len=" + getReLen()
                + "\n  re_pad=" + getRePad()
                + "\n  pgfree=" + getPagesFree()
                + "\n  first_recno=" + getFirstRecno()
                + "\n  cur_recno=" + getCurRecno()
                ;
    }
}
