/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.SDatabaseEntry;

/**
 * A ValueAdapter where the "value" is the key (the primary key in a primary
 * index or the secondary key in a secondary index).
 *
 * @author Mark Hayes
 */
class KeyValueAdapter<V> implements ValueAdapter<V> {

    private EntryBinding keyBinding;

    KeyValueAdapter(Class<V> keyClass, EntryBinding keyBinding) {
        this.keyBinding = keyBinding;
    }

    public SDatabaseEntry initKey() {
        return new SDatabaseEntry();
    }

    public SDatabaseEntry initPKey() {
        return null;
    }

    public SDatabaseEntry initData() {
        return BasicIndex.NO_RETURN_ENTRY;
    }

    public void clearEntries(SDatabaseEntry key,
                             SDatabaseEntry pkey,
                             SDatabaseEntry data) {
        key.setData(null);
    }

    public V entryToValue(SDatabaseEntry key,
                          SDatabaseEntry pkey,
                          SDatabaseEntry data) {
        return (V) keyBinding.entryToObject(key);
    }

    public void valueToData(V value, SDatabaseEntry data) {
        throw new UnsupportedOperationException
            ("Cannot change the data in a key-only index");
    }
}
