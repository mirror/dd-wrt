/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TMutexStat;

/**
 * Statistics about mutexes in a Berkeley DB database environment.
 */
public class SMutexStats {
    /** The Thrift object. */
    private final TMutexStat stat;

    SMutexStats(TMutexStat stat) {
        this.stat = stat;
    }

    /**
     * The mutex alignment, in bytes.
     *
     * @return the mutex alignment, in bytes
     */
    public int getMutexAlign() {
        return this.stat.mutexAlign;
    }

    /**
     * The number of times test-and-set mutexes will spin without blocking.
     *
     * @return the number of times test-and-set mutexes will spin without
     * blocking
     */
    public int getMutexTasSpins() {
        return this.stat.mutexTasSpins;
    }

    /**
     * The initial number of mutexes configured.
     *
     * @return the initial number of mutexes configured
     */
    public int getMutexInit() {
        return this.stat.mutexInit;
    }

    /**
     * The total number of mutexes configured.
     *
     * @return the total number of mutexes configured
     */
    public int getMutexCount() {
        return this.stat.mutexCount;
    }

    /**
     * The maximum number of mutexes.
     *
     * @return the maximum number of mutexes
     */
    public int getMutexMax() {
        return this.stat.mutexMax;
    }

    /**
     * The number of mutexes currently available.
     *
     * @return the number of mutexes currently available
     */
    public int getMutexFree() {
        return this.stat.mutexFree;
    }

    /**
     * The number of mutexes currently in use.
     *
     * @return the number of mutexes currently in use
     */
    public int getMutexInuse() {
        return this.stat.mutexInuse;
    }

    /**
     * The maximum number of mutexes ever in use.
     *
     * @return the maximum number of mutexes ever in use
     */
    public int getMutexInuseMax() {
        return this.stat.mutexInuseMax;
    }

    /**
     * The number of times that a thread of control was forced to wait before
     * obtaining the mutex region mutex.
     *
     * @return the number of times that a thread of control was forced to wait
     * before obtaining the mutex region mutex
     */
    public long getRegionWait() {
        return this.stat.regionWait;
    }

    /**
     * The number of times that a thread of control was able to obtain
     * the mutex region mutex without waiting.
     *
     * @return the number of times that a thread of control was able to obtain
     * the mutex region mutex without waiting
     */
    public long getRegionNowait() {
        return this.stat.regionNowait;
    }

    /**
     * The size of the mutex region, in bytes.
     *
     * @return the size of the mutex region, in bytes
     */
    public long getRegSize() {
        return this.stat.regSize;
    }

    /**
     * The max size of the mutex region size.
     *
     * @return the max size of the mutex region size
     */
    public long getRegmax() {
        return this.stat.regmax;
    }

    /**
     * For convenience, the SMutexStats class has a toString method that lists
     * all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "MutexStats:"
                + "\n  mutex_align=" + getMutexAlign()
                + "\n  mutex_tas_spins=" + getMutexTasSpins()
                + "\n  mutex_init=" + getMutexInit()
                + "\n  mutex_cnt=" + getMutexCount()
                + "\n  mutex_max=" + getMutexMax()
                + "\n  mutex_free=" + getMutexFree()
                + "\n  mutex_inuse=" + getMutexInuse()
                + "\n  mutex_inuse_max=" + getMutexInuseMax()
                + "\n  region_wait=" + getRegionWait()
                + "\n  region_nowait=" + getRegionNowait()
                + "\n  regsize=" + getRegSize()
                + "\n  regmax=" + getRegmax()
                ;
    }
}
