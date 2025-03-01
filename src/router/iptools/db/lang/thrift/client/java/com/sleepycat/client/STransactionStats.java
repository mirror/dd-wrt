/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TActiveTxnStat;
import com.sleepycat.thrift.TTransactionStat;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Transaction statistics for a database environment.
 */
public class STransactionStats {
    /** The Thrift object. */
    private final TTransactionStat stat;

    /** The LSN of the last checkpoint. */
    private final SLogSequenceNumber lastCkp;

    STransactionStats(TTransactionStat stat) {
        this.stat = stat;
        this.lastCkp =
                new SLogSequenceNumber(stat.lastCkpFile, stat.lastCkpOffset);
    }

    /**
     * The number of transactions that have been restored.
     *
     * @return the number of transactions that have been restored
     */
    public int getNumRestores() {
        return this.stat.numRestores;
    }

    /**
     * The LSN of the last checkpoint.
     *
     * @return the LSN of the last checkpoint
     */
    public SLogSequenceNumber getLastCkp() {
        return this.lastCkp;
    }

    /**
     * The time the last completed checkpoint finished (as the number of
     * seconds since the Epoch, returned by the IEEE/ANSI Std 1003.1
     * (POSIX) time interface).
     *
     * @return the time the last completed checkpoint finished
     */
    public long getTimeCkp() {
        return this.stat.timeCkp;
    }

    /**
     * The last transaction ID allocated.
     *
     * @return the last transaction ID allocated
     */
    public int getLastTxnId() {
        return this.stat.lastTxnId;
    }

    /**
     * The initial number of transactions configured.
     *
     * @return the initial number of transactions configured
     */
    public int getInittxns() {
        return this.stat.inittxns;
    }

    /**
     * The maximum number of active transactions configured.
     *
     * @return the maximum number of active transactions configured
     */
    public int getMaxTxns() {
        return this.stat.maxTxns;
    }

    /**
     * The number of transactions that have aborted.
     *
     * @return the number of transactions that have aborted
     */
    public long getNaborts() {
        return this.stat.naborts;
    }

    /**
     * The number of transactions that have begun.
     *
     * @return the number of transactions that have begun
     */
    public long getNumBegins() {
        return this.stat.numBegins;
    }

    /**
     * The number of transactions that have committed.
     *
     * @return the number of transactions that have committed
     */
    public long getNumCommits() {
        return this.stat.numCommits;
    }

    /**
     * The number of transactions that are currently active.
     *
     * @return the number of transactions that are currently active
     */
    public int getNactive() {
        return this.stat.nactive;
    }

    /**
     * The number of transactions on the snapshot list. These are transactions
     * which modified a database opened with
     * {@link SDatabaseConfig#setMultiversion}, and which have committed or
     * aborted, but the copies of pages they created are still in the cache.
     *
     * @return the number of transactions on the snapshot list
     */
    public int getNumSnapshot() {
        return this.stat.numSnapshot;
    }

    /**
     * The maximum number of active transactions at any one time.
     *
     * @return the maximum number of active transactions at any one time
     */
    public int getMaxNactive() {
        return this.stat.maxNactive;
    }

    /**
     * The maximum number of transactions on the snapshot list at any one time.
     *
     * @return the maximum number of transactions on the snapshot list at any
     * one time
     */
    public int getMaxNsnapshot() {
        return this.stat.maxNsnapshot;
    }

    /**
     * The number of times that a thread of control was forced to wait before
     * obtaining the transaction region mutex.
     *
     * @return the number of times that a thread of control was forced to wait
     * before obtaining the transaction region mutex
     */
    public long getRegionWait() {
        return this.stat.regionWait;
    }

    /**
     * The number of times that a thread of control was able to obtain the
     * transaction region mutex without waiting.
     *
     * @return the number of times that a thread of control was able to obtain
     * the transaction region mutex without waiting
     */
    public long getRegionNowait() {
        return this.stat.regionNowait;
    }

    /**
     * The size of the region.
     *
     * @return the size of the region
     */
    public long getRegSize() {
        return this.stat.regSize;
    }

