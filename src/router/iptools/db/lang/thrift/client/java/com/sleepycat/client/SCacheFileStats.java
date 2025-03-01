/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TCacheFileStat;

/**
 * Statistics for a file in the cache.
 */
public class SCacheFileStats {
    /** The Thrift object. */
    private final TCacheFileStat stat;

    SCacheFileStats(TCacheFileStat stat) {
        this.stat = stat;
    }

    /**
     * Page size in bytes.
     *
     * @return page size in bytes
     */
    public int getPageSize() {
        return this.stat.pageSize;
    }

    /**
     * Requested pages mapped into the process' address space.
     *
     * @return requested pages mapped into the process' address space
     */
    public int getMap() {
        return this.stat.pageMapped;
    }

    /**
     * Requested pages found in the cache.
     *
     * @return requested pages found in the cache
     */
    public long getCacheHit() {
        return this.stat.cacheHit;
    }

    /**
     * Requested pages not found in the cache.
     *
     * @return requested pages not found in the cache
     */
    public long getCacheMiss() {
        return this.stat.cacheMiss;
    }

    /**
     * Pages created in the cache.
     *
     * @return pages created in the cache
     */
    public long getPageCreate() {
        return this.stat.pageCreate;
    }

    /**
     * Pages read into the cache.
     *
     * @return pages read into the cache
     */
    public long getPageIn() {
        return this.stat.pageIn;
    }

    /**
     * Pages written from the cache to the backing file.
     *
     * @return pages written from the cache to the backing file
     */
    public long getPageOut() {
        return this.stat.pageOut;
    }

    /**
     * Spins while trying to back up the file.
     *
     * @return spins while trying to back up the file
     */
    public long getBackupSpins() {
        return this.stat.backupSpins;
    }

    /**
     * The name of the file.
     *
     * @return the name of the file
     */
    public String getFileName() {
        return this.stat.fileName;
    }

    /**
     * For convenience, the SCacheFileStats class has a toString method that
     * lists all the data fields.
     *
     * @return a String that lists all the fields
     */
    public String toString() {
        return "CacheFileStats:"
                + "\n  pagesize=" + getPageSize()
                + "\n  map=" + getMap()
                + "\n  cache_hit=" + getCacheHit()
                + "\n  cache_miss=" + getCacheMiss()
                + "\n  page_create=" + getPageCreate()
                + "\n  page_in=" + getPageIn()
                + "\n  page_out=" + getPageOut()
                + "\n  backup_spins=" + getBackupSpins()
                + "\n  file_name=" + getFileName()
                ;
    }
}
