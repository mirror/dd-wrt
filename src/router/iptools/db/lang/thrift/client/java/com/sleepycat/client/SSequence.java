/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TSequence;

/**
 * A SSequence handle is used to manipulate a sequence record in a database.
 * SSequence handles are opened using the {@link SDatabase#openSequence}
 * method.
 */
public class SSequence implements RemoteCallHelper, AutoCloseable {
    /** The remote sequence handle. */
    private final TSequence tSequence;

    /** The remote service client. */
    private final BdbService.Client client;

    /** The enclosing database. */
    private final SDatabase database;

    /** The sequence key. */
    private final SDatabaseEntry key;

    SSequence(TSequence tSequence, BdbService.Client client,
            SDatabase database, SDatabaseEntry key) {
        this.tSequence = tSequence;
        this.client = client;
        this.database = database;
        this.key = key;
    }

    /**
     * Return the SDatabase handle associated with this sequence.
     *
     * @return the SDatabase handle associated with this sequence
     */
    public SDatabase getDatabase() {
        return this.database;
    }

    /**
     * Return the SDatabaseEntry used to open this sequence.
     *
     * @return the SDatabaseEntry used to open this sequence
     */
    public SDatabaseEntry getKey() {
        return this.key;
    }

    /**
     * Close a sequence. Any unused cached values are lost.
     * <p>
     * The sequence handle may not be used again after this method has been
     * called, regardless of the method's success or failure.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void close() throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeSequenceHandle(this.tSequence);
            return null;
        });
    }

    /**
     * Return the next available element in the sequence and changes the
     * sequence value by delta. The value of delta must be greater than zero.
     * If there are enough cached values in the sequence handle then they will
     * be returned. Otherwise the next value will be fetched from the database
     * and incremented (decremented) by enough to cover the delta and the next
     * batch of cached values.
     * <p>
     * The txn handle must be null if the sequence handle was opened with a
     * non-zero cache size.
     * <p>
     * For maximum concurrency, a non-zero cache size should be specified prior
     * to opening the sequence handle, the txn handle should be null, and
     * {@link SSequenceConfig#setAutoCommitNoSync} should be called to disable
     * log flushes.
     *
     * @param txn an explicit transaction may be specified, or null may be
     * specified to use auto-commit
     * @param delta the amount by which to increment or decrement the sequence
     * @return the next available element in the sequence
     * @throws SDatabaseException if any error occurs
     */
    public long get(STransaction txn, int delta) throws SDatabaseException {
        return remoteCall(() -> this.client.sequenceGet(this.tSequence,
                STransaction.nullSafeGet(txn), delta));
    }
}
