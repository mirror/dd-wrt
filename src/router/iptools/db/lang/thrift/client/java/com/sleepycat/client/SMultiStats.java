/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TEnvStatResult;

import java.util.List;
import java.util.stream.Collectors;

/**
 * A SMultiStats contains multiple statistics for a database environment. If a
 * statistics is not available (e.g. not retrieved), {@code null} is returned
 * for that statistics.
 */
public class SMultiStats
        extends ThriftWrapper<TEnvStatResult, TEnvStatResult._Fields> {
    /**
     * Create an SMultiStats wrapping a TEnvStatResult.
     *
     * @param result the TEnvStatResult
     */
    SMultiStats(TEnvStatResult result) {
        super(result);
    }

    /**
     * Return statistics for individual files in the cache.
     *
     * @return statistics for individual files in the cache; or null if the
     * statistics is not available.
     */
    public SCacheFileStats[] getCacheFileStats() {
        if (getThriftObj().isSetCacheFileStat()) {
            List<SCacheFileStats> statsList =
                    getThriftObj().getCacheFileStat().stream()
                            .map(SCacheFileStats::new)
                            .collect(Collectors.toList());
            return statsList.toArray(new SCacheFileStats[statsList.size()]);
        } else {
            return null;
        }
    }

    /**
     * Returns the memory pool (that is, the buffer cache) subsystem
     * statistics.
     *
     * @return the memory pool (that is, the buffer cache) subsystem statistics;
     * or null if the statistics is not available
     */
    public SCacheStats getCacheStats() {
        if (getThriftObj().isSetCacheStat()) {
            return new SCacheStats(getThriftObj().getCacheStat());
        } else {
            return null;
        }
    }

    /**
     * Return the database environment's locking statistics.
     *
     * @return the database environment's locking statistics; or null if the
     * statistics is not available
     */
    public SLockStats getLockStats() {
        if (getThriftObj().isSetLockStat()) {
            return new SLockStats(getThriftObj().getLockStat());
        } else {
            return null;
        }
    }

    /**
     * Return the database environment's logging statistics.
     *
     * @return the database environment's logging statistics; or null if the
     * statistics is not available
     */
    public SLogStats getLogStats() {
        if (getThriftObj().isSetLogStat()) {
            return new SLogStats(getThriftObj().getLogStat());
        } else {
            return null;
        }
    }

    /**
     * Return the database environment's mutex statistics.
     *
     * @return the database environment's mutex statistics; or null if the
     * statistics is not available
     */
    public SMutexStats getMutexStats() {
        if (getThriftObj().isSetMutexStat()) {
            return new SMutexStats(getThriftObj().getMutexStat());
        } else {
            return null;
        }
    }

    /**
     * Return the database environment's transactional statistics.
     *
     * @return the database environment's transactional statistics; or null if
     * the statistics is not available
     */
    public STransactionStats getTransactionStats() {
        if (getThriftObj().isSetTxnStat()) {
            return new STransactionStats(getThriftObj().getTxnStat());
        } else {
            return null;
        }
    }
}
