/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TLockStat;

/**
 * Lock statistics for a database environment.
 */
public class SLockStats {
    /** The Thrift object. */
    private final TLockStat stat;

    SLockStats(TLockStat stat) {
        this.stat = stat;
    }

    /**
     * The last allocated locker ID.
     *
     * @return the last allocated locker ID
     */
    public int getId() {
        return this.stat.id;
    }

    /**
     * The current maximum unused locker ID.
     *
     * @return the current maximum unused locker ID
     */
    public int getCurMaxId() {
        return this.stat.curMaxId;
    }

    /**
     * The initial number of locks allocated in the lock table.
     *
     * @return the initial number of locks allocated in the lock table
     */
    public int getInitlocks() {
        return this.stat.initlocks;
    }

    /**
     * The initial number of lockers allocated in lock table.
     *
     * @return the initial number of lockers allocated in lock table
     */
    public int getInitlockers() {
        return this.stat.initlockers;
    }

    /**
     * The initial number of lock objects allocated in lock table.
     *
     * @return the initial number of lock objects allocated in lock table
     */
    public int getInitobjects() {
        return this.stat.initobjects;
    }

    /**
     * The current number of locks allocated in lock table.
     *
     * @return the current number of locks allocated in lock table
     */
    public int getLocks() {
        return this.stat.locks;
    }

    /**
     * The current number of lockers allocated in lock table.
     *
     * @return the current number of lockers allocated in lock table
     */
    public int getLockers() {
        return this.stat.lockers;
    }

    /**
     * The current number of lock objects allocated in lock table.
     *
     * @return the current number of lock objects allocated in lock table
     */
    public int getObjects() {
        return this.stat.objects;
    }

    /**
     * The maximum number of locks possible.
     *
     * @return the maximum number of locks possible
     */
    public int getMaxLocks() {
        return this.stat.maxLocks;
    }

    /**
     * The maximum number of lockers possible.
     *
     * @return the maximum number of lockers possible
     */
    public int getMaxLockers() {
        return this.stat.maxLockers;
    }

    /**
     * The maximum number of lock objects possible.
     *
     * @return the maximum number of lock objects possible
     */
    public int getMaxObjects() {
        return this.stat.maxObjects;
    }

    /**
     * The number of lock table partitions.
     *
     * @return the number of lock table partitions
     */
    public int getPartitions() {
        return this.stat.partitions;
    }

    /**
     * The size of object hash table.
     *
     * @return the size of object hash table
     */
    public int getTableSize() {
        return this.stat.tableSize;
    }

    /**
     * The number of lock modes.
     *
     * @return the number of lock modes
     */
    public int getNumModes() {
        return this.stat.numModes;
    }

    /**
     * The number of current lockers.
     *
     * @return the number of current lockers
     */
    public int getNumLockers() {
        return this.stat.numLockers;
    }

    /**
     * The number of current locks.
     *
     * @return the number of current locks
     */
    public int getNumLocks() {
        return this.stat.numLocks;
    }

    /**
     * The maximum number of locks at any one time.  Note that if there is more
     * than one partition, this is the sum of the maximum across all
     * partitions.
     *
     * @return the maximum number of locks at any one time
     */
    public int getMaxNlocks() {
        return this.stat.maxNlocks;
    }

    /**
     * The maximum number of locks in any hash bucket at any one time.
     *
     * @return the maximum number of locks in any hash bucket at any one time
     */
    public int getMaxHlocks() {
        return this.stat.maxHlocks;
    }

    /**
     * The maximum number of locks stolen by an empty partition.
     *
     * @return the maximum number of locks stolen by an empty partition
     */
    public long getLocksteals() {
        return this.stat.locksteals;
    }

    /**
     * The maximum number of lock steals for any one partition.
     *
     * @return the maxinum number of lock steals for any one partition
     */
    public long getMaxLsteals() {
        return this.stat.maxLsteals;
    }

    /**
     * The maximum number of lockers at any one time.
     *
     * @return the maximum number of lockers at any one time
     */
    public int getMaxNlockers() {
        return this.stat.maxNlockers;
    }

    /**
     * The number of current lock objects.
     *
     * @return the number of current lock objects
     */
    public int getNobjects() {
        return this.stat.nobjects;
    }

    /**
     * The maximum number of lock objects at any one time.  Note that if there
     * is more than one partition this is the sum of the maximum across all
     * partitions.
     *
     * @return the maximum number of lock objects at any one time
     */
    public int getMaxNobjects() {
        return this.stat.maxNobjects;
    }

    /**
     * The maximum number of objects in any hash bucket at any one time.
     *
     * @return the maximum number of objects in any hash bucket at any one time
     */
    public int getMaxHobjects() {
        return this.stat.maxHobjects;
    }

