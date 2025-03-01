/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import java.io.IOException;
import java.util.function.Function;

/**
 * TxnHelper implements a few helper methods for working with transactions.
 */
interface TxnHelper {
    /**
     * Return the environment used to create transactions.
     *
     * @return the environment used to create transactions
     */
    SEnvironment getEnvironment();

    /**
     * Run multiple operations in the context of a single transaction. If the
     * user-specified transaction is not null, operations are run in the
     * context of the user-specified transaction. Otherwise, the operations are
     * run in a new transaction.
     *
     * @param userTxn the user-specified transaction, may be null
     * @param operations the operations
     * @param <V> the type of the result
     * @return the result of the operations
     * @throws SDatabaseException on error
     */
    default <V> V runInSingleTxn(STransaction userTxn,
            Function<STransaction, V> operations) throws SDatabaseException {
        try {
            return runInSingleTxnWithIOException(userTxn, operations::apply);
        } catch (IOException e) {
            // Unreachable.
            assert false;
            return null;
        }
    }

    /**
     * Run multiple operations in the context of a single transaction. If the
     * user-specified transaction is not null, operations are run in the
     * context of the user-specified transaction. Otherwise, the operations are
     * run in a new transaction.
     * <p>
     * This method allows operations to throw IOException.
     *
     * @param userTxn the user-specified transaction, may be null
     * @param operations the operations
     * @param <V> the type of the result
     * @return the result of the operations
     * @throws IOException on I/O error
     * @throws SDatabaseException on error
     */
    default <V> V runInSingleTxnWithIOException(STransaction userTxn,
            TransactionOperations<V> operations)
            throws IOException, SDatabaseException {
        STransaction txn = userTxn;
        if (txn == null) {
            txn = getEnvironment().beginTransaction(null, null);
        }

        V result;
        try {
            result = operations.call(txn);
        } catch (IOException | RuntimeException e) {
            if (userTxn == null) {
                txn.abort();
            }
            throw e;
        }
        if (userTxn == null) {
            txn.commit();
        }
        return result;
    }

    @FunctionalInterface
    interface TransactionOperations<V> {
        V call(STransaction txn) throws IOException, SDatabaseException;
    }
}
