/*-
 * Automatically built by dist/s_java_stat.
 * Only the javadoc comments can be edited.
 *
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

package com.sleepycat.db;

/**
The HashStats object is used to return Hash database statistics.
*/
public class HashStats extends DatabaseStats {
    // no public constructor
    /* package */ HashStats() {}

    private int hash_magic;
    /**
    The magic number that identifies the file as a Hash file.
    @return the magic number that identifies the files as a Hash file
    */
    public int getMagic() {
        return hash_magic;
    }

    private int hash_version;
    /**
    The version of the Hash database.
    @return the version of the Hash database
    */
    public int getVersion() {
        return hash_version;
    }

    private int hash_metaflags;
    /**
    Reports internal flags. For internal use only.
    @return internal flags
    */
    public int getMetaFlags() {
        return hash_metaflags;
    }

    private int hash_nkeys;
    /**
    The number of unique keys in the database.
    <p>
    If the {@link com.sleepycat.db.Database#getStats Database.getStats} call was configured by the
    {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method, the count will be the last
    saved value unless it has never been calculated, in which case it
    will be 0.
    @return the number of unique keys in the database
    */
    public int getNumKeys() {
        return hash_nkeys;
    }

    private int hash_ndata;
    /**
    The number of key/data pairs in the database.
    <p>
    If the {@link com.sleepycat.db.Database#getStats Database.getStats} call was configured by the
    {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method, the count will be the last
    saved value unless it has never been calculated, in which case it
    will be 0.
    @return the number of key/data pairs in the database
    */
    public int getNumData() {
        return hash_ndata;
    }

    private int hash_ext_files;
    /**
    The number of external files.
    @return the number of external files
    */
    public int getExtFiles() {
        return hash_ext_files;
    }

    private int hash_nblobs;
    /**
    @deprecated Replaced with {@link #getExtFiles}.
    @return the number of external files
    */
    public int getNumBlobs() {
        return hash_nblobs;
    }

    private int hash_pagecnt;
    /**
    The number of pages in the database.
    <p>
    Returned if {@link StatsConfig#setFast} was configured.
    @return the number of pages in the database
    */
    public int getPageCount() {
        return hash_pagecnt;
    }

    private int hash_pagesize;
    /**
    The underlying Hash database page (and bucket) size, in bytes.
    @return the underlying Hash database page (and bucket) size, in bytes
    */
    public int getPageSize() {
        return hash_pagesize;
    }

    private int hash_ffactor;
    /**
    The desired fill factor specified at database-creation time.
    @return the desired fill factor specified at database-creation time
    */
    public int getFfactor() {
        return hash_ffactor;
    }

    private int hash_buckets;
    /**
    The number of hash buckets.
    @return the number of hash buckets
    */
    public int getBuckets() {
        return hash_buckets;
    }

    private int hash_free;
    /**
    The number of pages on the free list.
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of pages on the free list
    */
    public int getFree() {
        return hash_free;
    }

    private long hash_bfree;
    /**
    The number of bytes free on bucket pages.
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of bytes free on bucket pages
    */
    public long getBFree() {
        return hash_bfree;
    }

    private int hash_bigpages;
    /**
    The number of hash overflow pages (created when key/data is too big for
    the page).
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of hash overflow pages
    */
    public int getBigPages() {
        return hash_bigpages;
    }

    private long hash_big_bfree;
    /**
    The number of bytes free on hash overflow (big item) pages.
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of bytes free on hash overflow pages
    */
    public long getBigBFree() {
        return hash_big_bfree;
    }

    private int hash_overflows;
    /**
    The number of bucket overflow pages (bucket overflow pages are created when
    items did not fit on the main bucket page).
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of bucket overflow pages
    */
    public int getOverflows() {
        return hash_overflows;
    }

    private long hash_ovfl_free;
    /**
    The number of bytes free on bucket overflow pages.
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of bytes free on bucket overflow pages
    */
    public long getOvflFree() {
        return hash_ovfl_free;
    }

    private int hash_dup;
    /**
    The number of duplicate pages.
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of duplicate pages
    */
    public int getDup() {
        return hash_dup;
    }

    private long hash_dup_free;
    /**
    The number of bytes free on duplicate pages.
<p>
The information is only included if the {@link com.sleepycat.db.Database#getStats Database.getStats} call
was not configured by the {@link com.sleepycat.db.StatsConfig#setFast StatsConfig.setFast} method.
    @return the number of bytes free on duplicate pages
    */
    public long getDupFree() {
        return hash_dup_free;
    }

    /**
    For convenience, the HashStats class has a toString method
    that lists all the data fields.
    @return a String that lists all the data fields
    */
    public String toString() {
        return "HashStats:"
            + "\n  hash_magic=" + hash_magic
            + "\n  hash_version=" + hash_version
            + "\n  hash_metaflags=" + hash_metaflags
            + "\n  hash_nkeys=" + hash_nkeys
            + "\n  hash_ndata=" + hash_ndata
            + "\n  hash_ext_files=" + hash_ext_files
            + "\n  hash_nblobs=" + hash_nblobs
            + "\n  hash_pagecnt=" + hash_pagecnt
            + "\n  hash_pagesize=" + hash_pagesize
            + "\n  hash_ffactor=" + hash_ffactor
            + "\n  hash_buckets=" + hash_buckets
            + "\n  hash_free=" + hash_free
            + "\n  hash_bfree=" + hash_bfree
            + "\n  hash_bigpages=" + hash_bigpages
            + "\n  hash_big_bfree=" + hash_big_bfree
            + "\n  hash_overflows=" + hash_overflows
            + "\n  hash_ovfl_free=" + hash_ovfl_free
            + "\n  hash_dup=" + hash_dup
            + "\n  hash_dup_free=" + hash_dup_free
            ;
    }
}
