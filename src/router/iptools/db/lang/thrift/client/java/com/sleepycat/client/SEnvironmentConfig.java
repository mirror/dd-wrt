/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDurabilityPolicy;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TLockDetectMode;

/**
 * Specifies the attributes of an environment.
 * <p>
 * For a user created instance, no attribute is set by default. In addition,
 * calling the getter method of an unset attribute results in an
 * IllegalStateException. To set an attribute, call the setter method of the
 * attribute.
 * <p>
 * When used to construct an environment, system default values are used for
 * unset attributes. When used to modify the configuration of an existing
 * environment, only set attributes are modified; unset attributes are not
 * modified.
 */
public class SEnvironmentConfig
        extends ThriftWrapper<TEnvironmentConfig, TEnvironmentConfig._Fields> {

    /**
     * Create an empty SEnvironmentConfig with no attribute set.
     */
    public SEnvironmentConfig() {
        super(new TEnvironmentConfig());
    }

    /**
     * Create an SEnvironmentConfig wrapping a specified thrift object.
     *
     * @param tConfig the thrift config object
     */
    SEnvironmentConfig(TEnvironmentConfig tConfig) {
        super(tConfig);
    }

    /**
     * Return true if the environment is configured to create any underlying
     * files, as necessary.
     *
     * @return true if the environment is configured to create any underlying
     * files, as necessary
     */
    public boolean getAllowCreate() {
        return (boolean) getField(TEnvironmentConfig._Fields.ALLOW_CREATE);
    }

    /**
     * Configure the environment to create any underlying files, as necessary.
     *
     * @param allowCreate if true, configure the environment to create any
     * underlying files, as necessary
     * @return this
     */
    public SEnvironmentConfig setAllowCreate(final boolean allowCreate) {
        getThriftObj().setAllowCreate(allowCreate);
        return this;
    }

    /**
     * Return the number of shared memory buffer pools, that is, the number of
     * cache regions.
     *
     * @return the number of shared memory buffer pools
     */
    public int getCacheCount() {
        return (int) getField(TEnvironmentConfig._Fields.CACHE_COUNT);
    }

    /**
     * Set the number of shared memory buffer pools.
     * <p>
     * It is possible to specify caches large enough so that they cannot be
     * allocated contiguously on some architectures. This method allows
     * applications to break the cache into a number of equally sized, separate
     * pieces of memory.
     * <p>
     * This method configures an environment, including all handles on the
     * environment, not only the operations performed using a specified handle.
     * <p>
     * This attribute cannot be changed after the environment is constructed.
     * If joining an existing environment, this attribute is ignored.
     *
     * @param cacheCount the number of shared buffer pools
     * @return this
     */
    public SEnvironmentConfig setCacheCount(final int cacheCount) {
        getThriftObj().setCacheCount(cacheCount);
        return this;
    }

    /**
     * Return the size of the shared memory buffer pool, that is, the cache.
     *
     * @return the size of the cache
     */
    public long getCacheSize() {
        return (long) getField(TEnvironmentConfig._Fields.CACHE_SIZE);
    }

    /**
     * Set the size of the cache.
     * <p>
     * The cache should be the size of the normal working data set of the
     * application, with some small amount of additional memory for unusual
     * situations. (Note: the working set is not the same as the number of
     * pages
     * accessed simultaneously, and is usually much larger.)
     * <p>
     * The default cache size is 256KB, and may not be specified as less than
     * 20KB. Any cache size less than 500MB is automatically increased by 25%
     * to
     * account for buffer pool overhead; cache sizes larger than 500MB are used
     * as specified. The current maximum size of a single cache is 4GB. (All
     * sizes are in powers-of-two, that is, 256KB is 2^18 not 256,000.)
     * <p>
     * This method configures an environment, including all handles on the
     * environment, not only the operations performed using a specified handle.
     * <p>
     * This attribute cannot be changed after the environment is constructed.
     * If joining an existing environment, this attribute is ignored.
     *
     * @param cacheSize the size of the cache
     * @return this
     */
    public SEnvironmentConfig setCacheSize(final long cacheSize) {
        getThriftObj().setCacheSize(cacheSize);
        return this;
    }

    /**
     * Set the password used to perform encryption and decryption.
     *
     * @param password the password
     * @return this
     */
    public SEnvironmentConfig setEncrypted(final String password) {
        getThriftObj().setEncryptionKey(password);
        return this;
    }

    /**
     * Always return true. For compatibility with DPL APIs.
     *
     * @return always true
     */
    public boolean getInitializeCache() {
        return true;
    }

    /**
     * Always return true. For compatibility with DPL APIs.
     *
     * @return always true
     */
    public boolean getInitializeLocking() {
        return true;
    }

    /**
     * Always return false. For compatibility with DPL APIs.
     *
     * @return always false
     */
    public boolean getInitializeCDB() {
        return false;
    }

    /**
     * Return the policy used by the deadlock detector.
     *
     * @return the deadlock detector policy
     */
    public SLockDetectMode getLockDetectMode() {
        return SLockDetectMode.toBdb((TLockDetectMode) getField(
                TEnvironmentConfig._Fields.LOCK_DETECT_MODE));
    }

    /**
     * Set policy used by the deadlock detector.
     * <p>
     * This method configures an environment, including all handles on the
     * environment, not only the operations performed using a specified handle.
     * <p>
     * This attribute cannot be changed after the environment is constructed.
     * If joining an existing environment, this attribute is ignored.
     *
     * @param lockDetectMode the lock request to be rejected
     * @return this
     */
    public SEnvironmentConfig setLockDetectMode(
            final SLockDetectMode lockDetectMode) {
        getThriftObj()
                .setLockDetectMode(SLockDetectMode.toThrift(lockDetectMode));
        return this;
    }

    /**
     * Return true if the handle is configured to open all databases for
     * multiversion concurrency control.
     *
     * @return true if the handle is configured to open all databases for
     * multiversion concurrency control.
     */
    public boolean getMultiversion() {
        return (boolean) getField(TEnvironmentConfig._Fields.MULTIVERSION);
    }

    /**
     * Configure the database environment to open all databases that are not
     * using the queue access method for multiversion concurrency control.
     * <p>
     * This method only affects the specified {@link SEnvironment} handle (and
     * any other library handles opened within the scope of that handle). For
     * consistent behavior across the environment, all SEnvironment handles
     * opened in the database environment must call this method.
     *
     * @param multiversion if true, all databases that are not using the queue
     * access method will be opened for multiversion concurrency control.
     * @return this
     */
    public SEnvironmentConfig setMultiversion(final boolean multiversion) {
        getThriftObj().setMultiversion(multiversion);
        return this;
    }

    /**
     * Return true if the handle is configured to run normal recovery on the
     * environment before opening it for use.
     *
     * @return true if the handle is configured to run normal recovery before
     * opening the environment
     */
    public boolean getRunRecovery() {
        return (boolean) getField(TEnvironmentConfig._Fields.RUN_RECOVERY);
    }

    /**
     * Configure to run normal recovery on this environment before opening it
     * for normal use.
     * <p>
     * A standard part of the recovery process is to remove the existing
     * database environment and create a new one. If the thread of control
     * performing recovery does not specify the correct database environment
     * initialization information (for example, the correct memory pool cache
     * size), the result can be an application running in an environment with
     * incorrect cache and other subsystem sizes. For this reason, the thread
     * of control performing recovery should specify correct configuration
     * information before recovering the environment.
     * <p>
     * All recovery processing must be single-threaded; that is, only a single
     * thread of control may perform recovery or access a database environment
     * while recovery is being performed.
     *
     * @param runRecovery if true, configure to run normal recovery on this
     * environment before opening it for normal use.
     * @return this
     */
    public SEnvironmentConfig setRunRecovery(final boolean runRecovery) {
        getThriftObj().setRunRecovery(runRecovery);
        return this;
    }

    /**
     * Always return true. For compatibility with DPL APIs.
     *
     * @return always true
     */
    public boolean getTransactional() {
        return true;
    }

    /**
     * Return true if the transactions have been configured to not wait for
     * locks by default.
     *
     * @return true if the transactions have been configured to not wait for
     * locks by default.
     */
    public boolean getTxnNoWait() {
        return (boolean) getField(TEnvironmentConfig._Fields.TXN_NO_WAIT);
    }

    /**
     * If a lock is unavailable for any Berkeley DB operation performed in the
     * context of a transaction, cause the operation to throw
     * {@link SLockNotGrantedException} without waiting for the lock.
     * <p>
     * This method only affects the specified {@link SEnvironment} handle (and
     * any other library handles opened within the scope of that handle). For
     * consistent behavior across the environment, all SEnvironment handles
     * opened in the database environment must call this method.
     *
     * @param txnNoWait if true, configure transactions to not wait for locks
     * by default.
     * @return this
     */
    public SEnvironmentConfig setTxnNoWait(final boolean txnNoWait) {
        getThriftObj().setTxnNoWait(txnNoWait);
        return this;
    }

    /**
     * Return true if the handle is configured to run all transactions at
     * snapshot isolation.
     *
     * @return true if the handle is configured to run all transactions at
     * snapshot isolation.
     */
    public boolean getTxnSnapshot() {
        return (boolean) getField(TEnvironmentConfig._Fields.TXN_SNAPSHOT);
    }

    /**
     * Configure the database environment to run transactions at snapshot
     * isolation by default.
     * <p>
     * This method only affects the specified {@link SEnvironment} handle (and
     * any other library handles opened within the scope of that handle). For
     * consistent behavior across the environment, all SEnvironment handles
     * opened in the database environment must call this method.
     *
     * @param txnSnapshot if true, configure the system to default to snapshot
     * isolation.
     * @return this
     */
    public SEnvironmentConfig setTxnSnapshot(final boolean txnSnapshot) {
        getThriftObj().setTxnSnapshot(txnSnapshot);
        return this;
    }

    /**
     * Return true if the system has been configured to not write or
     * synchronously flush the log on transaction commit.
     *
     * @return true if the system has been configured to not write or
     * synchronously flush the log on transaction commit.
     */
    public boolean getTxnNoSync() {
        return isDurabilityPolicyEqual(TDurabilityPolicy.NO_SYNC);

    }

    /**
     * Configure the system to not write or synchronously flush the log on
     * transaction commit.
     * <p>
     * This means that transactions exhibit the ACI (atomicity, consistency,
     * and isolation) properties, but not D (durability); that is, database
     * integrity will be maintained, but if the server fails, it is possible
     * some number of the most recently committed transactions may be undone
     * during recovery. The number of transactions at risk is governed by how
     * many log updates can fit into the log buffer, how often the server
     * operating system flushes dirty buffers to disk, and how often the log is
     * checkpointed.
     * <p>
     * This method only affects the specified {@link SEnvironment} handle (and
     * any other library handles opened within the scope of that handle). For
     * consistent behavior across the environment, all SEnvironment handles
     * opened in the database environment must call this method.
     *
     * @param txnNoSync if true, configure the system to not write or
     * synchronously flush the log on transaction commit.
     * @return this
     */
    public SEnvironmentConfig setTxnNoSync(final boolean txnNoSync) {
        setDurabilityPolicy(TDurabilityPolicy.NO_SYNC, txnNoSync);
        return this;
    }

    /**
     * Return true if the system has been configured to write, but not
     * synchronously flush, the log on transaction commit.
     *
     * @return true if the system has been configured to write, but not
     * synchronously flush, the log on transaction commit.
     */
    public boolean getTxnWriteNoSync() {
        return isDurabilityPolicyEqual(TDurabilityPolicy.WRITE_NO_SYNC);
    }

    /**
     * Configure the system to write, but not synchronously flush, the log on
     * transaction commit.
     * <p>
     * This means that transactions exhibit the ACI (atomicity, consistency,
     * and isolation) properties, but not D (durability); that is, database
     * integrity will be maintained, but if the server fails, it is possible
     * some number of the most recently committed transactions may be undone
     * during recovery. The number of transactions at risk is governed by how
     * often the server system flushes dirty buffers to disk and how often the
     * log is checkpointed.
     * <p>
     * This method only affects the specified {@link SEnvironment} handle (and
     * any other library handles opened within the scope of that handle). For
     * consistent behavior across the environment, all SEnvironment handles
     * opened in the database environment must call this method.
     *
     * @param txnWriteNoSync if true, configure the system to write, but not
     * synchronously flush, the log on transaction commit.
     * @return this
     */
    public SEnvironmentConfig setTxnWriteNoSync(final boolean txnWriteNoSync) {
        setDurabilityPolicy(TDurabilityPolicy.WRITE_NO_SYNC, txnWriteNoSync);
        return this;
    }

    private boolean isDurabilityPolicyEqual(TDurabilityPolicy policy) {
        return getThriftObj().isSetDurability() &&
                policy.equals(getField(TEnvironmentConfig._Fields.DURABILITY));
    }

    private void setDurabilityPolicy(TDurabilityPolicy policy, boolean set) {
        if (set) {
            getThriftObj().setDurability(policy);
        } else if (isDurabilityPolicyEqual(policy)) {
            getThriftObj().unsetDurability();
        }
    }
}
