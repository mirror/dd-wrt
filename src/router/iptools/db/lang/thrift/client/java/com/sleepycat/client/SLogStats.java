/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TLogStat;

/**
 * Log statistics for a database environment.
 */
public class SLogStats {
    /** The Thrift object. */
    private final TLogStat stat;

    SLogStats(TLogStat stat) {
        this.stat = stat;
    }

    /**
     * The magic number that identifies a file as a log file.
     *
     * @return the magic number that identifies a file as a log file
     */
    public int getMagic() {
        return this.stat.magic;
    }

    /**
     * The version of the log file type.
     *
     * @return the version of the log file type
     */
    public int getVersion() {
        return this.stat.version;
    }

    /**
     * The mode of any created log files.
     *
     * @return the mode of any created log files
     */
    public int getMode() {
        return this.stat.mode;
    }

    /**
     * The in-memory log record cache size.
     *
     * @return the in-memory log record cache size
     */
    public int getLgBSize() {
        return this.stat.lgBSize;
    }

    /**
     * The current log file size.
     *
     * @return the current log file size
     */
    public int getLgSize() {
        return this.stat.lgSize;
    }

    /**
     * The number of bytes over and above {@link #getWcMbytes} written to
     * this log since the last checkpoint.
     *
     * @return the number of bytes over and above {@link #getWcMbytes} written
     * to this log since the last checkpoint
     */
    public int getWcBytes() {
        return this.stat.wcBytes;
    }

    /**
     * The number of megabytes written to this log since the last checkpoint.
     *
     * @return the number of megabytes written to this log since the last
     * checkpoint
     */
    public int getWcMbytes() {
        return this.stat.wcMbytes;
    }

    /**
     * The initial allocated file logging identifiers.
     *
     * @return the initial allocated file logging identifiers
     */
    public int getFileidInit() {
        return this.stat.fileidInit;
    }

    /**
     * The current number of file logging identifiers.
     *
     * @return the current number of file logging identifiers
     */
    public int getNumFileId() {
        return this.stat.numFileId;
    }

    /**
     * The maximum number of file logging identifiers used.
     *
     * @return the maximum number of file logging identifiers used
     */
    public int getMaxNfileId() {
        return this.stat.maxNfileId;
    }

    /**
     * The number of records written to this log.
     *
     * @return the number of records written to this log
     */
    public long getRecord() {
        return this.stat.record;
    }

    /**
     * The number of bytes over and above {@link #getWMbytes} written to this
     * log.
     *
     * @return The number of bytes over and above {@link #getWMbytes} written to
     * this log
     */
    public int getWBytes() {
        return this.stat.WBytes;
    }

    /**
     * The number of megabytes written to this log.
     *
     * @return the number of megabytes written to this log
     */
    public int getWMbytes() {
        return this.stat.WMbytes;
    }

    /**
     * The number of times the log has been written to disk.
     *
     * @return the number of times the log has been written to disk
     */
    public long getWCount() {
        return this.stat.WCount;
    }

    /**
     * The number of times the log has been written to disk because the
     * in-memory log record cache filled up.
     *
     * @return the number of times the log has been written to disk because the
     * in-memory log record cache filled up
     */
    public long getWCountFill() {
        return this.stat.WCountFill;
    }

    /**
     * The number of times the log has been read from disk.
     *
     * @return the number of times the log has been read from disk
     */
    public long getRCount() {
        return this.stat.RCount;
    }

    /**
     * The number of times the log has been flushed to disk.
     *
     * @return the number of times the log has been flushed to disk
     */
    public long getSCount() {
        return this.stat.SCount;
    }

    /**
     * The number of times that a thread of control was forced to wait
     * before obtaining the log region mutex.
     *
     * @return the number of times that a thread of control was forced to wait
     * before obtaining the log region mutex
     */
    public long getRegionWait() {
        return this.stat.regionWait;
    }

    /**
     * The number of times that a thread of control was able to obtain
     * the log region mutex without waiting.
     *
     * @return the number of times that a thread of control was able to obtain
     * the log region mutex without waiting
     */
    public long getRegionNowait() {
        return this.stat.regionNowait;
    }

    /**
     * The current log file number.
     *
     * @return the current log file number
     */
    public int getCurFile() {
        return this.stat.curFile;
    }

    /**
     * The byte offset in the current log file.
     *
     * @return the byte offset in the current log file
     */
    public int getCurOffset() {
        return this.stat.curOffset;
    }

    /**
     * The log file number of the last record known to be on disk.
     *
     * @return the log file number of the last record known to be on disk
     */
    public int getDiskFile() {
        return this.stat.diskFile;
    }

    /**
     * The byte offset of the last record known to be on disk.
     *
     * @return the byte offset of the last record known to be on disk
     */
    public int getDiskOffset() {
        return this.stat.diskOffset;
    }

    /**
     * The maximum number of commits contained in a single log flush.
     *
     * @return the maximum number of commits contained in a single log flush
     */
    public int getMaxCommitperflush() {
        return this.stat.maxCommitperflush;
    }

    /**
     * The minimum number of commits contained in a single log flush that
     * contained a commit.
     *
     * @return the minimum number of commits contained in a single log flush
     * that contained a commit
     */
    public int getMinCommitperflush() {
        return this.stat.minCommitperflush;
    }

    /**
     * The size of the region.
     *
     * @return the size of the region
     */
    public long getRegSize() {
        return this.stat.regSize;
    }

    /**
     * For convenience, the SLogStats class has a toString method that lists
     * all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "LogStats:"
                + "\n  magic=" + getMagic()
                + "\n  version=" + getVersion()
                + "\n  mode=" + getMode()
                + "\n  lg_bsize=" + getLgBSize()
                + "\n  lg_size=" + getLgSize()
                + "\n  wc_bytes=" + getWcBytes()
                + "\n  wc_mbytes=" + getWcMbytes()
                + "\n  fileid_init=" + getFileidInit()
                + "\n  nfileid=" + getNumFileId()
                + "\n  maxnfileid=" + getMaxNfileId()
                + "\n  record=" + getRecord()
                + "\n  w_bytes=" + getWBytes()
                + "\n  w_mbytes=" + getWMbytes()
                + "\n  wcount=" + getWCount()
                + "\n  wcount_fill=" + getWCountFill()
                + "\n  rcount=" + getRCount()
                + "\n  scount=" + getSCount()
                + "\n  region_wait=" + getRegionWait()
                + "\n  region_nowait=" + getRegionNowait()
                + "\n  cur_file=" + getCurFile()
                + "\n  cur_offset=" + getCurOffset()
                + "\n  disk_file=" + getDiskFile()
                + "\n  disk_offset=" + getDiskOffset()
                + "\n  maxcommitperflush=" + getMaxCommitperflush()
                + "\n  mincommitperflush=" + getMinCommitperflush()
                + "\n  regsize=" + getRegSize()
                ;
    }

}
