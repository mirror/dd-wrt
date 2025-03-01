/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TSequenceConfig;

/**
 * Specify the attributes of a sequence.
 * <p>
 * For a user created instance, no attribute is set by default. In addition,
 * calling the getter method of an unset attribute results in an
 * IllegalStateException. To set an attribute, call the setter method of the
 * attribute.
 * <p>
 * When used to create a sequence, system default values are used for
 * unset attributes.
 */
public class SSequenceConfig
        extends ThriftWrapper<TSequenceConfig, TSequenceConfig._Fields> {
    /**
     * Create an empty SSequenceConfig with no attribute set.
     */
    public SSequenceConfig() {
        super(new TSequenceConfig());
        setExclusiveCreate(false);
    }

    /**
     * Return true if the {@link SDatabase#openSequence} method is configured
     * to create the sequence if it does not already exist.
     *
     * @return true if the {@link SDatabase#openSequence} method is configured
     * to create the sequence if it does not already exist.
     */
    public boolean getAllowCreate() {
        return (boolean) getField(TSequenceConfig._Fields.ALLOW_CREATE);
    }

    /**
     * Configure the {@link SDatabase#openSequence} method to
     * create the sequence if it does not already exist.
     *
     * @param allowCreate if true, configure the {@link SDatabase#openSequence}
     * method to create the sequence if it does not already exist
     * @return this
     */
    public SSequenceConfig setAllowCreate(boolean allowCreate) {
        getThriftObj().setAllowCreate(allowCreate);
        return this;
    }

    /**
     * Return true if the auto-commit operations on the sequence are configure
     * to not flush the transaction log.
     *
     * @return true if the auto-commit operations on the sequence are configure
     * to not flush the transaction log
     */
    public boolean getAutoCommitNoSync() {
        return (boolean) getField(TSequenceConfig._Fields.AUTO_COMMIT_NO_SYNC);
    }

    /**
     * Configure auto-commit operations on the sequence to not flush the
     * transaction log.
     *
     * @param autoCommitNoSync if true, configure auto-commit operations on the
     * sequence to not flush the transaction log
     * @return this
     */
    public SSequenceConfig setAutoCommitNoSync(boolean autoCommitNoSync) {
        getThriftObj().setAutoCommitNoSync(autoCommitNoSync);
        return this;
    }

    /**
     * Return the number of elements cached by a sequence handle.
     *
     * @return the number of elements cached by a sequence handle
     */
    public int getCacheSize() {
        return (int) getField(TSequenceConfig._Fields.CACHE_SIZE);
    }

    /**
     * Set the number of elements cached by a sequence handle.
     *
     * @param cacheSize the number of elements cached by a sequence handle
     * @return this
     */
    public SSequenceConfig setCacheSize(int cacheSize) {
        getThriftObj().setCacheSize(cacheSize);
        return this;
    }

    /**
     * Return true if the sequence is configured to decrement.
     *
     * @return true if the sequence is configured to decrement
     */
    public boolean getDecrement() {
        return (boolean) getField(TSequenceConfig._Fields.DECREMENT);
    }

    /**
     * Specify that the sequence should be decremented.
     *
     * @param decrement if true, specify that the sequence should be
     * decremented
     * @return this
     */
    public SSequenceConfig setDecrement(boolean decrement) {
        getThriftObj().setDecrement(decrement);
        return this;
    }

    /**
     * Return true if the {@link SDatabase#openSequence} method is configured
     * to
     * fail if the database already exists.
     *
     * @return true if the {@link SDatabase#openSequence} method is configured
     * to fail if the database already exists
     */
    public boolean getExclusiveCreate() {
        return (boolean) getField(TSequenceConfig._Fields.EXCLUSIVE_CREATE);
    }

    /**
     * Configure the {@link SDatabase#openSequence} method to fail if the
     * database already exists.
     *
     * @param exclusiveCreate if true, configure the {@link
     * SDatabase#openSequence} method to fail if the database already exists
     * @return this
     */
    public SSequenceConfig setExclusiveCreate(boolean exclusiveCreate) {
        getThriftObj().setExclusiveCreate(exclusiveCreate);
        return this;
    }

    /**
     * Return the initial value for a sequence.
     *
     * @return the initial value for a sequence
     */
    public long getInitialValue() {
        return (long) getField(TSequenceConfig._Fields.INITIAL_VALUE);
    }

    /**
     * Set the initial value for a sequence.
     * <p>
     * This call is only effective when the sequence is being created.
     *
     * @param initialValue the initial value for a sequence
     * @return this
     */
    public SSequenceConfig setInitialValue(long initialValue) {
        getThriftObj().setInitialValue(initialValue);
        return this;
    }

    /**
     * Return the minimum value for the sequence.
     *
     * @return the minimum value for the sequence
     */
    public long getRangeMin() {
        return (long) getField(TSequenceConfig._Fields.MINIMUM);
    }

    /**
     * Return the maximum value for the sequence.
     *
     * @return the maximum value for the sequence
     */
    public long getRangeMax() {
        return (long) getField(TSequenceConfig._Fields.MAXIMUM);
    }

    /**
     * Configure a sequence range.
     * <p>
     * This call is only effective when the sequence is being created.
     *
     * @param min the minimum value for the sequence
     * @param max the maximum value for the sequence
     * @return this
     */
    public SSequenceConfig setRange(long min, long max) {
        getThriftObj().setMaximum(max).setMinimum(min);
        return this;
    }

    /**
     * Return true if the sequence will wrap around when it is incremented
     * (decremented) past the specified maximum (minimum) value.
     *
     * @return true if the sequence will wrap around when it is incremented
     * (decremented) past the specified maximum (minimum) value
     */
    public boolean getWrap() {
        return (boolean) getField(TSequenceConfig._Fields.WRAP);
    }

    /**
     * Specify that the sequence should wrap around when it is incremented
     * (decremented) past the specified maximum (minimum) value.
     *
     * @param wrap if true, specify that the sequence should wrap around when it
     * is incremented (decremented) past the specified maximum (minimum) value
     * @return this
     */
    public SSequenceConfig setWrap(boolean wrap) {
        getThriftObj().setWrap(wrap);
        return this;
    }
}
