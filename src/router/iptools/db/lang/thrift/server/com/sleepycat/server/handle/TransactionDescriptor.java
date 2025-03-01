/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.Transaction;
import com.sleepycat.thrift.TDurabilityPolicy;

import java.util.concurrent.atomic.AtomicReference;

/**
 * An TransactionDescriptor is a HandleDescriptor for a Transaction.
 */
public class TransactionDescriptor extends HandleDescriptor<Transaction> {

    /** The parent transaction. */
    private final TransactionDescriptor parent;

    /** The resolved status. */
    private AtomicReference<Status> status =
            new AtomicReference<>(Status.NOT_RESOLVED);

    /**
     * Create a TransactionDescriptor.
     *
     * @param txn the BDB transaction handle
     * @param env the enclosing environment
     * @param parentTxn the parent transaction
     */
    public TransactionDescriptor(Transaction txn, EnvironmentDescriptor env,
            TransactionDescriptor parentTxn) {
        super(txn, null, env, parentTxn);
        this.parent = parentTxn;
    }

    /**
     * Set the resolved status of this transaction. The resolved status can
     * only be set once.
     *
     * @param resolvedStatus the resolved status
     * @return true if status is set, false if the transaction has been resolved
     */
    public boolean resolve(Status resolvedStatus) {
        return this.status.compareAndSet(Status.NOT_RESOLVED, resolvedStatus);
    }

    @Override
    public ResourceKey[] resourceOwners() {
        return new ResourceKey[0];
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        switch (getStatus()) {
            case COMMITTED:
                getHandle().commit();
                break;
            case COMMITTED_NO_SYNC:
                getHandle().commitNoSync();
                break;
            case COMMITTED_SYNC:
                getHandle().commitSync();
                break;
            case COMMITTED_WRITE_NO_SYNC:
                getHandle().commitWriteNoSync();
                break;
            default:
                getHandle().abort();
        }
    }

    private Status getStatus() {
        if (this.status.get() == Status.NOT_RESOLVED && this.parent != null) {
            resolve(this.parent.getStatus());
        }
        if (this.status.get() == Status.NOT_RESOLVED) {
            resolve(Status.ABORTED);
        }
        return this.status.get();
    }

    /** The resolved status of a transaction. */
    public enum Status {
        NOT_RESOLVED, COMMITTED, COMMITTED_NO_SYNC, COMMITTED_SYNC,
        COMMITTED_WRITE_NO_SYNC, ABORTED;

        public static Status toStatus(TDurabilityPolicy durability) {
            if (durability == null)
                return COMMITTED;
            switch (durability) {
                case NO_SYNC:
                    return COMMITTED_NO_SYNC;
                case SYNC:
                    return COMMITTED_SYNC;
                case WRITE_NO_SYNC:
                    return COMMITTED_WRITE_NO_SYNC;
                default:
                    throw new IllegalArgumentException(durability.toString());
            }
        }
    }
}
