/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import java.util.Map;
import java.util.SortedMap;

import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.collections.StoredSortedMap;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.STransaction;

/**
 * The EntityIndex returned by SecondaryIndex.keysIndex().  This index maps
 * secondary key to primary key.  In Berkeley DB internal terms, this is a
 * secondary database that is opened without associating it with a primary.
 *
 * @author Mark Hayes
 */
class KeysIndex<SK, PK> extends BasicIndex<SK, PK> {

    private EntryBinding pkeyBinding;
    private SortedMap<SK, PK> map;

    KeysIndex(SDatabase db,
              Class<SK> keyClass,
              EntryBinding keyBinding,
              Class<PK> pkeyClass,
              EntryBinding pkeyBinding)
        throws SDatabaseException {

        super(db, keyClass, keyBinding,
              new DataValueAdapter<PK>(pkeyClass, pkeyBinding));
        this.pkeyBinding = pkeyBinding;
    }

    /*
     * Of the EntityIndex methods only get()/map()/sortedMap() are implemented
     * here.  All other methods are implemented by BasicIndex.
     */

    public PK get(SK key)
        throws SDatabaseException {

        return get(null, key, null);
    }

    public PK get(STransaction txn, SK key, SLockMode lockMode)
        throws SDatabaseException {


        SDatabaseEntry keyEntry = new SDatabaseEntry();
        SDatabaseEntry pkeyEntry = new SDatabaseEntry();
        keyBinding.objectToEntry(key, keyEntry);

        SOperationStatus status = db.get(txn, keyEntry, pkeyEntry, lockMode);

        if (status == SOperationStatus.SUCCESS) {
            return (PK) pkeyBinding.entryToObject(pkeyEntry);
        } else {
            return null;
        }
    }


    public Map<SK, PK> map() {
        return sortedMap();
    }

    public synchronized SortedMap<SK, PK> sortedMap() {
        if (map == null) {
            map = new StoredSortedMap(db, keyBinding, pkeyBinding, false);
        }
        return map;
    }

    boolean isUpdateAllowed() {
        return false;
    }
}
