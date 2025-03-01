/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.util.keyrange.KeyRange;
import com.sleepycat.client.util.keyrange.RangeCursor;

/**
 * Implements EntityIndex using a ValueAdapter.  This class is abstract and
 * does not implement get()/map()/sortedMap() because it doesn't have access
 * to the entity binding.
 *
 * @author Mark Hayes
 */
abstract class BasicIndex<K, E> implements EntityIndex<K, E> {

    static final SDatabaseEntry NO_RETURN_ENTRY;
    static {
        NO_RETURN_ENTRY = new SDatabaseEntry();
        NO_RETURN_ENTRY.setPartial(0, 0, true);
    }

    SDatabase db;
    boolean transactional;
    boolean sortedDups;
    boolean locking;
    boolean concurrentDB;
    Class<K> keyClass;
    EntryBinding keyBinding;
    KeyRange emptyRange;
    ValueAdapter<K> keyAdapter;
    ValueAdapter<E> entityAdapter;

    BasicIndex(SDatabase db,
               Class<K> keyClass,
               EntryBinding keyBinding,
               ValueAdapter<E> entityAdapter)
        throws SDatabaseException {

        this.db = db;
        SDatabaseConfig config = db.getConfig();
        transactional = config.getTransactional();
        sortedDups = config.getSortedDuplicates();
        locking =
            DbCompat.getInitializeLocking(db.getEnvironment().getConfig());
        SEnvironment env = db.getEnvironment();
        concurrentDB = DbCompat.getInitializeCDB(env.getConfig());
        this.keyClass = keyClass;
        this.keyBinding = keyBinding;
        this.entityAdapter = entityAdapter;

        emptyRange = new KeyRange(config.getBtreeComparator());
        keyAdapter = new KeyValueAdapter(keyClass, keyBinding);
    }

    public SDatabase getDatabase() {
        return db;
    }

    /*
     * Of the EntityIndex methods only get()/map()/sortedMap() are not
     * implemented here and therefore must be implemented by subclasses.
     */

    public boolean contains(K key)
        throws SDatabaseException {

        return contains(null, key, null);
    }

    public boolean contains(STransaction txn, K key, SLockMode lockMode)
        throws SDatabaseException {

        SDatabaseEntry keyEntry = new SDatabaseEntry();
        SDatabaseEntry dataEntry = NO_RETURN_ENTRY;
        keyBinding.objectToEntry(key, keyEntry);

        SOperationStatus status = db.get(txn, keyEntry, dataEntry, lockMode);
        return (status == SOperationStatus.SUCCESS);
    }

    public long count()
        throws SDatabaseException {

        if (DbCompat.DATABASE_COUNT) {
            return DbCompat.getDatabaseCount(db);
        } else {
            long count = 0;
            SDatabaseEntry key = NO_RETURN_ENTRY;
            SDatabaseEntry data = NO_RETURN_ENTRY;
            SCursorConfig cursorConfig = locking ?
                SCursorConfig.READ_UNCOMMITTED : null;
            SCursor cursor = db.openCursor(null, cursorConfig);
            try {
                SOperationStatus status = cursor.getFirst(key, data, null);
                while (status == SOperationStatus.SUCCESS) {
                    if (sortedDups) {
                        count += cursor.count();
                    } else {
                        count += 1;
                    }
                    status = cursor.getNextNoDup(key, data, null);
                }
            } finally {
                cursor.close();
            }
            return count;
        }
    }


    public boolean delete(K key)
        throws SDatabaseException {

        return delete(null, key);
    }

    public boolean delete(STransaction txn, K key)
        throws SDatabaseException {


        SDatabaseEntry keyEntry = new SDatabaseEntry();
        keyBinding.objectToEntry(key, keyEntry);

        SOperationStatus status = db.delete(txn, keyEntry);
        return (status == SOperationStatus.SUCCESS);
    }


    public EntityCursor<K> keys()
        throws SDatabaseException {

        return keys(null, null);
    }

    public EntityCursor<K> keys(STransaction txn, SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, emptyRange, keyAdapter, config);
    }

    public EntityCursor<E> entities()
        throws SDatabaseException {

        return cursor(null, emptyRange, entityAdapter, null);
    }

    public EntityCursor<E> entities(STransaction txn,
                                    SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, emptyRange, entityAdapter, config);
    }

    public EntityCursor<K> keys(K fromKey, boolean fromInclusive,
                                K toKey, boolean toInclusive)
        throws SDatabaseException {

        return cursor(null, fromKey, fromInclusive, toKey, toInclusive,
                      keyAdapter, null);
    }

    public EntityCursor<K> keys(STransaction txn,
                                K fromKey,
                                boolean fromInclusive,
                                K toKey,
                                boolean toInclusive,
                                SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, fromKey, fromInclusive, toKey, toInclusive,
                      keyAdapter, config);
    }

    public EntityCursor<E> entities(K fromKey, boolean fromInclusive,
                                    K toKey, boolean toInclusive)
        throws SDatabaseException {

        return cursor(null, fromKey, fromInclusive, toKey, toInclusive,
                      entityAdapter, null);
    }

    public EntityCursor<E> entities(STransaction txn,
                                    K fromKey,
                                    boolean fromInclusive,
                                    K toKey,
                                    boolean toInclusive,
                                    SCursorConfig config)
        throws SDatabaseException {

        return cursor(txn, fromKey, fromInclusive, toKey, toInclusive,
                      entityAdapter, config);
    }

    private <V> EntityCursor<V> cursor(STransaction txn,
                                       K fromKey,
                                       boolean fromInclusive,
                                       K toKey,
                                       boolean toInclusive,
                                       ValueAdapter<V> adapter,
                                       SCursorConfig config)
        throws SDatabaseException {

        SDatabaseEntry fromEntry = null;
        if (fromKey != null) {
            fromEntry = new SDatabaseEntry();
            keyBinding.objectToEntry(fromKey, fromEntry);
        }
        SDatabaseEntry toEntry = null;
        if (toKey != null) {
            toEntry = new SDatabaseEntry();
            keyBinding.objectToEntry(toKey, toEntry);
        }
        KeyRange range = emptyRange.subRange
            (fromEntry, fromInclusive, toEntry, toInclusive);
        return cursor(txn, range, adapter, config);
    }

    private <V> EntityCursor<V> cursor(STransaction txn,
                                       KeyRange range,
                                       ValueAdapter<V> adapter,
                                       SCursorConfig config)
        throws SDatabaseException {

        SCursor cursor = db.openCursor(txn, config);
        RangeCursor rangeCursor =
            new RangeCursor(range, null/*pkRange*/, sortedDups, cursor);
        return new BasicCursor<V>(rangeCursor, adapter, isUpdateAllowed());
    }

    abstract boolean isUpdateAllowed();

}