    /**
     * An array of {@code Active} objects, describing the currently active
     * transactions.
     *
     * @return an array of {@code Active} objects, describing the currently
     * active transactions
     */
    public Active[] getTxnarray() {
        List<Active> txnList = this.stat.activeTxns.stream()
                .map(Active::new).collect(Collectors.toList());
        return txnList.toArray(new Active[txnList.size()]);
    }

    /**
     * For convenience, the STransactionStats class has a toString method that
     * lists all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "TransactionStats:"
                + "\n  nrestores=" + getNumRestores()
                + "\n  last_ckp=" + getLastCkp()
                + "\n  time_ckp=" + getTimeCkp()
                + "\n  last_txnid=" + getLastTxnId()
                + "\n  inittxns=" + getInittxns()
                + "\n  maxtxns=" + getMaxTxns()
                + "\n  naborts=" + getNaborts()
                + "\n  nbegins=" + getNumBegins()
                + "\n  ncommits=" + getNumCommits()
                + "\n  nactive=" + getNactive()
                + "\n  nsnapshot=" + getNumSnapshot()
                + "\n  maxnactive=" + getMaxNactive()
                + "\n  maxnsnapshot=" + getMaxNsnapshot()
                + "\n  region_wait=" + getRegionWait()
                + "\n  region_nowait=" + getRegionNowait()
                + "\n  regsize=" + getRegSize()
                + "\n  txnarray=" + Arrays.toString(getTxnarray())
                ;
    }

    public static class Active {
        /** The Thrift object. */
        private final TActiveTxnStat stat;

        /** The log sequence number of the transaction's first log record. */
        private final SLogSequenceNumber lsn;

        /** The log sequence number of reads for snapshot transactions. */
        private final SLogSequenceNumber readLsn;

        Active(TActiveTxnStat stat) {
            this.stat = stat;
            this.lsn = new SLogSequenceNumber(stat.lsnFile, stat.lsnOffset);
            this.readLsn = new SLogSequenceNumber(stat.readLsnFile,
                    stat.readLsnOffset);
        }

        /**
         * The transaction ID of the transaction.
         *
         * @return the transaction ID of the transaction
         */
        public int getTxnId() {
            return this.stat.txnId;
        }

        /**
         * The transaction ID of the parent transaction (or 0, if no parent).
         *
         * @return the transaction ID of the parent transaction (or 0, if no
         * parent)
         */
        public int getParentId() {
            return this.stat.parentId;
        }

        /**
         * The process ID of the process that owns the transaction.
         *
         * @return the process ID of the process that owns the transaction
         */
        public int getPid() {
            return this.stat.pid;
        }

        /**
         * The log sequence number of the transaction's first log record.
         *
         * @return the log sequence number of the transaction's first log record
         */
        public SLogSequenceNumber getLsn() {
            return this.lsn;
        }

        /**
         * The log sequence number of reads for snapshot transactions.
         *
         * @return the log sequence number of reads for snapshot transactions
         */
        public SLogSequenceNumber getReadLsn() {
            return this.readLsn;
        }

        /**
         * The number of buffer copies created by this transaction that remain
         * in cache.
         *
         * @return the number of buffer copies created by this transaction that
         * remain in cache
         */
        public int getMultiversionRef() {
            return this.stat.multiversionRef;
        }

        /**
         * This transaction's deadlock resolution priority.
         *
         * @return this transaction's deadlock resolution priority
         */
        public int getPriority() {
            return this.stat.priority;
        }

        /**
         * The transaction name, including the thread name if available.
         *
         * @return the transaction name, including the thread name if available
         */
        public String getName() {
            return this.stat.name;
        }

        /** {@inheritDoc} */
        public String toString() {
            return "Active:"
                    + "\n      txnid=" + getTxnId()
                    + "\n      parentid=" + getParentId()
                    + "\n      pid=" + getPid()
                    + "\n      lsn=" + getLsn()
                    + "\n      read_lsn=" + getReadLsn()
                    + "\n      mvcc_ref=" + getMultiversionRef()
                    + "\n      priority=" + getPriority()
                    + "\n      name=" + getName()
                    ;
        }
    }
}
