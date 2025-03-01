/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TBtreeStat;

/**
 * The SBtreeStats object is used to return Btree or Recno database statistics.
 */
public class SBtreeStats implements SDatabaseStats {
    /** The Thrift object. */
    private final TBtreeStat stat;

    SBtreeStats(TBtreeStat stat) {
        this.stat = stat;
    }

    /**
     * The number of database duplicate pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of database duplicate pages
     */
    public int getDupPages() {
        return this.stat.dupPages;
    }

    /**
     * The number of bytes free in database duplicate pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free in the database duplciate pages
     */
    public long getDupPagesFree() {
        return this.stat.dupPagesFree;
    }

    /**
     * The number of empty database pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of empty database pages
     */
    public int getEmptyPages() {
        return this.stat.emptyPages;
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
     * The number of database internal pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of database internal pages
     */
    public int getIntPages() {
        return this.stat.intPages;
    }

    /**
     * The number of bytes free in database internal pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free in database internal pages
     */
    public long getIntPagesFree() {
        return this.stat.intPagesFree;
    }

    /**
     * The number of database leaf pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of database leaf pages
     */
    public int getLeafPages() {
        return this.stat.leafPages;
    }

    /**
     * The number of bytes free in database leaf pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free in database leaf pages
     */
    public long getLeafPagesFree() {
        return this.stat.leafPagesFree;
    }

    /**
     * The number of levels in the database.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of levels in the database
     */
    public int getLevels() {
        return this.stat.levels;
    }

    /**
     * The magic number that identifies the file as a Btree database.
     *
     * @return the magic number that identifies the file as a Btree database
     */
    public int getMagic() {
        return this.stat.magic;
    }

    /**
     * The minimum keys per page.
     *
     * @return the minimum keys per page
     */
    public int getMinKey() {
        return this.stat.minKey;
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
     * For the Btree Access Method, the number of key/data pairs in the
     * database. If the {@link SDatabase#getStats} call was not configured by
     * the {@link SStatsConfig#setFast} method, the count will be exact.
     * Otherwise, the count will be the last saved value unless it has never
     * been calculated, in which case it will be 0.
     *
     * @return the number of key/data pairs in the database
     */
    public int getNumData() {
        return this.stat.numData;
    }

    /**
     * The number of keys in the database.
     * <p>
     * For the Btree Access Method, the number of keys in the database. If the
     * {@link SDatabase#getStats} call was not configured by the {@link
     * SStatsConfig#setFast} method or the database was configured to support
     * retrieval by record number, the count will be exact. Otherwise, the
     * count will be the last saved value unless it has never been calculated,
     * in which case it will be 0.
     *
     * @return the number of keys in the database
     */
    public int getNumKeys() {
        return this.stat.numKeys;
    }

    /**
     * The number of database overflow pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of database overflow pages
     */
    public int getOverPages() {
        return this.stat.overPages;
    }

    /**
     * The number of bytes free in database overflow pages.
     * <p>
     * The information is only included if the {@link SDatabase#getStats} call
     * was not configured by the {@link SStatsConfig#setFast} method.
     *
     * @return the number of bytes free in database overflow pages
     */
    public long getOverPagesFree() {
        return this.stat.overPagesFree;
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
     * The underlying database page size, in bytes.
     *
     * @return the underlying database page size, in bytes
     */
    public int getPageSize() {
        return this.stat.pageSize;
    }

    /**
     * The length of fixed-length records.
     *
     * @return the length of fixed-length records
     */
    public int getReLen() {
        return this.stat.reLen;
    }

    /**
     * The padding byte value for fixed-length records.
     *
     * @return the padding byte value for fixed-length records
     */
    public int getRePad() {
        return this.stat.rePad;
    }

    /**
     * The version of the Btree database.
     *
     * @return the version of the Btree database
     */
    public int getVersion() {
        return this.stat.version;
    }

    /**
     * For convenience, the SBtreeStats class has a toString method
     * that lists all the data fields.
     *
     * @return a String that lists all the data fields
     */
    public String toString() {
        return "BtreeStats:"
                + "\n  magic=" + getMagic()
                + "\n  version=" + getVersion()
                + "\n  nkeys=" + getNumKeys()
                + "\n  ndata=" + getNumData()
                + "\n  pagecnt=" + getPageCount()
                + "\n  pagesize=" + getPageSize()
                + "\n  minkey=" + getMinKey()
                + "\n  ext_files=" + getExtFiles()
                + "\n  re_len=" + getReLen()
                + "\n  re_pad=" + getRePad()
                + "\n  levels=" + getLevels()
                + "\n  int_pg=" + getIntPages()
                + "\n  leaf_pg=" + getLeafPages()
                + "\n  dup_pg=" + getDupPages()
                + "\n  over_pg=" + getOverPages()
                + "\n  empty_pg=" + getEmptyPages()
                + "\n  free=" + getFree()
                + "\n  int_pgfree=" + getIntPagesFree()
                + "\n  leaf_pgfree=" + getLeafPagesFree()
                + "\n  dup_pgfree=" + getDupPagesFree()
                + "\n  over_pgfree=" + getOverPagesFree()
                ;
    }
}
