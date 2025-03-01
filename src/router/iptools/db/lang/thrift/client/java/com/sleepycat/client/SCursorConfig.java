/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TCursorConfig;
import com.sleepycat.thrift.TIsolationLevel;

/**
 * Specify the attributes of database cursor.
 * <p>
 * For a user created instance, no attribute is set by default. In addition,
 * calling the getter method of an unset attribute results in an
 * IllegalStateException. To set an attribute, call the setter method of the
 * attribute.
 * <p>
 * When used to create a cursor, system default values are used for
 * unset attributes.
 */
public class SCursorConfig
        extends ThriftWrapper<TCursorConfig, TCursorConfig._Fields> {
    /**
     * Default configuration used if null is passed to methods that create a
     * cursor.
     */
    public static final SCursorConfig DEFAULT = new SCursorConfig();

    /**
     * A convenience instance to configure a cursor for read committed
     * isolation.
     * <p>
     * This ensures the stability of the current data item read by the
     * cursor but permits data read by this cursor to be modified or
     * deleted prior to the commit of the transaction.
     */
    public static final SCursorConfig READ_COMMITTED =
            new SCursorConfig().setReadCommitted(true);

    /**
     * A convenience instance to configure read operations performed by the
     * cursor to return modified but not yet committed data.
     */
    public static final SCursorConfig READ_UNCOMMITTED =
            new SCursorConfig().setReadUncommitted(true);

    /**
     * A convenience instance to configure read operations performed by the
     * cursor to return values as they were when the cursor was opened, if
     * {@link SDatabaseConfig#setMultiversion} is configured.
     */
    public static final SCursorConfig SNAPSHOT =
            new SCursorConfig().setSnapshot(true);

    /**
     * Create an empty SCursorConfig with no attribute set.
     */
    public SCursorConfig() {
        super(new TCursorConfig());
    }

    /**
     * Create an SCursorConfig wrapping a specified thrift object.
     *
     * @param tConfig the thrift config object
     */
    SCursorConfig(TCursorConfig tConfig) {
        super(tConfig);
    }

    /**
     * Return if the cursor will be used to do bulk operations on the
     * underlying database.
     *
     * @return if the cursor will be used to do bulk operations on the
     * underlying database
     */
    public boolean getBulkCursor() {
        return (boolean) getField(TCursorConfig._Fields.BULK_CURSOR);
    }

    /**
     * Specify that the cursor will be used to do bulk operations on the
     * underlying database.
     *
     * @param bulkCursor if true, specify the cursor will be used to do bulk
     * operations on the underlying database
     * @return this
     */
    public SCursorConfig setBulkCursor(boolean bulkCursor) {
        getThriftObj().setBulkCursor(bulkCursor);
        return this;
    }

    /**
     * Return if the cursor is configured for read committed isolation.
     *
     * @return if the cursor is configured for read committed isolation
     */
    public boolean getReadCommitted() {
        return isIsolationLevelEqual(TIsolationLevel.READ_COMMITTED);
    }

    /**
     * Configure the cursor for read committed isolation.
     * <p>
     * This ensures the stability of the current data item read by the cursor
     * but permits data read by this cursor to be modified or deleted prior to
     * the commit of the transaction.
     *
     * @param readCommitted if true, configure the cursor for read committed
     * isolation
     * @return this
     */
    public SCursorConfig setReadCommitted(boolean readCommitted) {
        setIsolationLevel(TIsolationLevel.READ_COMMITTED, readCommitted);
        return this;
    }

    /**
     * Return if read operations performed by the cursor are configured to
     * return modified but not yet committed data.
     *
     * @return if read operations performed by the cursor are configured to
     * return modified but not yet committed data
     */
    public boolean getReadUncommitted() {
        return isIsolationLevelEqual(TIsolationLevel.READ_UNCOMMITTED);
    }

    /**
     * Configure read operations performed by the cursor to return modified but
     * not yet committed data.
     *
     * @param readUncommitted if true, configure read operations performed by
     * the cursor to return modified but not yet committed data
     * @return this
     */
    public SCursorConfig setReadUncommitted(boolean readUncommitted) {
        setIsolationLevel(TIsolationLevel.READ_UNCOMMITTED, readUncommitted);
        return this;
    }

    /**
     * Return if read operations performed by the cursor are configured to
     * return data as it was when the cursor was opened, without locking.
     *
     * @return if read operations performed by the cursor are configured to
     * return data as it was when the cursor was opened, without locking
     */
    public boolean getSnapshot() {
        return isIsolationLevelEqual(TIsolationLevel.SNAPSHOT);
    }

    /**
     * Configure read operations performed by the cursor to return data as it
     * was when the cursor opened without locking, if {@link
     * SDatabaseConfig#setMultiversion} was configured.
     *
     * @param snapshot if true, configure read operations performed by the
     * cursor to return data as it was when the cursor was opened, without
     * locking
     * @return this
     */
    public SCursorConfig setSnapshot(boolean snapshot) {
        setIsolationLevel(TIsolationLevel.SNAPSHOT, snapshot);
        return this;
    }

    private boolean isIsolationLevelEqual(TIsolationLevel level) {
        return getThriftObj().isSetIsoLevel() &&
                level.equals(getField(TCursorConfig._Fields.ISO_LEVEL));
    }

    private void setIsolationLevel(TIsolationLevel level, boolean set) {
        if (set) {
            getThriftObj().setIsoLevel(level);
        } else if (isIsolationLevelEqual(level)) {
            getThriftObj().unsetIsoLevel();
        }
    }
}
