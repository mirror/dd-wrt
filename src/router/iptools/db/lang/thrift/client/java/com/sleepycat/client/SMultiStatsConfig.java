/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TEnvStatConfig;
import com.sleepycat.thrift.TEnvStatOption;

/**
 * Specifies the attributes of multiple statistics retrieval operations.
 * <p>
 * SMultiStatsConfig specifies which statistics and their retrieval operation
 * attributes for {@link SEnvironment#getMultipleStats(SMultiStatsConfig)}.
 */
public class SMultiStatsConfig
        extends ThriftWrapper<TEnvStatConfig, TEnvStatConfig._Fields> {
    /**
     * Create an empty SMultiStatsConfig. With an empty SMultiStatsConfig,
     * {@link SEnvironment#getMultipleStats(SMultiStatsConfig)} retrieves no
     * statistics.
     */
    public SMultiStatsConfig() {
        super(new TEnvStatConfig());
    }

    /**
     * Specify if {@link SCacheFileStats} is retrieved and its retrieval
     * operation attributes.
     *
     * @param retrieve if {@link SCacheFileStats} is retrieved
     * @param config the statistics attributes
     * @return this
     */
    public SMultiStatsConfig setCacheFileConfig(boolean retrieve,
            SStatsConfig config) {
        setConfig(TEnvStatConfig._Fields.CACHE_FILE, retrieve, config);
        return this;
    }

    /**
     * Specify if {@link SCacheStats} is retrieved and its retrieval operation
     * attributes.
     *
     * @param retrieve if {@link SCacheStats} is retrieved
     * @param config the statistics attributes
     * @return this
     */
    public SMultiStatsConfig setCacheConfig(boolean retrieve,
            SStatsConfig config) {
        setConfig(TEnvStatConfig._Fields.CACHE, retrieve, config);
        return this;
    }

    /**
     * Specify if {@link SLockStats} is retrieved and its retrieval operation
     * attributes.
     *
     * @param retrieve if {@link SLockStats} is retrieved
     * @param config the statistics attributes
     * @return this
     */
    public SMultiStatsConfig setLockConfig(boolean retrieve,
            SStatsConfig config) {
        setConfig(TEnvStatConfig._Fields.LOCK, retrieve, config);
        return this;
    }

    /**
     * Specify if {@link SLogStats} is retrieved and its retrieval operation
     * attributes.
     *
     * @param retrieve if {@link SLogStats} is retrieved
     * @param config the statistics attributes
     * @return this
     */
    public SMultiStatsConfig setLogConfig(boolean retrieve,
            SStatsConfig config) {
        setConfig(TEnvStatConfig._Fields.LOG, retrieve, config);
        return this;
    }

    /**
     * Specify if {@link SMutexStats} is retrieved and its retrieval operation
     * attributes.
     *
     * @param retrieve if {@link SMutexStats} is retrieved
     * @param config the statistics attributes
     * @return this
     */
    public SMultiStatsConfig setMutexConfig(boolean retrieve,
            SStatsConfig config) {
        setConfig(TEnvStatConfig._Fields.MUTEX, retrieve, config);
        return this;
    }

    /**
     * Specify if {@link STransactionStats} is retrieved and its retrieval
     * operation attributes.
     *
     * @param retrieve if {@link STransactionStats} is retrieved
     * @param config the statistics attributes
     * @return this
     */
    public SMultiStatsConfig setTransactionConfig(boolean retrieve,
            SStatsConfig config) {
        setConfig(TEnvStatConfig._Fields.TRANSACTION, retrieve, config);
        return this;
    }

    private void setConfig(TEnvStatConfig._Fields field, boolean retrieve,
            SStatsConfig config) {
        TEnvStatOption option;
        if (!retrieve) {
            option = TEnvStatOption.EXCLUDE;
        } else {
            SStatsConfig c = config == null ? new SStatsConfig() : config;
            option = c.getClear() ? TEnvStatOption.CLEAR
                    : TEnvStatOption.INCLUDE;
        }
        getThriftObj().setFieldValue(field, option);
    }
}
