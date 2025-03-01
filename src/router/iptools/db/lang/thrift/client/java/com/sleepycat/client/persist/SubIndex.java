/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import java.util.Map;
import java.util.SortedMap;

import com.sleepycat.client.bind.EntityBinding;
import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.collections.StoredSortedMap;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.compat.DbCompat.OpResult;
import com.sleepycat.client.compat.DbCompat.OpWriteOptions;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.SSecondaryCursor;
import com.sleepycat.client.SSecondaryDatabase;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.util.keyrange.KeyRange;
import com.sleepycat.client.util.keyrange.RangeCursor;

/**
 * The EntityIndex returned by SecondaryIndex.subIndex.  A SubIndex, in JE
 * internal terms, is a duplicates btree for a single key in the main btree.
 * From the user's viewpoint, the keys are primary keys.  This class implements
 * that viewpoint.  In general, getSearchBoth and getSearchBothRange are used
 * where in a normal index getSearchKey and getSearchRange would be used.  The
 * main tree key is always implied, not passed as a parameter.
 *
 * @author Mark Hayes
 */
class SubIndex<PK, E> implements EntityIndex<PK, E> {

    private SecondaryIndex<?,PK,E> secIndex;
    private SSecondaryDatabase db;
    private boolean transactional;
    private boolean sortedDups;
    private boolean locking;
    private boolean concurrentDB;
    private SDatabaseEntry keyEntry;
    private Object keyObject;
    private KeyRange singleKeyRange;
    private EntryBinding pkeyBinding;
    private KeyRange emptyPKeyRange;
    private EntityBinding entityBinding;
    private ValueAdapter<PK> keyAdapter;
    private ValueAdapter<E> entityAdapter;
    private SortedMap<PK, E> map;

    <SK> SubIndex(SecondaryIndex<SK, PK, E> secIndex,
                  EntityBinding entityBinding,
                  SK key)
        throws SDatabaseException {

        this.secIndex = secIndex;
        db = secIndex.getDatabase();
        transactional = secIndex.transactional;
        sortedDups = secIndex.sortedDups;
        locking =
            DbCompat.getInitializeLocking(db.getEnvironment().getConfig());
        SEnvironment env = db.getEnvironment();
        concurrentDB = DbCompat.getInitializeCDB(env.getConfig());
        keyObject = key;
        keyEntry = new SDatabaseEntry();
        secIndex.keyBinding.objectToEntry(key, keyEntry);
        singleKeyRange = secIndex.emptyRange.subRange(keyEntry);

        PrimaryIndex<PK, E> priIndex = secIndex.getPrimaryIndex();
        pkeyBinding = priIndex.keyBinding;
        emptyPKeyRange = priIndex.emptyRange;
        this.entityBinding = entityBinding;

        keyAdapter = new PrimaryKeyValueAdapter<PK>
            (priIndex.keyClass, priIndex.keyBinding);
        entityAdapter = secIndex.entityAdapter;
    }

    public SDatabase getDatabase() {
        return db;
    }

    public boolean contains(PK key)
        throws SDatabaseException {

        return contains(null, key, null);
    }

    public boolean contains(STransaction txn, PK key, SLockMode lockMode)
        throws SDatabaseException {

        SDatabaseEntry pkeyEntry = new SDatabaseEntry();
        SDatabaseEntry dataEntry = BasicIndex.NO_RETURN_ENTRY;
        pkeyBinding.objectToEntry(key, pkeyEntry);

        SOperationStatus status =
            db.getSearchBoth(txn, keyEntry, pkeyEntry, dataEntry, lockMode);
        return (status == SOperationStatus.SUCCESS);
    }

    public E get(PK key)
        throws SDatabaseException {

        return get(null, key, null);
    }

    public E get(STransaction txn, PK key, SLockMode lockMode)
        throws SDatabaseException {


        SDatabaseEntry pkeyEntry = new SDatabaseEntry();
        SDatabaseEntry dataEntry = new SDatabaseEntry();
        pkeyBinding.objectToEntry(key, pkeyEntry);

        SOperationStatus status =
            db.getSearchBoth(txn, keyEntry, pkeyEntry, dataEntry, lockMode);

        if (status == SOperationStatus.SUCCESS) {
            return (E) entityBinding.entryToObject(pkeyEntry, dataEntry);
        } else {
            return null;
        }
    }


    public long count()
        throws SDatabaseException {

        SCursorConfig cursorConfig = locking ?
            SCursorConfig.READ_UNCOMMITTED : null;
        EntityCursor<PK> cursor = keys(null, cursorConfig);
        try {
            if (cursor.next() != null) {
                return cursor.count();
            } else {
                return 0;
            }
        } finally {
            cursor.close();
        }
    }


    public boolean delete(PK key)
        throws SDatabaseException {

        return delete(null, key);
    }

