/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.LockMode;
import com.sleepycat.thrift.TCursorGetConfig;
import com.sleepycat.thrift.TDbGetConfig;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TIsolationLevel;
import com.sleepycat.thrift.TJoinCursorGetConfig;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TKeyDataWithPKey;
import org.apache.thrift.TBase;
import org.apache.thrift.TFieldIdEnum;

/**
 * GetArgs represents the arguments for a database or cursor get call.
 */
public class GetArgs {
    /** The key argument. */
    public DatabaseEntry key;
    /** The pKey argument. */
    public DatabaseEntry pKey;
    /** The data argument. */
    public DatabaseEntry data;
    /** The lockMode argument. */
    public LockMode lockMode;

    /**
     * Construct the arguments for a Database get call.
     *
     * @param keyData the key / data pair
     * @param config the configuration
     */
    public GetArgs(TKeyData keyData, TDbGetConfig config) {
        constructEntries(keyData);
        handleConfig(config);
    }

    /**
     * Construct the arguments for a Database pGet call.
     *
     * @param tuple the key / pKey / data tuple
     * @param config the configuration
     */
    public GetArgs(TKeyDataWithPKey tuple, TDbGetConfig config) {
        constructEntries(tuple);
        handleConfig(config);
    }

    /**
     * Construct the arguments for a Cursor get call.
     *
     * @param keyData the key / data pair
     * @param config the configuration
     */
    public GetArgs(TKeyData keyData, TCursorGetConfig config) {
        constructEntries(keyData);
        handleConfig(config);
    }

    /**
     * Construct the arguments for a JoinCursor get call.
     * @param config the configuration
     */
    public GetArgs(TJoinCursorGetConfig config) {
        constructEntries(config.pair);
        if (config.isSetRmw() && config.rmw) {
            this.lockMode = LockMode.RMW;
        } else if (config.isSetReadUncommitted() && config.readUncommitted) {
            this.lockMode = LockMode.READ_UNCOMMITTED;
        } else {
            this.lockMode = LockMode.DEFAULT;
        }
    }

    /**
     * Construct the arguments for a Cursor pGet call.
     *
     * @param tuple the key / pKey / data tuple
     * @param config the configuration
     */
    public GetArgs(TKeyDataWithPKey tuple, TCursorGetConfig config) {
        constructEntries(tuple);
        handleConfig(config);
    }

    private void constructEntries(TKeyData keyData) {
        this.key = getDatabaseEntry(keyData, TKeyData._Fields.KEY);
        this.data = getDatabaseEntry(keyData, TKeyData._Fields.DATA);
    }

    private void constructEntries(TKeyDataWithPKey keyData) {
        this.key = getDatabaseEntry(keyData, TKeyDataWithPKey._Fields.SKEY);
        this.pKey = getDatabaseEntry(keyData, TKeyDataWithPKey._Fields.PKEY);
        this.data = getDatabaseEntry(keyData, TKeyDataWithPKey._Fields.PDATA);
    }

    private void handleConfig(TDbGetConfig config) {
        this.lockMode = getLockMode(config, TDbGetConfig._Fields.RMW,
                TDbGetConfig._Fields.ISO_LEVEL);
    }

    private void handleConfig(TCursorGetConfig config) {
        this.lockMode = getLockMode(config, TCursorGetConfig._Fields.RMW,
                TCursorGetConfig._Fields.ISO_LEVEL);
    }

    private <T extends TBase<T, F>, F extends TFieldIdEnum>
    DatabaseEntry getDatabaseEntry(T tObj, F field) {
        TDbt dbt = null;
        if (tObj != null && tObj.isSet(field)) {
            dbt = (TDbt) tObj.getFieldValue(field);
        }
        return Adapters.toBdbType(dbt);
    }

    private <T extends TBase<T, F>, F extends TFieldIdEnum>
    LockMode getLockMode(T tObj, F rmwField, F isoLevelField) {
        if (tObj.isSet(rmwField) && (boolean) tObj.getFieldValue(rmwField))
            return LockMode.RMW;
        if (!tObj.isSet(isoLevelField))
            return LockMode.DEFAULT;
        switch ((TIsolationLevel) tObj.getFieldValue(isoLevelField)) {
            case READ_COMMITTED:
                return LockMode.READ_COMMITTED;
            case READ_UNCOMMITTED:
                return LockMode.READ_UNCOMMITTED;
            default:
                throw new IllegalArgumentException("Invalid isolation level.");
        }
    }
}
