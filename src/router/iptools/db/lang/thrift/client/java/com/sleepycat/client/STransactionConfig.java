/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDurabilityPolicy;
import com.sleepycat.thrift.TIsolationLevel;
import com.sleepycat.thrift.TTransactionConfig;

/**
 * Specifies the attributes of a database environment transaction.
 * <p>
 * For a user created instance, no attribute is set by default. In addition,
 * calling the getter method of an unset attribute results in an
 * IllegalStateException. To set an attribute, call the setter method of the
 * attribute.
 * <p>
 * When used to begin a new transaction, system default values are used for
 * unset attributes.
 */
public class STransactionConfig
        extends ThriftWrapper<TTransactionConfig, TTransactionConfig._Fields> {

    /**
     * Create an empty STransactionConfig with no attribute set.
     */
    public STransactionConfig() {
        super(new TTransactionConfig());
    }

    /**
     * Return true if the Bulk attribute is set.
     *
     * @return the current setting of the Bulk attribute.
     */
    public boolean getBulk() {
        return (boolean) getField(TTransactionConfig._Fields.BULK);
    }

    /**
     * Configures the transaction to enable the transactional bulk insert
     * optimization. When this attribute is set, the transaction will avoid
     * logging the contents of insertions on newly allocated database pages. In
     * a transaction that inserts a large number of new records, the I/O
     * savings of choosing this option can be significant. Users of this option
     * should be aware of several issues. When the optimization is in effect,
     * page allocations that extend the database file are logged as usual; this
     * allows transaction aborts to work correctly, both online and during
     * recovery. At commit time, the database's pages are flushed to disk,
     * eliminating the need to roll-forward the transaction during normal
     * recovery.
     * <p>
     * The bulk insert optimization is effective only for top-level
     * transactions.
     *
     * @param bulk if true, configure the transaction to enable the bulk
     * optimization.
     * @return this
     */
    public STransactionConfig setBulk(final boolean bulk) {
        getThriftObj().setBulk(bulk);
        return this;
    }

    /**
     * Return if the transaction is configured to not write or synchronously
     * flush the log when it commits.
     *
     * @return if the transaction is configured to not write or synchronously
     * flush the log when it commits.
     */
    public boolean getNoSync() {
        return isDurabilityPolicyEqual(TDurabilityPolicy.NO_SYNC);
    }

    /**
     * Configure the transaction to not write or synchronously flush the log
     * when it commits.
     * <p>
     * This behavior may be set for a database environment using the {@link
     * SEnvironment#setConfig(SEnvironmentConfig)} method. Any value specified
     * to this method overrides that setting.
     *
     * @param noSync if true, transactions exhibit the ACI (atomicity,
     * consistency, and isolation) properties, but not D (durability); that is,
     * database integrity will be maintained, but if the server fails, it is
     * possible some number of the most recently committed transactions may be
     * undone during recovery. The number of transactions at risk is governed
     * by how many log updates can fit into the log buffer, how often the
     * server operating system flushes dirty buffers to disk, and how often the
     * log is checkpointed.
     * @return this
     */
    public STransactionConfig setNoSync(final boolean noSync) {
        setDurabilityPolicy(TDurabilityPolicy.NO_SYNC, noSync);
        return this;
    }

    /**
     * Return if the transaction is configured to write and synchronously
     * flush the log when it commits.
     *
     * @return if the transaction is configured to write and synchronously
     * flush the log when it commits.
     */
    public boolean getSync() {
        return isDurabilityPolicyEqual(TDurabilityPolicy.SYNC);
    }

    /**
     * Configure the transaction to write and synchronously flush the log
     * when it commits.
     *
     * @param sync if true, transactions exhibit all the ACID (atomicity,
     * consistency, isolation, and durability) properties.
     * @return this
     */
    public STransactionConfig setSync(final boolean sync) {
        setDurabilityPolicy(TDurabilityPolicy.SYNC, sync);
        return this;
    }

    /**
     * Return if the transaction is configured to write but not synchronously
     * flush the log when it commits.
     * <p>
     *
     * @return if the transaction is configured to not write or synchronously
     * flush the log when it commits.
     */
    public boolean getWriteNoSync() {
        return isDurabilityPolicyEqual(TDurabilityPolicy.WRITE_NO_SYNC);
    }

    /**
     * Configure the transaction to write but not synchronously flush the log
     * when it commits.
     * <p>
     * This behavior may be set for a database environment using the {@link
     * SEnvironment#setConfig(SEnvironmentConfig)} method. Any value specified
     * to this method overrides that setting.
     *
     * @param writeNoSync if true, transactions exhibit the ACI (atomicity,
     * consistency, and isolation) properties, but not D (durability); that is,
     * database integrity will be maintained, but if the server fails, it is
     * possible some number of the most recently committed transactions may be
     * undone during recovery. The number of transactions at risk is governed
     * by how many log updates can fit into the log buffer, how often the
     * server operating system flushes dirty buffers to disk, and how often the
     * log is checkpointed.
     * @return this
     */
    public STransactionConfig setWriteNoSync(final boolean writeNoSync) {
        setDurabilityPolicy(TDurabilityPolicy.WRITE_NO_SYNC, writeNoSync);
        return this;
    }

    private boolean isDurabilityPolicyEqual(TDurabilityPolicy policy) {
        return getThriftObj().isSetDurability() &&
                policy.equals(getField(TTransactionConfig._Fields.DURABILITY));
    }

    private void setDurabilityPolicy(TDurabilityPolicy policy, boolean set) {
        if (set) {
            getThriftObj().setDurability(policy);
        } else if (isDurabilityPolicyEqual(policy)) {
            getThriftObj().unsetDurability();
        }
    }

    /**
     * Return if the transaction is configured to not wait if a lock request
     * cannot be immediately granted.
     *
     * @return if the transaction is configured to not wait if a lock request
     * cannot be immediately granted.
     */
    public boolean getNoWait() {
        return !((boolean) getField(TTransactionConfig._Fields.WAIT));
    }

    /**
     * Configure the transaction to not wait if a lock request cannot be
     * immediately granted.
     *
     * @param noWait if true, transactions will not wait if a lock request
     * cannot be immediately granted, instead {@link SDeadlockException}
     * will be thrown.
     * @return this
     */
    public STransactionConfig setNoWait(final boolean noWait) {
        getThriftObj().setWait(!noWait);
        return this;
    }

    /**
     * Return if the transaction is configured to wait if a lock request cannot
     * be immediately granted.
     *
     * @return if the transaction is configured to wait if a lock request cannot
     * be immediately granted.
     */
    public boolean getWait() {
        return (boolean) getField(TTransactionConfig._Fields.WAIT);
    }

    /**
     * Configure the transaction to wait if a lock request cannot be
     * immediately granted.
     *
     * @param wait if true, transactions will wait if a lock request cannot be
     * immediately granted.
     * @return this
     */
    public STransactionConfig setWait(final boolean wait) {
        getThriftObj().setWait(wait);
        return this;
    }

    /**
     * Return if the transaction is configured for read committed isolation.
     *
     * @return if the transaction is configured for read committed isolation.
     */
    public boolean getReadCommitted() {
        return isIsolationLevelEqual(TIsolationLevel.READ_COMMITTED);
    }

    /**
     * Configure the transaction for read committed isolation.
     * <p>
     * This ensures the stability of the current data item read by the cursor
     * but permits data read by this transaction to be modified or deleted
     * prior to the commit of the transaction.
     *
     * @param readCommitted if true, configure the transaction for read
     * committed isolation.
     * @return this
     */
    public STransactionConfig setReadCommitted(final boolean readCommitted) {
        setIsolationLevel(TIsolationLevel.READ_COMMITTED, readCommitted);
        return this;
    }

    /**
     * Return if read operations performed by the transaction are configured to
     * return modified but not yet committed data.
     *
     * @return if read operations performed by the transaction are configured to
     * return modified but not yet committed data.
     */
    public boolean getReadUncommitted() {
        return isIsolationLevelEqual(TIsolationLevel.READ_UNCOMMITTED);
    }

    /**
     * Configure read operations performed by the transaction to return
     * modified
     * but not yet committed data.
     *
     * @param readUncommitted if true, configure read operations performed by
     * the transaction to return modified but not yet committed data.
     * @return this
     */
    public STransactionConfig setReadUncommitted(
            final boolean readUncommitted) {
        setIsolationLevel(TIsolationLevel.READ_UNCOMMITTED, readUncommitted);
        return this;
    }

    /**
     * Return true if the transaction is configured for Snapshot Isolation.
     *
     * @return true if the transaction is configured for Snapshot Isolation.
     */
    public boolean getSnapshot() {
        return isIsolationLevelEqual(TIsolationLevel.SNAPSHOT);
    }

    /**
     * This transaction will execute with snapshot isolation. For databases
     * configured with {@link SDatabaseConfig#setMultiversion}, data values
     * will be read as they are when the transaction begins, without taking
     * read locks.
     * <p>
     * Updates operations performed in the transaction will cause a
     * {@link SDeadlockException} to be thrown if data is modified between
     * reading and writing it.
     *
     * @param snapshot if this transaction will execute with snapshot isolation
     * @return this
     */
    public STransactionConfig setSnapshot(final boolean snapshot) {
        setIsolationLevel(TIsolationLevel.SNAPSHOT, snapshot);
        return this;
    }

    private boolean isIsolationLevelEqual(TIsolationLevel level) {
        return level.equals(getField(TTransactionConfig._Fields.ISO_LEVEL));
    }

    private void setIsolationLevel(TIsolationLevel level, boolean set) {
        if (set) {
            getThriftObj().setIsoLevel(level);
        } else if (isIsolationLevelEqual(level)) {
            getThriftObj().unsetIsoLevel();
        }
    }
}
