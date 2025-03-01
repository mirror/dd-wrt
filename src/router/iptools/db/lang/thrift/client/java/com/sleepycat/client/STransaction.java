/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TDurabilityPolicy;
import com.sleepycat.thrift.TTransaction;

import java.util.Objects;

/**
 * The STransaction object is the handle for a transaction. Methods off the
 * transaction handle are used to configure, abort and commit the transaction.
 * Transaction handles are provided to other Berkeley DB driver methods in
 * order to transactionally protect those operations.
 * <p>
 * Once the {@link #abort}, or {@link #commit} methods are called, the handle
 * may not be accessed again, regardless of the success or failure of the
 * method. In addition, parent transactions may not issue any Berkeley DB
 * driver operations while they have active child transactions (child
 * transactions that have not yet been committed or aborted) except for {@link
 * SEnvironment#beginTransaction}, {@link STransaction#abort} and {@link
 * STransaction#commit}.
 * <p>
 * To obtain a transaction with default attributes:
 * <pre>
 *  STransaction txn = myEnvironment.beginTransaction(null, null);
 * </pre>
 * To customize the attributes of a transaction:
 * <pre>
 *  STransactionConfig config = new STransactionConfig();
 *  config.setReadUncommitted(true);
 *  STransaction txn = myEnvironment.beginTransaction(null, config);
 * </pre>
 */
public class STransaction implements RemoteCallHelper {
    /** The remote transaction handle. */
    private final TTransaction tTransaction;

    /** The remote service client. */
    private final BdbService.Client client;

    STransaction(TTransaction transaction, BdbService.Client client) {
        this.tTransaction = Objects.requireNonNull(transaction);
        this.client = Objects.requireNonNull(client);
    }

    /**
     * Return the wrapped remote transaction handle. This method is safe to be
     * called with a null object, in which case null is returned.
     *
     * @param txn the transaction handle or null
     * @return the wrapped remote transaction handle or null if the given handle
     * is null
     */
    static TTransaction nullSafeGet(STransaction txn) {
        return txn == null ? null : txn.tTransaction;
    }

