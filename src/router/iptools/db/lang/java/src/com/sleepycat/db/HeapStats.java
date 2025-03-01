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
The HeapStats object is used to return Heap database statistics.
*/
public class HeapStats extends DatabaseStats {
    // no public constructor
    /* package */ HeapStats() {}

    private int heap_magic;
    /** 
	Magic number that identifies the file as a Heap file.
    @return magic number that identifies the file as a Heap file
    */
    public int getHeapMagic() {
        return heap_magic;
    }

    private int heap_version;
    /** 
	The version of the Heap database.
    @return the version of the Heap database
    */
    public int getHeapVersion() {
        return heap_version;
    }

    private int heap_metaflags;
    /**
       Reports internal flags. For internal use only.
       @return internal flags
    */
    public int getHeapMetaFlags() {
        return heap_metaflags;
    }

    private int heap_ext_files;
    /**
       The number of external files.
       @return the number of external files
    */
    public int getHeapExtFiles() {
        return heap_ext_files;
    }

    private int heap_nblobs;
    /**
        @deprecated Replaced with {@link #getHeapExtFiles}.
	@return the number of external files
    */
    public int getHeapNumBlobs() {
        return heap_nblobs;
    }

    private int heap_nrecs;
    /**
       Reports the number of records in the Heap database.
       @return the number of records in the Heap database
    */
    public int getHeapNumRecs() {
        return heap_nrecs;
    }

    private int heap_pagecnt;
    /**
       The number of pages in the database.
       @return the number of pages in the database
    */
    public int getHeapPageCount() {
        return heap_pagecnt;
    }

    private int heap_pagesize;
    /**
       The underlying database page (and bucket) size, in bytes.
       @return the underlying database page size, in bytes
    */
    public int getHeapPageSize() {
        return heap_pagesize;
    }

    private int heap_nregions;
    /**
       The number of regions in the Heap database.
       @return the number of regions in the Heap database
    */
    public int getHeapNumRegions() {
        return heap_nregions;
    }

    private int heap_regionsize;
    /** 
    The number of pages in a region in the Heap database. Returned if
    DB_FAST_STAT is set.
    @return the number of pages in a region in the Heap database
    */
    public int getHeapRegionSize() {
        return heap_regionsize;
    }

    /**
    For convenience, the HeapStats class has a toString method
    that lists all the data fields.
    @return a String that lists all the data fields
    */
    public String toString() {
        return "HeapStats:"
            + "\n  heap_magic=" + heap_magic
            + "\n  heap_version=" + heap_version
            + "\n  heap_metaflags=" + heap_metaflags
            + "\n  heap_ext_files=" + heap_ext_files
            + "\n  heap_nblobs=" + heap_nblobs
            + "\n  heap_nrecs=" + heap_nrecs
            + "\n  heap_pagecnt=" + heap_pagecnt
            + "\n  heap_pagesize=" + heap_pagesize
            + "\n  heap_nregions=" + heap_nregions
            + "\n  heap_regionsize=" + heap_regionsize
            ;
    }
}
