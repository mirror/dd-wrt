/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.THeapStat;

/**
 * The SHeapStats object is used to return Heap database statistics.
 */
public class SHeapStats implements SDatabaseStats {
    /** The Thrift object. */
    private final THeapStat stat;

    SHeapStats(THeapStat stat) {
        this.stat = stat;
    }

    /**
     * Magic number that identifies the file as a Heap file.
     *
     * @return the magic number that identifies the file as a Heap file
     */
    public int getHeapMagic() {
        return this.stat.heapMagic;
    }

    /**
     * The number of external files.
     *
     * @return the number of external files
     */
    public int getHeapExtFiles() {
        return this.stat.heapNumBlobs;
    }

    /**
     * Reports the number of records in the Heap database.
     *
     * @return the number of records in the Heap database
     */
    public int getHeapNumRecs() {
        return this.stat.heapNumRecs;
    }

    /**
     * The number of regions in the Heap database.
     *
     * @return the number of regions in the Heap database
     */
    public int getHeapNumRegions() {
        return this.stat.heapNumRegions;
    }

    /**
     * The number of pages in the database.
     *
     * @return the number of pages in the database
     */
    public int getHeapPageCount() {
        return this.stat.heapPageCount;
    }

    /**
     * The underlying database page (and bucket) size, in bytes.
     *
     * @return the underlying database page size, in bytes
     */
    public int getHeapPageSize() {
        return this.stat.heapPageSize;
    }

    /**
     * The number of pages in a region in the Heap database.
     *
     * @return the number of pages in a region in the Heap database
     */
    public int getHeapRegionSize() {
        return this.stat.heapRegionSize;
    }

    /**
     * The version of the Heap database.
     *
     * @return the version of the Heap database
     */
    public int getHeapVersion() {
        return this.stat.heapVersion;
    }

    /**
     * For convenience, the SHeapStats class has a toString method
     * that lists all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "HeapStats:"
                + "\n  magic=" + getHeapMagic()
                + "\n  version=" + getHeapVersion()
                + "\n  ext_files=" + getHeapExtFiles()
                + "\n  nrecs=" + getHeapNumRecs()
                + "\n  pagecnt=" + getHeapPageCount()
                + "\n  pagesize=" + getHeapPageSize()
                + "\n  nregions=" + getHeapNumRegions()
                + "\n  regionsize=" + getHeapRegionSize()
                ;
    }
}