    /**
     * The maximum number of objects stolen by an empty partition.
     *
     * @return the maximum number of objects stolen by an empty partition
     */
    public long getObjectsteals() {
        return this.stat.objectsteals;
    }

    /**
     * The maximum number of object steals for any one partition.
     *
     * @return the maximum number of object steals for any one partition
     */
    public long getMaxOsteals() {
        return this.stat.maxOsteals;
    }

    /**
     * The total number of locks requested.
     *
     * @return the total number of locks requested
     */
    public long getNumRequests() {
        return this.stat.numRequests;
    }

    /**
     * The total number of locks released.
     *
     * @return the total number of locks released
     */
    public long getNumReleases() {
        return this.stat.numReleases;
    }

    /**
     * The total number of locks upgraded.
     *
     * @return the total number of locks upgraded
     */
    public long getNumUpgrade() {
        return this.stat.numUpgrade;
    }

    /**
     * The total number of locks downgraded.
     *
     * @return the total number of locks downgraded
     */
    public long getNumDowngrade() {
        return this.stat.numDowngrade;
    }

    /**
     * The number of lock requests not immediately available due to conflicts,
     * for which the thread of control waited.
     *
     * @return the number of lock requests not immediately available due to
     * conflicts, for which the thread of control waited
     */
    public long getLockWait() {
        return this.stat.lockWait;
    }

    /**
     * The number of lock requests not immediately available due to conflicts,
     * for which the thread of control did not wait.
     *
     * @return the number of lock requests not immediately available due to
     * conflicts, for which the thread of control did not wait
     */
    public long getLockNowait() {
        return this.stat.lockNowait;
    }

    /**
     * The number of deadlocks.
     *
     * @return the number of deadlocks
     */
    public long getNumDeadlocks() {
        return this.stat.numDeadlocks;
    }

    /**
     * Lock timeout value.
     *
     * @return lock timeout value
     */
    public int getLockTimeout() {
        return this.stat.lockTimeout;
    }

    /**
     * The number of lock requests that have timed out.
     *
     * @return the number of lock requests that have timed out
     */
    public long getNumLockTimeouts() {
        return this.stat.numLockTimeouts;
    }

    /**
     * Transaction timeout value.
     *
     * @return transaction timeout value
     */
    public int getTxnTimeout() {
        return this.stat.txnTimeout;
    }

    /**
     * The number of transactions that have timed out.  This value is also
     * a component of {@link #getNumDeadlocks}, the total number of deadlocks
     * detected.
     *
     * @return the number of transactions that have timed out
     */
    public long getNumTxnTimeouts() {
        return this.stat.numTxnTimeouts;
    }

    /**
     * The number of times that a thread of control was forced to wait before
     * obtaining a lock partition mutex.
     *
     * @return the number of times that a thread of control was forced to wait
     * before obtaining a lock partition mutex
     */
    public long getPartWait() {
        return this.stat.partWait;
    }

    /**
     * The number of times that a thread of control was able to obtain a lock
     * partition mutex without waiting.
     *
     * @return the number of times that a thread of control was able to obtain a
     * lock partition mutex without waiting
     */
    public long getPartNowait() {
        return this.stat.partNowait;
    }

    /**
     * The maximum number of times that a thread of control was forced to wait
     * before obtaining any one lock partition mutex.
     *
     * @return the maximum number of times that a thread of control was forced
     * to wait before obtaining any one lock partition mutex
     */
    public long getPartMaxWait() {
        return this.stat.partMaxWait;
    }

    /**
     * The number of times that a thread of control was able to obtain any one
     * lock partition mutex without waiting.
     *
     * @return the number of times that a thread of control was able to obtain
     * any one lock partition mutex without waiting
     */
    public long getPartMaxNowait() {
        return this.stat.partMaxNowait;
    }

    /**
     * The number of requests to allocate or deallocate an object for which the
     * thread of control waited.
     *
     * @return the number of requests to allocate or deallocate an object for
     * which the thread of control waited
     */
    public long getObjsWait() {
        return this.stat.objsWait;
    }

    /**
     * The number of requests to allocate or deallocate an object for which the
     * thread of control did not wait.
     *
     * @return the number of requests to allocate or deallocate an object for
     * which the thread of control did not wait
     */
    public long getObjsNowait() {
        return this.stat.objsNowait;
    }

    /**
     * The number of requests to allocate or deallocate a locker for which the
     * thread of control waited.
     *
     * @return the number of requests to allocate or deallocate a locker for
     * which the thread of control waited
     */
    public long getLockersWait() {
        return this.stat.lockersWait;
    }

