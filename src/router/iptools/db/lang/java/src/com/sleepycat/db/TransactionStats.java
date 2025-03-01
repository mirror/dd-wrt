/*-
 * Automatically built by dist/s_java_stat.
 * Only the javadoc comments can be edited.
 *
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

package com.sleepycat.db;

import com.sleepycat.db.internal.DbUtil;

/**
Transaction statistics for a database environment.
*/
public class TransactionStats {
    // no public constructor
    /* package */ TransactionStats() {}

    public static class Active {
        // no public constructor
        /* package */ Active() {}

        private int txnid;
        /**
        The transaction ID of the transaction.
        @return the transaction ID of the transaction
        */
    public int getTxnId() {
            return txnid;
        }

        private int parentid;
        /**
        The transaction ID of the parent transaction (or 0, if no parent).
        @return the transaction ID of the parent transaction (or 0, if no parent)
        */
    public int getParentId() {
            return parentid;
        }

        private int pid;
        /**
        The process ID of the process that owns the transaction.
        @return the process ID of the process that owns the transaction
        */
    public int getPid() {
            return pid;
        }

        private LogSequenceNumber lsn;
        /**
        The log sequence number of the transaction's first log record.
        @return the log sequence number of the transaction's first log record
        */
    public LogSequenceNumber getLsn() {
            return lsn;
        }

        private LogSequenceNumber read_lsn;
        /**
        The log sequence number of reads for snapshot transactions.
        @return the log sequence number of reads for snapshot transactions
        */
    public LogSequenceNumber getReadLsn() {
            return read_lsn;
        }

        private int mvcc_ref;
        /**
        The number of buffer copies created by this transaction that remain in cache.
        @return the number of buffer copies created by this transaction that remain in cache
        */
    public int getMultiversionRef() {
            return mvcc_ref;
        }

        private int priority;
        /**
        This transaction's deadlock resolution priority.
        @return this transaction's deadlock resolution priority
        */
    public int getPriority() {
            return priority;
        }

        private byte[] gid;
        /**
        Return the transaction's Global ID, if the transaction was prepared using
        {@link Transaction#prepare}. Otherwise, return an undefined value.
        @return the transaction's global id
        */
        public byte[] getGId() {
            return gid;
        }

        private String name;
        /**
        The transaction name, including the thread name if available.
        @return the transaction name, including the thread name if available
        */
        public String getName() {
            return name;
        }

        private int[] slice_txns;
        /**
        An array of transaction ids of slices' active subordinate transactions.
        @return transaction ids of slices' active subordinate transactions
        */
    public int[] getSliceTxns() {
            return slice_txns;
        }

        /** {@inheritDoc} */
    public String toString() {
            return "Active:"
                + "\n      txnid=" + txnid
                + "\n      parentid=" + parentid
                + "\n      pid=" + pid
                + "\n      lsn=" + lsn
                + "\n      read_lsn=" + read_lsn
                + "\n      mvcc_ref=" + mvcc_ref
                + "\n      priority=" + priority
                + "\n      gid=" + DbUtil.byteArrayToString(gid)
                + "\n      name=" + name
                + "\n      slice_txns=" + DbUtil.integerArrayToString(slice_txns)
                ;
        }
    };

    private int st_nrestores;
    /**
    The number of transactions that have been restored.
    @return the number of transactions that have been restored
    */
    public int getNumRestores() {
        return st_nrestores;
    }

    private LogSequenceNumber st_last_ckp;
    /**
    The LSN of the last checkpoint.
    @return the LSN of the last checkpoint
    */
    public LogSequenceNumber getLastCkp() {
        return st_last_ckp;
    }

    private long st_time_ckp;
    /**
    The time the last completed checkpoint finished (as the number of
    seconds since the Epoch, returned by the IEEE/ANSI Std 1003.1
    (POSIX) time interface).
    @return the time the last completed checkpoint finished
    */
    public long getTimeCkp() {
        return st_time_ckp;
    }

    private int st_last_txnid;
    /**
    The last transaction ID allocated.
    @return the last transaction ID allocated
    */
    public int getLastTxnId() {
        return st_last_txnid;
    }

