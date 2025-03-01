/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.THashStat;

/**
 * The SHashStats object is used to return Hash database statistics.
 */
public class SHashStats implements SDatabaseStats {
    /** The Thrift object. */
    private final THashStat stat;

    SHashStats(THashStat stat) {
        this.stat = stat;
    }

    /**
     * The number of bytes free on bucket pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free on bucket pages
     */
    public long getBFree() {
        return this.stat.BFree;
    }

    /**
     * The number of bytes free on hash overflow (big item) pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free on hash overflow pages
     */
    public long getBigBFree() {
        return this.stat.bigBFree;
    }

    /**
     * The number of hash overflow pages (created when key/data is too big for
     * the page).
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of hash overflow pages
     */
    public int getBigPages() {
        return this.stat.bigPages;
    }

    /**
     * The number of hash buckets.
     *
     * @return the number of hash bucketes
     */
    public int getBuckets() {
        return this.stat.buckets;
    }

    /**
     * The number of duplicate pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of duplicate pages
     */
    public int getDup() {
        return this.stat.dup;
    }

    /**
     * The number of bytes free on duplicate pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free on duplicate pages
     */
    public long getDupFree() {
        return this.stat.dupFree;
    }

    /**
     * The desired fill factor specified at database-creation time.
     *
     * @return the desired fill factor specified at database-creation time
     */
    public int getFfactor() {
        return this.stat.ffactor;
    }

    /**
     * The number of pages on the free list.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of pages on the free list
     */
    public int getFree() {
        return this.stat.free;
    }

    /**
     * The magic number that identifies the file as a Hash file.
     *
     * @return the magic number that identifies the file as a Hash file
     */
    public int getMagic() {
        return this.stat.magic;
    }

    /**
     * The number of external files.
     *
     * @return the number of external files
     */
    public int getExtFiles() {
        return this.stat.numBlobs;
    }

    /**
     * The number of key/data pairs in the database.
     * <p>
     * If the {@link SDatabase#getStats} call was configured by the {@link
     * SStatsConfig#setFast} method, the count will be the last saved value
     * unless it has never been calculated, in which case it will be 0.
     *
     * @return the number of key/data pairs in the database
     */
    public int getNumData() {
        return this.stat.numData;
    }

    /**
     * The number of unique keys in the database.
     * <p>
     * If the {@link SDatabase#getStats} call was configured by the {@link
     * SStatsConfig#setFast} method, the count will be the last saved value
     * unless it has never been calculated, in which case it will be 0.
     *
     * @return the number of unique keys in the database
     */
    public int getNumKeys() {
        return this.stat.numKeys;
    }

    /**
     * The number of bucket overflow pages (bucket overflow pages are created
     * when items did not fit on the main bucket page).
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bucket overflow pages
     */
    public int getOverflows() {
        return this.stat.overflows;
    }

    /**
     * The number of bytes free on bucket overflow pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free on bucket overflow pages
     */
    public long getOvflFree() {
        return this.stat.ovflFree;
    }

    /**
     * The number of pages in the database.
     *
     * @return the number of pages in the database
     */
    public int getPageCount() {
        return this.stat.pageCount;
    }

    /**
     * The underlying Hash database page (and bucket) size, in bytes.
     *
     * @return the underlying Hash database page size, in bytes
     */
    public int getPageSize() {
        return this.stat.pageSize;
    }

    /**
     * The version of the Hash database.
     *
     * @return the version of the Hash database
     */
    public int getVersion() {
        return this.stat.version;
    }

    /**
     * For convenience, the SHashStats class has a toString method
     * that lists all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "HashStats:"
                + "\n  magic=" + getMagic()
                + "\n  version=" + getVersion()
                + "\n  nkeys=" + getNumKeys()
                + "\n  ndata=" + getNumData()
                + "\n  ext_files=" + getExtFiles()
                + "\n  pagecnt=" + getPageCount()
                + "\n  pagesize=" + getPageSize()
                + "\n  ffactor=" + getFfactor()
                + "\n  buckets=" + getBuckets()
                + "\n  free=" + getFree()
                + "\n  bfree=" + getBFree()
                + "\n  bigpages=" + getBigPages()
                + "\n  big_bfree=" + getBigBFree()
                + "\n  overflows=" + getOverflows()
                + "\n  ovfl_free=" + getOvflFree()
                + "\n  dup=" + getDup()
                + "\n  dup_free=" + getDupFree()
                ;
    }
}