    /**
     * The number of requests to allocate or deallocate a locker for which the
     * thread of control did not wait.
     *
     * @return the number of requests to allocate or deallocate a locker for
     * which the thread of control did not wait
     */
    public long getLockersNowait() {
        return this.stat.lockersNowait;
    }

    /**
     * The number of times that a thread of control was forced to wait before
     * obtaining the lock region mutex.
     *
     * @return the number of times that a thread of control was forced to wait
     * before obtaining the lock region mutex
     */
    public long getRegionWait() {
        return this.stat.regionWait;
    }

    /**
     * The number of times that a thread of control was able to obtain the lock
     * region mutex without waiting.
     *
     * @return the number of times that a thread of control was able to obtain
     * the lock region mutex without waiting
     */
    public long getRegionNowait() {
        return this.stat.regionNowait;
    }

    /**
     * The number of hits in the thread locker cache.
     *
     * @return the number of hits in the thread locker cache
     */
    public long getNumLockersHit() {
        return this.stat.numLockersHit;
    }

    /**
     * Total number of lockers reused.
     *
     * @return total number of lockers reused
     */
    public long getNumLockersReused() {
        return this.stat.numLockersReused;
    }

    /**
     * Maximum length of a lock hash bucket.
     *
     * @return maximum length of a lock hash bucket
     */
    public int getHashLen() {
        return this.stat.hashLen;
    }

    /**
     * The size of the lock region.
     *
     * @return the size of the lock region
     */
    public long getRegSize() {
        return this.stat.regSize;
    }

    /**
     * For convenience, the SLockStats class has a toString method
     * that lists all the data fields.
     *
     * @return a String that lists all fields
     */
    public String toString() {
        return "LockStats:"
                + "\n  id=" + getId()
                + "\n  cur_maxid=" + getCurMaxId()
                + "\n  initlocks=" + getInitlocks()
                + "\n  initlockers=" + getInitlockers()
                + "\n  initobjects=" + getInitobjects()
                + "\n  locks=" + getLocks()
                + "\n  lockers=" + getLockers()
                + "\n  objects=" + getObjects()
                + "\n  maxlocks=" + getMaxLocks()
                + "\n  maxlockers=" + getMaxLockers()
                + "\n  maxobjects=" + getMaxObjects()
                + "\n  partitions=" + getPartitions()
                + "\n  tablesize=" + getTableSize()
                + "\n  nmodes=" + getNumModes()
                + "\n  nlockers=" + getNumLockers()
                + "\n  nlocks=" + getNumLocks()
                + "\n  maxnlocks=" + getMaxNlocks()
                + "\n  maxhlocks=" + getMaxHlocks()
                + "\n  locksteals=" + getLocksteals()
                + "\n  maxlsteals=" + getMaxLsteals()
                + "\n  maxnlockers=" + getMaxNlockers()
                + "\n  nobjects=" + getNobjects()
                + "\n  maxnobjects=" + getMaxNobjects()
                + "\n  maxhobjects=" + getMaxHobjects()
                + "\n  objectsteals=" + getObjectsteals()
                + "\n  maxosteals=" + getMaxOsteals()
                + "\n  nrequests=" + getNumRequests()
                + "\n  nreleases=" + getNumReleases()
                + "\n  nupgrade=" + getNumUpgrade()
                + "\n  ndowngrade=" + getNumDowngrade()
                + "\n  lock_wait=" + getLockWait()
                + "\n  lock_nowait=" + getLockNowait()
                + "\n  ndeadlocks=" + getNumDeadlocks()
                + "\n  locktimeout=" + getLockTimeout()
                + "\n  nlocktimeouts=" + getNumLockTimeouts()
                + "\n  txntimeout=" + getTxnTimeout()
                + "\n  ntxntimeouts=" + getNumTxnTimeouts()
                + "\n  part_wait=" + getPartWait()
                + "\n  part_nowait=" + getPartNowait()
                + "\n  part_max_wait=" + getPartMaxWait()
                + "\n  part_max_nowait=" + getPartMaxNowait()
                + "\n  objs_wait=" + getObjsWait()
                + "\n  objs_nowait=" + getObjsNowait()
                + "\n  lockers_wait=" + getLockersWait()
                + "\n  lockers_nowait=" + getLockersNowait()
                + "\n  region_wait=" + getRegionWait()
                + "\n  region_nowait=" + getRegionNowait()
                + "\n  nlockers_hit=" + getNumLockersHit()
                + "\n  nlockers_reused=" + getNumLockersReused()
                + "\n  hash_len=" + getHashLen()
                + "\n  regsize=" + getRegSize()
                ;
    }
}