    public boolean delete(STransaction txn, PK key)
        throws SDatabaseException {

        return deleteInternal(txn, key, OpWriteOptions.EMPTY).isSuccess();
    }


    private OpResult deleteInternal(STransaction txn,
                                    PK key,
                                    OpWriteOptions options)
        throws SDatabaseException {

        SDatabaseEntry pkeyEntry = new SDatabaseEntry();
        SDatabaseEntry dataEntry = BasicIndex.NO_RETURN_ENTRY;
        pkeyBinding.objectToEntry(key, pkeyEntry);

        boolean autoCommit = false;
        SEnvironment env = db.getEnvironment();
        if (transactional &&
            txn == null &&
            DbCompat.getThreadTransaction(env) == null) {
            txn = env.beginTransaction
                (null, secIndex.getAutoCommitTransactionConfig());
            autoCommit = true;
        }

        boolean failed = true;
        SCursorConfig cursorConfig = null;
        if (concurrentDB) {
            cursorConfig = new SCursorConfig();
            DbCompat.setWriteCursor(cursorConfig, true);
        } 
        SSecondaryCursor cursor = db.openCursor(txn, cursorConfig);
        try {
            SOperationStatus status = cursor.getSearchBoth
                (keyEntry, pkeyEntry, dataEntry,
                 locking ? SLockMode.RMW : null);
            if (status == SOperationStatus.SUCCESS) {
                status = cursor.delete();
            }
            failed = false;
            return OpResult.make(status);
        } finally {
            cursor.close();
            if (autoCommit) {
                if (failed) {
                    txn.abort();
                } else {
                    txn.commit();
                }
            }
        }
    }

    public EntityCursor<PK> keys()
        throws SDatabaseException {

        return keys(null, null);
    }

    public EntityCursor<PK> keys(STransaction txn, SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, null, keyAdapter, config);
    }

    public EntityCursor<E> entities()
        throws SDatabaseException {

        return cursor(null, null, entityAdapter, null);
    }

    public EntityCursor<E> entities(STransaction txn,
                                    SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, null, entityAdapter, config);
    }

    public EntityCursor<PK> keys(PK fromKey,
                                 boolean fromInclusive,
                                 PK toKey,
                                 boolean toInclusive)
        throws SDatabaseException {

        return cursor(null, fromKey, fromInclusive, toKey, toInclusive,
                      keyAdapter, null);
    }

    public EntityCursor<PK> keys(STransaction txn,
                                 PK fromKey,
                                 boolean fromInclusive,
                                 PK toKey,
                                 boolean toInclusive,
                                 SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, fromKey, fromInclusive, toKey, toInclusive,
                      keyAdapter, config);
    }

    public EntityCursor<E> entities(PK fromKey,
                                    boolean fromInclusive,
                                    PK toKey,
                                    boolean toInclusive)
        throws SDatabaseException {

        return cursor(null, fromKey, fromInclusive, toKey, toInclusive,
                      entityAdapter, null);
    }

    public EntityCursor<E> entities(STransaction txn,
                                    PK fromKey,
                                    boolean fromInclusive,
                                    PK toKey,
                                    boolean toInclusive,
                                    SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, fromKey, fromInclusive, toKey, toInclusive,
                      entityAdapter, config);
    }

    private <V> EntityCursor<V> cursor(STransaction txn,
                                       PK fromKey,
                                       boolean fromInclusive,
                                       PK toKey,
                                       boolean toInclusive,
                                       ValueAdapter<V> adapter,
                                       SCursorConfig config)
        throws SDatabaseException {

        SDatabaseEntry fromEntry = null;
        if (fromKey != null) {
            fromEntry = new SDatabaseEntry();
            pkeyBinding.objectToEntry(fromKey, fromEntry);
        }
        SDatabaseEntry toEntry = null;
        if (toKey != null) {
            toEntry = new SDatabaseEntry();
            pkeyBinding.objectToEntry(toKey, toEntry);
        }
        KeyRange pkeyRange = emptyPKeyRange.subRange
            (fromEntry, fromInclusive, toEntry, toInclusive);
        return cursor(txn, pkeyRange, adapter, config);
    }

    private <V> EntityCursor<V> cursor(STransaction txn,
                                       KeyRange pkeyRange,
                                       ValueAdapter<V> adapter,
                                       SCursorConfig config)
        throws SDatabaseException {

        SCursor cursor = db.openCursor(txn, config);
        RangeCursor rangeCursor =
            new RangeCursor(singleKeyRange, pkeyRange, sortedDups, cursor);
        return new SubIndexCursor<V>(rangeCursor, adapter);
    }

    public Map<PK, E> map() {
        return sortedMap();
    }

    public synchronized SortedMap<PK, E> sortedMap() {
        if (map == null) {
            map = (SortedMap) ((StoredSortedMap) secIndex.sortedMap()).
                duplicatesMap(keyObject, pkeyBinding);
        }
        return map;
    }
}