    /**
     * Cause an abnormal termination of the transaction.
     * <p>
     * The log is played backward, and any necessary undo operations are done.
     * Before this method returns, any locks held by the transaction will
     * have been released.
     * <p>
     * In the case of nested transactions, aborting a parent transaction causes
     * all children (unresolved or not) of the parent transaction to be
     * aborted.
     * <p>
     * All cursors opened within the transaction must be closed before the
     * transaction is aborted. This method closes all open {@link SCursor}
     * handles. And if a close operation fails, the rest of the cursors are
     * closed, and the database environment is set to the panic state.
     * <p>
     * After this method has been called, regardless of its return, this handle
     * may not be accessed again.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void abort() throws SDatabaseException {
        remoteCall(() -> {
            this.client.txnAbort(this.tTransaction);
            return null;
        });
    }

    /**
     * End the transaction. If the environment is configured for synchronous
     * commit, the transaction will be committed synchronously to stable
     * storage before the call returns. This means the transaction will exhibit
     * all of the ACID (atomicity, consistency, isolation, and durability)
     * properties.
     * <p>
     * If the environment is not configured for synchronous commit,
     * the commit will not necessarily have been committed to stable storage
     * before the call returns. This means the transaction will exhibit the ACI
     * (atomicity, consistency, and isolation) properties, but not D
     * (durability); that is, database integrity will be maintained, but it is
     * possible this transaction may be undone during recovery.
     * <p>
     * In the case of nested transactions, if the transaction is a parent
     * transaction, committing the parent transaction causes all unresolved
     * children of the parent to be committed. In the case of nested
     * transactions, if the transaction is a child transaction, its locks are
     * not released, but are acquired by its parent. Although the commit of the
     * child transaction will succeed, the actual resolution of the child
     * transaction is postponed until the parent transaction is committed or
     * aborted; that is, if its parent transaction commits, it will be
     * committed; and if its parent transaction aborts, it will be aborted.
     * <p>
     * All cursors opened within the transaction must be closed before the
     * transaction is committed. If there are {@link SCursor} handles open when
     * this method is called, they are all closed inside this method. And if
     * there are errors when closing the cursor handles, the transaction is
     * aborted and the first such error is returned.
     * <p>
     * After this method returns, this handle may not be accessed again,
     * regardless of the method's success or failure. If the method encounters
     * an error, the transaction and all child transactions of the transaction
     * will have been aborted when the call returns.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void commit() throws SDatabaseException {
        remoteCall(() -> {
            this.client.txnCommit(this.tTransaction, null);
            return null;
        });
    }

    /**
     * End the transaction, not committing synchronously. This means the
     * transaction will exhibit the ACI (atomicity, consistency, and isolation)
     * properties, but not D (durability); that is, database integrity will be
     * maintained, but it is possible this transaction may be undone during
     * recovery.
     * <p>
     * This behavior may be set for a database environment using the
     * {@link SEnvironmentConfig#setTxnNoSync} method or for a single
     * transaction using the {@link SEnvironment#beginTransaction} method. Any
     * value specified to this method overrides both of those settings.
     * <p>
     * In the case of nested transactions, if the transaction is a parent
     * transaction, committing the parent transaction causes all unresolved
     * children of the parent to be committed. In the case of nested
     * transactions, if the transaction is a child transaction, its locks are
     * not released, but are acquired by its parent. Although the commit of the
     * child transaction will succeed, the actual resolution of the child
     * transaction is postponed until the parent transaction is committed or
     * aborted; that is, if its parent transaction commits, it will be
     * committed; and if its parent transaction aborts, it will be aborted.
     * <p>
     * All cursors opened within the transaction must be closed before the
     * transaction is committed. If there are {@link SCursor} handles open when
     * this method is called, they are all closed inside this method. And if
     * there are errors when closing the cursor handles, the transaction is
     * aborted and the first such error is returned.
     * <p>
     * After this method returns, this handle may not be accessed again,
     * regardless of the method's success or failure. If the method encounters
     * an error, the transaction and all child transactions of the transaction
     * will have been aborted when the call returns.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void commitNoSync() throws SDatabaseException {
        remoteCall(() -> {
            this.client.txnCommit(this.tTransaction, TDurabilityPolicy.NO_SYNC);
            return null;
        });
    }

    /**
     * End the transaction, committing synchronously. This means the
     * transaction will exhibit all of the ACID (atomicity, consistency,
     * isolation, and durability) properties.
     * <p>
     * This behavior is the default for database environments unless otherwise
     * configured using the {@link SEnvironmentConfig#setTxnNoSync} method.
     * This behavior may also be set for a single transaction using the
     * {@link SEnvironment#beginTransaction} method. Any value specified to
     * this method overrides both of those settings.
     * <p>
     * In the case of nested transactions, if the transaction is a parent
     * transaction, committing the parent transaction causes all unresolved
     * children of the parent to be committed. In the case of nested
     * transactions, if the transaction is a child transaction, its locks are
     * not released, but are acquired by its parent. Although the commit of the
     * child transaction will succeed, the actual resolution of the child
     * transaction is postponed until the parent transaction is committed or
     * aborted; that is, if its parent transaction commits, it will be
     * committed; and if its parent transaction aborts, it will be aborted.
     * <p>
     * All cursors opened within the transaction must be closed before the
     * transaction is committed. If there are {@link SCursor} handles open when
     * this method is called, they are all closed inside this method. And if
     * there are errors when closing the cursor handles, the transaction is
     * aborted and the first such error is returned.
     * <p>
     * After this method returns, this handle may not be accessed again,
     * regardless of the method's success or failure. If the method encounters
     * an error, the transaction and all child transactions of the transaction
     * will have been aborted when the call returns.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void commitSync() throws SDatabaseException {
        remoteCall(() -> {
            this.client.txnCommit(this.tTransaction, TDurabilityPolicy.SYNC);
            return null;
        });
    }

    /**
     * End the transaction, writing but not flushing the log. This means the
     * transaction will exhibit the ACI (atomicity, consistency, and isolation)
     * properties, but not D (durability); that is, database integrity will be
     * maintained, but it is possible this transaction may be undone during
     * recovery in the event that the operating system crashes. This option
     * provides more durability than an asynchronous commit and has less
     * performance cost than a synchronous commit.
     * <p>
     * This behavior may be set for a database environment using the {@link
     * SEnvironmentConfig#setTxnWriteNoSync} method or for a single transaction
     * using the {@link SEnvironment#beginTransaction} method. Any value
     * specified to this method overrides both of those settings.
     * <p>
     * In the case of nested transactions, if the transaction is a parent
     * transaction, committing the parent transaction causes all unresolved
     * children of the parent to be committed. In the case of nested
     * transactions, if the transaction is a child transaction, its locks are
     * not released, but are acquired by its parent. Although the commit of the
     * child transaction will succeed, the actual resolution of the child
     * transaction is postponed until the parent transaction is committed or
     * aborted; that is, if its parent transaction commits, it will be
     * committed; and if its parent transaction aborts, it will be aborted.
     * <p>
     * All cursors opened within the transaction must be closed before the
     * transaction is committed. If there are {@link SCursor} handles open when
     * this method is called, they are all closed inside this method. And if
     * there are errors when closing the cursor handles, the transaction is
     * aborted and the first such error is returned.
     * <p>
     * After this method returns, this handle may not be accessed again,
     * regardless of the method's success or failure. If the method encounters
     * an error, the transaction and all child transactions of the transaction
     * will have been aborted when the call returns.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void commitWriteNoSync() throws SDatabaseException {
        remoteCall(() -> {
            this.client.txnCommit(this.tTransaction,
                    TDurabilityPolicy.WRITE_NO_SYNC);
            return null;
        });
    }

    /**
     * Get the transaction's deadlock priority.
     *
     * @return the transaction's deadlock priority
     * @throws SDatabaseException if any error occurs
     */
    public int getPriority() throws SDatabaseException {
        return remoteCall(() -> this.client.txnGetPriority(this.tTransaction));
    }

    /**
     * Set the deadlock priority for this transaction. The deadlock detector
     * will reject lock requests from lower priority transactions before those
     * from higher priority transactions.
     *
     * @param priority the deadlock priority for the transaction.
     * @throws SDatabaseException if any error occurs
     */
    public void setPriority(int priority) throws SDatabaseException {
        remoteCall(() -> {
            this.client.txnSetPriority(this.tTransaction, priority);
            return null;
        });
    }
}