    private int st_inittxns;
    /**
    The initial number of transactions configured.
    @return the initial number of transactions configured
    */
    public int getInittxns() {
        return st_inittxns;
    }

    private int st_maxtxns;
    /**
    The maximum number of active transactions configured.
    @return the maximum number of active transactions configured
    */
    public int getMaxTxns() {
        return st_maxtxns;
    }

    private long st_naborts;
    /**
    The number of transactions that have aborted.
    @return the number of transactions that have aborted
    */
    public long getNaborts() {
        return st_naborts;
    }

    private long st_nbegins;
    /**
    The number of transactions that have begun.
    @return the number of transactions that have begun
    */
    public long getNumBegins() {
        return st_nbegins;
    }

    private long st_ncommits;
    /**
    The number of transactions that have committed.
    @return the number of transactions that have committed
    */
    public long getNumCommits() {
        return st_ncommits;
    }

    private int st_nactive;
    /**
    The number of transactions that are currently active.
    @return the number of transactions that are currently active
    */
    public int getNactive() {
        return st_nactive;
    }

    private int st_nsnapshot;
    /**
    The number of transactions on the snapshot list.  These are transactions
    which modified a database opened with {@link
    DatabaseConfig#setMultiversion}, and which have committed or aborted, but
    the copies of pages they created are still in the cache.
    @return the number of transactions on the snapshot list
    */
    public int getNumSnapshot() {
        return st_nsnapshot;
    }

    private int st_maxnactive;
    /**
    The maximum number of active transactions at any one time.
    @return the maximum number of active transactions at any one time
    */
    public int getMaxNactive() {
        return st_maxnactive;
    }

    private int st_maxnsnapshot;
    /**
    The maximum number of transactions on the snapshot list at any one time.
    @return the maximum number of transactions on the snapshot list at any one time
    */
    public int getMaxNsnapshot() {
        return st_maxnsnapshot;
    }

    private long st_region_wait;
    /**
    The number of times that a thread of control was forced to wait before
    obtaining the transaction region mutex.
    @return the number of times that a thread of control was forced to wait before
    obtaining the transaction region mutex
    */
    public long getRegionWait() {
        return st_region_wait;
    }

    private long st_region_nowait;
    /**
    The number of times that a thread of control was able to obtain the
    transaction region mutex without waiting.
    @return the number of times that a thread of control was able to obtain the
    transaction region mutex without waiting
    */
    public long getRegionNowait() {
        return st_region_nowait;
    }

    private long st_regsize;
    /**
    The size of the region.
    @return the size of the region
    */
    public long getRegSize() {
        return st_regsize;
    }

    private Active[] st_txnarray;
    /**
    An array of {@code Active} objects, describing the currently active transactions.
    @return an array of Active transactions
    */
    public Active[] getTxnarray() {
        return st_txnarray;
    }

    /** {@inheritDoc} */
    public String toString() {
        return "TransactionStats:"
            + "\n  st_nrestores=" + st_nrestores
            + "\n  st_last_ckp=" + st_last_ckp
            + "\n  st_time_ckp=" + st_time_ckp
            + "\n  st_last_txnid=" + st_last_txnid
            + "\n  st_inittxns=" + st_inittxns
            + "\n  st_maxtxns=" + st_maxtxns
            + "\n  st_naborts=" + st_naborts
            + "\n  st_nbegins=" + st_nbegins
            + "\n  st_ncommits=" + st_ncommits
            + "\n  st_nactive=" + st_nactive
            + "\n  st_nsnapshot=" + st_nsnapshot
            + "\n  st_maxnactive=" + st_maxnactive
            + "\n  st_maxnsnapshot=" + st_maxnsnapshot
            + "\n  st_region_wait=" + st_region_wait
            + "\n  st_region_nowait=" + st_region_nowait
            + "\n  st_regsize=" + st_regsize
            + "\n  st_txnarray=" + DbUtil.objectArrayToString(st_txnarray, "st_txnarray")
            ;
    }
}
